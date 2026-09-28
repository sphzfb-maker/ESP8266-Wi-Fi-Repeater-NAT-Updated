#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <lwip/napt.h>
#include <lwip/dns.h>
#include <lwip/netif.h>
#if defined(__has_include)
#if __has_include(<LwipDhcpServer-NonOS.h>)
#include <LwipDhcpServer-NonOS.h>
#define HAVE_DHCP_NONOS 1
#endif
#endif
#include "config_storage.h"
#include "web_portal.h"

extern "C" {
#include "user_interface.h"
// Exists in the SDK library but is not always declared in the header
bool wifi_softap_deauth(uint8_t mac[6]);
}

#define STATUS_LED LED_BUILTIN
#define RECONNECT_INTERVAL_MS 10000UL
#define FAILOVER_AFTER_MS 25000UL
#define RESET_BUTTON_PIN 0          // NodeMCU "FLASH" button
#define RESET_HOLD_MS 5000UL

ESP8266WebServer server(80);
DNSServer dnsServer;
RepeaterConfig currentConfig;

bool naptActive = false;
bool isSetupMode = true;
unsigned long lastBlink = 0;
bool ledState = false;
unsigned long scheduledReboot = 0;
int lockedChannel = 1;
unsigned long lastReconnectAttempt = 0;
unsigned long disconnectedSince = 0;
bool wasConnected = false;
uint8_t activeUp = 0;
String apIpStr = "192.168.4.1";
unsigned long btnDownSince = 0;
unsigned long lastFilterTick = 0;

// ---------------------------------------------------------------------------
// Traffic statistics (counted on the WAN and LAN network interfaces)
// ---------------------------------------------------------------------------
static netif_input_fn staOrigInput = nullptr;
static netif_input_fn apOrigInput = nullptr;
static volatile uint32_t rxBytesWan = 0;   // downstream: router -> repeater
static volatile uint32_t txBytesWan = 0;   // upstream: clients -> repeater
static uint32_t lastRx = 0, lastTx = 0;
static uint32_t rxRate = 0, txRate = 0;    // bytes per second
static uint64_t rxTotal = 0, txTotal = 0;  // bytes since boot
static unsigned long lastStatTick = 0;

static err_t IRAM_ATTR staInputHook(struct pbuf *p, struct netif *inp) {
    rxBytesWan += p->tot_len;
    return staOrigInput(p, inp);
}

static err_t IRAM_ATTR apInputHook(struct pbuf *p, struct netif *inp) {
    txBytesWan += p->tot_len;
    return apOrigInput(p, inp);
}

void installTrafficHooks() {
    uint32_t staIp = WiFi.localIP().v4();
    uint32_t apIpv = WiFi.softAPIP().v4();
    for (struct netif *n = netif_list; n != NULL; n = n->next) {
        uint32_t ip = ip4_addr_get_u32(netif_ip4_addr(n));
        if (ip == 0) continue;
        if (ip == staIp) {
            if (n->input != staInputHook) {
                staOrigInput = n->input;
                n->input = staInputHook;
            }
        } else if (ip == apIpv) {
            if (n->input != apInputHook) {
                apOrigInput = n->input;
                n->input = apInputHook;
            }
        }
    }
}

void tickTraffic(unsigned long now) {
    unsigned long dt = now - lastStatTick;
    if (dt < 1000) return;
    lastStatTick = now;
    uint32_t rx = rxBytesWan;
    uint32_t tx = txBytesWan;
    uint32_t dRx = rx - lastRx;
    uint32_t dTx = tx - lastTx;
    lastRx = rx;
    lastTx = tx;
    rxTotal += dRx;
    txTotal += dTx;
    rxRate = (uint32_t)(((uint64_t)dRx * 1000ULL) / dt);
    txRate = (uint32_t)(((uint64_t)dTx * 1000ULL) / dt);
}

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------
void setLed(bool on) {
    digitalWrite(STATUS_LED, on ? LOW : HIGH);
}

