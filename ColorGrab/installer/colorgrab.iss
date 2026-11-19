; Inno Setup script for ColorGrab
#include "version.iss"

[Setup]
AppID={{F98E5074-CB71-4BCD-9ED9-BB1B1A882735}}
AppName=ColorGrab
AppVersion={#AppVersionStr}
AppVerName=ColorGrab {#AppVersionStr}
AppPublisher=Larry Panganiban
AppPublisherURL=https://github.com/larry-panganiban/ColorGrab
AppSupportURL=https://github.com/larry-panganiban/ColorGrab/issues
AppUpdatesURL=https://github.com/larry-panganiban/ColorGrab/releases
DefaultDirName={commonpf32}\ColorGrab
DefaultGroupName=ColorGrab
LicenseFile=..\..\LICENSE.txt
InfoBeforeFile=..\ReadMe.txt
OutputDir=..\Output
OutputBaseFilename=colorgrab-setup
SetupIconFile=..\Res\idr_main.ico
Compression=lzma
SolidCompression=yes
VersionInfoVersion={#AppVersionStr}
DisableDirPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; Flags: unchecked

[Files]
Source: "..\ReadMe.txt"; DestDir: "{app}"
Source: "..\Release\ColorGrab.exe"; DestDir: "{app}"
Source: "..\packaging\ColorGrab.chm"; DestDir: "{app}"
Source: "..\..\LICENSE.txt"; DestDir: "{app}"

; --- VC++ Runtime bundling ---
; Place vc_redist.x86.exe next to this .iss file (in /installer)
Source: "vc_redist.x86.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall

[INI]
Filename: "{app}\ColorGrab.url"; Section: "InternetShortcut"; Key: "URL"; String: "https://github.com/larry-panganiban/ColorGrab"

[Icons]
Name: "{group}\ColorGrab"; Filename: "{app}\ColorGrab.exe"
Name: "{group}\Website"; Filename: "{app}\ColorGrab.url"
Name: "{commondesktop}\ColorGrab"; Filename: "{app}\ColorGrab.exe"; Tasks: desktopicon

[Run]
; Install VC++ runtime if needed
Filename: "{tmp}\vc_redist.x86.exe"; \
    Parameters: "/install /quiet /norestart"; \
    StatusMsg: "Installing Microsoft Visual C++ Runtime..."; \
    Check: NeedsVC2015Runtime

Filename: "{app}\ColorGrab.exe"; Description: "{cm:LaunchProgram,ColorGrab}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{app}\ColorGrab.url"

[Code]
function NeedsVC2015Runtime: Boolean;
var
  installed: Cardinal;
begin
  // Microsoft’s official detection key for the 2015–2022 VC++ runtime
  Result := not RegQueryDWordValue(
    HKLM,
    'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86',
    'Installed',
    installed
  ) or (installed <> 1);
end;
