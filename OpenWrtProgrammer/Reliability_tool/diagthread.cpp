#include "diagthread.h"


DiagThread::DiagThread(QObject *parent,MainWindow *window) :
    QThread(parent),m_mainWindow(window)
{

}

DiagThread::~DiagThread()
{
    //DisConnectMS_NV();
    //delete m_mainWindow;
}

void DiagThread::run()
{
    if(Get_Device  == false)
    {
        emit signalPopupMsgWithOK("ReliabilityTest","please insert a device to test!");
        while(true)
        {
            if(m_mainWindow->getIfUsrRsp())
            {
                 break;
            }
             SLEEP(100);
        }
        m_mainWindow->restUsrRsp();
        emit signalDiagOver();
        return;
    }

    DIAG_ReadFIRMWAREVersion();
    DIAG_PassWord_Read();
    DIAG_ReadSSID();
    DIAG_ReadIMEI();
    DIAG_MAC_READ();
    emit signalDiagOver();

}

bool DiagThread::DIAG_ReadFIRMWAREVersion()
{
    QString key[]={"Q6_FIRMWARE_VERSION", "QCN_VERSION", "DASHBOARD_VERSION", "Q6_RESOURCE_VERSION",
                   "LINUXKER_VERSION", "LINUXSYS_VERSION", "LINUXUSER_VERSION", "RESOURCE_VERSION",
                   "SBL_VERSION", "SDI_VERSION","TZ_VERSION", "MBA_VERSION",
                   "RPM_VERSION", "RECOVERY_VERSION","RECOVERYFS_VERSION", "WEBUI_VERSION",
                   "PARTITION_VERSION", "EXTERNAL_VERSION", "APPSBL_VERSION"};

    jrd_diag_get_version_req_type req;
    jrd_diag_get_version_rsp_type resp;
    short resp_len = sizeof(jrd_diag_get_version_rsp_type);
    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_SYS;
    req.hdr.cmd_code = E_JRD_DIAG_SYS_GET_VER;
    int i=-1;

    for(int Read_Item=E_JRD_Q6_FIRMWARE_VERSION;Read_Item<E_JRD_VERSION_DIAG_MAX;Read_Item++)
    {
        i++;
        char *FirmVer="";

        req.which_version= Read_Item ;
        if(QLIB_SendSync(m_QHandle,sizeof(jrd_diag_get_version_req_type),
                (unsigned char *)(&req), &resp_len,(unsigned char *)&resp,
                10000) != TRUE)
        {
            ERROR(FILE_LINE,"firmware send command error");
            emit signalDisplayDiagResult(key[i],"firmware send command error",false);
            SLEEP(100);
            continue;
        }
        if(resp.version_len!= 0)
        {
            memcpy(FirmVer,resp.version,strlen(resp.version));
            //qDebug()<<resp.version;
            FirmVer[strlen(FirmVer)] = '\0';
            QString Firmware_Version;
            Firmware_Version=QString::fromLocal8Bit(resp.version);
            //qDebug()<<key[i]<<"   "<<Firmware_Version;
            emit signalDisplayDiagResult(key[i],Firmware_Version,true);
            SLEEP(100);
            continue;
        }
        else
        {
            ERROR(FILE_LINE,"firmware response error");
            emit signalDisplayDiagResult(key[i],"firmware response error",false);
            SLEEP(100);
            continue;
        }
        return TRUE;
    }
}