// Parses "AA:BB:CC:DD:EE:FF" into 6 raw bytes. Returns false on bad format.
bool parseMac(const String &in, uint8_t *out) {
    String str = in;
    str.trim();
    if (str.length() != 17) return false;
    int vals[6];
    if (sscanf(str.c_str(), "%x:%x:%x:%x:%x:%x",
               &vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5]) != 6) {
        return false;
    }
    for (int i = 0; i < 6; i++) {
        if (vals[i] < 0 || vals[i] > 255) return false;
        out[i] = (uint8_t)vals[i];
    }
    return true;
}

// A usable device MAC must be unicast (lowest bit of the first byte = 0)
bool validUnicastMac(const uint8_t *m) {
    return (m[0] & 0x01) == 0;
}

bool parseIp(const String &in, uint8_t *out) {
    String s = in;
    s.trim();
    int a, b, c, d;
    char extra;
    if (sscanf(s.c_str(), "%d.%d.%d.%d%c", &a, &b, &c, &d, &extra) != 4) return false;
    if (a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255 || d < 0 || d > 255) return false;
    out[0] = (uint8_t)a;
    out[1] = (uint8_t)b;
    out[2] = (uint8_t)c;
    out[3] = (uint8_t)d;
    return true;
}

String macToStr(const uint8_t *mac) {
    char buf[18];
    sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

String jsonEscape(const String &s) {
    String o;
    o.reserve(s.length() + 4);
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (c == '"' || c == '\\') {
            o += '\\';
            o += c;
        } else if (c == '\n') {
            o += "\\n";
        } else if ((uint8_t)c < 0x20) {
            // drop other control characters
        } else {
            o += c;
        }
    }
    return o;
}

static void jStr(String &j, const char *k, const String &v) {
    j += '"'; j += k; j += "\":\""; j += jsonEscape(v); j += "\",";
}
static void jNum(String &j, const char *k, long v) {
    j += '"'; j += k; j += "\":"; j += String(v); j += ',';
}
static void jBool(String &j, const char *k, bool v) {
    j += '"'; j += k; j += "\":"; j += (v ? "true" : "false"); j += ',';
}
static void jEnd(String &j) {
    if (j.endsWith(",")) j.remove(j.length() - 1);
    j += '}';
}

IPAddress currentApIp() {
    return IPAddress(currentConfig.ap_ip[0], currentConfig.ap_ip[1], currentConfig.ap_ip[2], currentConfig.ap_ip[3]);
}

uint8_t countUpstreams() {
    uint8_t c = 0;
    for (int i = 0; i < MAX_UPSTREAMS; i++) {
        if (currentConfig.sta[i].ssid[0]) c++;
    }
    return c;
}

uint8_t nextUpstream(uint8_t cur) {
    for (int k = 1; k <= MAX_UPSTREAMS; k++) {
        uint8_t i = (cur + k) % MAX_UPSTREAMS;
        if (currentConfig.sta[i].ssid[0]) return i;
    }
    return cur;
}

// ---------------------------------------------------------------------------
// MAC filter helpers
// ---------------------------------------------------------------------------
bool macInList(const RepeaterConfig &c, const uint8_t *mac) {
    for (int i = 0; i < c.filter_count; i++) {
        if (memcmp(c.filter[i], mac, 6) == 0) return true;
    }
    return false;
}

void removeFromList(RepeaterConfig &c, const uint8_t *mac) {
    for (int i = 0; i < c.filter_count; i++) {
        if (memcmp(c.filter[i], mac, 6) == 0) {
            for (int k = i; k < c.filter_count - 1; k++) memcpy(c.filter[k], c.filter[k + 1], 6);
            c.filter_count--;
            return;
        }
    }
}

// Finds the MAC address of the device that sent the current HTTP request
bool getRequesterMac(uint8_t *out) {
    uint32_t rip = server.client().remoteIP().v4();
    bool found = false;
    struct station_info *st = wifi_softap_get_station_info();
    while (st != NULL) {
        if (st->ip.addr == rip) {
            memcpy(out, st->bssid, 6);
            found = true;
            break;
        }
        st = STAILQ_NEXT(st, next);
    }
    wifi_softap_free_station_info();
    return found;
}

