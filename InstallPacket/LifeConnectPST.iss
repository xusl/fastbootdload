; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{89B6FB07-6797-4204-8855-80DD85E95B86}
AppName=LifeConnectPST
AppVersion=1.5.0.0
;AppVerName=LifeConnectPST 1.3
AppPublisher=JRD (Shenzhen), Inc.
AppPublisherURL=www.jrdcom.com
AppSupportURL=www.jrdcom.com
AppUpdatesURL=www.jrdcom.com
DefaultDirName={pf}\LifeConnectPST
DefaultGroupName=LifeConnectPST
;OutputDir= .\
OutputBaseFilename=LifeConnectPST-setup
SetupIconFile=..\OpenWrtProgrammer\LifeConnectFirmwareDownload\res\LifeConnectFirmwareDownload.ico
Compression=lzma
SolidCompression=yes
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
; On all other architectures it will install in "32-bit mode".
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\OpenWrtProgrammer\x64\Release\LifeConnectPST_x64_Release.exe"; DestDir: "{app}"; Flags: ignoreversion; DestName: "LifeConnectPST.exe"; Check: Is64BitInstallMode
Source: "..\OpenWrtProgrammer\Release\LifeConnectPST_Win32_Release.exe"; DestDir: "{app}"; Flags: ignoreversion; DestName: "LifeConnectPST.exe"; Check: not Is64BitInstallMode
Source: "..\OpenWrtProgrammer\LifeConnectFirmwareDownload\Config.ini"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\LifeConnectPST"; Filename: "{app}\LifeConnectPST.exe"
Name: "{commondesktop}\LifeConnectPST"; Filename: "{app}\LifeConnectPST.exe"; Tasks: desktopicon

;remove , for x64 platform work inproperly.
;[Run]
;Filename: "{app}\LifeConnectPST.exe"; Description: "{cm:LaunchProgram,LifeConnectPST}"; Flags: nowait postinstall skipifsilent

