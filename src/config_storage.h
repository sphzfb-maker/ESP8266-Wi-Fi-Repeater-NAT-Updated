#pragma once
#include <Arduino.h>
#include <EEPROM.h>

#define CONFIG_MAGIC 0xAA55B010
#define EEPROM_SIZE 1024
#define MAX_UPSTREAMS 3
#define MAX_MAC_FILTER 12

struct UpstreamNet {
    char ssid[33];
    char pass[65];
};

struct RepeaterConfig {
    uint32_t magic;

    // Upstream (WAN) networks: [0] = primary, [1..2] = optional backups
    UpstreamNet sta[MAX_UPSTREAMS];
    bool use_custom_mac;          // clone/override the WAN (station) MAC
    uint8_t custom_mac[6];

    // Repeated (AP) network
    char ap_ssid[33];
    char ap_pass[65];
    bool use_ap_mac;              // override the AP MAC
    uint8_t ap_mac[6];
    bool ap_hidden;
    uint8_t ap_max_clients;       // 1..8
    uint8_t ap_channel;           // 0 = auto (follows upstream router), 1..13
    uint8_t tx_power;             // dBm, 5..20

    // LAN side
    uint8_t ap_ip[4];             // repeater IP (netmask is fixed /24)
    uint8_t dhcp_start;           // last octet
    uint8_t dhcp_end;             // last octet
    uint8_t dns[4];               // 0.0.0.0 = hand out the upstream router's DNS

    // Device access (MAC filter)
    uint8_t filter_mode;          // 0 = off, 1 = block listed, 2 = allow only listed
    uint8_t filter_count;
    uint8_t filter[MAX_MAC_FILTER][6];

    uint32_t checksum;
};

static_assert(sizeof(RepeaterConfig) <= EEPROM_SIZE, "RepeaterConfig is too big for EEPROM_SIZE");

class ConfigStorage {
public:
    static void begin() {
        EEPROM.begin(EEPROM_SIZE);
    }

    static void defaults(RepeaterConfig &cfg) {
        memset(&cfg, 0, sizeof(cfg));
        cfg.ap_max_clients = 8;
        cfg.ap_channel = 0;
        cfg.tx_power = 20;
        cfg.ap_ip[0] = 192;
        cfg.ap_ip[1] = 168;
        cfg.ap_ip[2] = 4;
        cfg.ap_ip[3] = 1;
        cfg.dhcp_start = 100;
        cfg.dhcp_end = 200;
    }

    // Keep every value inside a safe range (protects against corrupted-but-checksummed data)
    static void sanitize(RepeaterConfig &cfg) {
        for (int i = 0; i < MAX_UPSTREAMS; i++) {
            cfg.sta[i].ssid[32] = 0;
            cfg.sta[i].pass[64] = 0;
        }
        cfg.ap_ssid[32] = 0;
        cfg.ap_pass[64] = 0;
        if (cfg.ap_max_clients < 1 || cfg.ap_max_clients > 8) cfg.ap_max_clients = 8;
        if (cfg.ap_channel > 13) cfg.ap_channel = 0;
        if (cfg.tx_power < 5 || cfg.tx_power > 20) cfg.tx_power = 20;
        if (cfg.ap_ip[3] < 1 || cfg.ap_ip[3] > 254) cfg.ap_ip[3] = 1;
        if (cfg.dhcp_start < 1 || cfg.dhcp_start > 254 || cfg.dhcp_end < cfg.dhcp_start || cfg.dhcp_end > 254 ||
            (cfg.dhcp_end - cfg.dhcp_start) > 100) {
            cfg.dhcp_start = 100;
            cfg.dhcp_end = 200;
        }
        if (cfg.filter_mode > 2) cfg.filter_mode = 0;
        if (cfg.filter_count > MAX_MAC_FILTER) cfg.filter_count = 0;
    }

    static uint32_t calcChecksum(const RepeaterConfig &cfg) {
        uint32_t sum = 0;
        const uint8_t *p = (const uint8_t *)&cfg;
        size_t len = sizeof(RepeaterConfig) - sizeof(uint32_t);
        for (size_t i = 0; i < len; i++) {
            sum = (sum * 31) + p[i];
        }
        return sum;
    }

    // Returns true when a valid configuration with a primary upstream network exists.
    // On failure cfg is reset to defaults so the web UI always has sane values.
    static bool load(RepeaterConfig &cfg) {
        EEPROM.get(0, cfg);
        bool ok = (cfg.magic == CONFIG_MAGIC) && (cfg.checksum == calcChecksum(cfg));
        if (!ok) {
            defaults(cfg);
            return false;
        }
        sanitize(cfg);
        return cfg.sta[0].ssid[0] != 0;
    }

    static bool save(const RepeaterConfig &cfg) {
        RepeaterConfig toSave;
        memcpy(&toSave, &cfg, sizeof(toSave));
        toSave.magic = CONFIG_MAGIC;
        toSave.checksum = calcChecksum(toSave);
        EEPROM.put(0, toSave);
        return EEPROM.commit();
    }

    static void clear() {
        RepeaterConfig emptyCfg;
        memset(&emptyCfg, 0, sizeof(emptyCfg));
        EEPROM.put(0, emptyCfg);
        EEPROM.commit();
    }
};
