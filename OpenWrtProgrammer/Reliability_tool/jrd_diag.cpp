#include "jrd_diag.h"
#include "src/log/log.h"
#include "src/device/device.h"
#include "src/QMSL/QLib.h"
#include "src/QMSL/QLib_Defines.h"
#include <QMessageBox>

HANDLE  	m_QHandle   = NULL;
HANDLE  	m2_QHandle  = NULL;

bool JrdDiag::DIAG_ReadSSID(int testIndex,char *ssid)
{
        jrd_diag_wifi_ssid_wr_req_rsp_type req,resp;
        short resp_len = sizeof(jrd_diag_wifi_ssid_wr_req_rsp_type);

        if(ssid == NULL )
        {
            ERROR("ssid string is NULL");
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
            ERROR("ssid read send command error");
            //DisplayTestResult(testIndex,"ssid read send command error",false);
            return FALSE;
        }

        if(resp.diag_errno == 0)
        {
            memcpy(ssid,resp.ssid,MAX_SSID_LEN+1);
        }
        else
        {
            ERROR("ssid read response error");
           // DisplayTestResult(testIndex,"ssid read response error",false);
            return FALSE;
        }


       // DisplayTestResult(testIndex,"ssid read successfull",false);
        return TRUE;
}

int JrdDiag::ConnectMS_NV(void)
{
    int i=0;
    int status = 0;
    char itemBuf[256]="";
    char resultBuf[256]="";
    unsigned char _bUserSelectQPST = FALSE;//TRUE;//FALSE;
    uint16 iPortNum;
    uint16 pPortList[30];
    unsigned short comport = -1;

    if(NULL != m_QHandle)
    {
        QLIB_DisconnectServer(m_QHandle);
        m_QHandle = NULL;
    }
    if(NULL != m2_QHandle)
    {
        QLIB_DisconnectServer(m2_QHandle);
        m_QHandle = NULL;
    }
    QLIB_SetLibraryMode( _bUserSelectQPST  );


    for(i = 0; i<10;  i++)
    {
        uint16 ComId[32] = {0};
        uint16 count = 0;
        CDeviceList devlist;
        count = devlist.GetComPortList(PORTTYPE_DIAG, ComId);
       // testPort = ComId[0];

        //TPort_GetAvailableDIAGPortList(&iPortNum, pPortList);//20090202
        if(count > 0)
        {
            comport = ComId[0];
           // ui->lineEdit->setText(QString("COM%1").arg(comport));
            break;
        }
        Sleep(500);
        sprintf(resultBuf,"ConnectMS_NV search com:%d",comport);
    }

    m_QHandle = QLIB_ConnectServer(comport);
    if(m_QHandle == NULL)
    {
        return 0;
    }

#if 1
    for(i = 0; i < 5;  i++)
    {
        m_QHandle = QLIB_ConnectServer(comport);
        if( QLIB_IsPhoneConnected(m_QHandle))
        {
            QLIB_GetComPortNumber(m_QHandle,&comport);
        }
        else
        {
            comport =-1;
        }
        if(comport == QLIB_COM_AUTO_DETECT)
        {
            m_QHandle = QLIB_ConnectServer(comport);
        }

        if(m_QHandle!=NULL)
        {
            m_QHandle = QLIB_ConnectServerWithWait(comport, 2000+i*100);
        }
        status = QLIB_GetComPortNumber(m_QHandle,&comport);

        if(QLIB_IsPhoneConnected(m_QHandle))
        {
            break;
        }
    }
    if(m_QHandle == NULL)
    {
        return 0;
    }

#endif

    for(i=0;i<5;i++)
    {
        status = 0;
        status = QLIB_IsPhoneConnected(m_QHandle);
        if(status)
        {
            break;
        }
        else
        {
            //File_WriteLog("connect phone fail! 2");
            return 0;
        }
    }

    //strcpy(itemBuf,"ConnectMS");
    //sprintf(resultBuf, "COM%d", comport);
    //DisplayTestItem(itemBuf,resultBuf,status,p_s_da_cal->globalsetdata.display_info);
    return 1;
}