// Kicks every associated device that is not allowed by the filter
void enforceMacFilter() {
    if (currentConfig.filter_mode == 0) return;
    struct station_info *st = wifi_softap_get_station_info();
    while (st != NULL) {
        uint8_t mac[6];
        memcpy(mac, st->bssid, 6);
        bool listed = macInList(currentConfig, mac);
        bool deny = (currentConfig.filter_mode == 1) ? listed : !listed;
        if (deny) {
            Serial.printf("[FILTER] Kicking %s\n", macToStr(mac).c_str());
            wifi_softap_deauth(mac);
        }
        st = STAILQ_NEXT(st, next);
    }
    wifi_softap_free_station_info();
}

// ---------------------------------------------------------------------------
// Web handlers
// ---------------------------------------------------------------------------
static void bad(const char *msg) {
    server.send(400, "text/plain", msg);
}

// Copies a form argument into dst. Returns false if it does not fit.
static bool argCopy(const char *name, char *dst, size_t cap) {
    String v = server.arg(name);
    if (v.length() >= cap) return false;
    strcpy(dst, v.c_str());
    return true;
}

void handleRoot() {
    server.send_P(200, "text/html", INDEX_HTML);
}

void handleScan() {
    Serial.println(F("[HTTP] Scan requested"));
    int n = WiFi.scanNetworks();
    String j = "[";
    bool first = true;
    for (int i = 0; i < n; ++i) {
        String s = WiFi.SSID(i);
        if (s.length() == 0) continue;
        if (!first) j += ",";
        first = false;
        j += "{\"ssid\":\"" + jsonEscape(s) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + ",\"enc\":";
        j += ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "false" : "true");
        j += "}";
    }
    j += "]";
    WiFi.scanDelete();
    server.send(200, "application/json", j);
}

void handleStatus() {
    bool up = (WiFi.status() == WL_CONNECTED);
    int rssi = up ? WiFi.RSSI() : 0;
    int sig = (rssi <= -100) ? 0 : (rssi >= -50 ? 100 : 2 * (rssi + 100));
    IPAddress apIp = WiFi.softAPIP();
    // Both networks in the same subnet would break routing
    uint32_t m = WiFi.subnetMask().v4() & 0x00FFFFFFUL;
    bool conflict = up && ((WiFi.localIP().v4() & m) == (apIp.v4() & m));

    String j = "{";
    jBool(j, "setup_mode", isSetupMode);
    jBool(j, "sta_connected", up);
    jStr(j, "sta_ssid", up ? WiFi.SSID() : String(currentConfig.sta[activeUp].ssid));
    jNum(j, "sta_slot", activeUp);
    jStr(j, "sta_ip", up ? WiFi.localIP().toString() : String("0.0.0.0"));
    jStr(j, "sta_mac", WiFi.macAddress());
    jStr(j, "ap_mac", WiFi.softAPmacAddress());
    jNum(j, "rssi", rssi);
    jNum(j, "signal_pct", sig);
    jNum(j, "channel", WiFi.channel());
    jStr(j, "ap_ssid", String(currentConfig.ap_ssid[0] ? currentConfig.ap_ssid : "ESP8266-Repeater-Setup"));
    jStr(j, "ap_ip", apIp.toString());
    jNum(j, "ap_clients", WiFi.softAPgetStationNum());
    jNum(j, "free_heap", ESP.getFreeHeap());
    jNum(j, "uptime_s", millis() / 1000);
    jNum(j, "rx_bps", rxRate);
    jNum(j, "tx_bps", txRate);
    jNum(j, "rx_kb", (long)(rxTotal >> 10));
    jNum(j, "tx_kb", (long)(txTotal >> 10));
    jBool(j, "conflict", conflict);
    jEnd(j);
    server.send(200, "application/json", j);
}

