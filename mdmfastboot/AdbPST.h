#pragma once
#include "PST.h"
#include "adbhost.h"
#include "fastbootflash.h"

class AdbPST
{
public:
    AdbPST(BOOL force_mode, MODULE_NAME module);
    ~AdbPST(void);
    BOOL DoPST(UsbWorkData* data, flash_image* img, DeviceInterfaces *dev);

private:
     UINT adb_hw_check(adbhost& adb, UsbWorkData* data);
     UINT adb_sw_version_cmp(adbhost& adb, UsbWorkData* data);
     UINT sw_version_parse(UsbWorkData* data,PCCH key, PCCH value);
     UINT adb_shell_command(adbhost& adb, UsbWorkData* data, PCCH command,
                                UI_INFO_TYPE info = UI_DEFAULT);
     UINT adb_write_IMEI(adbhost& adb, UsbWorkData* data);
     UINT adb_update_NV(adbhost& adb, UsbWorkData* data,  flash_image *const image);

private:
  BOOL force_update;
  MODULE_NAME m_module_name;
};