bool JrdDiag::DIAG_WriteSSID(int testIndex,char *ssid)
{
    jrd_diag_wifi_ssid_wr_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_ssid_wr_req_rsp_type);

    if(ssid == NULL || strlen(ssid)>32)
    {
        ERROR("ssid string is NULL or the strlen is too long");
        //DisplayTestResult(testIndex,"ssid string is NULL or the strlen is too long",false);
        return FALSE;
    }

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_SSID_RW;

    req.rw_flag = AP1_SSID_WRITE;
    memcpy(req.ssid,ssid,MAX_SSID_LEN+1);

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_wifi_ssid_wr_req_rsp_type),
                (unsigned char *)&req,
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("ssid write send command error");
        //DisplayTestResult(testIndex,"ssid write send command error",false);
        return FALSE;
    }

    if(resp.diag_errno != 0)
    {
        ERROR("ssid write response error");
      //  DisplayTestResult(testIndex,"ssid write response error",false);
        return FALSE;
    }

    return TRUE;
}

bool JrdDiag::DIAG_RestartWiFi(int testIndex)
{
    jrd_diag_wifi_switch_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_switch_req_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_ONOFF;

    req.op= AP_RESTART;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_wifi_switch_req_rsp_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("wifi restart send command error");
        //DisplayTestResult(testIndex,"wifi restart send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("wifi restart read response error");
        //DisplayTestResult(testIndex,"wifi restart read response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"wifi restart successful",false);
    return TRUE;
}

bool JrdDiag::DIAG_DisableSecurity(int testIndex)
{
    jrd_diag_wifi_sec_wr_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_sec_wr_req_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_SECURITY_RW;

    req.rw_flag = AP1_SEC_INFO_WRITE;
    req.security_mode = SEC_DISABLED;
      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_wifi_sec_wr_req_rsp_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("wifi disable security send command error");
        //DisplayTestResult(testIndex,"wifi disable security send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("wifi disable security read response error");
        //DisplayTestResult(testIndex,"wifi disable security read response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"wifi disable security successful",false);
    return TRUE;
}

bool JrdDiag::DIAG_CheckStaConnect(int testIndex)
{
    jrd_diag_wifi_connect_sta_req_type req;
    jrd_diag_wifi_connect_sta_rsp_type resp;
    short resp_len = sizeof(jrd_diag_wifi_connect_sta_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_CONNECT_STATUS;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_wifi_connect_sta_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("wifi check sta connect send command error");
        //DisplayTestResult(testIndex,"wifi check connect status send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("wifi check sta connect  read response error");
        //DisplayTestResult(testIndex,"wifi check connect status read response error",false);
        return FALSE;
    }
    if(resp.con_sta_num != 0)
    {
        //DisplayTestResult(testIndex,"wifi check connect status successfull",true);
        return TRUE;
    }
    else
    {
        //DisplayTestResult(testIndex,"wifi check connect status fail",true);
        return FALSE;
    }

}

bool JrdDiag::DIAG_Charger_TEST(int testIndex)
{
    jrd_diag_charger_status_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_charger_status_req_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_CHARGER_STATUS;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_charger_status_req_rsp_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("charger test send command error");
        //DisplayTestResult(testIndex,"charger test send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("charger test read response error");
        //DisplayTestResult(testIndex,"charger test read response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"charger test successful",false);
    return TRUE;
}