// Saved settings for pre-filling the web form (passwords are never sent back)
void handleConfig() {
    String j = "{";
    jBool(j, "configured", !isSetupMode);
    for (int i = 0; i < MAX_UPSTREAMS; i++) {
        char k1[16], k2[20];
        snprintf(k1, sizeof(k1), "sta_ssid%d", i);
        snprintf(k2, sizeof(k2), "sta_has_pass%d", i);
        jStr(j, k1, String(currentConfig.sta[i].ssid));
        jBool(j, k2, currentConfig.sta[i].pass[0] != 0);
    }
    jBool(j, "mac_on", currentConfig.use_custom_mac);
    jStr(j, "mac", macToStr(currentConfig.custom_mac));
    jStr(j, "ap_ssid", String(currentConfig.ap_ssid));
    jBool(j, "ap_has_pass", currentConfig.ap_pass[0] != 0);
    jBool(j, "ap_mac_on", currentConfig.use_ap_mac);
    jStr(j, "ap_mac", macToStr(currentConfig.ap_mac));
    jBool(j, "hidden", currentConfig.ap_hidden);
    jNum(j, "max_clients", currentConfig.ap_max_clients);
    jNum(j, "channel", currentConfig.ap_channel);
    jNum(j, "tx_power", currentConfig.tx_power);
    jStr(j, "ap_ip", currentApIp().toString());
    jNum(j, "dhcp_start", currentConfig.dhcp_start);
    jNum(j, "dhcp_end", currentConfig.dhcp_end);
    String dns = "";
    if (currentConfig.dns[0] != 0) {
        dns = IPAddress(currentConfig.dns[0], currentConfig.dns[1], currentConfig.dns[2], currentConfig.dns[3]).toString();
    }
    jStr(j, "dns", dns);
    jNum(j, "filter_mode", currentConfig.filter_mode);
    String fl = "";
    for (int i = 0; i < currentConfig.filter_count; i++) {
        if (i) fl += "\n";
        fl += macToStr(currentConfig.filter[i]);
    }
    jStr(j, "filter_list", fl);
    jEnd(j);
    server.send(200, "application/json", j);
}

// Lists devices currently associated to our repeated (SoftAP) network: MAC + IP.
void handleClients() {
    String json = "[";
    struct station_info *stat_info = wifi_softap_get_station_info();
    bool first = true;
    while (stat_info != NULL) {
        if (!first) json += ",";
        first = false;
        IPAddress ip(stat_info->ip.addr);
        json += "{\"mac\":\"" + macToStr(stat_info->bssid) + "\",\"ip\":\"" + ip.toString() + "\"}";
        stat_info = STAILQ_NEXT(stat_info, next);
    }
    wifi_softap_free_station_info();
    json += "]";
    server.send(200, "application/json", json);
}

// Quick "Block" button from the clients list
void handleBlock() {
    if (isSetupMode) { bad("Finish the initial setup first"); return; }
    uint8_t m[6];
    if (!parseMac(server.arg("mac"), m)) { bad("Invalid MAC address"); return; }
    uint8_t me[6];
    if (getRequesterMac(me) && memcmp(me, m, 6) == 0) {
        bad("You can't block the device you are using right now");
        return;
    }
    if (currentConfig.filter_mode == 2) {
        removeFromList(currentConfig, m);      // allow-list: blocking = removing from the list
    } else {
        if (!macInList(currentConfig, m)) {
            if (currentConfig.filter_count >= MAX_MAC_FILTER) { bad("Filter list is full"); return; }
            memcpy(currentConfig.filter[currentConfig.filter_count++], m, 6);
        }
        currentConfig.filter_mode = 1;
    }
    ConfigStorage::save(currentConfig);
    wifi_softap_deauth(m);
    server.send(200, "text/plain", "OK");
}

