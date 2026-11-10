@echo off
setlocal EnableExtensions DisableDelayedExpansion
title WAVES V16 OFFLINE BUILDER - EXE PLUS BIN
color 0A

rem Windows token check: independent of the LanmanServer service.
set "WAVES_BUILDER_SELF=%~f0"
set "WAVES_BUILDER_DIR=%~dp0"
set "PS_EXE=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
if exist "%SystemRoot%\Sysnative\WindowsPowerShell\v1.0\powershell.exe" set "PS_EXE=%SystemRoot%\Sysnative\WindowsPowerShell\v1.0\powershell.exe"
if not exist "%PS_EXE%" goto :MissingPowerShell
"%PS_EXE%" -NoLogo -NoProfile -Command "try { $p = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent()); if ($p.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) { exit 0 }; exit 1 } catch { Write-Host $_.Exception.Message; exit 2 }"
set "ADMIN_RESULT=%errorlevel%"
if "%ADMIN_RESULT%"=="0" goto :Build
if not "%ADMIN_RESULT%"=="1" goto :AdminCheckFailed
if /I "%~1"=="--elevated" goto :AdminCheckFailed
echo Requesting Administrator permission...
"%PS_EXE%" -NoLogo -NoProfile -Command "try { Start-Process -FilePath $env:WAVES_BUILDER_SELF -ArgumentList '--elevated' -WorkingDirectory $env:WAVES_BUILDER_DIR -Verb RunAs -ErrorAction Stop; exit 0 } catch { Write-Host $_.Exception.Message; exit 1 }"
if errorlevel 1 goto :ElevationFailed
exit /b 0

:Build
"%PS_EXE%" -NoLogo -NoProfile -Command "if ([Environment]::Is64BitOperatingSystem) { exit 0 }; exit 1"
if errorlevel 1 goto :Requires64Bit
set "WORK_DIR=%~dp0"
set "PAYLOAD=%WORK_DIR%Waves_Payload"
set "BUILD_LOG=%WORK_DIR%Waves_Build.log"
set "PF64=%ProgramW6432%"
if not defined PF64 set "PF64=%ProgramFiles%"
set "CF64=%CommonProgramW6432%"
if not defined CF64 set "CF64=%CommonProgramFiles%"
set "REG_EXE=%SystemRoot%\System32\reg.exe"
if exist "%SystemRoot%\Sysnative\reg.exe" set "REG_EXE=%SystemRoot%\Sysnative\reg.exe"
set "SOURCE_WAVES=%ProgramFiles(x86)%\Waves"
set "SOURCE_DATA=%ProgramData%\Waves Audio"
set "SOURCE_PUBLIC=%PUBLIC%\Waves Audio"
set "BUILD_STAGE=Validate source and destination paths"
"%PS_EXE%" -NoLogo -NoProfile -Command "$ErrorActionPreference='Stop'; $work=[IO.Path]::GetFullPath($env:WAVES_BUILDER_DIR).TrimEnd([char]92); foreach ($root in @($env:SOURCE_WAVES,$env:SOURCE_DATA,$env:SOURCE_PUBLIC)) { $source=[IO.Path]::GetFullPath($root).TrimEnd([char]92); if ($work.Equals($source,[StringComparison]::OrdinalIgnoreCase) -or $work.StartsWith($source+[char]92,[StringComparison]::OrdinalIgnoreCase)) { throw 'Extract this builder outside all Waves source folders.' } }"
if errorlevel 1 goto :BuildFailed
> "%BUILD_LOG%" echo Waves offline builder - collection log


echo =====================================================================
echo   BUOC 1: GOM DU LIEU WAVES V16 ^& VISUAL C++ TU MAY ANH
echo =====================================================================
if not exist "%SOURCE_WAVES%\Plug-Ins V16" (
    echo [LOI] Khong tim thay "%SOURCE_WAVES%\Plug-Ins V16" tren may!
    pause
    exit /b 1
)