bool DiagThread::DIAG_PassWord_Read()
{
    bool isDisable=false;
    jrd_diag_wifi_sec_wr_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_sec_wr_req_rsp_type);
    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_SECURITY_RW;
    req.rw_flag = E_JRD_READ;

    if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_wifi_sec_wr_req_rsp_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR(FILE_LINE,"Password read send command error");
        emit signalDisplayDiagResult("Wifi Password","Password read send command error",false);
        return FALSE;
    }

    if(resp.diag_errno == 0)
    {
        QString PassWord;
        PassWord=QString::fromLocal8Bit(resp.password);
        QString Security;
        QString Encryption;
        if(resp.security_mode==SEC_STATIC_WEP)
        {
            Security="Security: WEB; ";
            if(resp.sub_mode==OPEN_SYSTEM)
                Encryption="Encryption: Open; ";
            else
                Encryption="Encryption: Share;  ";
        }
        else
            if(resp.security_mode==SEC_WPA_PSK)
            {
               Security="Security: WPA PSK; ";
               if(resp.sub_mode==WPA_TKIP)
                   Encryption="Cipher Type: TKIP; ";
               else
                   if(resp.sub_mode==WPA_CCMP)
                       Encryption="Cipher Type: AES; ";
               else
                   Encryption="Cipher Type: AUTO; ";
            }
        else
            if(resp.security_mode==SEC_WPA2_PSK)
            {
               Security="Security: WPA2 PSK; ";
               if(resp.sub_mode==WPA_TKIP)
                   Encryption="Cipher Type: TKIP; ";
               else
                   if(resp.sub_mode==WPA_CCMP)
                       Encryption="Cipher Type: AES; ";
               else
                   Encryption="Cipher Type: AUTO; ";
            }
        else
            if(resp.security_mode==SEC_MIXED_WPA_PSK)
            {
               Security="WPA/WPA2 PSK; ";
               if(resp.sub_mode==WPA_TKIP)
                   Encryption="Cipher Type: TKIP; ";
               else
                   if(resp.sub_mode==WPA_CCMP)
                       Encryption="Cipher Type: AES; ";
               else
                   Encryption="Cipher Type: AUTO; ";
            }
        else
            if(resp.security_mode==SEC_DISABLED)
            {
                Security="Security: Disable; ";
                isDisable=true;
            }
        if(isDisable==false)
            Security+=Encryption;


        emit signalDisplayDiagResult(Security,PassWord,true);
    }
    else
    {
        ERROR(FILE_LINE,"Password read response error");
        emit signalDisplayDiagResult("Wifi Password","Password read response error",false);
        return FALSE;
    }

    return TRUE;
}