void handleSave() {
    RepeaterConfig n;
    ConfigStorage::defaults(n);

    // ---- Upstream networks (primary + 2 backups) ----
    for (int i = 0; i < MAX_UPSTREAMS; i++) {
        char kS[12], kP[12], kK[8];
        snprintf(kS, sizeof(kS), "sta_ssid%d", i);
        snprintf(kP, sizeof(kP), "sta_pass%d", i);
        snprintf(kK, sizeof(kK), "keep%d", i);
        if (!argCopy(kS, n.sta[i].ssid, sizeof(n.sta[i].ssid))) { bad("Network name is too long (max 32 characters)"); return; }
        if (!argCopy(kP, n.sta[i].pass, sizeof(n.sta[i].pass))) { bad("Password is too long (max 64 characters)"); return; }
        if (n.sta[i].ssid[0] == 0) {
            n.sta[i].pass[0] = 0;
        } else if (server.arg(kK) == "1" && strcmp(n.sta[i].ssid, currentConfig.sta[i].ssid) == 0) {
            strcpy(n.sta[i].pass, currentConfig.sta[i].pass);   // keep the saved password
        }
    }
    if (n.sta[0].ssid[0] == 0) { bad("Primary source network is required"); return; }

    // ---- Repeated network ----
    if (!argCopy("ap_ssid", n.ap_ssid, sizeof(n.ap_ssid))) { bad("Repeater name is too long (max 32 characters)"); return; }
    if (n.ap_ssid[0] == 0) { bad("Repeater network name is required"); return; }
    if (!argCopy("ap_pass", n.ap_pass, sizeof(n.ap_pass))) { bad("Repeater password is too long"); return; }
    if (server.arg("keep_ap") == "1") strcpy(n.ap_pass, currentConfig.ap_pass);
    size_t apl = strlen(n.ap_pass);
    if (apl != 0 && (apl < 8 || apl > 63)) { bad("Repeater password must be 8-63 characters (or empty for an open network)"); return; }

    n.ap_hidden = (server.arg("hidden") == "1");
    int mc = server.arg("max_clients").toInt();
    n.ap_max_clients = (mc < 1 || mc > 8) ? 8 : (uint8_t)mc;
    int ch = server.arg("channel").toInt();
    n.ap_channel = (ch < 0 || ch > 13) ? 0 : (uint8_t)ch;
    int tp = server.arg("tx_power").toInt();
    n.tx_power = (tp < 5 || tp > 20) ? 20 : (uint8_t)tp;

    // ---- MAC clone (WAN) and AP MAC ----
    n.use_custom_mac = (server.arg("mac_on") == "1");
    if (n.use_custom_mac) {
        if (!parseMac(server.arg("custom_mac"), n.custom_mac) || !validUnicastMac(n.custom_mac)) {
            bad("Invalid WAN MAC. Use AA:BB:CC:DD:EE:FF (first byte must be even, e.g. 02, 00, A4)");
            return;
        }
    }
    n.use_ap_mac = (server.arg("ap_mac_on") == "1");
    if (n.use_ap_mac) {
        if (!parseMac(server.arg("ap_mac"), n.ap_mac) || !validUnicastMac(n.ap_mac)) {
            bad("Invalid repeater MAC. Use AA:BB:CC:DD:EE:FF (first byte must be even, e.g. 02, 00, A4)");
            return;
        }
    }
    if (n.use_custom_mac && n.use_ap_mac && memcmp(n.custom_mac, n.ap_mac, 6) == 0) {
        bad("WAN MAC and repeater MAC must be different");
        return;
    }

    // ---- LAN: IP, DHCP range, DNS ----
    if (!parseIp(server.arg("ap_ip"), n.ap_ip) || n.ap_ip[0] == 0 || n.ap_ip[0] == 127 || n.ap_ip[0] > 223 ||
        n.ap_ip[3] < 1 || n.ap_ip[3] > 254) {
        bad("Invalid repeater IP address");
        return;
    }
    int ds = server.arg("dhcp_start").toInt();
    int de = server.arg("dhcp_end").toInt();
    if (ds < 1 || ds > 254 || de < 1 || de > 254 || ds > de) { bad("Invalid DHCP range (use 1-254, start <= end)"); return; }
    if ((de - ds) > 100) { bad("DHCP range can hold at most 100 addresses"); return; }
    if (n.ap_ip[3] >= ds && n.ap_ip[3] <= de) { bad("The repeater IP must be outside the DHCP range"); return; }
    n.dhcp_start = (uint8_t)ds;
    n.dhcp_end = (uint8_t)de;
    String dnsStr = server.arg("dns");
    dnsStr.trim();
    if (dnsStr.length() > 0) {
        if (!parseIp(dnsStr, n.dns) || n.dns[0] == 0) { bad("Invalid DNS server address"); return; }
    }

    // ---- MAC filter ----
    int fm = server.arg("filter_mode").toInt();
    n.filter_mode = (fm < 0 || fm > 2) ? 0 : (uint8_t)fm;
    n.filter_count = 0;
    {
        String s = server.arg("filter_list");
        String tok = "";
        for (size_t i = 0; i <= s.length(); i++) {
            char c = (i < s.length()) ? s[i] : ' ';
            bool sep = (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == ',' || c == ';');
            if (!sep) { tok += c; continue; }
            if (tok.length() == 0) continue;
            uint8_t m[6];
            if (!parseMac(tok, m)) {
                String msg = "Invalid MAC in device list: " + tok.substring(0, 24);
                bad(msg.c_str());
                return;
            }
            tok = "";
            if (macInList(n, m)) continue;                       // ignore duplicates
            if (n.filter_count >= MAX_MAC_FILTER) { bad("Device list can hold at most 12 MAC addresses"); return; }
            memcpy(n.filter[n.filter_count++], m, 6);
        }
    }
    // Lock-out protection for the device that is saving these settings
    uint8_t me[6];
    bool haveMe = getRequesterMac(me);
    if (n.filter_mode == 1 && haveMe && macInList(n, me)) {
        bad("You can't block the device you are using right now");
        return;
    }
    if (n.filter_mode == 2) {
        if (haveMe && !macInList(n, me) && n.filter_count < MAX_MAC_FILTER) {
            memcpy(n.filter[n.filter_count++], me, 6);           // keep this device allowed
        }
        if (n.filter_count == 0) { bad("Allow-list is empty: every device would be blocked"); return; }
    }

    ConfigStorage::save(n);
    server.send(200, "text/plain", "OK");
    Serial.println(F("[CONFIG] Saved. Restarting in 1s..."));
    scheduledReboot = millis() + 1000;
}

