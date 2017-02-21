#include <StdAfx.h>
#include "jrdmmidiag.h"
#include "log.h"
#include "device.h"
#include "QLib_Defines.h"
#include "QLib.h"
#define RETRY_TIMES 5
HANDLE  	m_QHandle   = NULL;

bool DIAG_ReadSSID(int testIndex,char *ssid)
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

int DisconnectMs() {
    if(NULL != m_QHandle)
    {
        QLIB_DisconnectServer(m_QHandle);
        m_QHandle = NULL;
    }
    return 0;
}

int ConnectMS(unsigned short comport)
{
    int i=0;
    int status = 0;

    if(NULL != m_QHandle)
    {
        QLIB_DisconnectServer(m_QHandle);
        m_QHandle = NULL;
    }

    QLIB_SetLibraryMode( FALSE);

    m_QHandle = QLIB_ConnectServer(comport);
    if(m_QHandle == NULL)
    {
        return 0;
    }

#if 0
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
            LOGE("connect phone fail! 2");
            return 0;
        }
    }

    //strcpy(itemBuf,"ConnectMS");
    //sprintf(resultBuf, "COM%d", comport);
    //DisplayTestItem(itemBuf,resultBuf,status,p_s_da_cal->globalsetdata.display_info);
    return 1;
}

bool DIAG_WriteSSID(int testIndex,char *ssid)
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

bool DIAG_RestartWiFi(int testIndex)
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

bool DIAG_DisableSecurity(int testIndex)
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

bool DIAG_CheckStaConnect(int testIndex)
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

bool DIAG_Charger_TEST(int testIndex)
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

bool DIAG_EnterLCDScreenTEST(int testIndex)
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

bool DIAG_LCDScreen_Turn_On(int testIndex)
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

bool DIAG_LCDScreen_Turn_Off(int testIndex)
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

bool DIAG_EXITLCDScreenTEST(int testIndex)
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

bool DIAG_EnterLEDTEST(int testIndex)
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

bool DIAG_EXITLEDTEST(int testIndex)
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

bool DIAG_REDLEDTEST(int testIndex)
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


bool DIAG_GREENLEDTEST(int testIndex)
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

bool DIAG_BLUELEDTEST(int testIndex)
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


bool DIAG_ENTRYKEYTEST(int testIndex)
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

bool DIAG_KEYTEST(int testIndex)
{
#if 0
    jrd_diag_key_req_type req;
    jrd_diag_key_rsp_type resp;
    short resp_len = sizeof(jrd_diag_key_rsp_type);
    int Cyctimes = 0;
//    QTime Startime = QTime::currentTime();
    clock_t Startime = clock();

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
#endif
    return TRUE;
}

bool DIAG_EXITKEYTEST(int testIndex)
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

bool DIAG_ReadFIRMWAREVersion(int testIndex,char* FirmVer)
{
#if 0
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
    #else
    LOGE("CAUTION:: Not implement");
    return FALSE;
    #endif
}

bool DIAG_CheckSD_Card(int testIndex)
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

BOOL DIAG_CheckSIM_Card(string& msg)
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
        msg = "send check SIM command error, ";
        return FALSE;
    }

     if(resp.diag_errno !=0)
     {
        msg = "diag does not response!";
        return FALSE;
     }

    if(resp.status_len == 0)
    {
        msg = "check SIM read response error";
        return FALSE;
    }

    if(strstr(resp.sim_status,"READY") == NULL && strstr(resp.sim_status,"PIN") == NULL)
    {
        msg = "SIM test error, ";
        msg += resp.sim_status;
        return FALSE;
    }

    msg = resp.sim_status;

    //DisplayTestResult(testIndex,"check SIM status successful",false);
    return TRUE;
}

BOOL DIAG_TurnTelRing(BOOL on, string& msg)
{
    jrd_diag_rj11_req_type req;
    jrd_diag_rj11_rsp_type resp;
    short resp_len = sizeof(jrd_diag_rj11_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_RJ11;
	req.ring_toggle = on ? 4 : 5;

     if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_rj11_req_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        msg = "send check RJ11 Ring command error, ";
        return FALSE;
    }

    msg = "command send";

    //DisplayTestResult(testIndex,"check SIM status successful",false);
    return resp.diag_errno == 0;
}

