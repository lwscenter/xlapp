@echo off
set boot_in=%1%
set app_in=%2%
set app_out=%3%

echo: boot:%boot_in%, app: %app_in% out: %app_out%

set var=":00000001FF"
findstr /v %var% %boot_in% > _boot.hex
copy /b _boot.hex+%app_in% %app_out%
del _boot.hex
echo.
