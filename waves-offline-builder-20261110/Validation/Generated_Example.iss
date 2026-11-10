[Setup]
AppId={{8F9E7D6C-5B4A-4321-9876-WAVESV16OFFLINE}
AppName=Waves V16 Offline Custom Package
AppVersion=16.0
AppPublisher=Waves Audio Offline Pack
DefaultDirName={pf32}\Waves
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir=Output\Build_QA
OutputBaseFilename=Setup_Waves_V16_Offline_Pro
Compression=lzma2/fast
SolidCompression=no
DiskSpanning=yes
DiskSliceSize=2100000000
SlicesPerDisk=1
SetupLogging=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[DIRS]
Name: "{userappdata}\Waves Audio\Preferences"
Name: "{userappdata}\Waves Audio\Caches"
Name: "{localappdata}\Waves Audio"
Name: "{commonappdata}\Waves Audio\Licenses"

[InstallDelete]
Type: files; Name: "{commonappdata}\Waves Audio\Licenses\*.wle"

[Files]
Source: "Waves_Payload\PF_x86\Waves\*"; DestDir: "{pf32}\Waves"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "Waves_Payload\CF_x64\VST3\*"; DestDir: "{cf64}\VST3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "Waves_Payload\CF_x86\VST3\*"; DestDir: "{cf32}\VST3"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "Waves_Payload\ProgramData\Waves Audio\*"; DestDir: "{commonappdata}\Waves Audio"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "Waves_Payload\Public\Waves Audio\*"; DestDir: "{commondocs}\..\Waves Audio"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "Waves_Payload\Waves_WOW64.reg"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall skipifsourcedoesntexist
Source: "Waves_Payload\Waves_x64.reg"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall skipifsourcedoesntexist
Source: "VC_redist.x64.exe"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall
Source: "VC_redist.x86.exe"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall skipifsourcedoesntexist
Source: "Patch.exe"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall skipifsourcedoesntexist

[Run]
Filename: "{tmp}\VC_redist.x64.exe"; Parameters: "/install /quiet /norestart"; Flags: waituntilterminated; StatusMsg: "Dang cai ngam thu vien Microsoft Visual C++ (x64)..."; Check: FileExists(ExpandConstant('{tmp}\VC_redist.x64.exe'))
Filename: "{tmp}\VC_redist.x86.exe"; Parameters: "/install /quiet /norestart"; Flags: waituntilterminated; StatusMsg: "Dang cai ngam thu vien Microsoft Visual C++ (x86)..."; Check: FileExists(ExpandConstant('{tmp}\VC_redist.x86.exe'))
Filename: "{sys}\reg.exe"; Parameters: "import ""{tmp}\Waves_WOW64.reg"""; Flags: runhidden waituntilterminated; StatusMsg: "Dang dang ky Registry Waves V16 (32-bit node)..."; Check: FileExists(ExpandConstant('{tmp}\Waves_WOW64.reg'))
Filename: "{sys}\reg.exe"; Parameters: "import ""{tmp}\Waves_x64.reg"""; Flags: runhidden waituntilterminated; StatusMsg: "Dang dang ky Registry Waves V16 (64-bit node)..."; Check: FileExists(ExpandConstant('{tmp}\Waves_x64.reg'))
Filename: "{tmp}\Patch.exe"; Parameters: "/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /SP-"; Flags: waituntilterminated; StatusMsg: "Dang chay cong cu Patch.exe duoc kem theo..."; Check: FileExists(ExpandConstant('{tmp}\Patch.exe'))

[Code]
function PrepareToInstall(var NeedsRestart: Boolean): String;
var ResultCode: Integer;
begin
  Result := '';
  Exec(ExpandConstant('{sys}\taskkill.exe'), '/F /IM WavesLocalServer.exe /T', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Log('WavesLocalServer stop attempted before copying files.');
end;