bool JrdDiag::DIAG_EnterLCDScreenTEST(int testIndex)
{
    jrd_diag_screen_req_type req;
    jrd_diag_screen_rsp_type resp;
    short resp_len = sizeof(jrd_diag_screen_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SCREEN;
    req.event_type = ENTRY_LCD_TEST ;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_screen_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("entry screen send command error");
        //DisplayTestResult(testIndex,"entry screen send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("entry screen response error");
        //DisplayTestResult(testIndex,"entry screen response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"entry screen successful",true);
    return TRUE;
}

bool JrdDiag::DIAG_LCDScreen_Turn_On(int testIndex)
{
    jrd_diag_screen_req_type req;

    jrd_diag_screen_rsp_type resp;
    short resp_len = sizeof(jrd_diag_screen_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SCREEN;
    req.event_type = LCD_TURN_ON ;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_screen_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("screen turn on send command error");
        //DisplayTestResult(testIndex,"screen turn on send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("screen turn on response error");
        //DisplayTestResult(testIndex,"screen turn on response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"screen turn successful",true);
    return TRUE;
}

bool JrdDiag::DIAG_LCDScreen_Turn_Off(int testIndex)
{
    jrd_diag_screen_req_type req;
    jrd_diag_screen_rsp_type resp;
    short resp_len = sizeof(jrd_diag_screen_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SCREEN;
    req.event_type = LCD_TURN_OFF ;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_screen_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("screen turn off send command error");
        //DisplayTestResult(testIndex,"screen turn off send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("screen turn off response error");
        //DisplayTestResult(testIndex,"screen turn off response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"screen turn off successful",true);
    return TRUE;
}

bool JrdDiag::DIAG_EXITLCDScreenTEST(int testIndex)
{
    jrd_diag_screen_req_type req;
    jrd_diag_screen_rsp_type resp;
    short resp_len = sizeof(jrd_diag_screen_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SCREEN;
    req.event_type = EXIT_LCD_TEST ;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_screen_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("exit screen send command error");
        //DisplayTestResult(testIndex,"exit screen send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("exit screen response error");
        //DisplayTestResult(testIndex,"exit screen response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"exit screen successful",true);
    return TRUE;
}

bool JrdDiag::DIAG_EnterLEDTEST(int testIndex)
{
    jrd_diag_led_req_type req;
    jrd_diag_led_rsp_type resp;
    short resp_len = sizeof(jrd_diag_led_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = ENTRY_LED_TEST ;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_led_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("entry led send command error");
        //DisplayTestResult(testIndex,"entry led send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("entry led response error");
        //DisplayTestResult(testIndex,"entry led response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"entry led successful",true);
    return TRUE;
}

bool JrdDiag::DIAG_EXITLEDTEST(int testIndex)
{
    jrd_diag_led_req_type req;
    jrd_diag_led_rsp_type resp;
    short resp_len = sizeof(jrd_diag_led_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = EXIT_LED_TEST ;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_led_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("exit  led send command error");
        //DisplayTestResult(testIndex,"exit  led send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("exit led response error");
        //DisplayTestResult(testIndex,"exit led response error",false);
        return FALSE;
    }
    //DisplayTestResult(testIndex,"exit led successful",false);
    return TRUE;
}

bool JrdDiag::DIAG_REDLEDTEST(int testIndex)
{
    jrd_diag_led_req_type req;
    jrd_diag_led_rsp_type resp;
    short resp_len = sizeof(jrd_diag_led_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = RED_LED_TEST ;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_led_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("red  led send command error");
        //DisplayTestResult(testIndex,"red led send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("red led response error");
        //DisplayTestResult(testIndex,"red led response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"red led test successful",true);
    return TRUE;
}


bool JrdDiag::DIAG_GREENLEDTEST(int testIndex)
{
    jrd_diag_led_req_type req;
    jrd_diag_led_rsp_type resp;
    short resp_len = sizeof(jrd_diag_led_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = GREEN_LED_TEST ;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_led_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("green  led send command error");
        //DisplayTestResult(testIndex,"green led send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("green led response error");
        //DisplayTestResult(testIndex,"green led response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"green led test successful",true);
    return TRUE;
}

bool JrdDiag::DIAG_BLUELEDTEST(int testIndex)
{
    jrd_diag_led_req_type req;
    jrd_diag_led_rsp_type resp;
    short resp_len = sizeof(jrd_diag_led_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = BLUE_LED_TEST ;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_led_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("blue led send command error");
        //DisplayTestResult(testIndex,"blue led send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("blue led response error");
        //DisplayTestResult(testIndex,"blue led response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"blue led test successful",true);
    return TRUE;
}


bool JrdDiag::DIAG_ENTRYKEYTEST(int testIndex)
{
    jrd_diag_key_req_type req;
    jrd_diag_key_rsp_type resp;
    short resp_len = sizeof(jrd_diag_key_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_KEY;
    req.event_type= ENTRY_KEY_TEST ;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_key_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("entry key send command error");
        //DisplayTestResult(testIndex,"entry key send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("entry key response error");
        //DisplayTestResult(testIndex,"entry key response error",false);
        return FALSE;
    }

    return TRUE;
}

bool JrdDiag::DIAG_KEYTEST(int testIndex)
{
    jrd_diag_key_req_type req;
    jrd_diag_key_rsp_type resp;
    short resp_len = sizeof(jrd_diag_key_rsp_type);
    int Cyctimes = 0;
    QTime Startime = QTime::currentTime();
   // clock_t Startime = clock();

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_KEY;
    req.event_type= QUERY_KEY_TEST ;

    do
    {
        if(QLIB_SendSync(
                    m_QHandle,
                    sizeof(jrd_diag_key_req_type),
                    (unsigned char *)(&req),
                    &resp_len,
                    (unsigned char *)&resp,
                    10000) != TRUE)
        {
            ERROR("key test send command error");
            //DisplayTestResult(testIndex,"key test send command error",false);
            return FALSE;
        }

        QMessageBox::information(NULL, "Key Test", "Please press the side key",
                                 QMessageBox::Ok);
        if(resp.diag_errno == 0)
        {
            //DisplayTestResult(testIndex,"key test successful",true);
            return TRUE;
        }

        if( (QTime::currentTime().secsTo(Startime)) >= (60*1000))
        {
            ERROR("key test out of time");
            //DisplayTestResult(testIndex,"key test out of time",false);
            return FALSE;
        }
    }while(1);

    return TRUE;
}

bool JrdDiag::DIAG_EXITKEYTEST(int testIndex)
{
    jrd_diag_key_req_type req;
    jrd_diag_key_rsp_type resp;
    short resp_len = sizeof(jrd_diag_key_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_KEY;
    req.event_type= EXIT_KEY_TEST ;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_key_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("exit key send command error");
        //DisplayTestResult(testIndex,"exit key send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("exit key response error");
        //DisplayTestResult(testIndex,"exit key response error",false);
        return FALSE;
    }

    return TRUE;
}

bool JrdDiag::DIAG_ReadFIRMWAREVersion(int testIndex,char* FirmVer)
{
    jrd_diag_get_version_req_type req;
    jrd_diag_get_version_rsp_type resp;
    short resp_len = sizeof(jrd_diag_get_version_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_SYS;
    req.hdr.cmd_code = E_JRD_DIAG_SYS_GET_VER;
    req.which_version= E_JRD_Q6_FIRMWARE_VERSION ;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_get_version_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("firmware send command error");
        //DisplayTestResult(testIndex,"firmware send command error",false);
        return FALSE;
    }

    if(resp.version_len!= 0)
    {
        memcpy(FirmVer,resp.version,strlen(resp.version));
        FirmVer[strlen(FirmVer)] = '\0';
    }
    else
    {
        ERROR("firmware response error");
        //DisplayTestResult(testIndex,"firmware response error",false);
        return FALSE;
    }

    return TRUE;
}

bool JrdDiag::DIAG_CheckSD_Card(int testIndex)
{
    jrd_diag_sdcard_status_req_rsp_type req;
    jrd_diag_sdcard_status_req_rsp_type resp;
    short resp_len = sizeof(jrd_diag_sdcard_status_req_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SD_CARD_STATUS;

     if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_sdcard_status_req_rsp_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("check SD Card send command error");
        //DisplayTestResult(testIndex,"check SD Card send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR("check SD Card  read response error");
        //DisplayTestResult(testIndex,"check SD Card  read response error",false);
        return FALSE;
    }
    if(resp.sdcard_status==1)
    {
        ERROR("Didn't find SD Card");
        //DisplayTestResult(testIndex,"Didn't find SD Card",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"check SD Card successful",true);
    return TRUE;
}

bool JrdDiag::DIAG_CheckSIM_Card(int testIndex)
{
    jrd_diag_sim_status_req_type req;
    jrd_diag_sim_status_rsp_type resp;
    short resp_len = sizeof(jrd_diag_sim_status_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SIM_STATUS;

     if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_sim_status_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("check SIM send command error");
        //DisplayTestResult(testIndex,"check SIM send command error",false);
        return FALSE;
    }

     if(resp.diag_errno!=0)
     {
        ERROR("resp.sim_status response error!");
        //DisplayTestResult(testIndex,"check sim status response error!",false);
        return FALSE;
     }

    if(resp.status_len!= 0)
    {
        if(strstr(resp.sim_status,"READY") == NULL)
        {
            ERROR("resp.sim_status response didn't have READY!");
            //DisplayTestResult(testIndex,"sim is not ready!",false);
            return FALSE;
        }
    }
    else
    {
        ERROR("check SIM  read response error");
        //DisplayTestResult(testIndex,"check SIM  read response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"check SIM status successful",false);
    return TRUE;
}

bool JrdDiag::DIAG_CheckNetWork(int testIndex)
{
    jrd_diag_nw_register_status_req_type req;
    jrd_diag_nw_register_status_rsp_type resp;
    short resp_len = sizeof(jrd_diag_nw_register_status_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_NETWORK;
    req.hdr.cmd_code = E_JRD_DIAG_NW_REG_STATUS;

     if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_nw_register_status_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("check network send command error");
       // DisplayTestResult(testIndex,"check network send command error",false);
        return FALSE;
    }

     if(resp.diag_errno!=0)
     {
        ERROR("check network read response error!");
        //DisplayTestResult(testIndex,"check network read response error",false);
        return FALSE;
     }

     //DisplayTestResult(testIndex,"check network status successful",true);
     return TRUE;
}

bool JrdDiag::DIAG_TraceAbilityRead(int testIndex,TraceInfo *pTrace)
{
    jrd_diag_trace_info_req_type req;
    jrd_diag_trace_info_rsp_type resp;
    short resp_len = sizeof(jrd_diag_trace_info_rsp_type);

    //printf("resp len start is %d\n",resp_len);

    if(pTrace == NULL )
    {
        ERROR("trace info is NULL");
        return FALSE;
    }

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_SYS;
    req.hdr.cmd_code = E_JRD_DIAG_SYS_TRACE_INFO;

    req.rw_flag = TRACE_INFO_READ;

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_trace_info_req_type),
                (unsigned char *)&req,
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("trace info read send command error");
        //DisplayTestResult(testIndex,"trace info read send command error",false);
        return FALSE;
    }
    //printf("trace info len %d\n",resp.trace_info_len);

    if(resp.diag_errno == 0 && TRACE_INFO_LEN >= resp.trace_info_len)
    {
        memcpy(pTrace->All, resp.trace_info, resp.trace_info_len);
    }
    else
    {
        ERROR("trace info read response error");
        //DisplayTestResult(testIndex,"trace info read response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"get trace info successful",false);
    return TRUE;
}

bool JrdDiag::DIAG_TraceAbilityWrite(int testIndex,char *traceInfo)
{
    jrd_diag_trace_info_req_type req;
    jrd_diag_trace_info_rsp_type resp;
    short resp_len = sizeof(jrd_diag_trace_info_rsp_type);

    if(traceInfo == NULL )
    {
        ERROR("trace info is NULL");
        //DisplayTestResult(testIndex,"trace info is NULL",false);
        return FALSE;
    }

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_SYS;
    req.hdr.cmd_code = E_JRD_DIAG_SYS_TRACE_INFO;

    req.rw_flag = TRACE_INFO_WRITE;
    req.trace_info_len = TRACE_INFO_LEN;
    memcpy(req.trace_info, traceInfo, TRACE_INFO_LEN);

      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_trace_info_req_type),
                (unsigned char *)&req,
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR("trace info write send command error");
        //DisplayTestResult(testIndex,"trace info write send command error",false);
        return FALSE;
    }

    if(resp.diag_errno != 0 )
    {
        ERROR("trace info write response error");
        //DisplayTestResult(testIndex,"trace info write response error",false);
        return FALSE;
    }

    //DisplayTestResult(testIndex,"write trace info successful ",false);
    return TRUE;
}

bool JrdDiag::DIAG_TraceAbilityLocal(int testIndex,TraceInfo *pTrace)
{
    memcpy(pTrace->PCBAInfo,pTrace->All,27);
    memcpy(pTrace->HandsetInfo,pTrace->All + 27,23);
    memcpy(pTrace->MiniInfo,pTrace->All + 50,43);
    memcpy(pTrace->GoldenSample,pTrace->All + 93,4);
    memcpy(pTrace->HDTB,pTrace->All + 97,6);
    memcpy(pTrace->PT1Test,pTrace->All + 103,8);
    memcpy(pTrace->PT2Test,pTrace->All + 111,8);
    memcpy(pTrace->PT3Test,pTrace->All + 119,8);
    memcpy(pTrace->BluetoothTest,pTrace->All + 127,8);
    memcpy(pTrace->WiFiTest,pTrace->All + 135,8);
    memcpy(pTrace->GPSTest,pTrace->All + 143,9);
    memcpy(pTrace->FinalTest,pTrace->All + 119,8);
    memcpy(pTrace->HDT,pTrace->All + 160,6);
    memcpy(pTrace->CUSWinfo,pTrace->All + 166,123);
    memcpy(pTrace->SSID,pTrace->All + 421,32);
    memcpy(pTrace->PasswordAT,pTrace->All + 453,15);
    memcpy(pTrace->PT4Test,pTrace->All + 289,132);
    memcpy(pTrace->SpaceRegion,pTrace->All + 468,45);

    /*CopyString (pTrace->PCBAInfo, 0, pTrace->All, 0, 27);
    CopyString (pTrace->HandsetInfo, 0, pTrace->All, 27, 23);
    CopyString (pTrace->MiniInfo, 0, pTrace->All, 50, 43);
    CopyString (pTrace->GoldenSample, 0, pTrace->All, 93, 4);
    CopyString (pTrace->HDTB, 0, pTrace->All, 97, 6);
    CopyString (pTrace->PT1Test, 0, pTrace->All, 103, 8);
    CopyString (pTrace->PT2Test, 0, pTrace->All, 111, 8);
    CopyString (pTrace->PT3Test, 0, pTrace->All, 119, 8);
    CopyString (pTrace->BluetoothTest, 0, pTrace->All, 127, 8);
    CopyString (pTrace->WiFiTest, 0, pTrace->All, 135, 8);
    CopyString (pTrace->GPSTest, 0, pTrace->All, 143, 9);
    CopyString (pTrace->FinalTest, 0, pTrace->All, 152, 8);
    CopyString (pTrace->HDT, 0, pTrace->All, 160, 6);
    CopyString (pTrace->CUSWinfo, 0, pTrace->All, 166, 123);
    CopyString (pTrace->SSID, 0, pTrace->All, 421, 32);
    CopyString (pTrace->PasswordAT, 0, pTrace->All, 453, 15);
    CopyString (pTrace->PT4Test, 0, pTrace->All, 289, 132);
    CopyString (pTrace->SpaceRegion, 0, pTrace->All, 468, 45);*/
    strcpy(TraceSSID,pTrace->SSID);
    strcpy(TracePassword,pTrace->PasswordAT);

    return TRUE;
}

