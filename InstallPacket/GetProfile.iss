; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{5767B7E4-F978-4FE1-87F2-10D63A851DAB}
AppName=GetProfile
AppVersion=1.0
;AppVerName=GetProfile 1.0
AppPublisher=TCL Communication Technology Holding Ltd.
AppPublisherURL=http://www.tcl.com/
AppSupportURL=http://www.tcl.com/
AppUpdatesURL=http://www.tcl.com/
DefaultDirName={pf}\GetProfile
DefaultGroupName=GetProfile
OutputDir= .\
OutputBaseFilename=GetProfile_setup
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\Release\GetProfile.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\config\adb_usb.ini"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\AdbWinApiEx.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Release\AdbWinUsbApiEx.dll"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\GetProfile"; Filename: "{app}\GetProfile.exe"
Name: "{commondesktop}\GetProfile"; Filename: "{app}\GetProfile.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\GetProfile.exe"; Description: "{cm:LaunchProgram,GetProfile}"; Flags: nowait postinstall skipifsilent

