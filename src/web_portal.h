#pragma once
#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP8266 Wi-Fi Turbo Repeater v1.2.2</title>
    <style>
        :root {
            --bg: #0b1120;
            --card: #1e293b;
            --card-sub: #0f172a;
            --border: #334155;
            --text: #f8fafc;
            --text-dim: #94a3b8;
            --primary: #3b82f6;
            --primary-hover: #2563eb;
            --success: #10b981;
            --warning: #f59e0b;
            --danger: #ef4444;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
        html, body { max-width: 100%; overflow-x: hidden; }
        body { background-color: var(--bg); color: var(--text); padding: 16px; display: flex; justify-content: center; min-height: 100vh; }
        .container { width: 100%; min-width: 0; max-width: 500px; }
        .header { text-align: center; margin-bottom: 20px; }
        .header h1 { font-size: 1.4rem; font-weight: 700; color: #fff; margin-bottom: 4px; display: flex; align-items: center; justify-content: center; gap: 8px; }
        .header p { color: var(--text-dim); font-size: 0.825rem; }
        .card { min-width: 0; background: var(--card); border: 1px solid var(--border); border-radius: 12px; padding: 18px; margin-bottom: 14px; box-shadow: 0 4px 6px -1px rgba(0,0,0,0.3); }
        .card-title { font-size: 1rem; font-weight: 600; margin-bottom: 12px; display: flex; justify-content: space-between; align-items: center; }
        .badge { font-size: 0.725rem; padding: 3px 8px; border-radius: 9999px; font-weight: 600; }
        .badge-success { background: rgba(16,185,129,0.2); color: var(--success); }
        .badge-warning { background: rgba(245,158,11,0.2); color: var(--warning); }
        .stat-grid { display: grid; grid-template-columns: repeat(2, minmax(0, 1fr)); gap: 10px; }
        .stat-item { background: var(--card-sub); padding: 10px; border-radius: 8px; border: 1px solid var(--border); }
        .stat-label { font-size: 0.7rem; color: var(--text-dim); margin-bottom: 3px; }
        .stat-val { font-size: 0.9rem; font-weight: 600; word-break: break-all; }
        .stat-sub { display: block; font-size: 0.7rem; color: var(--text-dim); font-weight: 400; }
        .form-group { margin-bottom: 14px; }
        label { display: block; font-size: 0.825rem; font-weight: 500; margin-bottom: 5px; color: var(--text); }
        .check-label { display: flex; align-items: center; gap: 8px; cursor: pointer; }
        .check-label input[type=checkbox] { width: auto; }
        .input-wrapper { position: relative; display: flex; align-items: center; }
        select, input, textarea { width: 100%; min-width: 0; max-width: 100%; padding: 10px 12px; border-radius: 8px; background: var(--card-sub); border: 1px solid var(--border); color: #fff; font-size: 0.875rem; outline: none; transition: border-color 0.2s; }
        textarea { min-height: 84px; resize: vertical; font-family: monospace; }
        input[type=range] { padding: 0; height: 28px; }
        select:focus, input:focus, textarea:focus { border-color: var(--primary); }
        input:disabled { opacity: 0.5; }
        .eye-btn { position: absolute; right: 10px; background: none; border: none; color: var(--text-dim); cursor: pointer; padding: 4px; display: flex; align-items: center; }
        .eye-btn:hover { color: #fff; }
        .hint { font-size: 0.725rem; color: var(--text-dim); margin-top: 3px; }
        .row2 { display: grid; grid-template-columns: repeat(2, minmax(0, 1fr)); gap: 10px; }
        details { border: 1px solid var(--border); border-radius: 10px; margin-bottom: 12px; background: var(--card-sub); }
        summary { cursor: pointer; padding: 12px; font-weight: 600; font-size: 0.9rem; }
        .dbody { padding: 0 12px 4px; }
        .sub-title { font-size: 0.75rem; color: var(--text-dim); margin: 4px 0 8px; text-transform: uppercase; letter-spacing: 0.04em; }
        button.btn-main { width: 100%; padding: 12px; background: var(--primary); color: #fff; border: none; border-radius: 8px; font-weight: 600; font-size: 0.925rem; cursor: pointer; transition: background 0.2s; display: flex; justify-content: center; align-items: center; gap: 8px; }
        button.btn-main:hover { background: var(--primary-hover); }
        button.btn-outline { width: 100%; padding: 10px; background: transparent; border: 1px solid var(--border); color: var(--text); border-radius: 8px; font-weight: 500; font-size: 0.85rem; cursor: pointer; margin-top: 8px; }
        button.btn-outline:hover { background: rgba(255,255,255,0.05); }
        button.btn-danger { width: 100%; padding: 10px; background: rgba(239,68,68,0.15); border: 1px solid var(--danger); color: #fca5a5; border-radius: 8px; font-weight: 500; font-size: 0.85rem; cursor: pointer; margin-top: 8px; }
        button.btn-danger:hover { background: var(--danger); color: #fff; }
        .scan-btn { width: auto; padding: 5px 10px; font-size: 0.75rem; background: #334155; color: #fff; border: none; border-radius: 6px; cursor: pointer; }
        .scan-btn:hover { background: #475569; }
        .mini-btn { padding: 4px 9px; font-size: 0.7rem; background: rgba(239,68,68,0.15); border: 1px solid var(--danger); color: #fca5a5; border-radius: 6px; cursor: pointer; }
        .mini-btn:hover { background: var(--danger); color: #fff; }
        .spinner { border: 2px solid rgba(255,255,255,0.3); border-radius: 50%; border-top: 2px solid #fff; width: 14px; height: 14px; animation: spin 0.8s linear infinite; display: inline-block; }
        @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
        .alert { padding: 12px; border-radius: 8px; margin-bottom: 12px; font-size: 0.825rem; display: none; }
        .alert-info { background: rgba(59,130,246,0.15); border: 1px solid var(--primary); color: #93c5fd; }
        .alert-success { background: rgba(16,185,129,0.15); border: 1px solid var(--success); color: #6ee7b7; }
        .alert-warn { background: rgba(245,158,11,0.15); border: 1px solid var(--warning); color: #fcd34d; }
        .client-row { display: flex; justify-content: space-between; align-items: center; gap: 8px; padding: 8px 10px; background: var(--card-sub); border: 1px solid var(--border); border-radius: 8px; margin-bottom: 6px; font-size: 0.8rem; }
        .client-row:last-child { margin-bottom: 0; }
        .client-info { display: flex; flex-direction: column; min-width: 0; overflow-wrap: anywhere; }
        .client-mac { font-weight: 600; }
        .client-ip { color: var(--text-dim); }
        .client-name { font-weight: 700; margin-bottom: 2px; }
        .client-name-source { color: var(--text-dim); font-size: 0.68rem; margin-left: 5px; }
        .wan-ip-box { margin-top: 10px; padding: 10px; border: 1px dashed var(--border); border-radius: 8px; }
        .wan-ip-box .sub-title { margin-top: 0; }
        .empty-hint { color: var(--text-dim); font-size: 0.8rem; text-align: center; padding: 10px 0; }
        .diag-grid { display: grid; grid-template-columns: repeat(2, minmax(0, 1fr)); gap: 8px; }
        .diag-item { padding: 8px 10px; background: var(--card-sub); border: 1px solid var(--border); border-radius: 8px; }
        .diag-label { display: block; color: var(--text-dim); font-size: 0.68rem; margin-bottom: 2px; }
        .diag-val { font-family: monospace; font-size: 0.76rem; word-break: break-all; }
        .tool-row { display: grid; grid-template-columns: repeat(3, minmax(0, 1fr)); gap: 8px; margin-top: 10px; }
        .tool-btn { padding: 9px 6px; background: transparent; border: 1px solid var(--border); color: var(--text); border-radius: 8px; font-size: 0.75rem; cursor: pointer; }
        .tool-btn:hover { background: rgba(255,255,255,0.05); }
        @media (max-width: 420px) { .tool-row { grid-template-columns: 1fr; } }
        .tool-btn { min-width: 0; overflow-wrap: anywhere; }
    </style>
</head>
<body>
<div class="container">
    <div class="header">
        <h1>
            <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="#3b82f6" stroke-width="2.5"><path d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M1.42 9a16 16 0 0 1 21.16 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>
            Wi-Fi Turbo Repeater
        </h1>
        <p>160MHz L3 NAT Engine &bull; Zero-Sleep RF</p>
    </div>

    <div id="warn" class="alert alert-warn"></div>

    <!-- Status Card -->
    <div class="card">
        <div class="card-title">
            <span>Link Status</span>
            <span id="status-badge" class="badge badge-warning">Loading...</span>
        </div>
        <div class="stat-grid">
            <div class="stat-item">
                <div class="stat-label">Home Network (WAN)</div>
                <div id="stat-sta-ssid" class="stat-val">---</div>
            </div>
            <div class="stat-item">
                <div class="stat-label">Assigned IP (WAN)</div>
                <div id="stat-sta-ip" class="stat-val">---</div>
            </div>
            <div class="stat-item">
                <div class="stat-label">Wi-Fi Signal (RSSI)</div>
                <div id="stat-rssi" class="stat-val">---</div>
            </div>
            <div class="stat-item">
                <div class="stat-label">Channel</div>
                <div id="stat-channel" class="stat-val">---</div>
            </div>
            <div class="stat-item">
                <div class="stat-label">Current MAC (WAN)</div>
                <div id="stat-mac" class="stat-val">---</div>
            </div>
            <div class="stat-item">
                <div class="stat-label">Uptime</div>
                <div id="stat-uptime" class="stat-val">---</div>
            </div>
        </div>
    </div>

    <!-- Traffic Card -->
    <div class="card">
        <div class="card-title"><span>Traffic</span><span class="stat-sub">since boot</span></div>
        <div class="stat-grid">
            <div class="stat-item">
                <div class="stat-label">Download speed</div>
                <div id="stat-rx" class="stat-val">---</div>
            </div>
            <div class="stat-item">
                <div class="stat-label">Upload speed</div>
                <div id="stat-tx" class="stat-val">---</div>
            </div>
            <div class="stat-item">
                <div class="stat-label">Total downloaded</div>
                <div id="stat-rx-total" class="stat-val">---</div>
            </div>
            <div class="stat-item">
                <div class="stat-label">Total uploaded</div>
                <div id="stat-tx-total" class="stat-val">---</div>
            </div>
        </div>
    </div>

    <!-- Connected Clients Card -->
    <div class="card">
        <div class="card-title">
            <span>Connected Clients</span>
            <span id="client-count-badge" class="badge badge-success">0</span>
        </div>
        <div id="clients-list">
            <div class="empty-hint">No devices connected yet.</div>
        </div>
    </div>

    <!-- Diagnostics & Tools -->
    <div class="card">
        <div class="card-title"><span>Diagnostics &amp; Tools</span><span id="test-badge" class="badge badge-warning">Ready</span></div>
        <div class="diag-grid">
            <div class="diag-item"><span class="diag-label">WAN gateway</span><span id="diag-gateway" class="diag-val">---</span></div>
            <div class="diag-item"><span class="diag-label">WAN DNS</span><span id="diag-dns" class="diag-val">---</span></div>
            <div class="diag-item"><span class="diag-label">Free RAM</span><span id="diag-heap" class="diag-val">---</span></div>
            <div class="diag-item"><span class="diag-label">Free sketch space</span><span id="diag-sketch" class="diag-val">---</span></div>
            <div class="diag-item"><span class="diag-label">Flash</span><span id="diag-flash" class="diag-val">---</span></div>
            <div class="diag-item"><span class="diag-label">CPU / core</span><span id="diag-core" class="diag-val">---</span></div>
            <div class="diag-item"><span class="diag-label">Reset reason</span><span id="diag-reset" class="diag-val">---</span></div>
            <div class="diag-item"><span class="diag-label">Upstream slot</span><span id="diag-upstream" class="diag-val">---</span></div>
            <div class="diag-item"><span class="diag-label">Hooks: DHCP / AP out / WAN out</span><span id="diag-hooks" class="diag-val">---</span></div>
        </div>
        <div class="tool-row">
            <button type="button" class="tool-btn" onclick="reconnectNow()">Reconnect WAN</button>
            <button type="button" class="tool-btn" onclick="runInternetTest()">Test Internet</button>
            <button type="button" class="tool-btn" onclick="resetTraffic()">Reset Traffic</button>
        </div>
        <div id="diag-result" class="hint" style="margin-top:8px;text-align:center;"></div>
    </div>

    <!-- Configuration Form -->
    <div class="card">
        <div class="card-title">
            <span>Settings</span>
            <button class="scan-btn" onclick="scanNetworks()" id="scan-btn">Scan Networks</button>
        </div>

        <div id="alert-box" class="alert"></div>

        <form id="config-form" onsubmit="saveConfig(event)">

            <details open>
                <summary>1. Source Wi-Fi (Main Router)</summary>
                <div class="dbody">
                    <div class="form-group">
                        <label for="sta_ssid0">Primary network</label>
                        <select id="sta_ssid_select" onchange="onSsidSelect(this.value)">
                            <option value="">-- Scan to list networks --</option>
                        </select>
                        <input type="text" id="sta_ssid0" list="ssid-list" placeholder="Network name (SSID)" style="margin-top: 6px;">
                        <div class="hint">Hidden Wi-Fi networks are shown by BSSID/channel during scanning. Type the hidden SSID manually to connect.</div>
                    </div>
                    <div class="form-group">
                        <label for="sta_pass0">Primary password</label>
                        <div class="input-wrapper">
                            <input type="password" id="sta_pass0" placeholder="Wi-Fi password" oninput="this.dataset.dirty='1'">
                            <button type="button" class="eye-btn" onclick="togglePass('sta_pass0')">👁️</button>
                        </div>
                    </div>
                    <div class="wan-ip-box">
                        <div class="sub-title">Primary network IP assignment</div>
                        <div class="form-group">
                            <label for="sta_mode0">WAN IP mode</label>
                            <select id="sta_mode0" onchange="toggleWanStatic(0)"><option value="0">DHCP (automatic)</option><option value="1">Static IP (manual)</option></select>
                        </div>
                        <div id="sta_static_box0" style="display:none;">
                            <div class="row2">
                                <div class="form-group"><label for="sta_ip0">IP address</label><input type="text" id="sta_ip0" placeholder="192.168.1.50"></div>
                                <div class="form-group"><label for="sta_gateway0">Gateway</label><input type="text" id="sta_gateway0" placeholder="192.168.1.1"></div>
                            </div>
                            <div class="row2">
                                <div class="form-group"><label for="sta_subnet0">Subnet mask</label><input type="text" id="sta_subnet0" placeholder="255.255.255.0"></div>
                                <div class="form-group"><label for="sta_dns0">DNS server</label><input type="text" id="sta_dns0" placeholder="8.8.8.8"></div>
                            </div>
                            <div class="hint">Use an unused address in the main router's subnet. Gateway is normally the main router address.</div>
                        </div>
                    </div>

                    <div class="form-group">
                        <label class="check-label">
                            <input type="checkbox" id="mac_enable" onchange="syncMac()">
                            Clone / Set Custom MAC (WAN side)
                        </label>
                        <input type="text" id="custom_mac" placeholder="AA:BB:CC:DD:EE:FF" disabled style="margin-top: 6px;">
                        <div class="hint">MAC used to connect to the main router (see "Current MAC" above for the default). Useful if your ISP/router locks access to one device MAC. First byte must be even (e.g. 02, 00, A4).</div>
                    </div>

                    <div class="sub-title">Backup networks (auto failover)</div>
                    <div class="form-group">
                        <input type="text" id="sta_ssid1" list="ssid-list" placeholder="Backup 1 name (SSID)">
                        <div class="input-wrapper" style="margin-top: 6px;">
                            <input type="password" id="sta_pass1" placeholder="Backup 1 password" oninput="this.dataset.dirty='1'">
                            <button type="button" class="eye-btn" onclick="togglePass('sta_pass1')">👁️</button>
                        </div>
                    </div>
                    <div class="wan-ip-box">
                        <div class="sub-title">Backup 1 IP assignment</div>
                        <div class="form-group">
                            <label for="sta_mode1">WAN IP mode</label>
                            <select id="sta_mode1" onchange="toggleWanStatic(1)"><option value="0">DHCP (automatic)</option><option value="1">Static IP (manual)</option></select>
                        </div>
                        <div id="sta_static_box1" style="display:none;">
                            <div class="row2">
                                <div class="form-group"><label for="sta_ip1">IP address</label><input type="text" id="sta_ip1" placeholder="192.168.1.50"></div>
                                <div class="form-group"><label for="sta_gateway1">Gateway</label><input type="text" id="sta_gateway1" placeholder="192.168.1.1"></div>
                            </div>
                            <div class="row2">
                                <div class="form-group"><label for="sta_subnet1">Subnet mask</label><input type="text" id="sta_subnet1" placeholder="255.255.255.0"></div>
                                <div class="form-group"><label for="sta_dns1">DNS server</label><input type="text" id="sta_dns1" placeholder="8.8.8.8"></div>
                            </div>
                            <div class="hint">Use an unused address in the main router's subnet. Gateway is normally the main router address.</div>
                        </div>
                    </div>
                    <div class="form-group">
                        <input type="text" id="sta_ssid2" list="ssid-list" placeholder="Backup 2 name (SSID)">
                        <div class="input-wrapper" style="margin-top: 6px;">
                            <input type="password" id="sta_pass2" placeholder="Backup 2 password" oninput="this.dataset.dirty='1'">
                            <button type="button" class="eye-btn" onclick="togglePass('sta_pass2')">👁️</button>
                        </div>
                        <div class="hint">If the current network is unreachable for ~25 seconds, the repeater switches to the next configured one. Leave empty to disable.</div>
                    </div>
                    <div class="wan-ip-box">
                        <div class="sub-title">Backup 2 IP assignment</div>
                        <div class="form-group">
                            <label for="sta_mode2">WAN IP mode</label>
                            <select id="sta_mode2" onchange="toggleWanStatic(2)"><option value="0">DHCP (automatic)</option><option value="1">Static IP (manual)</option></select>
                        </div>
                        <div id="sta_static_box2" style="display:none;">
                            <div class="row2">
                                <div class="form-group"><label for="sta_ip2">IP address</label><input type="text" id="sta_ip2" placeholder="192.168.1.50"></div>
                                <div class="form-group"><label for="sta_gateway2">Gateway</label><input type="text" id="sta_gateway2" placeholder="192.168.1.1"></div>
                            </div>
                            <div class="row2">
                                <div class="form-group"><label for="sta_subnet2">Subnet mask</label><input type="text" id="sta_subnet2" placeholder="255.255.255.0"></div>
                                <div class="form-group"><label for="sta_dns2">DNS server</label><input type="text" id="sta_dns2" placeholder="8.8.8.8"></div>
                            </div>
                            <div class="hint">Use an unused address in the main router's subnet. Gateway is normally the main router address.</div>
                        </div>
                    </div>
                    <datalist id="ssid-list"></datalist>
                </div>
            </details>

            <details open>
                <summary>2. Repeated Wi-Fi (Extender)</summary>
                <div class="dbody">
                    <div class="form-group">
                        <label for="ap_ssid">Network name (SSID)</label>
                        <input type="text" id="ap_ssid" placeholder="E.g: MyHome_EXT">
                    </div>
                    <div class="form-group">
                        <label for="ap_pass">Password</label>
                        <div class="input-wrapper">
                            <input type="password" id="ap_pass" placeholder="8-63 characters (empty = open)" oninput="this.dataset.dirty='1'">
                            <button type="button" class="eye-btn" onclick="togglePass('ap_pass')">👁️</button>
                        </div>
                        <div class="hint">Leave empty for an open network, or use 8-63 characters for WPA2.</div>
                    </div>

                    <div class="row2">
                        <div class="form-group">
                            <label for="max_clients">Max clients</label>
                            <select id="max_clients">
                                <option value="1">1</option><option value="2">2</option><option value="3">3</option><option value="4">4</option>
                                <option value="5">5</option><option value="6">6</option><option value="7">7</option><option value="8" selected>8</option>
                            </select>
                        </div>
                        <div class="form-group">
                            <label for="channel">Channel</label>
                            <select id="channel"></select>
                        </div>
                    </div>
                    <div class="hint" style="margin: -6px 0 12px;">The ESP8266 has one radio, so while connected the repeater always uses your router's channel. A fixed channel only applies when the router is not connected.</div>

                    <div class="form-group">
                        <label for="tx_power">TX power: <span id="tx_val">20 dBm</span></label>
                        <input type="range" id="tx_power" min="5" max="20" step="1" value="20" oninput="document.getElementById('tx_val').innerText = this.value + ' dBm'">
                    </div>

                    <div class="form-group">
                        <label class="check-label">
                            <input type="checkbox" id="hidden">
                            Hide network name (hidden SSID)
                        </label>
                        <div class="hint">Devices must type the name manually to connect.</div>
                    </div>

                    <div class="form-group">
                        <label class="check-label">
                            <input type="checkbox" id="ap_mac_enable" onchange="syncMac()">
                            Set Custom MAC (repeater side)
                        </label>
                        <input type="text" id="ap_mac" placeholder="AA:BB:CC:DD:EE:FF" disabled style="margin-top: 6px;">
                        <div class="hint">Must differ from the WAN MAC. First byte must be even.</div>
                    </div>
                </div>
            </details>

            <details>
                <summary>3. Network (IP / DHCP / DNS)</summary>
                <div class="dbody">
                    <div class="form-group">
                        <label for="ap_ip">Repeater IP address</label>
                        <input type="text" id="ap_ip" placeholder="192.168.4.1">
                        <div class="hint">Must be in a different subnet than your main router (e.g. router 192.168.1.x, repeater 192.168.4.x). If you change it, open the new address after the restart.</div>
                    </div>
                    <div class="row2">
                        <div class="form-group">
                            <label for="dhcp_start">DHCP start (last number)</label>
                            <input type="number" id="dhcp_start" min="1" max="254" value="100">
                        </div>
                        <div class="form-group">
                            <label for="dhcp_end">DHCP end (last number)</label>
                            <input type="number" id="dhcp_end" min="1" max="254" value="200">
                        </div>
                    </div>
                    <div class="hint" style="margin: -6px 0 12px;">Max 100 addresses. The repeater IP must be outside this range.</div>
                    <div class="form-group">
                        <label for="dns">Custom DNS server</label>
                        <input type="text" id="dns" placeholder="e.g. 8.8.8.8 (empty = use router's DNS)">
                        <div class="hint">Handed out to connected devices via DHCP.</div>
                    </div>
                </div>
            </details>

            <details>
                <summary>4. Device Access (MAC filter)</summary>
                <div class="dbody">
                    <div class="form-group">
                        <label for="filter_mode">Filter mode</label>
                        <select id="filter_mode">
                            <option value="0">Off (everyone can connect)</option>
                            <option value="1">Block listed devices</option>
                            <option value="2">Allow only listed devices</option>
                        </select>
                    </div>
                    <div class="form-group">
                        <label for="filter_list">Device list (one MAC per line, max 12)</label>
                        <textarea id="filter_list" placeholder="AA:BB:CC:DD:EE:FF"></textarea>
                        <div class="hint">Unwanted devices are disconnected within about a second. You can also press "Block" next to a device in the Connected Clients list. The device you are using now is protected from being blocked.</div>
                    </div>
                </div>
            </details>

            <button type="submit" class="btn-main" id="save-btn">Save &amp; Connect</button>
            <button type="button" class="btn-outline" onclick="rebootDevice()">Restart Device</button>
            <button type="button" class="btn-danger" onclick="resetConfig()">Factory Reset</button>
            <div class="hint" style="text-align:center; margin-top:10px;">Locked out? Hold the FLASH button on the board for 5 seconds to factory reset.</div>
        </form>
    </div>
</div>

<script>
const $ = id => document.getElementById(id);
let cfg = {};

function showAlert(text, type) {
    const box = $('alert-box');
    box.className = 'alert ' + (type || 'alert-info');
    box.innerText = text;
    box.style.display = 'block';
    box.scrollIntoView({ behavior: 'smooth', block: 'center' });
}

function togglePass(id) {
    const input = $(id);
    input.type = input.type === 'password' ? 'text' : 'password';
}

function syncMac() {
    $('custom_mac').disabled = !$('mac_enable').checked;
    $('ap_mac').disabled = !$('ap_mac_enable').checked;
}

function formatUptime(sec) {
    if (sec === undefined || sec === null) return '---';
    const h = Math.floor(sec / 3600);
    const m = Math.floor((sec % 3600) / 60);
    const s = Math.floor(sec % 60);
    return h + 'h ' + m + 'm ' + s + 's';
}

function fmtRate(bps) {
    const mbps = (bps * 8) / 1000000;
    return mbps.toFixed(2) + ' Mbps';
}

function fmtSize(kb) {
    if (kb >= 1048576) return (kb / 1048576).toFixed(2) + ' GB';
    if (kb >= 1024) return (kb / 1024).toFixed(1) + ' MB';
    return kb + ' KB';
}

function updateStatus() {
    fetch('/status')
        .then(r => r.json())
        .then(data => {
            let name = data.sta_ssid || 'Not configured';
            if (data.sta_connected && data.sta_slot > 0) name += ' (backup ' + data.sta_slot + ')';
            $('stat-sta-ssid').innerText = name;
            const ipEl = $('stat-sta-ip');
            ipEl.innerText = data.sta_ip || '0.0.0.0';
            const ipMode = document.createElement('span');
            ipMode.className = 'stat-sub';
            ipMode.innerText = data.wan_static ? 'Static IP' : 'DHCP';
            ipEl.appendChild(ipMode);
            $('stat-rssi').innerText = data.sta_connected ? `${data.rssi} dBm (${data.signal_pct}%)` : 'Disconnected';
            $('stat-channel').innerText = data.channel || '---';
            $('stat-mac').innerText = data.sta_mac || '---';
            $('stat-uptime').innerText = formatUptime(data.uptime_s);
            $('stat-rx').innerText = fmtRate(data.rx_bps || 0);
            $('stat-tx').innerText = fmtRate(data.tx_bps || 0);
            $('stat-rx-total').innerText = fmtSize(data.rx_kb || 0);
            $('stat-tx-total').innerText = fmtSize(data.tx_kb || 0);

            const badge = $('status-badge');
            if (data.sta_connected) {
                badge.innerText = 'Routing NAT (Online)';
                badge.className = 'badge badge-success';
            } else {
                badge.innerText = data.setup_mode ? 'Setup Mode' : 'Reconnecting...';
                badge.className = 'badge badge-warning';
            }

            const warn = $('warn');
            if (data.conflict) {
                warn.innerText = 'Warning: the repeater IP (' + data.ap_ip + ') is in the same subnet as your main router. Internet will not work. Change the repeater IP under Settings > Network.';
                warn.style.display = 'block';
            } else {
                warn.style.display = 'none';
            }
        })
        .catch(e => console.error(e));
}

function esc(s) {
    return String(s === undefined || s === null ? '' : s).replace(/[&<>"']/g, ch =>
        ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[ch]));
}

function updateClients() {
    fetch('/clients')
        .then(r => r.json())
        .then(list => {
            $('client-count-badge').innerText = list.length;
            const container = $('clients-list');
            if (list.length === 0) {
                container.innerHTML = '<div class="empty-hint">No devices connected yet.</div>';
                return;
            }
            container.innerHTML = list.map(c =>
                `<div class="client-row"><div class="client-info"><span class="client-name">${esc(c.name || 'Unknown device')}<span class="client-name-source">${c.name_source === 'DHCP' ? 'DHCP name' : 'MAC label'}</span></span><span class="client-mac">${esc(c.mac)}</span><span class="client-ip">${esc(c.ip)}</span></div><button class="mini-btn" onclick="blockClient('${esc(c.mac)}')">Block</button></div>`
            ).join('');
        })
        .catch(e => console.error(e));
}

function setDiag(id, value) {
    const e = $(id);
    if (e) e.innerText = value;
}

function loadDiagnostics() {
    fetch('/diagnostics')
        .then(r => r.json())
        .then(d => {
            setDiag('diag-gateway', d.gateway || '---');
            setDiag('diag-dns', [d.dns1, d.dns2].filter(x => x && x !== '0.0.0.0' && x.indexOf('unset') < 0).join(' / ') || '---');
            setDiag('diag-heap', fmtSize(Math.round((d.free_heap || 0) / 1024)) + ' (block ' + fmtSize(Math.round((d.max_block || 0) / 1024)) + ')');
            setDiag('diag-sketch', fmtSize(Math.round((d.free_sketch || 0) / 1024)));
            setDiag('diag-flash', fmtSize(Math.round((d.flash_size || 0) / 1024)) + ' / real ' + fmtSize(Math.round((d.flash_real_size || 0) / 1024)));
            setDiag('diag-core', (d.cpu_mhz || '?') + ' MHz / ' + (d.core_version || '?'));
            setDiag('diag-reset', d.reset_reason || '---');
            setDiag('diag-upstream', (d.active_upstream || '?') + ' / ' + (d.configured_upstreams || '?'));
            setDiag('diag-hooks', (d.hook_dhcp || 0) + ' / ' + (d.hook_ap_out || 0) + ' / ' + (d.hook_sta_out || 0));
        })
        .catch(() => {});
}

function reconnectNow() {
    const out = $('diag-result');
    out.innerText = 'Starting WAN reconnect...';
    fetch('/reconnect', { method: 'POST' })
        .then(r => r.text().then(t => ({ ok: r.ok, text: t })))
        .then(res => {
            out.innerText = res.text;
            setTimeout(loadDiagnostics, 1200);
        })
        .catch(() => { out.innerText = 'Reconnect request failed'; });
}

function runInternetTest() {
    const badge = $('test-badge');
    const out = $('diag-result');
    badge.innerText = 'Testing...';
    badge.className = 'badge badge-warning';
    out.innerText = 'Testing TCP connectivity to 1.1.1.1:80...';
    fetch('/internet-test')
        .then(r => r.json())
        .then(d => {
            if (d.ok) {
                badge.innerText = d.latency_ms + ' ms';
                badge.className = 'badge badge-success';
                out.innerText = 'TCP connection succeeded. This is a reachability test, not an ICMP ping.';
            } else {
                badge.innerText = 'Failed';
                badge.className = 'badge badge-warning';
                out.innerText = 'TCP connection failed. Check WAN Wi-Fi, gateway and DNS/router access.';
            }
        })
        .catch(() => {
            badge.innerText = 'Error';
            badge.className = 'badge badge-warning';
            out.innerText = 'Test request failed.';
        });
}

function resetTraffic() {
    fetch('/traffic/reset', { method: 'POST' })
        .then(r => r.text())
        .then(t => {
            $('diag-result').innerText = t;
            updateStatus();
        })
        .catch(() => { $('diag-result').innerText = 'Traffic reset failed'; });
}

function blockClient(mac) {
    if (!confirm('Block ' + mac + ' ?')) return;
    fetch('/block', { method: 'POST', body: new URLSearchParams({ mac: mac }) })
        .then(r => r.text().then(t => ({ ok: r.ok, text: t })))
        .then(res => {
            if (!res.ok) { alert(res.text); return; }
            updateClients();
            loadConfig(true);
        })
        .catch(e => console.error(e));
}

function scanNetworks() {
    const btn = $('scan-btn');
    btn.innerHTML = '<span class="spinner"></span> Scanning...';
    btn.disabled = true;

    fetch('/scan')
        .then(r => r.json())
        .then(networks => {
            const select = $('sta_ssid_select');
            const dl = $('ssid-list');
            select.innerHTML = '<option value="">-- Select a detected network --</option>';
            dl.innerHTML = '';

            networks.sort((a, b) => (b.rssi || -100) - (a.rssi || -100));
            networks.forEach(net => {
                const opt = document.createElement('option');
                opt.value = net.hidden ? '' : net.ssid;
                const label = net.hidden
                    ? `🔒 Hidden (${net.rssi} dBm, ch ${net.channel}, ${net.bssid})`
                    : `${net.ssid} (${net.rssi} dBm ${net.enc ? '🔒' : '🔓'}, ch ${net.channel})`;
                opt.innerText = label;
                if (net.hidden) opt.disabled = true;
                select.appendChild(opt);

                if (!net.hidden) {
                    const o2 = document.createElement('option');
                    o2.value = net.ssid;
                    dl.appendChild(o2);
                }
            });
            btn.innerText = 'Scan Networks';
            btn.disabled = false;
            $('diag-result').innerText = networks.length + ' Wi-Fi network(s) detected, including hidden entries where the SDK exposes them.';
        })
        .catch(e => {
            btn.innerText = 'Scan Networks';
            btn.disabled = false;
            $('diag-result').innerText = 'Wi-Fi scan failed.';
        });
}

function onSsidSelect(ssid) {
    if (!ssid) return;
    $('sta_ssid0').value = ssid;
    const apInput = $('ap_ssid');
    if (!apInput.value || apInput.value.endsWith('_EXT')) {
        apInput.value = ssid + '_EXT';
    }
}

function setVal(id, v) {
    const e = $(id);
    if (e && v !== undefined && v !== null) e.value = v;
}

function setChk(id, v) {
    const e = $(id);
    if (e) e.checked = !!v;
}

function toggleWanStatic(i) {
    const mode = $('sta_mode' + i);
    const box = $('sta_static_box' + i);
    if (box) box.style.display = (mode && mode.value === '1') ? 'block' : 'none';
}

function loadConfig(filterOnly) {
    return fetch('/config')
        .then(r => r.json())
        .then(c => {
            cfg = c;
            setVal('filter_mode', c.filter_mode);
            setVal('filter_list', c.filter_list);
            if (filterOnly) return c;

            for (let i = 0; i < 3; i++) {
                setVal('sta_ssid' + i, c['sta_ssid' + i]);
                const pe = $('sta_pass' + i);
                pe.value = '';
                pe.dataset.dirty = '';
                pe.placeholder = c['sta_has_pass' + i] ? 'Saved (leave blank to keep)' : 'Wi-Fi password (empty = open)';
                setVal('sta_mode' + i, c['sta_static' + i] ? '1' : '0');
                setVal('sta_ip' + i, c['sta_ip' + i] || '');
                setVal('sta_gateway' + i, c['sta_gateway' + i] || '');
                setVal('sta_subnet' + i, c['sta_subnet' + i] || '255.255.255.0');
                setVal('sta_dns' + i, c['sta_dns' + i] || '');
                toggleWanStatic(i);
            }
            setChk('mac_enable', c.mac_on);
            setVal('custom_mac', c.mac_on ? c.mac : '');

            setVal('ap_ssid', c.ap_ssid);
            const ap = $('ap_pass');
            ap.value = '';
            ap.dataset.dirty = '';
            ap.placeholder = c.ap_has_pass ? 'Saved (leave blank to keep)' : '8-63 characters (empty = open)';
            setChk('hidden', c.hidden);
            setVal('max_clients', c.max_clients);
            setVal('channel', c.channel);
            setVal('tx_power', c.tx_power);
            $('tx_val').innerText = c.tx_power + ' dBm';
            setChk('ap_mac_enable', c.ap_mac_on);
            setVal('ap_mac', c.ap_mac_on ? c.ap_mac : '');

            setVal('ap_ip', c.ap_ip);
            setVal('dhcp_start', c.dhcp_start);
            setVal('dhcp_end', c.dhcp_end);
            setVal('dns', c.dns);
            syncMac();
            return c;
        });
}

function saveConfig(e) {
    e.preventDefault();
    const saveBtn = $('save-btn');
    saveBtn.innerHTML = '<span class="spinner"></span> Saving &amp; Connecting...';
    saveBtn.disabled = true;

    const p = new URLSearchParams();
    for (let i = 0; i < 3; i++) {
        const ssid = $('sta_ssid' + i).value.trim();
        const pe = $('sta_pass' + i);
        p.set('sta_ssid' + i, ssid);
        p.set('sta_pass' + i, pe.value);
        p.set('keep' + i, (!pe.dataset.dirty && ssid === (cfg['sta_ssid' + i] || '')) ? '1' : '0');
        p.set('sta_static' + i, $('sta_mode' + i).value);
        p.set('sta_ip' + i, $('sta_ip' + i).value.trim());
        p.set('sta_gateway' + i, $('sta_gateway' + i).value.trim());
        p.set('sta_subnet' + i, $('sta_subnet' + i).value.trim());
        p.set('sta_dns' + i, $('sta_dns' + i).value.trim());
    }
    p.set('mac_on', $('mac_enable').checked ? '1' : '0');
    p.set('custom_mac', $('custom_mac').value.trim());

    p.set('ap_ssid', $('ap_ssid').value.trim());
    p.set('ap_pass', $('ap_pass').value);
    p.set('keep_ap', $('ap_pass').dataset.dirty ? '0' : '1');
    p.set('hidden', $('hidden').checked ? '1' : '0');
    p.set('max_clients', $('max_clients').value);
    p.set('channel', $('channel').value);
    p.set('tx_power', $('tx_power').value);
    p.set('ap_mac_on', $('ap_mac_enable').checked ? '1' : '0');
    p.set('ap_mac', $('ap_mac').value.trim());

    p.set('ap_ip', $('ap_ip').value.trim());
    p.set('dhcp_start', $('dhcp_start').value);
    p.set('dhcp_end', $('dhcp_end').value);
    p.set('dns', $('dns').value.trim());

    p.set('filter_mode', $('filter_mode').value);
    p.set('filter_list', $('filter_list').value);

    const newIp = $('ap_ip').value.trim();

    fetch('/save', { method: 'POST', body: p })
        .then(r => r.text().then(text => ({ ok: r.ok, text })))
        .then(res => {
            if (!res.ok) {
                showAlert(res.text || 'Error saving configuration.', 'alert-warn');
                saveBtn.innerText = 'Save & Connect';
                saveBtn.disabled = false;
                return;
            }
            showAlert('Saved. The device is restarting... If you changed the repeater IP, reconnect to the Wi-Fi and open http://' + newIp + '/', 'alert-success');
        })
        .catch(err => {
            showAlert('Configuration sent. Restarting...', 'alert-success');
        });
}

function rebootDevice() {
    if (confirm('Do you want to restart the ESP8266?')) {
        fetch('/reboot', { method: 'POST' });
        showAlert('Restarting...', 'alert-info');
    }
}

function resetConfig() {
    if (confirm('Do you want to erase all configuration and return to setup mode?')) {
        fetch('/reset', { method: 'POST' })
            .then(() => showAlert('Configuration erased. Restarting...', 'alert-info'));
    }
}

// ---- init ----
(function buildChannels() {
    const s = $('channel');
    let h = '<option value="0">Auto (follow router)</option>';
    for (let i = 1; i <= 13; i++) h += '<option value="' + i + '">Channel ' + i + '</option>';
    s.innerHTML = h;
})();

loadConfig()
    .then(c => { if (!c.configured) scanNetworks(); })
    .catch(() => scanNetworks());
updateStatus();
updateClients();
loadDiagnostics();
setInterval(updateStatus, 3000);
setInterval(updateClients, 5000);
setInterval(loadDiagnostics, 10000);
</script>
</body>
</html>
)rawliteral";
