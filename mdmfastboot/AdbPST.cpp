#include "StdAfx.h"

#include "resource.h"
#include "AdbPST.h"

#include "PortStateUI.h"

#include "mdmfastbootDlg.h"

AdbPST::AdbPST(BOOL force_mode, MODULE_NAME module):
force_update(force_mode),
    m_module_name(module)
{
    //this->force_update = force_mode;
}


AdbPST::~AdbPST(void)
{
}

BOOL AdbPST::Reboot(UsbWorkData* data, DeviceInterfaces *dev) {
    usb_handle * handle = data->usb;
    adbhost adb(handle , dev->GetDevId());
    adb.shell("reboot-bootloader", NULL, NULL);
	//adb.reboot_bootloader();
    return TRUE;
}

BOOL AdbPST::DoPST(UsbWorkData* data, flash_image* img, DeviceInterfaces *dev) {
    usb_handle * handle = data->usb;
    adbhost adb(handle , dev->GetDevId());
    const wchar_t *conf_file = data->mPAppConf->GetPackageConfig()->GetPkgConfXmlPath();
    char *conf_file_char;
    int result;

    //adb_shell_command(adb,data, "cat /proc/version",LINUX_VER);
    //adb_shell_command(adb,data, "cat /etc/version",SYSTEM_VER);
    //adb_shell_command(adb,data, "cat /usr/version",USERDATA_VER);

    if (force_update) {
        sw_version_parse(data, "a5", "mismatch");
        sw_version_parse(data, "q6", "mismatch");
        sw_version_parse(data, "qcn", "mismatch");
    } else {
        if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(conf_file)) {
            data->SetInfo(ADB_CHK_ABORT, "no config.xml in the package, abort!");
            return FALSE;
        }

        conf_file_char = WideStrToMultiStr(conf_file);
        if (conf_file_char == NULL) {
            data->SetInfo(ADB_CHK_ABORT, "out of memory, abort!");
            return FALSE;
        }

        adb.sync_push(conf_file_char, "/tmp/config.xml");
        data->SetPromptMsg("Copy config.xml to /tmp/config.xml.");
        delete conf_file_char;

        result = adb_hw_check(adb,data);
        if (result != 0) {
            data->SetInfo(ADB_CHK_ABORT, "hardware check failed, abort!");
            return FALSE;
        }

        result = adb_sw_version_cmp(adb,data);
        if (result < 0) {
            data->SetInfo(ADB_CHK_ABORT, "firmware version check failed, abort!");
            return FALSE;
        } else if (result ==0) {
            DeviceInterfaces *dev = data->mActiveDevIntf;
            dev->SetDeviceStatus(DEVICE_REMOVED);

            data->SetInfo(FLASH_DONE, "firmware is the NEWEST. DO NOT UPDATE.");
            return TRUE;
        }
    }

    //prepare , do something that before flash image.
    //adb_update_NV(adb, data, img);
    //adb_write_IMEI(adb, data);
    //adb_shell_command(adb,data, "trace -r");
    //adb_shell_command(adb,data, "backup");
    if (data->partition_nr > 0) {
	    if (MODULE_M850== m_module_name)
		{
			return adb.shell("sys_reboot bootloader", NULL, NULL);
		}
		else
		{
			return adb.shell("reboot-bootloader", NULL, NULL);
		}		
	
        data->SetPromptMsg("reboot bootloader");
    } else {
        // that is mean just update qcn
        data->SetInfo(FLASH_DONE, "Finish. QCN updated.");
    }
    return TRUE;
}

UINT AdbPST::adb_shell_command(adbhost& adb,
                               UsbWorkData* data,
                               PCCH command,
                               UI_INFO_TYPE info_type) {
    PCHAR resp = NULL;
    int  resp_len;
    int ret;
    ret = adb.shell(command, (void **)&resp, &resp_len);
    if (ret ==0 && resp != NULL) {
        if (UI_DEFAULT == info_type) {
            data->SetPromptMsg(command, PROMPT_TITLE);
            data->SetPromptMsg(resp);
        } else {
            data->SetPromptMsg(resp, info_type);
        }
        free(resp);
        return 0;
    }

    return -1;
}

//"nv read %d" , id
//"nv write %d %s" ,id, value
UINT AdbPST::adb_update_NV(adbhost& adb, UsbWorkData* data,  flash_image  *const image) {
    unsigned int index = 0;
    const char* cmd;
    PCHAR resp = NULL;
    int  resp_len;
    int ret;

    if (data == NULL || NULL == image) {
        ERROR("Bad parameter.");
        return -1;
    }

    if (!data->update_qcn) {
        INFO("Do not need update qcn.");
        return -1;
    }
    //TODO:: first enter offline-mode.

    if (image->qcn_cmds_enum_init(NULL) == FALSE) {
        ERROR("qcn_cmds_enum_init FAILED.");
        return -1;
    }

    while ((cmd = image->qcn_cmds_enum_next(index)) != NULL) {
        //DEBUG("cmd is %s", cmd);
        ret = adb.shell(cmd, (void **)&resp, &resp_len);
        FREE_IF(resp);
        index++;
    }

    return 0;
}