bool IMEICheck(string &IMEI)
{
    if(IMEI.size() !=15)
        return FALSE;

    for(unsigned int i=0;i<IMEI.size();i++)
    {
    	if (IMEI.at(i) > '9' || IMEI.at(i) < '0')
			return FALSE;
#if 0
        if(IMEI.mid(i,1)=="0"||IMEI.mid(i,1)=="1"||IMEI.mid(i,1)=="2"||IMEI.mid(i,1)=="3"||
           IMEI.mid(i,1)=="4"||IMEI.mid(i,1)=="5"||IMEI.mid(i,1)=="6"||IMEI.mid(i,1)=="7"||
           IMEI.mid(i,1)=="8"||IMEI.mid(i,1)=="9")
            ;
        else
            return false;
#endif
    }
    string temp="08";
#if 0
    temp.append(IMEI.mid(0,1)).append("a").append(IMEI.mid(2,1)).append(IMEI.mid(1,1)).append(IMEI.mid(4,1))
          .append(IMEI.mid(3,1)).append(IMEI.mid(6,1)).append(IMEI.mid(5,1)).append(IMEI.mid(8,1)).append(IMEI.mid(7,1)).append(IMEI.mid(10,1))
          .append(IMEI.mid(9,1)).append(IMEI.mid(12,1)).append(IMEI.mid(11,1)).append(IMEI.mid(14,1)).append(IMEI.mid(13,1));
#endif
     temp += IMEI.at(0);
     temp += "a";
     temp += IMEI.at(2);
     temp += IMEI.at(1);
     temp += IMEI.at(4);
     temp += IMEI.at(3);
     temp += IMEI.at(6);
     temp += IMEI.at(5);
     temp += IMEI.at(8);
     temp += IMEI.at(7);
     temp += IMEI.at(10);
     temp += IMEI.at(9);
     temp += IMEI.at(12);
     temp += IMEI.at(11);
     temp += IMEI.at(14);
     temp += IMEI.at(13);
    IMEI=temp;
    return TRUE;
}

BOOL DIAG_IMEIWrite(string& imei, string& msg)
{
    if(!IMEICheck(imei))
    {
        msg = "Wrong IMEI Input!\n For example: 863459020000414";
        return FALSE;
    }
    const char *IMEI=imei.c_str();

    jrd_diag_sys_imei_write_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_sys_imei_write_req_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_SYS;
    req.hdr.cmd_code = E_JRD_DIAG_SYS_IMEI_WRITE;
    req.rw_flag = E_JRD_WRITE;

    sscanf(IMEI, "%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                               &req.imei[0], &req.imei[1], &req.imei[2], &req.imei[3], &req.imei[4], &req.imei[5]
                               , &req.imei[6], &req.imei[7], &req.imei[8]);

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                    m_QHandle,
                    sizeof(jrd_diag_sys_imei_write_req_rsp_type),
                    (unsigned char *)&req,
                    &resp_len,
                    (unsigned char *)&resp,
                    10000) != TRUE) {
            if(i == RETRY_TIMES - 1)
            {
                LOGE("IMEI write send command error!");
                msg = "IMEI write send command error!";
                return FALSE;
            } else {
                SLEEP(200);
            }
        } else {
            LOGE("IMEI write send command ok,the IMEI = %s",IMEI);
            msg = "Write IMEI successfully!";
            break;
        }
    }
    if(resp.diag_errno != 0) {
        LOGE("IMEI write response error! %d", resp.diag_errno);
        msg = "IMEI write response error!";
        return FALSE;
    }
    return TRUE;
}



bool DIAG_CheckNetWork(int testIndex)
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

bool DIAG_TraceAbilityRead(int testIndex,TraceInfo *pTrace)
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

bool DIAG_TraceAbilityWrite(int testIndex,char *traceInfo)
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

bool DIAG_TraceAbilityLocal(int testIndex,TraceInfo *pTrace)
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
    CopyString (pTrace->SpaceRegion, 0, pTrace->All, 468, 45);
    strcpy(TraceSSID,pTrace->SSID);
    strcpy(TracePassword,pTrace->PasswordAT);
    */
    return TRUE;
}

