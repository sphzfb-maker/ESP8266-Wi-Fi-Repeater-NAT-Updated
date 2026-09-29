# Changelog

## v1.1.0
- Added hidden-AP scanning using the ESP8266 scan API's hidden-network option.
- Added scan BSSID, channel and hidden/security metadata.
- Added RSSI-sorted scan results in the mobile UI.
- Added live diagnostics for WAN gateway/DNS, RAM, sketch space, flash, CPU/core, reset reason and upstream slot.
- Added manual WAN reconnect action.
- Added lightweight TCP internet reachability test.
- Added traffic-counter reset action.
- Explicitly selected the 4 MB flash linker layout.
- Preserved existing WISP/NAPT, failover, zero-sleep, channel-locking, MAC-filter and client-management features.

## v1.2.0 - WAN IP modes + client names
- Added per-upstream WAN IP mode: DHCP or Static IP.
- Static WAN fields: IP address, gateway, subnet mask, and DNS.
- Static/DHCP settings are stored independently for primary and backup networks and applied during failover.
- Added backward-compatible migration from the v1.1.0 EEPROM configuration layout.
- Connected-client list now shows DHCP-provided client hostnames when available, with a deterministic MAC-based fallback label when a client does not provide a hostname.
- Added DHCP hostname snooping on the SoftAP side; this is best-effort and does not require internet access.

## v1.2.1 - review fixes
- Fixed DHCP hostname snooping: the DHCP magic-cookie check was wrong (client names were never detected). It now also handles Ethernet-framed packets and chained pbufs, and parses from a private copy to avoid a buffer race.
- Client hostnames are filtered to safe characters on the device and HTML-escaped in the web UI (prevents script injection by a malicious client hostname).
- Fixed the WAN status showing "(DHCP)" while a static IP was active (`wan_static` was missing from /status).
- Fixed subnet-mask handling: masks such as 255.255.0.0 were silently rewritten to 255.255.255.0 by the config sanitizer.
- Same-subnet conflict warning now uses the real WAN netmask instead of a hard-coded /24.
- Static WAN validation: contiguous netmask check, gateway != IP, unicast checks, DNS optional (defaults to the gateway), and unused upstream slots can no longer block saving.
- Legacy config migration now uses bounded string copies.
- Version strings unified (firmware banner, web UI title, README, NOTE.txt).
