@echo off
setlocal
REM ============================================================
REM Attach to a running/frozen STM32F103 via ST-Link and dump
REM the CPU state. Run this AFTER the board freezes.
REM
REM Usage:
REM   1. Reproduce the freeze (motor stuck, LED off, serial mute).
REM   2. Leave the board powered.
REM   3. Double-click this .bat or run from cmd.
REM   4. Read the report printed to console; save output for me.
REM
REM Requires: PlatformIO installed at %USERPROFILE%\.platformio
REM ============================================================

set PROJDIR=%~dp0..
set OPENOCD=%USERPROFILE%\.platformio\packages\tool-openocd\bin\openocd.exe
set OPENOCD_SCRIPTS=%USERPROFILE%\.platformio\packages\tool-openocd\openocd\scripts
set GDB=%USERPROFILE%\.platformio\packages\toolchain-gccarmnoneeabi\bin\arm-none-eabi-gdb.exe
set ELF=%PROJDIR%\.pio\build\TWO_AXIS_VARIANT\firmware.elf

if not exist "%OPENOCD%" (
    echo ERROR: OpenOCD not found at %OPENOCD%
    echo Install PlatformIO first, or edit this script with the correct path.
    pause
    exit /b 1
)
if not exist "%ELF%" (
    echo ERROR: firmware.elf not found at %ELF%
    echo Build first with: pio run -e TWO_AXIS_VARIANT
    pause
    exit /b 1
)

echo Starting OpenOCD server in background...
start /B "" "%OPENOCD%" -s "%OPENOCD_SCRIPTS%" -f interface/stlink.cfg -f target/stm32f1x.cfg -c "gdb_port 3333" -c "reset_config none separate"

echo Waiting 2 seconds for OpenOCD to come up...
timeout /t 2 /nobreak >nul

echo.
echo Launching GDB and dumping state...
echo.
"%GDB%" -q -batch -x "%~dp0dump_state.gdb" "%ELF%"

echo.
echo Killing OpenOCD server...
taskkill /F /IM openocd.exe >nul 2>&1

echo.
echo Done. Copy the output above and send it back for analysis.
pause