set "BUILD_STAGE=Prepare payload folder"
"%PS_EXE%" -NoLogo -NoProfile -Command "$ErrorActionPreference='Stop'; if (Test-Path -LiteralPath $env:PAYLOAD) { $item=Get-Item -LiteralPath $env:PAYLOAD -Force; if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Payload folder must not be a junction or symlink.' } }"
if errorlevel 1 goto :BuildFailed
if exist "%PAYLOAD%" rmdir /s /q "%PAYLOAD%"
if exist "%PAYLOAD%" goto :BuildFailed
mkdir "%PAYLOAD%\PF_x86\Waves" 2>nul
mkdir "%PAYLOAD%\CF_x64\VST3" 2>nul
mkdir "%PAYLOAD%\CF_x86\VST3" 2>nul
mkdir "%PAYLOAD%\ProgramData\Waves Audio" 2>nul
mkdir "%PAYLOAD%\Public\Waves Audio" 2>nul
if not exist "%PAYLOAD%\Public\Waves Audio" goto :BuildFailed

:: Tu dong tim VC_redist co san trong Waves Central (neu anh chua chep vao thu muc)
if not exist "%WORK_DIR%VC_redist.x64.exe" (
    if exist "%PF64%\Waves Central\resources\res\external\VC_redist.x64.exe" (
        echo [+] Tu dong lay VC_redist.x64.exe tu thu muc Waves Central...
        copy /y "%PF64%\Waves Central\resources\res\external\VC_redist.x64.exe" "%WORK_DIR%VC_redist.x64.exe" >nul
    )
)
if not exist "%WORK_DIR%VC_redist.x86.exe" (
    if exist "%PF64%\Waves Central\resources\res\external\VC_redist.x86.exe" (
        echo [+] Tu dong lay VC_redist.x86.exe tu thu muc Waves Central...
        copy /y "%PF64%\Waves Central\resources\res\external\VC_redist.x86.exe" "%WORK_DIR%VC_redist.x86.exe" >nul
    )
)

if not exist "%WORK_DIR%VC_redist.x64.exe" (
    echo [LOI] Dat VC_redist.x64.exe canh BAT de dong goi cho may moi.
    pause
    exit /b 1
)
if not exist "%WORK_DIR%VC_redist.x86.exe" echo [INFO] VC_redist.x86.exe khong co; runtime x86 se khong duoc dong goi.

echo [1/5] Dang gom C:\Program Files (x86)\Waves ...
set "BUILD_STAGE=Copy Waves program files"
robocopy "%SOURCE_WAVES%" "%PAYLOAD%\PF_x86\Waves" /E /COPY:DAT /DCOPY:DAT /R:2 /W:1 /XJ /NP /LOG+:"%BUILD_LOG%"
if errorlevel 8 goto :BuildFailed
if exist "%PAYLOAD%\PF_x86\Waves\Waves Central" rmdir /s /q "%PAYLOAD%\PF_x86\Waves\Waves Central"

echo [2/5] Dang gom WaveShell VST3 (64-bit va 32-bit)...
set "SHELL_SOURCE=%CF64%\VST3"
set "SHELL_DEST=%PAYLOAD%\CF_x64\VST3"
set "BUILD_STAGE=Copy complete x64 WaveShell files or bundles"
"%PS_EXE%" -NoLogo -NoProfile -Command "$ErrorActionPreference='Stop'; $items=@(Get-ChildItem -LiteralPath $env:SHELL_SOURCE -Filter 'WaveShell*16*.vst3' -Force); if ($items.Count -eq 0) { throw 'No x64 WaveShell V16 found.' }; foreach ($item in $items) { if ($item.PSIsContainer) { $modules=@(Get-ChildItem -LiteralPath $item.FullName -Recurse -File | Where-Object { $_.Length -gt 0 -and ($_.Extension -eq '.vst3' -or $_.Extension -eq '.dll') }); if ($modules.Count -eq 0) { throw 'WaveShell bundle has no native module.' } } elseif ($item.Length -eq 0) { throw 'WaveShell file is empty.' }; Copy-Item -LiteralPath $item.FullName -Destination $env:SHELL_DEST -Recurse -Force }" >> "%BUILD_LOG%" 2>&1
if errorlevel 1 goto :BuildFailed
set "SHELL_SOURCE=%CommonProgramFiles(x86)%\VST3"
set "SHELL_DEST=%PAYLOAD%\CF_x86\VST3"
set "BUILD_STAGE=Copy optional x86 WaveShell files or bundles"
"%PS_EXE%" -NoLogo -NoProfile -Command "$ErrorActionPreference='Stop'; if (Test-Path -LiteralPath $env:SHELL_SOURCE) { foreach ($item in @(Get-ChildItem -LiteralPath $env:SHELL_SOURCE -Filter 'WaveShell*16*.vst3' -Force)) { Copy-Item -LiteralPath $item.FullName -Destination $env:SHELL_DEST -Recurse -Force } }" >> "%BUILD_LOG%" 2>&1
if errorlevel 1 goto :BuildFailed