void handleReset() {
    ConfigStorage::clear();
    server.send(200, "text/plain", "Reset OK");
    Serial.println(F("[CONFIG] Reset. Restarting in 1s..."));
    scheduledReboot = millis() + 1000;
}

void handleReboot() {
    server.send(200, "text/plain", "Rebooting");
    scheduledReboot = millis() + 500;
}

// ---------------------------------------------------------------------------
// Radio / network setup
// ---------------------------------------------------------------------------
void applyRadioTuning() {
    // 1. Force hardware sleep OFF at SDK level
    wifi_set_sleep_type(NONE_SLEEP_T);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);

    // 2. RF output power (user selectable, 20 dBm = maximum)
    float p = (currentConfig.tx_power >= 20) ? 20.5f : (float)currentConfig.tx_power;
    WiFi.setOutputPower(p);
}

// Repeater IP + DHCP range
void applyApNetwork() {
    IPAddress ip = currentApIp();
    IPAddress netMsk(255, 255, 255, 0);
    WiFi.softAPConfig(ip, ip, netMsk);
#ifdef HAVE_DHCP_NONOS
    struct dhcps_lease lease;
    memset(&lease, 0, sizeof(lease));
    lease.enable = true;
    lease.start_ip.addr = IPAddress(currentConfig.ap_ip[0], currentConfig.ap_ip[1], currentConfig.ap_ip[2], currentConfig.dhcp_start).v4();
    lease.end_ip.addr = IPAddress(currentConfig.ap_ip[0], currentConfig.ap_ip[1], currentConfig.ap_ip[2], currentConfig.dhcp_end).v4();
    wifi_softap_dhcps_stop();
    bool ok = wifi_softap_set_dhcps_lease(&lease);
    wifi_softap_dhcps_start();
    Serial.printf("[DHCP] Range .%d-.%d %s\n", currentConfig.dhcp_start, currentConfig.dhcp_end, ok ? "applied" : "FAILED (using default)");
#endif
}

