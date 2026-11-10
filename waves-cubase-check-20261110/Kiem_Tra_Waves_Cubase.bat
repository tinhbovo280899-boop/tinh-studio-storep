@echo off
setlocal EnableExtensions DisableDelayedExpansion
title KIEM TRA WAVES TRONG CUBASE
set "PS_EXE=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
if exist "%SystemRoot%\Sysnative\WindowsPowerShell\v1.0\powershell.exe" set "PS_EXE=%SystemRoot%\Sysnative\WindowsPowerShell\v1.0\powershell.exe"
if not exist "%PS_EXE%" goto :MissingPowerShell
if not exist "%~dp0Kiem_Tra_Waves_Cubase.ps1" goto :MissingScript
echo Chay tren may Cubase khong thay Waves, bang tai khoan anh dung de mo Cubase.
echo Khong can Run as administrator.
"%PS_EXE%" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Kiem_Tra_Waves_Cubase.ps1"
set "DIAG_RESULT=%errorlevel%"
echo.
if not "%DIAG_RESULT%"=="0" goto :Failed
echo Da xong. File KetQua_Waves_Cubase_*.txt nam canh file BAT nay.
pause
exit /b 0
:Failed
echo Chua tao thanh cong ket qua. Giu lai dong loi ben tren.
pause
exit /b 1
:MissingPowerShell
echo Khong tim thay Windows PowerShell.
pause
exit /b 1
:MissingScript
echo Giai nen ca ZIP vao mot thu muc truoc khi chay BAT.
pause
exit /b 1