echo [3/5] Dang gom C:\ProgramData\Waves Audio (bo qua cache va license cu)...
set "BUILD_STAGE=Copy Waves shared data"
if not exist "%SOURCE_DATA%" goto :BuildFailed
robocopy "%SOURCE_DATA%" "%PAYLOAD%\ProgramData\Waves Audio" /E /COPY:DAT /DCOPY:DAT /R:2 /W:1 /XJ /NP /LOG+:"%BUILD_LOG%"
if errorlevel 8 goto :BuildFailed
if exist "%PAYLOAD%\ProgramData\Waves Audio\Waves Central" rmdir /s /q "%PAYLOAD%\ProgramData\Waves Audio\Waves Central"
if exist "%PAYLOAD%\ProgramData\Waves Audio\Central" rmdir /s /q "%PAYLOAD%\ProgramData\Waves Audio\Central"
del /a /f /q "%PAYLOAD%\ProgramData\Waves Audio\Licenses\*.wle" >nul 2>&1
if exist "%PAYLOAD%\ProgramData\Waves Audio\Licenses\*.wle" goto :BuildFailed

echo [4/5] Dang gom Public Waves Audio (neu co)...
set "BUILD_STAGE=Copy optional public Waves data"
if exist "%SOURCE_PUBLIC%" (
    robocopy "%SOURCE_PUBLIC%" "%PAYLOAD%\Public\Waves Audio" /E /COPY:DAT /DCOPY:DAT /R:2 /W:1 /XJ /NP /LOG+:"%BUILD_LOG%"
    if errorlevel 8 goto :BuildFailed
)

echo [5/5] Dang xuat khoa Registry Waves V16...
"%REG_EXE%" export "HKLM\SOFTWARE\WOW6432Node\Waves Audio" "%PAYLOAD%\Waves_WOW64.reg" /y >> "%BUILD_LOG%" 2>&1
if errorlevel 1 echo [INFO] WOW64 Waves registry key not exported.
"%REG_EXE%" export "HKLM\SOFTWARE\Waves Audio" "%PAYLOAD%\Waves_x64.reg" /y >> "%BUILD_LOG%" 2>&1
if errorlevel 1 echo [INFO] x64 Waves registry key not exported.

echo.
echo =====================================================================
echo   BUOC 2: TAO KICH BAN BO CAI GIAO DIEN NEXT -^> INSTALL (.ISS)
echo =====================================================================
set "ISS_FILE=%WORK_DIR%Waves_V16_Pro.iss"
:ChooseOutput
set "BUILD_REL=Output\Build_%RANDOM%_%RANDOM%"
set "OUTPUT_DIR=%WORK_DIR%%BUILD_REL%"
if exist "%OUTPUT_DIR%" goto :ChooseOutput
mkdir "%OUTPUT_DIR%"
if not exist "%OUTPUT_DIR%" goto :BuildFailed