void startAp(int channel) {
    const char *psk = (strlen(currentConfig.ap_pass) >= 8) ? currentConfig.ap_pass : nullptr;
    WiFi.softAP(currentConfig.ap_ssid, psk, channel, currentConfig.ap_hidden ? 1 : 0, currentConfig.ap_max_clients);
}

void checkResetButton(unsigned long now) {
    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
        if (btnDownSince == 0) {
            btnDownSince = now;
        } else if (now - btnDownSince > RESET_HOLD_MS) {
            Serial.println(F("[SYSTEM] FLASH button held: factory reset"));
            ConfigStorage::clear();
            delay(200);
            ESP.restart();
        }
    } else {
        btnDownSince = 0;
    }
}

void setup() {
    Serial.begin(115200);
    delay(200);
    pinMode(STATUS_LED, OUTPUT);
    setLed(false);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F(" ESP8266 Low-Latency Turbo Repeater     "));
    Serial.println(F("========================================"));

    ConfigStorage::begin();
    bool hasConfig = ConfigStorage::load(currentConfig);

    IPAddress apIP = currentApIp();
    apIpStr = apIP.toString();
    IPAddress netMsk(255, 255, 255, 0);

    // Don't let the SDK auto-persist Wi-Fi state to flash on every connect;
    // we manage our own EEPROM config instead. Reduces flash wear & boot time.
    WiFi.persistent(false);

    if (hasConfig) {
        Serial.printf("[BOOT] Starting Repeater: STA='%s' -> AP='%s'\n", currentConfig.sta[0].ssid, currentConfig.ap_ssid);
        isSetupMode = false;
        WiFi.mode(WIFI_AP_STA);

        if (currentConfig.use_custom_mac) {
            wifi_set_macaddr(STATION_IF, currentConfig.custom_mac);
            Serial.printf("[MAC] Custom WAN MAC applied: %s\n", macToStr(currentConfig.custom_mac).c_str());
        }
        if (currentConfig.use_ap_mac) {
            wifi_set_macaddr(SOFTAP_IF, currentConfig.ap_mac);
            Serial.printf("[MAC] Custom repeater MAC applied: %s\n", macToStr(currentConfig.ap_mac).c_str());
        }

        applyRadioTuning();
        applyApNetwork();
        lockedChannel = currentConfig.ap_channel ? currentConfig.ap_channel : 1;
        startAp(lockedChannel);

        WiFi.setAutoReconnect(true);
        WiFi.begin(currentConfig.sta[0].ssid, currentConfig.sta[0].pass);
    } else {
        Serial.println(F("[BOOT] Mode Setup: SSID 'ESP8266-Repeater-Setup'"));
        isSetupMode = true;
        WiFi.mode(WIFI_AP);
        applyRadioTuning();

        WiFi.softAPConfig(apIP, apIP, netMsk);
        WiFi.softAP("ESP8266-Repeater-Setup");
        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer.start(53, "*", apIP);
    }

    server.on("/", HTTP_GET, handleRoot);
    server.on("/scan", HTTP_GET, handleScan);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/config", HTTP_GET, handleConfig);
    server.on("/clients", HTTP_GET, handleClients);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/block", HTTP_POST, handleBlock);
    server.on("/reset", HTTP_POST, handleReset);
    server.on("/reboot", HTTP_POST, handleReboot);

    server.on("/generate_204", HTTP_GET, []() {
        server.sendHeader("Location", "http://" + apIpStr + "/", true);
        server.send(302, "text/plain", "");
    });
    server.on("/hotspot-detect.html", HTTP_GET, []() {
        server.sendHeader("Location", "http://" + apIpStr + "/", true);
        server.send(302, "text/plain", "");
    });
    server.onNotFound([]() {
        String host = server.hostHeader();
        if (host.length() > 0 && !host.startsWith(apIpStr)) {
            server.sendHeader("Location", "http://" + apIpStr + "/", true);
            server.send(302, "text/plain", "");
        } else {
            server.send(404, "text/plain", "Not Found");
        }
    });

    server.begin();
    Serial.printf("[BOOT] Ready at http://%s\n", apIpStr.c_str());
}

