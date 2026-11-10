@echo off
setlocal EnableExtensions DisableDelayedExpansion
title WAVES - THU THAP TRANG THAI DANG CHAY
set "PS_EXE=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
if exist "%SystemRoot%\Sysnative\WindowsPowerShell\v1.0\powershell.exe" set "PS_EXE=%SystemRoot%\Sysnative\WindowsPowerShell\v1.0\powershell.exe"
if not exist "%PS_EXE%" goto :Missing
if not exist "%~dp0Thu_Thap_Waves_Dang_Chay.ps1" goto :Missing
echo Mo DAW va giu mot plugin Waves dang mo, roi de cua so nay chay.
echo Cong cu chi doc thong tin va tao bao cao ZIP, khong can Waves Central.
"%PS_EXE%" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0Thu_Thap_Waves_Dang_Chay.ps1"
set "WAVES_REPORT_RESULT=%errorlevel%"
echo.
if not "%WAVES_REPORT_RESULT%"=="0" goto :Failed
echo Gui file KetQua_Waves_*.zip nam canh BAT nay.
pause
exit /b 0
:Missing
echo Giai nen ca ZIP truoc khi chay. Can Windows PowerShell co san trong Windows.
pause
exit /b 1
:Failed
echo Chua thu thap thanh cong. Giu lai dong loi ben tren.
pause
exit /b 1