UINT AdbPST::adb_write_IMEI(adbhost& adb, UsbWorkData* data) {
    PCHAR resp = NULL;
    int  resp_len;
    int ret;
    ret = adb.shell("nv write 15002 1", (void **)&resp, &resp_len);

    FREE_IF(resp);

    //imei 15 letters.
    adb.shell("imeiop write 860440020101686", (void **)&resp, &resp_len);
    FREE_IF(resp);
    ret = adb.shell("nv write 15002 0", (void **)&resp, &resp_len);
    FREE_IF(resp);
    return 0;
}

//#define VERSION_CMP_TEST
UINT AdbPST::adb_hw_check(adbhost& adb, UsbWorkData* data) {
    PCHAR resp = NULL;
    int  resp_len;
    int ret;
    ret = adb.shell("hwinfo_check", (void **)&resp, &resp_len);
    if (ret ==0 && resp != NULL) {
        ret = strcmp(resp, "match");
        if (ret == 0) {
            data->SetPromptMsg("hardware is match!");
        } else {
            data->SetPromptMsg("hwinfo_check return ", PROMPT_TITLE);
            data->SetPromptMsg(resp);
            WARN("hwinfo_check return \"%s\"", resp);
        }

        free(resp);
#ifdef VERSION_CMP_TEST
        return 0;
#else
        return ret;
#endif
    } else {
        return -1;
    }
}

UINT AdbPST::adb_sw_version_cmp(adbhost& adb, UsbWorkData* data){
    PCHAR resp = NULL;
    int  resp_len;
    int ret;
    ret = adb.shell("swinfo_compare", (void **)&resp, &resp_len);

    //test code
#ifdef VERSION_CMP_TEST
    free(resp);
    resp = strdup("A5:match,Q6:match,QCN:mismatch");
#endif

    if (ret ==0 && resp != NULL) {
        //A5:mismatch,Q6:match,QCN:mismatch
        // strtok_s OR strtok split string by replcae delimited char into '\0'
        // so constant string is not applied.
        char *result, *sub_result, *token, *subtoken;
        char *context, *sub_context;

        for (result = resp; ; result = NULL) {
            token = strtok_s(result, ",", &context);
            if (token == NULL) {
                ERROR("%s contain none \",\"", result);
                break;
            }

            for (sub_result = token; ; sub_result = NULL) {
                subtoken = strtok_s(sub_result, ":", &sub_context);
                if (subtoken == NULL) {
                    ERROR("%s contain none \":\"", sub_result);
                } else {
                    ret+=sw_version_parse(data, sub_result, sub_context);
                }
                break;
            }
        }
        free(resp);
    }
    return ret;

}

UINT AdbPST::sw_version_parse(UsbWorkData* data,PCCH key, PCCH value) {
    PWCHAR a5_partition[] = {L"boot", L"system", L"modem", L"aboot", L"jrdresource", L"rpm",L"efs2",L"cdrom",L"tz",L"sbl"};
    PWCHAR q6_partition[] = {L"dsp1", L"dsp2", L"dsp3", L"mibib", L"sbl2", L"adsp", L"qdsp", L"mba",  L"sdi"};
    PWCHAR *partition;
    int count,i;
    // int index;

    if (stricmp(value, "mismatch")) {
        INFO("Not valid value %s", value);
        return 0;
    }

    if (stricmp(key , "a5") == 0) {
        partition = a5_partition;
        count = sizeof(a5_partition)/ sizeof(a5_partition[0]);
        data->SetPromptMsg("A5 firmware can update");
    } else if (stricmp(key , "q6") == 0) {
        partition = q6_partition;
        count = sizeof(q6_partition)/ sizeof(q6_partition[0]);
        data->SetPromptMsg("Q6 firmware can update.");
    } else if (stricmp(key , "qcn") == 0) {
        data->SetPromptMsg("QCN can update.");
        data->update_qcn = TRUE;
        return 1;
    } else {
        ERROR("Not valid key %d", key);
        return 0;
    }

    //for (index = 0;  index < PARTITION_NUM_MAX; index++)
    //  if (NULL == data->flash_partition[index])
    //    break;

    for (i =0; i < count && data->partition_nr < PARTITION_NUM_MAX; i++) {
        data->flash_partition[data->partition_nr] =
            data->mProjectPackage->get_partition_info(*(partition+i), NULL, NULL);
        if (data->flash_partition[data->partition_nr] != NULL) {
            data->partition_nr++;
        } else {
            ERROR("Partition %S : no data available.", *(partition+i));
        }
    }

    return 1;
}
