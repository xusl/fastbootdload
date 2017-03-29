;===========================================================
;
; EmbedDownload Setup
;
; Compiler: Inno Setup V5.4.2
; Creator : sunshp
; time    : 2013/08/09
;
;===========================================================
#define AppVersion "1.0.0.8"
#define AppVersionNum 8
#define ReleaseDateTimeString GetDateTimeString('dd/mm/yyyy hh:nn:ss', '-', ':');
[Setup]
AppName=TPST-CPE-{#AppVersion}
AppVerName={#AppVersion}        
AppPublisher=JRD (Shenzhen), Inc.
AppPublisherURL=www.jrdcom.com
AppSupportURL=www.jrdcom.com
AppUpdatesURL=www.jrdcom.com
VersionInfoCompany=TCL Communication Technology Holdings Limited
VersionInfoCopyright=Copyright (C) 2006-2015 TCL Communication Technology Holdings Limited
VersionInfoDescription=Dedicate tool for update Life Connect, supported by SCD Connected Object
VersionInfoVersion={#AppVersion}
DefaultDirName={pf}\JRD\TPST-CPE
DefaultGroupName=JRD\TPST-CPE
DisableProgramGroupPage=yes
OutputDir= .\
OutputBaseFilename=TPST-CPE-{#AppVersion}-Setup
SetupIconFile=..\mdmfastboot\res\mdmfastboot.ico 
WizardImageFile=.\icons\orange.bmp
WizardSmallImageFile=.\icons\orange_s.bmp
Compression=lzma
SolidCompression=yes
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
; On all other architectures it will install in "32-bit mode".
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin

[Languages]
Name: "En"; MessagesFile: "Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"


[Files]
Source: "..\x64\Release\mdmfastboot.exe"; DestDir: "{app}"; Flags: ignoreversion; DestName: "TPST-CPE.exe"; Check: Is64BitInstallMode
Source: "..\x64\Release\*.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "..\Win32\Release\mdmfastboot.exe"; DestDir: "{app}"; Flags: ignoreversion; DestName: "TPST-CPE.exe"; Check: not Is64BitInstallMode
Source: "..\Win32\Release\*.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "..\mdmfastboot\configs\*"; DestDir: "{app}";Flags: ignoreversion allowunsafefiles; Permissions:everyone-full
Source: "..\mdmfastboot\res\mdmfastboot.ico"; DestDir: "{app}";Flags: ignoreversion allowunsafefiles;DestName: "Icon.ico";Permissions:everyone-full

[Icons]
Name: "{group}\TPST-CPE"; IconFilename: {app}\Icon.ico;Filename: "{app}\TPST-CPE.exe"; WorkingDir: "{app}"
Name: "{group}\{cm:UninstallProgram,TPST-CPE}"; IconFilename: {app}\Icon.ico;Filename: "{uninstallexe}"
Name: "{userdesktop}\TPST-CPE"; Filename: "{app}\TPST-CPE.exe"; IconFilename: {app}\Icon.ico;Tasks: desktopicon

[Run]
Filename: "{app}\TPST-CPE.exe"; Description: "{cm:LaunchProgram,TPST-CPE}"; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKLM; Subkey: "Software\JRD"; Flags: createvalueifdoesntexist
Root: HKLM; Subkey: "Software\JRD\TPST-CPE"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\JRD\TPST-CPE"; ValueType: string; ValueName: "Path"; ValueData: "{app}"
Root: HKLM; Subkey: "Software\JRD\TPST-CPE"; ValueType: string; ValueName: "Version"; ValueData: "{#AppVersion}"
Root: HKLM; Subkey: "Software\JRD\TPST-CPE"; ValueType: dword; ValueName: "VersionNum"; ValueData: "{#AppVersionNum}"
Root: HKLM; Subkey: "Software\JRD\TPST-CPE"; ValueType: string; ValueName: "Release Date"; ValueData: "{#ReleaseDateTimeString}"

[UninstallDelete]
;Type: files; Name: "{app}\*.log"
;Type: files; Name: "{app}\*.qcn"

[Code]
function InitializeSetup(): Boolean;
var
   Version: string;
   strPath: string;
   ResultCode: Integer;   
   Str: String;
   Res_0: Boolean;
   Res_1: Boolean;
begin
   Result:= TRUE;
   
  Res_1 := CheckForMutexes('TPST-CPE_Install_Mutex');
  Res_0 := CheckForMutexes('TPST-CPE_Running_MUTEX');
  
  if Res_0 or Res_1 then
    begin
      Str := ExpandConstant('{cm:CloseRunAppNote}');
      MsgBox(Str, mbInformation, MB_OK);
      Result := FALSE;
    end else
    begin
      CreateMutex('TPST-CPE_Install_Mutex');
      Result := TRUE;    
      //read the version
      if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'Software\JRD\TPST-CPE',
         'Version', Version) then
      begin
        // Successfully read the value
        Version := Format('Exist version %s, uninstall it?',[Version]);
        if MsgBox(Version,mbConfirmation,MB_YESNO) = IDYES then
        begin
         //read the path
         if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'Software\JRD\TPST-CPE',
         'Path', strPath) then
          begin
             CreateMutex('TPST-CPE_Uninstall_Anymore_Mutex');
             // Successfully read the value
             strPath:= strPath + '\unins000.exe';
             Exec(ExpandConstant(strPath), '', '', SW_SHOW,
                  ewWaitUntilTerminated, ResultCode);
          end;
        end else
        Result:= FALSE;
      end;
    end;
end;

function InitializeUninstall(): Boolean;
var
  Str: String;
  Res_0: Boolean;
  Res_1: Boolean;
  Res_2: Boolean;
begin
  Res_0 := CheckForMutexes('TPST-CPE_Running_MUTEX');
  Res_1 := CheckForMutexes('TPST-CPE_Install_Mutex');
  Res_2 := CheckForMutexes('TPST-CPE_Uninstall_Anymore_Mutex');
  
  if Res_2 then
  begin
    Result := TRUE;
  end else
    if Res_0 or Res_1 then
    begin
      Str := ExpandConstant('{cm:CloseRunAppNote}');
      MsgBox(Str, mbInformation, MB_OK);
      Result := FALSE;
    end else
      Result := TRUE;
  end;
end.;