bool DiagThread::DIAG_MAC_READ()
{
    jrd_diag_wifi_mac_addr_wr_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_mac_addr_wr_req_rsp_type);
    char *MAC="";
    if(MAC == NULL )
    {
        ERROR(FILE_LINE,"MAC string is NULL");
        return FALSE;
    }
    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_MAC_RW;
    req.rw_flag = E_JRD_READ;

    if(QLIB_SendSync(
                    m_QHandle,
                    sizeof(jrd_diag_wifi_mac_addr_wr_req_rsp_type),
                    (unsigned char *)(&req),
                    &resp_len,
                    (unsigned char *)&resp,
                    10000) != TRUE)
        {
            ERROR(FILE_LINE,"MAC read send command error");
            emit signalDisplayDiagResult("MAC","MAC read send command error",false);
            return FALSE;
        }

        if(resp.diag_errno == 0)
        {
            sprintf(MAC,"%02x:%02x:%02x:%02x:%02x:%02x",resp.mac_addr[0],resp.mac_addr[1],resp.mac_addr[2],resp.mac_addr[3],resp.mac_addr[4],resp.mac_addr[5]);

            QString temp1;
            temp1=QString::fromLocal8Bit(MAC);
            qDebug()<<temp1;

            emit signalDisplayDiagResult("MAC",temp1,true);
            qDebug()<<temp1;

        }
        else
        {
            ERROR(FILE_LINE,"MAC read response error");
            emit signalDisplayDiagResult("MAC","MAC read response error",false);
            return FALSE;
        }
        return TRUE;
}
bool DiagThread::DIAG_ReadIMEI()
{
        jrd_diag_sys_imei_write_req_rsp_type req,resp;
        short resp_len = sizeof(jrd_diag_sys_imei_write_req_rsp_type);
        char *IMEI="";

        if(IMEI == NULL )
        {
            ERROR(FILE_LINE,"IMEI string is NULL");
            return FALSE;
        }

        req.hdr.cmd_entry = JRD_DIAG_CMD_F;
        req.hdr.class_code = E_JRD_DIAG_SYS;
        req.hdr.cmd_code = E_JRD_DIAG_SYS_IMEI_WRITE;

        req.rw_flag = E_JRD_READ;

        if(QLIB_SendSync(
                    m_QHandle,
                    sizeof(jrd_diag_sys_imei_write_req_rsp_type),
                    (unsigned char *)(&req),
                    &resp_len,
                    (unsigned char *)&resp,
                    10000) != TRUE)
        {
            ERROR(FILE_LINE,"IMEI read send command error");
            emit signalDisplayDiagResult("IMEI","IMEI read send command error",false);
            return FALSE;
        }

        if(resp.diag_errno == 0)
        {

            sprintf(IMEI,"%02x%02x%02x%02x%02x%02x%02x%02x%02x",resp.imei[0],resp.imei[1],resp.imei[2],resp.imei[3],resp.imei[4],resp.imei[5]
                    ,resp.imei[6],resp.imei[7],resp.imei[8]);
            qDebug()<<resp.imei;

            QString Imei;
            Imei=QString::fromLocal8Bit(IMEI);
            qDebug()<<Imei;

            Imei=Imei.mid(2,16);
            QString temp2;
           temp2.append(Imei.mid(1,1)).append(Imei.mid(0,1)).append(Imei.mid(3,1)).append(Imei.mid(2,1)).append(Imei.mid(5,1))
                 .append(Imei.mid(4,1)).append(Imei.mid(7,1)).append(Imei.mid(6,1)).append(Imei.mid(9,1)).append(Imei.mid(8,1)).append(Imei.mid(11,1))
                 .append(Imei.mid(10,1)).append(Imei.mid(13,1)).append(Imei.mid(12,1)).append(Imei.mid(15,1)).append(Imei.mid(14,1));
            temp2=temp2.mid(1,15);

            emit signalDisplayDiagResult("IMEI",temp2,true);
        }
        else
        {
            ERROR(FILE_LINE,"IMEI read response error");
            emit signalDisplayDiagResult("IMEI","IMEI read response error",false);
            return FALSE;
        }


        return TRUE;
}


bool DiagThread::DIAG_ReadSSID()
{
        jrd_diag_wifi_ssid_wr_req_rsp_type req,resp;
        short resp_len = sizeof(jrd_diag_wifi_ssid_wr_req_rsp_type);
        char *ssid="";

        if(ssid == NULL )
        {
            ERROR(FILE_LINE,"ssid string is NULL");
            return FALSE;
        }

        req.hdr.cmd_entry = JRD_DIAG_CMD_F;
        req.hdr.class_code = E_JRD_DIAG_WIFI;
        req.hdr.cmd_code = E_JRD_DIAG_WIFI_SSID_RW;

        req.rw_flag = AP1_SSID_READ;

        if(QLIB_SendSync(
                    m_QHandle,
                    sizeof(jrd_diag_wifi_ssid_wr_req_rsp_type),
                    (unsigned char *)(&req),
                    &resp_len,
                    (unsigned char *)&resp,
                    10000) != TRUE)
        {
            ERROR(FILE_LINE,"ssid read send command error");
            emit signalDisplayDiagResult("SSID","ssid read send command error",false);
            return FALSE;
        }

        if(resp.diag_errno == 0)
        {
            memcpy(ssid,resp.ssid,MAX_SSID_LEN+1);
            ssid[strlen(ssid)] = '\0';
            QString SSID;
            SSID=QString::fromLocal8Bit(ssid);

            emit signalDisplayDiagResult("SSID",SSID,true);
        }
        else
        {
            ERROR(FILE_LINE,"ssid read response error");
            emit signalDisplayDiagResult("SSID","ssid read response error",false);
            return FALSE;
        }

        return TRUE;
}