void loop() {
    server.handleClient();

    if (isSetupMode) {
        dnsServer.processNextRequest();
    }

    unsigned long now = millis();
    checkResetButton(now);

    if (!isSetupMode) {
        tickTraffic(now);

        if (currentConfig.filter_mode != 0 && now - lastFilterTick > 1000) {
            lastFilterTick = now;
            enforceMacFilter();
        }

        if (WiFi.status() == WL_CONNECTED) {
            wasConnected = true;
            disconnectedSince = 0;
            if (!naptActive) {
                int ch = WiFi.channel();
                Serial.printf("[WIFI] Connected! WAN IP: %s on Channel %d\n", WiFi.localIP().toString().c_str(), ch);

                // Align SoftAP to exact same radio channel as upstream router (single radio)
                if (ch != lockedChannel && ch > 0) {
                    lockedChannel = ch;
                    applyApNetwork();
                    startAp(lockedChannel);
                    Serial.printf("[WIFI] SoftAP locked to synchronized Channel %d (zero hopping latency)\n", lockedChannel);
                }

                err_t ret = ip_napt_init(1000, 32);
                if (ret == ERR_OK) {
                    ret = ip_napt_enable_no(SOFTAP_IF, 1);
                    if (ret == ERR_OK) {
                        naptActive = true;
                        Serial.println(F("[NAPT] 160MHz LwIP NAPT active and forwarding packets!"));
                    }
                }

                // DNS handed out to connected devices: custom one, or the router's
                auto &dhcp = WiFi.softAPDhcpServer();
                if (currentConfig.dns[0] != 0) {
                    dhcp.setDns(IPAddress(currentConfig.dns[0], currentConfig.dns[1], currentConfig.dns[2], currentConfig.dns[3]));
                } else {
                    dhcp.setDns(WiFi.dnsIP(0));
                }
            }
            installTrafficHooks();
            setLed((now % 2000) < 1900);
        } else {
            naptActive = false;
            if (wasConnected) {
                Serial.println(F("[WIFI] Upstream link lost. Will retry..."));
                wasConnected = false;
                lastReconnectAttempt = now;
                disconnectedSince = now;
            }
            if (disconnectedSince == 0) disconnectedSince = now;

            if (countUpstreams() > 1 && now - disconnectedSince > FAILOVER_AFTER_MS) {
                // Primary (or current) network unreachable for a while: try the next one
                activeUp = nextUpstream(activeUp);
                Serial.printf("[FAILOVER] Trying upstream #%d '%s'\n", activeUp + 1, currentConfig.sta[activeUp].ssid);
                WiFi.begin(currentConfig.sta[activeUp].ssid, currentConfig.sta[activeUp].pass);
                disconnectedSince = now;
                lastReconnectAttempt = now;
            } else if (now - lastReconnectAttempt > RECONNECT_INTERVAL_MS) {
                // Belt-and-suspenders reconnect in case the SDK's auto-reconnect stalls
                WiFi.reconnect();
                lastReconnectAttempt = now;
            }
            if (now - lastBlink > 400) {
                ledState = !ledState;
                setLed(ledState);
                lastBlink = now;
            }
        }
    } else {
        if (now - lastBlink > 150) {
            ledState = !ledState;
            setLed(ledState);
            lastBlink = now;
        }
    }

    if (scheduledReboot > 0 && now >= scheduledReboot) {
        Serial.println(F("[SYSTEM] Restarting..."));
        ESP.restart();
    }

    // Zero delay for immediate packet processing
    yield();
}
