#!/usr/bin/env bash
set -e

PORT=${1:-/dev/ttyUSB0}
BIN_PATH=".pio/build/esp12e/firmware.bin"

echo "=================================================="
echo "  Building & Flashing ESP8266 Turbo Wi-Fi Repeater "
echo "=================================================="
echo "Target port: $PORT"

if ! command -v pio &> /dev/null; then
    echo "[!] PlatformIO (pio) not found in PATH."
    echo "    Install it first: pip install platformio"
    exit 1
fi

echo "[*] Building firmware..."
pio run

if ! command -v esptool &> /dev/null && ! command -v esptool.py &> /dev/null; then
    echo "[!] esptool not found in PATH. Trying python module..."
    PYTHON_CMD="python3 -m esptool"
else
    PYTHON_CMD="esptool"
fi

echo "[*] Flashing $BIN_PATH ..."
$PYTHON_CMD --port "$PORT" --baud 460800 write_flash --flash_size 4MB --flash_freq 80m --flash_mode dio 0x00000 "$BIN_PATH"

echo "=================================================="
echo "[+] Flashing completed successfully!"
echo "[+] Connect to Wi-Fi SSID 'ESP8266-Repeater-Setup' and navigate to http://192.168.4.1"
echo "=================================================="
