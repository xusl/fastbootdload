;===========================================================
;
; EmbedDownload Setup
;
; Compiler: Inno Setup V5.4.2
; Creator : sunshp
; time    : 2013/08/09
;
;===========================================================
[Setup]
AppName=mdmfastboot
AppVerName=mdmfastboot
AppPublisher=JRD
DefaultDirName={pf}\JRD\mdmfastboot
DefaultGroupName=JRD\mdmfastboot
DisableProgramGroupPage=yes
OutputDir= .\
OutputBaseFilename=mdmfastboot_Setup
SolidCompression=yes


[Languages]
Name: "En"; MessagesFile: "Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"


[Files]
Source: "fastbootdownload\*"; DestDir: "{app}";Flags: ignoreversion allowunsafefiles; Permissions:everyone-full

[Icons]
Name: "{group}\mdmfastboot"; IconFilename: {app}\Icon.ico;Filename: "{app}\mdmfastboot.exe"; WorkingDir: "{app}"
Name: "{group}\{cm:UninstallProgram,mdmfastboot}"; IconFilename: {app}\Icon.ico;Filename: "{uninstallexe}"
Name: "{userdesktop}\mdmfastboot"; Filename: "{app}\mdmfastboot.exe"; IconFilename: {app}\Icon.ico;Tasks: desktopicon

[Run]
Filename: "{app}\mdmfastboot.exe"; Description: "{cm:LaunchProgram,mdmfastboot}"; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKLM; Subkey: "Software\JRD"; Flags: createvalueifdoesntexist
Root: HKLM; Subkey: "Software\JRD\mdmfastboot"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\JRD\mdmfastboot"; ValueType: string; ValueName: "Path"; ValueData: "{app}"
Root: HKLM; Subkey: "Software\JRD\mdmfastboot"; ValueType: string; ValueName: "Version"; ValueData: "mdmfastboot"
Root: HKLM; Subkey: "Software\JRD\mdmfastboot"; ValueType: dword; ValueName: "VersionNum"; ValueData: "0000"
Root: HKLM; Subkey: "Software\JRD\mdmfastboot"; ValueType: string; ValueName: "Release Date"; ValueData: "2013/08/09"

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
   
  Res_1 := CheckForMutexes('mdmfastboot_Install_Mutex');
  Res_0 := CheckForMutexes('mdmfastboot_Running_MUTEX');
  
  if Res_0 or Res_1 then
    begin
      Str := ExpandConstant('{cm:CloseRunAppNote}');
      MsgBox(Str, mbInformation, MB_OK);
      Result := FALSE;
    end else
    begin
      CreateMutex('mdmfastboot_Install_Mutex');
      Result := TRUE;    
      //read the version
      if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'Software\JRD\mdmfastboot',
         'Version', Version) then
      begin
        // Successfully read the value
        Version := Format('Exist the version %s,whether uninstall it?',[Version]);
        if MsgBox(Version,mbConfirmation,MB_YESNO) = IDYES then
        begin
         //read the path
         if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'Software\JRD\mdmfastboot',
         'Path', strPath) then
          begin
             CreateMutex('mdmfastboot_Uninstall_Anymore_Mutex');
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
  Res_0 := CheckForMutexes('mdmfastboot_Running_MUTEX');
  Res_1 := CheckForMutexes('mdmfastboot_Install_Mutex');
  Res_2 := CheckForMutexes('mdmfastboot_Uninstall_Anymore_Mutex');
  
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


