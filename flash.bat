@echo off
setlocal
set PORT=%1
if "%PORT%"=="" set PORT=COM3
set BIN_PATH=.pio\build\esp12e\firmware.bin

echo ==================================================
echo   Building ^& Flashing ESP8266 Turbo Wi-Fi Repeater
echo ==================================================
echo Target port: %PORT% (Usage: flash.bat COMx)

where pio >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [!] PlatformIO ^(pio^) not found in PATH.
    echo     Install it first: pip install platformio
    pause
    exit /b 1
)

echo [*] Building firmware...
pio run
if %ERRORLEVEL% NEQ 0 (
    echo [!] Build failed.
    pause
    exit /b 1
)

echo [*] Flashing %BIN_PATH% ...
python -m esptool --port %PORT% --baud 460800 write_flash --flash_size 4MB --flash_freq 80m --flash_mode dio 0x00000 %BIN_PATH%

if %ERRORLEVEL% EQU 0 (
    echo ==================================================
    echo [+] Flashing completed successfully!
    echo [+] Connect to Wi-Fi SSID 'ESP8266-Repeater-Setup' and open http://192.168.4.1
    echo ==================================================
) else (
    echo [!] Flashing failed. Please check COM port and cable connection.
)
pause
