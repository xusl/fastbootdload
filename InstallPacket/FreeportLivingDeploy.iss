; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{2B410A37-D183-4496-AB6C-C0DC16D9E53F}
AppName=Freeport Living Deploy
AppVersion=0.0.2
;AppVerName=Freeport Living Deploy 0.0.2
AppPublisher=TCL Communication Technology Holdings Limited
AppPublisherURL=http://www.tcl.com/
AppSupportURL=http://www.tcl.com/
AppUpdatesURL=http://www.tcl.com/
DefaultDirName={pf}\Freeport Living Deploy
DefaultGroupName=Freeport Living Deploy
OutputDir= .\
OutputBaseFilename=FreeportLivingDeploy_setup_V0.0.2
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "..\Release\FreeportLiveDeploy.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\FreeportLiveDeploy\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\config\adb_usb.ini"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\AdbWinApiEx.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\AdbWinUsbApiEx.dll"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Freeport Living Deploy"; Filename: "{app}\FreeportLiveDeploy.exe"
Name: "{group}\{cm:UninstallProgram,Freeport Living Deploy}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Freeport Living Deploy"; Filename: "{app}\FreeportLiveDeploy.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Freeport Living Deploy"; Filename: "{app}\FreeportLiveDeploy.exe"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\FreeportLiveDeploy.exe"; Description: "{cm:LaunchProgram,Freeport Living Deploy}"; Flags: nowait postinstall skipifsilent