echo [Setup] > "%ISS_FILE%"
echo AppId={{8F9E7D6C-5B4A-4321-9876-WAVESV16OFFLINE} >> "%ISS_FILE%"
echo AppName=Waves V16 Offline Custom Package >> "%ISS_FILE%"
echo AppVersion=16.0 >> "%ISS_FILE%"
echo AppPublisher=Waves Audio Offline Pack >> "%ISS_FILE%"
echo DefaultDirName={pf32}\Waves >> "%ISS_FILE%"
echo DisableDirPage=yes >> "%ISS_FILE%"
echo DisableProgramGroupPage=yes >> "%ISS_FILE%"
echo OutputDir=%BUILD_REL% >> "%ISS_FILE%"
echo OutputBaseFilename=Setup_Waves_V16_Offline_Pro >> "%ISS_FILE%"
echo Compression=lzma2/fast >> "%ISS_FILE%"
echo SolidCompression=no >> "%ISS_FILE%"
echo DiskSpanning=yes >> "%ISS_FILE%"
echo DiskSliceSize=2100000000 >> "%ISS_FILE%"
echo SlicesPerDisk=1 >> "%ISS_FILE%"
echo SetupLogging=yes >> "%ISS_FILE%"
echo WizardStyle=modern >> "%ISS_FILE%"
echo PrivilegesRequired=admin >> "%ISS_FILE%"
echo ArchitecturesAllowed=x64 >> "%ISS_FILE%"
echo ArchitecturesInstallIn64BitMode=x64 >> "%ISS_FILE%"
echo. >> "%ISS_FILE%"
echo [DIRS] >> "%ISS_FILE%"
echo Name: "{userappdata}\Waves Audio\Preferences" >> "%ISS_FILE%"
echo Name: "{userappdata}\Waves Audio\Caches" >> "%ISS_FILE%"
echo Name: "{localappdata}\Waves Audio" >> "%ISS_FILE%"
echo Name: "{commonappdata}\Waves Audio\Licenses" >> "%ISS_FILE%"
echo. >> "%ISS_FILE%"
echo [InstallDelete] >> "%ISS_FILE%"
echo Type: files; Name: "{commonappdata}\Waves Audio\Licenses\*.wle" >> "%ISS_FILE%"
echo. >> "%ISS_FILE%"
echo [Files] >> "%ISS_FILE%"
echo Source: "Waves_Payload\PF_x86\Waves\*"; DestDir: "{pf32}\Waves"; Flags: ignoreversion recursesubdirs createallsubdirs >> "%ISS_FILE%"
echo Source: "Waves_Payload\CF_x64\VST3\*"; DestDir: "{cf64}\VST3"; Flags: ignoreversion recursesubdirs createallsubdirs >> "%ISS_FILE%"
echo Source: "Waves_Payload\CF_x86\VST3\*"; DestDir: "{cf32}\VST3"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist >> "%ISS_FILE%"
echo Source: "Waves_Payload\ProgramData\Waves Audio\*"; DestDir: "{commonappdata}\Waves Audio"; Flags: ignoreversion recursesubdirs createallsubdirs >> "%ISS_FILE%"
echo Source: "Waves_Payload\Public\Waves Audio\*"; DestDir: "{commondocs}\..\Waves Audio"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist >> "%ISS_FILE%"
echo Source: "Waves_Payload\Waves_WOW64.reg"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall skipifsourcedoesntexist >> "%ISS_FILE%"
echo Source: "Waves_Payload\Waves_x64.reg"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall skipifsourcedoesntexist >> "%ISS_FILE%"
echo Source: "VC_redist.x64.exe"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall >> "%ISS_FILE%"
echo Source: "VC_redist.x86.exe"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall skipifsourcedoesntexist >> "%ISS_FILE%"
echo Source: "Patch.exe"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall skipifsourcedoesntexist >> "%ISS_FILE%"
echo. >> "%ISS_FILE%"
echo [Run] >> "%ISS_FILE%"
echo Filename: "{tmp}\VC_redist.x64.exe"; Parameters: "/install /quiet /norestart"; Flags: waituntilterminated; StatusMsg: "Dang cai ngam thu vien Microsoft Visual C++ (x64)..."; Check: FileExists(ExpandConstant('{tmp}\VC_redist.x64.exe')) >> "%ISS_FILE%"
echo Filename: "{tmp}\VC_redist.x86.exe"; Parameters: "/install /quiet /norestart"; Flags: waituntilterminated; StatusMsg: "Dang cai ngam thu vien Microsoft Visual C++ (x86)..."; Check: FileExists(ExpandConstant('{tmp}\VC_redist.x86.exe')) >> "%ISS_FILE%"
echo Filename: "{sys}\reg.exe"; Parameters: "import ""{tmp}\Waves_WOW64.reg"""; Flags: runhidden waituntilterminated; StatusMsg: "Dang dang ky Registry Waves V16 (32-bit node)..."; Check: FileExists(ExpandConstant('{tmp}\Waves_WOW64.reg')) >> "%ISS_FILE%"
echo Filename: "{sys}\reg.exe"; Parameters: "import ""{tmp}\Waves_x64.reg"""; Flags: runhidden waituntilterminated; StatusMsg: "Dang dang ky Registry Waves V16 (64-bit node)..."; Check: FileExists(ExpandConstant('{tmp}\Waves_x64.reg')) >> "%ISS_FILE%"
echo Filename: "{tmp}\Patch.exe"; Parameters: "/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /SP-"; Flags: waituntilterminated; StatusMsg: "Dang chay cong cu Patch.exe duoc kem theo..."; Check: FileExists(ExpandConstant('{tmp}\Patch.exe')) >> "%ISS_FILE%"

echo. >> "%ISS_FILE%"
echo [Code] >> "%ISS_FILE%"
echo function PrepareToInstall(var NeedsRestart: Boolean): String; >> "%ISS_FILE%"
echo var ResultCode: Integer; >> "%ISS_FILE%"
echo begin >> "%ISS_FILE%"
echo   Result := ''; >> "%ISS_FILE%"
echo   Exec(ExpandConstant('{sys}\taskkill.exe'), '/F /IM WavesLocalServer.exe /T', '', SW_HIDE, ewWaitUntilTerminated, ResultCode); >> "%ISS_FILE%"
echo   Log('WavesLocalServer stop attempted before copying files.'); >> "%ISS_FILE%"
echo end; >> "%ISS_FILE%"

echo.
echo =====================================================================
echo   BUOC 3: BIEN DICH RA FILE SETUP_WAVES_V16_OFFLINE_PRO.EXE
echo =====================================================================
set "ISCC="
if exist "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" set "ISCC=%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe"
if exist "%PF64%\Inno Setup 6\ISCC.exe" set "ISCC=%PF64%\Inno Setup 6\ISCC.exe"
if not defined ISCC goto :MissingCompiler

set "BUILD_STAGE=Compile installer with Inno Setup"
echo Dang bien dich EXE nho va cac file BIN...
"%ISCC%" "%ISS_FILE%"
set "COMPILE_RESULT=%errorlevel%"
if not "%COMPILE_RESULT%"=="0" goto :BuildFailed
if not exist "%OUTPUT_DIR%\Setup_Waves_V16_Offline_Pro.exe" goto :BuildFailed
dir /b /a-d "%OUTPUT_DIR%\Setup_Waves_V16_Offline_Pro-*.bin" >nul 2>&1
if errorlevel 1 goto :BuildFailed

echo.
echo [HOAN TAT] Bo cai nam trong:
echo "%OUTPUT_DIR%"
echo Phai mang CA file EXE VA tat ca file BIN trong cung thu muc.
echo May dich khong can Inno Setup hoac Waves Central de chay bo cai.
start "" "%OUTPUT_DIR%"
pause
exit /b 0

:MissingCompiler
echo [LOI] Khong tim thay Inno Setup 6. Chua tao bo cai moi.
echo Cai Inno Setup 6 tren may dong goi roi chay lai BAT.
echo ISS da tao: "%ISS_FILE%"
pause
exit /b 1

:BuildFailed
echo.
echo [LOI] Dung tai buoc: %BUILD_STAGE%
echo Chua xac nhan tao thanh cong bo cai moi.
echo Xem chi tiet trong cua so nay va log: "%BUILD_LOG%"
pause
exit /b 1

:MissingPowerShell
echo [LOI] Khong tim thay Windows PowerShell.
pause
exit /b 1

:AdminCheckFailed
echo [LOI] Khong xac nhan duoc token Administrator. Khong lap lai UAC.
echo Thu bam chuot phai BAT va chon Run as administrator.
pause
exit /b 1

:ElevationFailed
echo [LOI] UAC da bi huy hoac khong the mo cua so Administrator.
pause
exit /b 1

:Requires64Bit
echo [LOI] Bo dong goi nay can Windows 64-bit.
pause
exit /b 1
