#include "testthread.h"
#include "src/QMSL/QLib.h"
#include "src/QMSL/QLib_Defines.h"
#include "src/device/device.h"
#include <QMessageBox>
#include "src/log/log.h"
#include <time.h>

extern int             itemNum;
extern QVector<QString> vecColorsValue;
extern QVector<uint32>    vecUiLedDiagOrder;



extern HANDLE  	m2_QHandle  = NULL;
extern bool  	Get_Device  = false;
extern bool Is_Wifi_Password_Read=false;
extern bool Is_Wifi_Password_Quit=false;
extern bool Is_Wifi_Password_Write=false;
extern HANDLE  	m_QHandle   = NULL;

TestThread::TestThread(QObject *parent,MainWindow *window) :
    QThread(parent),m_mainWindow(window)
{
    mapTestItems = m_mainWindow->getMapTestItems();
    testItems = m_mainWindow->getTestItems();
}

TestThread::~TestThread()
{
    DisConnectMS_NV();
}

void TestThread::run()
{
    testItems = m_mainWindow->getTestItems();

    uint16 ComId[32] = {0};
    uint16 count = 0;
    CDeviceList devlist;
    count = devlist.GetComPortList(PORTTYPE_DIAG, ComId);
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
        emit signalTestOver();
        return;
    }

    //ConnectMS_NV();

    int itemSize = testItems.size();
    for(int i = 0; i < itemSize; i++)
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
            emit signalTestOver();
            return;
        }
        QString aItem = testItems.at(i);
        QMap<QString,int>::const_iterator itr = mapTestItems.find(aItem);
        int Itemvalue;
        while (itr != mapTestItems.end())
        {
            if(itr.key() == aItem)
            {
                Itemvalue = itr.value();
                break;
            }
            ++itr;
        }

        switch(Itemvalue)
        {
            emit signalHighlightTestItems(Itemvalue);
            case  Wifi_Calibration:
            {
                if(Enter_test_mode(i))
                {
                    if(Read_Power_level_File(i))
                        save_wifi_power(i);
                }


            }
            break;
        case  Wifi_Connecting_2G:
        case  Wifi_Connecting:
            {
//#if defined Y900 || defined Y901
                Wifi_mode_flag_enum WIFI_mode=WIFI_2G;
                bool Change_wifi_mode_successful=Change_Wifi_mode(i,WIFI_mode);
                if(Change_wifi_mode_successful==false)
                    break;
//#endif

                srand((unsigned)time(NULL));
                int rmData = rand() % 1000 + 1000;
                QString ssid = QString("SSID_%1").arg(rmData);
                sec_info_rw_enum SSID_W=AP1_SEC_INFO_WRITE;
                bool success = DIAG_WriteSSID(i,ssid.toLatin1().data(),SSID_W);
                if(success==false)
                    break;
                SLEEP(2000);
                sec_info_rw_enum Disable_Password_flag=AP1_SEC_INFO_WRITE;
                bool Disable_success = DIAG_DisableSecurity(i,Disable_Password_flag);
                if(Disable_success==false)
                    break;

                if(success)
                {
                    SLEEP(2000);
                    bool restartOk = DIAG_RestartWiFi(i);
                    if(restartOk)
                    {
                        SLEEP(3000);

                        QString msg = QString("please connect  %1").arg(ssid);

                        emit signalPopupMsgWithOK("Wifi connect test",msg);

                        while(true)
                        {
                            if(m_mainWindow->getIfUsrRsp())
                            {
                                break;
                            }
                            SLEEP(100);
                        }
                        m_mainWindow->restUsrRsp();
#ifdef W800LZ_TEST
                        emit signalPopupMsgWithYesNo("WIFI Test","The WIFI connect is OK?");

                        while(true)
                        {
                            if(m_mainWindow->getIfUsrRsp())
                            {
                                break;
                            }
                            SLEEP(100);
                        }

                        if(m_mainWindow->getIfOK())
                        {
                            emit signalDisplayTestResult(i,"WIFI is OK!",true);
                        }
                        else
                        {
                            emit signalDisplayTestResult(i,"WIFI is not OK!",false);
                        }
#else


                        DIAG_CheckStaConnect(i);
#endif
                    }
                }

            }
            break;
        case  Wifi_Connecting_5G:
            {
//#if defined Y900 || defined Y901

                Wifi_mode_flag_enum WIFI_mode=WIFI_5G;
                bool Change_wifi_mode_successful=Change_Wifi_mode(i,WIFI_mode);
                if(Change_wifi_mode_successful==false)
                    break;
//#endif

                srand((unsigned)time(NULL));
                int rmData = rand() % 1000 + 1000;
                QString ssid = QString("SSID_%1").arg(rmData);
                sec_info_rw_enum SSID_W=AP2_SEC_INFO_WRITE;
                bool success = DIAG_WriteSSID(i,ssid.toLatin1().data(),SSID_W);
                if(success==false)
                    break;
                SLEEP(2000);
                sec_info_rw_enum Disable_Password_flag=AP2_SEC_INFO_WRITE;
                bool Disable_success = DIAG_DisableSecurity(i,Disable_Password_flag);

                if(Disable_success==false)
                    break;
                SLEEP(2000);
                if(success)
                {
                    SLEEP(2000);
                    bool restartOk = DIAG_RestartWiFi(i);
                    if(restartOk)
                    {
                        SLEEP(3000);

                        QString msg = QString("please connect 5G %1").arg(ssid);

                        emit signalPopupMsgWithOK("Wifi connect test",msg);

                        while(true)
                        {
                            if(m_mainWindow->getIfUsrRsp())
                            {
                                break;
                            }
                            SLEEP(100);
                        }
                        m_mainWindow->restUsrRsp();

                        DIAG_CheckStaConnect(i);
                    }
                }
            }
            break;
        case Side_Key_Checking:
            {
                bool isOk = DIAG_ENTRYKEYTEST(i);
                if(isOk)
                {
                    bool get_result=DIAG_KEYTEST(i);
                    //if(get_result==false)
                        //break;
                    DIAG_EXITKEYTEST(i);
                }
            }
            break;
        case Firmware_Version:
            {
                char *FirmVer="";
                DIAG_ReadFIRMWAREVersion(i,FirmVer);
            }
            break;

        case SSID_Read:
            {
                char *SSID="";
                DIAG_ReadSSID(i,SSID);

            }
            break;
        case IMEI_Read:
            {
                char *IMEI="";
                DIAG_ReadIMEI(i,IMEI);

            }
            break;

        case MAC_Read:
            {
                char *MAC="";

                DIAG_MAC_READ(i,MAC);

            }
            break;
        case Activate_Sim_Lock:
            {
                DIAG_Activate_Sim_Lock(i);
            }

            break;
        case Erase_Sim_Lock:
            {
                DIAG_Erase_Sim_Lock(i);
            }

            break;

        case Sd_Card:
            {
                DIAG_CheckSD_Card(i);
            }
            break;
        case SIM_Card:
            {
                DIAG_CheckSIM_Card(i);
            }
            break;
        case Network_Register_Checking:
            {
                DIAG_CheckNetWork(i);
            }
            break;
        case OLED_Screen:
            {
                DIAG_EnterLCDScreenTEST(i);
                bool isSuccess = DIAG_LCDScreen_Turn_On(i);

                if(isSuccess)
                {
                    emit signalPopupMsgWithYesNo("OLED Test","The screen is OK?");

                    while(true)
                    {
                        if(m_mainWindow->getIfUsrRsp())
                        {
                            break;
                        }
                        SLEEP(100);
                    }

                    if(m_mainWindow->getIfOK())
                    {
                        emit signalDisplayTestResult(i,"OLED test OK!",true);
                    }
                    else
                    {
                        emit signalDisplayTestResult(i,"OLED test fail!",false);
                    }
                    m_mainWindow->restUsrRsp();
                    bool OLED_turn_off=DIAG_LCDScreen_Turn_Off(i);
                    if(OLED_turn_off==false)
                        break;
                }
                else
                    break;
                DIAG_EXITLCDScreenTEST(i);
            }
            break;

        case WIFI_PassWord_Read:
            {
                char *SSID="";
                DIAG_PassWord_Read(i,SSID);

            }
            break;

        case WIFI_PassWord_Write:
            {
                emit signalOnPopupMsgwithWriteWIFI_PassWord("Write WIFI PassWord","WIFI PassWord:");
                while(true)
                {
                    if(Is_Wifi_Password_Write)
                    {
                        emit signalDisplayTestResult(i,"WIFI_PassWord Write successfully!",true);
                        m_mainWindow->restUsrRsp();

                        break;
                    }
                    if(Is_Wifi_Password_Quit==true)
                    {
                        if(Is_Wifi_Password_Write==false)
                        {
                            emit signalDisplayTestResult(i,"WIFI_PassWord Write failed!",false);
                            m_mainWindow->restUsrRsp();
                            break;
                        }

                    }
                    else
                        SLEEP(100);
                }


            }
            break;
        case Single_LED_Test:
            {
                bool isSuccess = DIAG_EnterLEDTEST(i);
#ifdef Y900
                if(isSuccess)
                    isSuccess =DIAG_EnterLEDLight(i);    //is only for Y900
                else
                {
                    emit signalDisplayTestResult(i,"The enter Led test mode failed!",false);
                    m_mainWindow->restUsrRsp();
                    DIAG_EXITLEDTEST(i);
                    break;
                }
#endif
#ifdef W800LZ
                isSuccess =DIAG_BLUELEDTEST(i);
#endif

                if(isSuccess)
                {
                    emit signalPopupMsgWithYesNo("LED Test","The LED is OK?");

                    while(true)
                    {
                        if(m_mainWindow->getIfUsrRsp())
                        {
                            break;
                        }
                        SLEEP(100);
                    }

                    if(m_mainWindow->getIfOK())
                    {
                        emit signalDisplayTestResult(i,"The LED is OK!",true);
                    }
                    else
                    {
                        emit signalDisplayTestResult(i,"The LED is not OK!",false);
                    }

                    m_mainWindow->restUsrRsp();

                    DIAG_EXITLEDTEST(i);
                }
                else
                {
                    emit signalDisplayTestResult(i,"The Led Light failed!",false);
                    m_mainWindow->restUsrRsp();
                    DIAG_EXITLEDTEST(i);
                    break;
                }
            }
            break;
        case Charger_Test:
            {
                DIAG_Charger_TEST(i);
            }
            break;
        case LED_Test:
            {
                bool led_successful=DIAG_EnterLEDTEST(i);
                bool isOK = true;
                QString msg = "";
                if(led_successful==false)
                    break;
                //begin Red LED test

                for(int k = 0; k < itemNum; k++)
                {
                   led_successful = DIAG_LEDTEST(i,k);

                   emit signalPopupMsgWithYesNo(QString("%1 LED Test").arg(vecColorsValue.at(k)),QString("The %1 LED is OK?").arg(vecColorsValue.at(k)));

                   while(true)
                   {
                       if(m_mainWindow->getIfUsrRsp())
                       {
                           break;
                       }
                       SLEEP(100);
                   }

                   if(m_mainWindow->getIfOK())
                   {
                       //emit signalDisplayTestResult(i,"Red LED Test OK!",true);
                       msg.append(QString("%1 LED OK,").arg(vecColorsValue.at(k)));
                       isOK &= true;
                   }
                   else
                   {
                       //emit signalDisplayTestResult(i,"Red LED Test fail!",false);
                       msg.append(QString("%1 LED not OK,").arg(vecColorsValue.at(k)));
                       isOK &= false;
                   }
                   m_mainWindow->restUsrRsp();

                }
//                led_successful=DIAG_REDLEDTEST(i);

//                emit signalPopupMsgWithYesNo("Red LED Test","The Red LED is OK?");

//                while(true)
//                {
//                    if(m_mainWindow->getIfUsrRsp())
//                    {
//                        break;
//                    }
//                    SLEEP(100);
//                }

//                if(m_mainWindow->getIfOK())
//                {
//                    //emit signalDisplayTestResult(i,"Red LED Test OK!",true);
//                    msg.append("Red LED OK,");
//                    isOK &= true;
//                }
//                else
//                {
//                    //emit signalDisplayTestResult(i,"Red LED Test fail!",false);
//                    msg.append("Red LED not OK,");
//                    isOK &= false;
//                }
//                m_mainWindow->restUsrRsp();

//                led_successful=DIAG_BLUELEDTEST(i);

//                emit signalPopupMsgWithYesNo("Blue LED Test","The Blue LED is OK?");

//                while(true)
//                {
//                    if(m_mainWindow->getIfUsrRsp())
//                    {
//                        break;
//                    }
//                    SLEEP(100);
//                }

//                if(m_mainWindow->getIfOK())
//                {


//                    msg.append("Blue LED OK,");

//                    isOK &= true;
//                }
//                else
//                {


//                    msg.append("Blue LED not OK,");

//                    isOK &= false;
//                }
//                m_mainWindow->restUsrRsp();

//                led_successful=DIAG_YELLOWLEDTEST(i);
//                if(led_successful==false)
//                    break;
//                emit signalPopupMsgWithYesNo("LED Test","The Green LED is OK?");
//                while(true)
//                {
//                    if(m_mainWindow->getIfUsrRsp())
//                    {
//                        break;
//                    }
//                    SLEEP(100);
//                }

//                if(m_mainWindow->getIfOK())
//                {


//                    msg.append("Yellow LED is OK!");

//                    isOK &= true;
//                }
//                else
//                {

//                    msg.append("Yellow LED not OK,");
//                    isOK &= false;
//                }
//                m_mainWindow->restUsrRsp();



                //end Green LED test

                emit signalDisplayTestResult(i,msg,isOK);

                DIAG_EXITLEDTEST(i);
            }
            break;
        case Three_LED_Test:
            {

                DIAG_EnterLEDTEST(i);
                //begin Red LED test
                DIAG_REDLEDTEST(i);
                emit signalPopupMsgWithYesNo("Red LED Test","The Red LED is OK?");

                bool isOK = true;
                QString msg = "";
                while(true)
                {
                    if(m_mainWindow->getIfUsrRsp())
                    {
                        break;
                    }
                    SLEEP(100);
                }

                if(m_mainWindow->getIfOK())
                {
                    //emit signalDisplayTestResult(i,"Red LED Test OK!",true);
                    msg.append("Red LED OK,");
                    isOK &= true;
                }
                else
                {
                    //emit signalDisplayTestResult(i,"Red LED Test fail!",false);
                    msg.append("Red LED not OK,");
                    isOK &= false;
                }
                m_mainWindow->restUsrRsp();
                //end red LED test

                //begin Green LED test
                DIAG_GREENLEDTEST(i);
                emit signalPopupMsgWithYesNo("Green LED Test","The Green LED is OK?");

                while(true)
                {
                    if(m_mainWindow->getIfUsrRsp())
                    {
                        break;
                    }
                    SLEEP(100);
                }

                if(m_mainWindow->getIfOK())
                {
                    // emit signalDisplayTestResult(i,"Green LED Test OK!",true);
                    msg.append("Green LED OK,");
                    isOK &= true;
                }
                else
                {
                    //emit signalDisplayTestResult(i,"Green LED Test fail!",false);
                    msg.append("Green LED not OK,");
                    isOK &= false;
                }
                m_mainWindow->restUsrRsp();
                //end Green LED test

                //begin Blue LED test
                DIAG_BLUELEDTEST(i);
                emit signalPopupMsgWithYesNo("Blue LED Test","The Blue LED is OK?");

                while(true)
                {
                    if(m_mainWindow->getIfUsrRsp())
                    {
                        break;
                    }
                    SLEEP(100);
                }

                if(m_mainWindow->getIfOK())
                {
                    //emit signalDisplayTestResult(i,"Blue LED Test OK!",true);
                    msg.append("Blue LED OK!");
                    isOK &= true;
                }
                else
                {
                    //emit signalDisplayTestResult(i,"Blue LED Test fail!",false);
                    msg.append("Blue LED not OK!");
                    isOK &= false;
                }
                m_mainWindow->restUsrRsp();
                //end Blue LED test
                emit signalDisplayTestResult(i,msg,isOK);
                DIAG_EXITLEDTEST(i);
            }
            break;
        case Screen_Test:
            {
                bool result=false;
                screen_msg_id which_item=JRD_SCREEN_ENTRY_MSG_ID;
                bool isOK = true;
                QString msg = "";

                if(result=DIAG_Screen_TEST(which_item))
                {
                    which_item=JRD_SCREEN_TURN_RED_MSG_ID;
                    //begin Red screen test
                    if(result=DIAG_Screen_TEST(which_item))
                        emit signalPopupMsgWithYesNo("Red Screen Test","The Red Screen is OK?");

                    while(true)
                    {
                        if(m_mainWindow->getIfUsrRsp())
                        {
                            break;
                        }
                        SLEEP(100);
                    }
                    if(m_mainWindow->getIfOK())
                    {
                        msg.append("Red Screen OK,");
                        isOK &= true;
                    }
                    else
                    {
                        msg.append("Red Screen not OK,");
                        isOK &= false;
                    }
                    m_mainWindow->restUsrRsp();
                    //end red LED test
                    //begin Green LED test
                    which_item=JRD_SCREEN_TURN_GREEN_MSG_ID;
                    DIAG_Screen_TEST(which_item);
                    emit signalPopupMsgWithYesNo("Green Screen Test","The Green Screen is OK?");
                    while(true)
                    {
                        if(m_mainWindow->getIfUsrRsp())
                        {
                            break;
                        }
                        SLEEP(100);
                    }
                    if(m_mainWindow->getIfOK())
                    {
                        msg.append("Green Screen OK,");
                        isOK &= true;
                    }
                    else
                    {
                        msg.append("Green Screen not OK,");
                        isOK &= false;
                    }
                    m_mainWindow->restUsrRsp();
                    //end Green LED test
                    //begin Blue LED test
                    which_item=JRD_SCREEN_TURN_BLUE_MSG_ID;
                    DIAG_Screen_TEST(which_item);
                    emit signalPopupMsgWithYesNo("Blue Screen Test","The Blue Screen is OK?");
                    while(true)
                    {
                        if(m_mainWindow->getIfUsrRsp())
                        {
                            break;
                        }
                        SLEEP(100);
                    }
                    if(m_mainWindow->getIfOK())
                    {
                        msg.append("Blue Screen OK!");
                        isOK &= true;
                    }
                    else
                    {
                        msg.append("Blue Screen not OK!");
                        isOK &= false;
                    }
                }
                else
                {
                    msg.append("enter test mode failed!");
                    isOK &= false;

                }
                m_mainWindow->restUsrRsp();
                //end Blue LED test
                emit signalDisplayTestResult(i,msg,isOK);
                which_item=JRD_SCREEN_EXIT_MSG_ID;
                DIAG_Screen_TEST(which_item);
            }
            break;
            default:
            break;
        }
    } 
    //DisConnectMS_NV();
    emit signalTestOver();
}


bool TestThread::DIAG_Erase_Sim_Lock(int testIndex)
{
    jrd_diag_sim_lock_req_type req;
    jrd_diag_sim_lock_req_type resp;
    short resp_len = sizeof(jrd_diag_screen_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SIM_LOCK;
    req.lock = E_JRD_DIAG_PERI_SIM_SWITCH ;

    for(int i = 0; i < RETRY_TIMES; i++)
    {

        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_sim_lock_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if( i == RETRY_TIMES - 1)
          {
               ERROR(FILE_LINE,"Erase Sim Lock send command error");
               emit signalDisplayTestResult(testIndex,"Erase Sim Lock send command error",false);
               return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"Erase Sim Lock send command OK");
          emit signalDisplayTestResult(testIndex,"Erase Sim Lock successful",true);
          break;
      }

    }
/*
    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"Erase Sim Lock response error");
        emit signalDisplayTestResult(testIndex,"Erase Sim Lock response error",false);
        return FALSE;
    }
    */

    return TRUE;

}

bool TestThread::DIAG_Activate_Sim_Lock(int testIndex)
{
    jrd_diag_sim_lock_req_type req;
    jrd_diag_sim_lock_req_type resp;
    short resp_len = sizeof(jrd_diag_screen_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SIM_LOCK;
    req.lock = E_JRD_DIAG_PERI_SIM_LOCK ;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_sim_lock_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if( i == RETRY_TIMES - 1)
          {
               ERROR(FILE_LINE,"Activate Sim Lock send command error");
               emit signalDisplayTestResult(testIndex,"Activate Sim Lock send command error",false);
               return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"Activate Sim Lock send command OK");
          emit signalDisplayTestResult(testIndex,"Activate Sim Lock successful",true);
          break;
      }

    }

    return TRUE;


}

bool TestThread::DIAG_PassWord_Read(int testIndex,char *Password)
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
        emit signalDisplayTestResult(testIndex,"Password read send command error",false);
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
            Security+=Encryption+"Password: "+PassWord;


        emit signalDisplayTestResult(testIndex,Security,true);
    }
    else
    {
        ERROR(FILE_LINE,"Password read response error");
        emit signalDisplayTestResult(testIndex,"Password read response error",false);
        return FALSE;
    }

    return TRUE;
}
bool TestThread::DIAG_MAC_READ(int testIndex,char *MAC)
{
    jrd_diag_wifi_mac_addr_wr_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_mac_addr_wr_req_rsp_type);
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
            emit signalDisplayTestResult(testIndex,"MAC read send command error",false);
            return FALSE;
        }

        if(resp.diag_errno == 0)
        {
            sprintf(MAC,"%02x:%02x:%02x:%02x:%02x:%02x",resp.mac_addr[0],resp.mac_addr[1],resp.mac_addr[2],resp.mac_addr[3],resp.mac_addr[4],resp.mac_addr[5]);

            QString temp1;
            temp1=QString::fromLocal8Bit(MAC);
            qDebug()<<temp1;

            emit signalDisplayTestResult(testIndex,temp1,true);
            qDebug()<<temp1;

        }
        else
        {
            ERROR(FILE_LINE,"MAC read response error");
            emit signalDisplayTestResult(testIndex,"MAC read response error",false);
            return FALSE;
        }
        return TRUE;
}
bool TestThread::DIAG_ReadIMEI(int testIndex,char *IMEI)
{
        jrd_diag_sys_imei_write_req_rsp_type req,resp;
        short resp_len = sizeof(jrd_diag_sys_imei_write_req_rsp_type);

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
            emit signalDisplayTestResult(testIndex,"IMEI read send command error",false);
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

            emit signalDisplayTestResult(testIndex,"IMEI is "+temp2,true);
        }
        else
        {
            ERROR(FILE_LINE,"IMEI read response error");
            emit signalDisplayTestResult(testIndex,"IMEI read response error",false);
            return FALSE;
        }


        //emit signalDisplayTestResult(testIndex,"ssid read successfull",false);
        return TRUE;
}


bool TestThread::DIAG_ReadSSID(int testIndex,char *ssid)
{
        jrd_diag_wifi_ssid_wr_req_rsp_type req,resp;
        short resp_len = sizeof(jrd_diag_wifi_ssid_wr_req_rsp_type);

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
            emit signalDisplayTestResult(testIndex,"ssid read send command error",false);
            return FALSE;
        }

        if(resp.diag_errno == 0)
        {
            memcpy(ssid,resp.ssid,MAX_SSID_LEN+1);
            ssid[strlen(ssid)] = '\0';
            QString SSID;
            SSID=QString::fromLocal8Bit(ssid);

            emit signalDisplayTestResult(testIndex,"SSID is "+SSID,true);
        }
        else
        {
            ERROR(FILE_LINE,"ssid read response error");
            emit signalDisplayTestResult(testIndex,"ssid read response error",false);
            return FALSE;
        }


        //emit signalDisplayTestResult(testIndex,"ssid read successfull",false);
        return TRUE;
}

int TestThread::ConnectMS_NV(void)
{
    int i=0;
    int status = 0;
    //char itemBuf[256]="";
    char resultBuf[256]="";
    unsigned char _bUserSelectQPST = FALSE;//TRUE;//FALSE;
    //uint16 iPortNum;
    //uint16 pPortList[30];
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


    for(i = 0; i < 10;  i++)
    {
        uint16 ComId[32] = {0};
        uint16 count = 0;
        CDeviceList devlist;
        count = devlist.GetComPortList(PORTTYPE_DIAG, ComId);
        testPort = ComId[0];

        //INFO(FILE_LINE, "in ConnectMS_NV testPort = %d",testPort);

        //TPort_GetAvailableDIAGPortList(&iPortNum, pPortList);//20090202
        if(count > 0)
        {
            comport = ComId[0];
            //ui->textComport->setText(QString("COM%1").arg(comport));
            break;
        }
        Sleep(500);
        sprintf(resultBuf,"ConnectMS_NV search com:%d",comport);
    }

    m_QHandle = QLIB_ConnectServer(comport);
    if(m_QHandle == NULL)
    {
        ERROR(FILE_LINE, "in ConnectMS_NV, m_QHandle == NULL");
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

    for(i=0;i < 5;i++)
    {
        status = 0;
        status = QLIB_IsPhoneConnected(m_QHandle);
        if(status)
        {
           // INFO(FILE_LINE, "in ConnectMS_NV, phone is connected");
            break;
        }
        else
        {
            //File_WriteLog("connect phone fail! 2");
            return 0;
        }
    }
    return 1;
}

int TestThread::DisConnectMS_NV(void)
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

    return 1;
}

bool TestThread::DIAG_WriteSSID(int testIndex,char *ssid,sec_info_rw_enum RW)
{
    jrd_diag_wifi_ssid_wr_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_ssid_wr_req_rsp_type);
    if(ssid == NULL || strlen(ssid) > 32)
    {
        ERROR(FILE_LINE,"ssid string is NULL or the strlen is too long");
        emit signalDisplayTestResult(testIndex,"ssid string is NULL or the strlen is too long",false);
        return FALSE;
    }

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_SSID_RW;

    req.rw_flag = RW;
    memcpy(req.ssid,ssid,MAX_SSID_LEN+1);

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                    m_QHandle,
                    sizeof(jrd_diag_wifi_ssid_wr_req_rsp_type),
                    (unsigned char *)&req,
                    &resp_len,
                    (unsigned char *)&resp,
                    10000) != TRUE)
        {
            if(i == RETRY_TIMES - 1)
            {
                ERROR(FILE_LINE,"ssid write send command error!");
                emit signalDisplayTestResult(testIndex,"ssid write send command error",false);
                m_mainWindow->restUsrRsp();

                return FALSE;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            INFO(FILE_LINE,"ssid write send command ok,the ssid = %s",ssid);
            emit signalDisplayTestResult(testIndex,"ssid write send successfully!",true);
            m_mainWindow->restUsrRsp();

            break;
        }
    }

    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"ssid write response error!");
        emit signalDisplayTestResult(testIndex,"ssid write response error!",false);
        m_mainWindow->restUsrRsp();
        return FALSE;
    }

    return TRUE;
}

bool TestThread::Enter_test_mode(int testIndex)
{
    jrd_diag_wifi_set_testmode_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_set_testmode_req_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_SET_TESTMODE;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_wifi_set_testmode_req_rsp_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if( i == RETRY_TIMES - 1)
          {
               ERROR(FILE_LINE,"enter test mode send command error!");
               //QMessageBox::information(NULL, "ReliabilityTool", "Send Command Failed!",QMessageBox::Ok);
               emit signalDisplayTestResult(testIndex,"enter test mode send command error!",false);
               return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"enter test mode send command OK!");
          break;
      }
    }

    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"enter test mode read response error!");
        //QMessageBox::information(NULL, "ReliabilityTool", "Device response failed!",QMessageBox::Ok);
        emit signalDisplayTestResult(testIndex,"enter test mode read response error!",false);
        return FALSE;
    }

    INFO(FILE_LINE,"enter test mode read response ok!");
    emit signalDisplayTestResult(testIndex,"enter test mode successfully!",true);
    return TRUE;
}

bool TestThread::Change_Wifi_mode(int testIndex,Wifi_mode_flag_enum WIFI_mode)
{
    jrd_diag_wifi_mode_wr_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_mode_wr_req_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_MODE_RW;

    req.rw_flag = MODE_WRITE;
    req.mode=WIFI_mode;


    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                    m_QHandle,
                    sizeof(jrd_diag_wifi_ssid_wr_req_rsp_type),
                    (unsigned char *)&req,
                    &resp_len,
                    (unsigned char *)&resp,
                    10000) != TRUE)
        {
            if(i == RETRY_TIMES - 1)
            {
                ERROR(FILE_LINE,"Change WIFI mode send command error!");
                emit signalDisplayTestResult(testIndex,"Change WIFI mode send command error",false);
                m_mainWindow->restUsrRsp();

                return FALSE;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            //INFO(FILE_LINE,"ssid write send command ok,the ssid = %s",ssid);
            emit signalDisplayTestResult(testIndex,"Change WIFI mode send successfully!",true);
            m_mainWindow->restUsrRsp();

            break;
        }
    }

    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"ssid write response error!");
        emit signalDisplayTestResult(testIndex,"ssid write response error!",false);
        m_mainWindow->restUsrRsp();
        return FALSE;
    }

    return TRUE;
}

bool TestThread::DIAG_WriteImei(int testIndex, char *IMEI)
{

    jrd_diag_sys_imei_write_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_sys_imei_write_req_rsp_type);

    if(IMEI == NULL )
    {
        ERROR(FILE_LINE,"IMEI string is NULL");
        return FALSE;
    }

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
                    10000) != TRUE)
        {
            if(i == RETRY_TIMES - 1)
            {
                ERROR(FILE_LINE,"IMEI write send command error!");
                emit signalDisplayTestResult(testIndex,"IMEI write send command error",false);
                m_mainWindow->restUsrRsp();

                return FALSE;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            INFO(FILE_LINE,"IMEI write send command ok,the ssid = %s",IMEI);
            emit signalDisplayTestResult(testIndex,"IMEI write send successfully!",true);
            m_mainWindow->restUsrRsp();
            break;
        }
    }
    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"IMEI write response error!");
        emit signalDisplayTestResult(testIndex,"IMEI write response error!",false);
        m_mainWindow->restUsrRsp();
        return FALSE;
    }
    return TRUE;
}

bool TestThread::DIAG_WriteMac(int testIndex, char *MAC)
{
    jrd_diag_wifi_mac_addr_wr_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_mac_addr_wr_req_rsp_type);

    if(MAC == NULL )
    {
        ERROR(FILE_LINE,"MAC string is NULL");
        return FALSE;
    }
    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_MAC_RW;
    req.rw_flag = E_JRD_WRITE;

    sscanf(MAC, "%02x:%02x:%02x:%02x:%02x:%02x",
                               &req.mac_addr[0], &req.mac_addr[1], &req.mac_addr[2], &req.mac_addr[3], &req.mac_addr[4], &req.mac_addr[5]);

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                    m_QHandle,
                    sizeof(jrd_diag_wifi_mac_addr_wr_req_rsp_type),
                    (unsigned char *)&req,
                    &resp_len,
                    (unsigned char *)&resp,
                    10000) != TRUE)
        {
            if(i == RETRY_TIMES - 1)
            {
                ERROR(FILE_LINE,"MAC write send command error!");
                emit signalDisplayTestResult(testIndex,"MAC write send command error",false);
                m_mainWindow->restUsrRsp();

                return FALSE;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            INFO(FILE_LINE,"MAC write send command ok,the ssid = %s",MAC);
            emit signalDisplayTestResult(testIndex,"MAC write send successfully!",true);
            m_mainWindow->restUsrRsp();
            break;
        }
    }
    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"MAC write response error!");
        emit signalDisplayTestResult(testIndex,"MAC write response error!",false);
        m_mainWindow->restUsrRsp();
        return FALSE;
    }
    return TRUE;
}

bool TestThread::DIAG_RestartWiFi(int testIndex)
{
    jrd_diag_wifi_switch_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_switch_req_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_ONOFF;

    req.op= AP_RESTART;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_wifi_switch_req_rsp_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if( i == RETRY_TIMES - 1)
          {
               ERROR(FILE_LINE,"wifi restart send command error!");
               emit signalDisplayTestResult(testIndex,"wifi restart send command error!",false);
               return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"wifi restart send command OK!");
          break;
      }
    }

    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"wifi restart read response error!");
        emit signalDisplayTestResult(testIndex,"wifi restart read response error!",false);
        return FALSE;
    }

    INFO(FILE_LINE,"wifi restart read response ok!");
    return TRUE;
}

bool TestThread::DIAG_DisableSecurity(int testIndex,sec_info_rw_enum Disable_Password_flag)
{
    jrd_diag_wifi_sec_wr_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_sec_wr_req_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_SECURITY_RW;

    req.rw_flag = Disable_Password_flag;   //AP1_SEC_INFO_WRITE;
    req.security_mode = SEC_DISABLED;
    //send_data 254 0 3 2 3 0
      if(QLIB_SendSync(
                m_QHandle,
                sizeof(jrd_diag_wifi_sec_wr_req_rsp_type),
                (unsigned char *)(&req),
                &resp_len,
                (unsigned char *)&resp,
                10000) != TRUE)
    {
        ERROR(FILE_LINE,"wifi disable security send command error");
        emit signalDisplayTestResult(testIndex,"wifi disable security send command error",false);
        return FALSE;
    }

    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"wifi disable security read response error");
        emit signalDisplayTestResult(testIndex,"wifi disable security read response error",false);
        return FALSE;
    }

    return TRUE;
}

bool TestThread::DIAG_CheckStaConnect(int testIndex)
{
    jrd_diag_wifi_connect_sta_req_type req;
    jrd_diag_wifi_connect_sta_rsp_type resp;
    short resp_len = sizeof(jrd_diag_wifi_connect_sta_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_CONNECT_STATUS;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_wifi_connect_sta_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES - 1)
          {
                ERROR(FILE_LINE,"wifi check sta connect send command error!");
                emit signalDisplayTestResult(testIndex,"wifi check connect status send command error!",false);
                return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
            INFO(FILE_LINE,"wifi check sta connect send command ok!");
            break;
      }

    }

    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"wifi check sta connect  read response error");
        emit signalDisplayTestResult(testIndex,"wifi check connect status read response error!",false);
        return FALSE;
    }
    if(resp.con_sta_num != 0)
    {
        QString msg = QString("check wifi connect status successfull! the connect number is %1.").arg(resp.con_sta_num);
        INFO(FILE_LINE,"check wifi connect status successfull! the connect number is %d",resp.con_sta_num);
        emit signalDisplayTestResult(testIndex,msg,true);
        return TRUE;
    }
    else
    {
        emit signalDisplayTestResult(testIndex,"wifi check connect status fail!",false);
        ERROR(FILE_LINE,"wifi check connect status fail!");
        return FALSE;
    }
}

bool TestThread::DIAG_Charger_TEST(int testIndex)
{
    jrd_diag_charger_status_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_charger_status_req_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_CHARGER_STATUS;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_charger_status_req_rsp_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES - 1)
          {
              ERROR(FILE_LINE,"charger test send command error");
              emit signalDisplayTestResult(testIndex,"charger test send command error!",false);
              return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"charger test send command OK");
          break;
      }

    }

    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"charger test read response error");
        emit signalDisplayTestResult(testIndex,"charger test read response error!",false);
        return FALSE;
    }

    emit signalDisplayTestResult(testIndex,"charger test successful!",true);
    return TRUE;
}

bool TestThread::DIAG_EnterLCDScreenTEST(int testIndex)
{
    jrd_diag_screen_req_type req;
    jrd_diag_screen_rsp_type resp;
    short resp_len = sizeof(jrd_diag_screen_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SCREEN;
    req.event_type = ENTRY_LCD_TEST ;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_screen_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES - 1)
          {
              ERROR(FILE_LINE,"entry screen send command error");
              emit signalDisplayTestResult(testIndex,"entry screen send command error!",false);
              return FALSE;
          }
          else
          {
              SLEEP(200);
          }  
      }
      else
      {
          INFO(FILE_LINE,"entry screen send command OK");
          break;
      }

    }

    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"entry screen response error");
        emit signalDisplayTestResult(testIndex,"entry screen response error!",false);
        return FALSE;
    }

    //emit signalDisplayTestResult(testIndex,"entry screen successful",true);
    return TRUE;
}

bool TestThread::DIAG_LCDScreen_Turn_On(int testIndex)
{
    jrd_diag_screen_req_type req;

    jrd_diag_screen_rsp_type resp;
    short resp_len = sizeof(jrd_diag_screen_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SCREEN;
    req.event_type = LCD_TURN_ON ;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_screen_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if( i == RETRY_TIMES - 1)
          {
              ERROR(FILE_LINE,"screen turn on send command error");
              emit signalDisplayTestResult(testIndex,"screen turn on send command error",false);
              return FALSE;
          }
          else
          {
               SLEEP(200);
          }

      }
      else
      {
          INFO(FILE_LINE,"screen turn on send command OK");
          break;
      }

    }

    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"screen turn on response error");
        emit signalDisplayTestResult(testIndex,"screen turn on response error",false);
        return FALSE;
    }

    emit signalDisplayTestResult(testIndex,"screen turn successful",true);
    return TRUE;
}

bool TestThread::DIAG_LCDScreen_Turn_Off(int testIndex)
{
    jrd_diag_screen_req_type req;
    jrd_diag_screen_rsp_type resp;
    short resp_len = sizeof(jrd_diag_screen_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SCREEN;
    req.event_type = LCD_TURN_OFF ;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_screen_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if( i == RETRY_TIMES - 1)
          {
               ERROR(FILE_LINE,"screen turn off send command error");
               emit signalDisplayTestResult(testIndex,"screen turn off send command error",false);
               return FALSE;
          }
          else
          {
              SLEEP(200);
          }    
      }
      else
      {
          INFO(FILE_LINE,"screen turn off send command OK");
          break;
      }

    }

    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"screen turn off response error");
        emit signalDisplayTestResult(testIndex,"screen turn off response error",false);
        return FALSE;
    }

    //emit signalDisplayTestResult(testIndex,"screen turn off successful",true);
    return TRUE;
}

bool TestThread::DIAG_EXITLCDScreenTEST(int testIndex)
{
    jrd_diag_screen_req_type req;
    jrd_diag_screen_rsp_type resp;
    short resp_len = sizeof(jrd_diag_screen_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SCREEN;
    req.event_type = EXIT_LCD_TEST ;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_screen_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES -1)
          {
              ERROR(FILE_LINE,"exit screen send command error");
              emit signalDisplayTestResult(testIndex,"exit screen send command error",false);
              return FALSE;
          }
          else
          {
              SLEEP(200);
          }

      }
      else
      {
           INFO(FILE_LINE,"exit screen send command OK");
           break;
      }
    }

    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"exit screen response error");
        emit signalDisplayTestResult(testIndex,"exit screen response error",false);
        return FALSE;
    }

    //emit signalDisplayTestResult(testIndex,"exit screen successful",true);
    return TRUE;
}

bool TestThread::Read_Power_level_File(int testIndex)
{
    bool Write_result=false;
    jrd_diag_wifi_dB_gain_adjust req;
    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_MODE_RW;
    QFile *file=new QFile("./Wifi_Power.ini");
    if(file->open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QString data = QString(file->readAll());
        int temp_location=0;
        int line_end_location=0;
        int end_flag=data.indexOf("end",0);
        int next_line_end=0;
        for(int i=0;i<end_flag;end_flag=line_end_location+1)
        {
            //temp_location=line_end_location;

            if((next_line_end=data.indexOf("\n",line_end_location))!=-1);
            {
                bool ok;
                QString value=data.mid(line_end_location,next_line_end-line_end_location);
                int Ant_value_location=value.indexOf(",",temp_location);
                if(Ant_value_location!=-1)
                {
                    QString Ant_value=value.mid(0,Ant_value_location);
                    int Ant=Ant_value.toInt(&ok,10);
                    if(ok)
                    {
                        req.AT_flag=Ant;
                    }
                    else
                        break;
                }
                else
                    break;
                Ant_value_location=Ant_value_location+1;
                int channel_value_location=value.indexOf(",",Ant_value_location);
                if(channel_value_location!=-1)
                {
                    QString channel_value=value.mid(Ant_value_location,channel_value_location-Ant_value_location);
                    int channel=channel_value.toInt(&ok,10);
                    if(ok)
                    {
                        req.wifi_channel=channel;
                    }
                    else
                        break;
                }
                else
                    break;
                channel_value_location=channel_value_location+1;
                if(value.size()-channel_value_location>0)
                {
                    QString channel_value=value.mid(channel_value_location,value.size()-channel_value_location);
                    int gain_value=channel_value.toInt(&ok,10);
                    if(ok)
                    {
                        req.gain_value=gain_value;
                    }
                    else
                        break;
                }
                else
                    break;
                line_end_location=next_line_end+1;
                Write_result=Write_Wifi_Power_level(testIndex,req);
                if(Write_result==false)
                    break;
            }
        }
    }

    else
        emit signalDisplayTestResult(testIndex,"Open the file \"Wifi_Power.ini\" failed!",false);
    return Write_result;
}
bool TestThread::save_wifi_power(int testIndex)
{
    jrd_diag_wifi_gain_file_w_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_gain_file_w_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_GAIN_FILE_W;
    req.W_file_flag=MODE_WRITE;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_wifi_gain_file_w_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if( i == RETRY_TIMES - 1)
          {
               ERROR(FILE_LINE,"save Wifi power level send command error!");
               emit signalDisplayTestResult(testIndex,"save Wifi power level send command error!",false);
               return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"Save Wifi Power level OK!");
          break;
      }
    }

    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"enter test mode read response error!");
        //QMessageBox::information(NULL, "ReliabilityTool", "save Wifi power response failed!",QMessageBox::Ok);
        emit signalDisplayTestResult(testIndex,"save Wifi power level response error!",false);
        return FALSE;
    }

    INFO(FILE_LINE,"save Wifi power read response ok!");
    emit signalDisplayTestResult(testIndex,"Save Wifi power successfully!",true);
    return TRUE;

}

bool TestThread::Write_Wifi_Power_level(int testIndex,jrd_diag_wifi_dB_gain_adjust req)
{
    QString Channel=QString::number(req.wifi_channel,10);
    Channel="Channel="+Channel;
    jrd_diag_wifi_dB_gain_adjust resp;
    short resp_len = sizeof(jrd_diag_wifi_dB_gain_adjust);
    for(int i = 0; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_wifi_dB_gain_adjust),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if( i == RETRY_TIMES -1)
          {
              ERROR(FILE_LINE,"send write chanel %d power level command error!",req.wifi_channel);
              emit signalDisplayTestResult(testIndex,"send write  power level command failed!"+Channel,false);
              return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"write chanel %d power level successfully",req.wifi_channel);
          break;
      }
    }

    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"send write chanel %d power level response error",req.wifi_channel);
        emit signalDisplayTestResult(testIndex,"send write power level response error!"+Channel,false);
        return FALSE;
    }
    emit signalDisplayTestResult(testIndex,"Write power level successfully!"+Channel,false);
    return TRUE;

}

bool TestThread::DIAG_EnterLEDTEST(int testIndex)
{
    jrd_diag_led_req_type req;
    jrd_diag_led_rsp_type resp;
    short resp_len = sizeof(jrd_diag_led_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = ENTRY_LED_TEST ;
    req.LedLight0 = 0;
    req.LedLight1 = 0x00000000;
    //req.LedLight2 = 0;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_led_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if( i == RETRY_TIMES -1)
          {
              ERROR(FILE_LINE,"entry led send command error");
              emit signalDisplayTestResult(testIndex,"entry led send command error!",false);
              return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"entry led send command OK");
          break;
      }
    }

    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"entry led response error");
        emit signalDisplayTestResult(testIndex,"entry led response error!",false);
        return FALSE;
    }

    return TRUE;
}
bool TestThread::DIAG_Screen_TEST(screen_msg_id which_item)
{
    jrd_diag_Screen_req_type req={0};
    jrd_diag_screen_rsp_type resp={0};
    short resp_len = sizeof(jrd_diag_screen_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SCREEN;
    req.event_type = which_item ;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_Screen_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if( i == RETRY_TIMES -1)
          {
              ERROR(FILE_LINE,"entry led send command error comand= 254 0 2 8 %d",which_item);
              //emit signalDisplayTestResult(testIndex,"entry led send command error!",false);
              return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"entry led send command OK");
          break;
      }
    }

    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"entry led response error comand= 254 0 2 8 %d,diag_errno=d%",which_item,resp.diag_errno);
        //emit signalDisplayTestResult(testIndex,"entry led response error!",false);
        return FALSE;
    }

    return TRUE;
}
bool TestThread::DIAG_EnterLEDLight(int testIndex)
{

    jrd_diag_led_req_type req;
    jrd_diag_led_rsp_type resp;
    short resp_len = sizeof(jrd_diag_led_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = RED_LED_TEST ;
    req.LedLight1=0XFFFF;
    //req.LedLight2=0XFF;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_led_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if( i == RETRY_TIMES -1)
          {
              ERROR(FILE_LINE,"entry led send command error");
              emit signalDisplayTestResult(testIndex,"entry led send command error!",false);
              return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"entry led send command OK");
          break;
      }
    }

    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"entry led response error");
        emit signalDisplayTestResult(testIndex,"entry led response error!",false);
        return FALSE;
    }

    return TRUE;
}

bool TestThread::DIAG_EXITLEDTEST(int testIndex)
{
    jrd_diag_led_req_type req;
    jrd_diag_led_rsp_type resp;
    short resp_len = sizeof(jrd_diag_led_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    //req.event_type = EXIT_LED_TEST ;
    req.event_type = 0 ;
    req.LedLight0 = 0;
    req.LedLight1 = 0x00000000;
    //req.LedLight2 = 0;


    for(int i = 0; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_led_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {

            if(i == RETRY_TIMES - 1)
            {
                ERROR(FILE_LINE,"exit  led send command error");
                emit signalDisplayTestResult(testIndex,"exit  led send command error!",false);
                return FALSE;
            }
            else
            {
                SLEEP(200);
            }
      }
      else
      {
          INFO(FILE_LINE,"exit  led send command OK");
          break;
      }

    }

    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"exit led response error");
        emit signalDisplayTestResult(testIndex,"exit led response error!",false);
        return FALSE;
    }
    //OnDisplayTestResult(testIndex,"exit led successful",true);
    return TRUE;
}

bool TestThread::DIAG_REDLEDTEST(int testIndex)
{
    jrd_diag_led_req_type req;
    jrd_diag_led_rsp_type resp;
    short resp_len = sizeof(jrd_diag_led_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = 2 ;
    req.LedLight0 = 0;
    req.LedLight1 = 128;
    //req.LedLight1 = uiLedRed;
    //req.LedLight2 = 0;
        qDebug()<<"red============"<<req.LedLight1;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_led_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES - 1)
          {
              ERROR(FILE_LINE,"Red  led send command error");
              emit signalDisplayTestResult(testIndex,"Red led send command error!",false);
              return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"Red led send command ok");
          break;
      }

    }

    if(resp.diag_errno!= 0)

    {
        ERROR(FILE_LINE,"Red led response error");
        emit signalDisplayTestResult(testIndex,"Red led response error!",false);
        return FALSE;
    }

    //emit signalDisplayTestResult(testIndex,"red led test successful!",true);
    return TRUE;
}

bool TestThread::DIAG_LEDTEST(int testIndex,int ledNo)
{
    jrd_diag_led_req_type req;
    jrd_diag_led_rsp_type resp;
    short resp_len = sizeof(jrd_diag_led_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = 2 ;
    req.LedLight0 = 0;
    req.LedLight1 = vecUiLedDiagOrder.at(ledNo);

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_led_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES - 1)
          {

              ERROR(FILE_LINE,"%s led send command error",vecColorsValue.at(ledNo).data());
              emit signalDisplayTestResult(testIndex,QString("%1 led send command error!").arg(vecColorsValue.at(ledNo)),false);
              return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"%s led send command ok",vecColorsValue.at(ledNo).data());
          break;
      }

    }

    if(resp.diag_errno!= 0)

    {
        ERROR(FILE_LINE,"%s led response error",vecColorsValue.at(ledNo).data());
        emit signalDisplayTestResult(testIndex,QString("%1 led response error!").arg(vecColorsValue.at(ledNo)),false);
        return FALSE;
    }

    //emit signalDisplayTestResult(testIndex,"red led test successful!",true);
    return TRUE;
}

bool TestThread::DIAG_GREENLEDTEST(int testIndex)
{
#if defined Y854 || defined Y853
    jrd_leds_msg_general_req_type req;
    jrd_leds_msg_general_rsp_type resp;
    short resp_len = sizeof(jrd_leds_msg_general_req_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.LedLight_Order = 6 ;
#ifdef Y853
    req.leds_state=11807;
#else
    req.leds_state=12248;
#endif
    //req.Light_Switch=16;
#else

    jrd_diag_led_req_Y858_type req;
    jrd_diag_led_rsp_type resp;
    short resp_len = sizeof(jrd_diag_led_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = GREEN_LED_TEST ;
    req.LedLight_Order=LED1;
    req.Light_Switch=LED_LIGHT;

#endif

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  resp_len,
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES - 1)
          {
               ERROR(FILE_LINE,"Blue  led send command error");
               emit signalDisplayTestResult(testIndex,"Blue led send command error!",false);
               return FALSE;
          }
          else
          {
               SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"Blue  led send command OK");
          break;
      }

    }

    if(resp.diag_errno!= 0)

    {
        ERROR(FILE_LINE,"Blue led response error");
        emit signalDisplayTestResult(testIndex,"Blue led response error!",false);
        return FALSE;
    }



#if defined Y853 || defined Y854

#else




    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = GREEN_LED_TEST ;
    req.LedLight_Order=LED2;
    req.Light_Switch=LED_LIGHT;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_led_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES - 1)
          {
               ERROR(FILE_LINE,"Blue  led send command error");
               emit signalDisplayTestResult(testIndex,"Blue led send command error!",false);
               return FALSE;
          }
          else
          {
               SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"Blue  led send command OK");
          break;
      }

    }

    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"Blue led response error");
        emit signalDisplayTestResult(testIndex,"Blue led response error!",false);
        return FALSE;
    }
    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = GREEN_LED_TEST ;
    req.LedLight_Order=LED3;
    req.Light_Switch=LED_LIGHT;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_led_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES - 1)
          {
               ERROR(FILE_LINE,"Blue  led send command error");
               emit signalDisplayTestResult(testIndex,"Blue led send command error!",false);
               return FALSE;
          }
          else
          {
               SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"Blue  led send command OK");
          break;
      }

    }

    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"Blue led response error");
        emit signalDisplayTestResult(testIndex,"Blue led response error!",false);
        return FALSE;
    }
    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = GREEN_LED_TEST ;
    req.LedLight_Order=LED4;
    req.Light_Switch=LED_LIGHT;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_led_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES - 1)
          {
               ERROR(FILE_LINE,"Blue  led send command error");
               emit signalDisplayTestResult(testIndex,"Blue led send command error!",false);
               return FALSE;
          }
          else
          {
               SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"Blue  led send command OK");
          break;
      }

    }

    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"Blue led response error");
        emit signalDisplayTestResult(testIndex,"Blue led response error!",false);
        return FALSE;
    }
#endif




    //emit signalDisplayTestResult(testIndex,"Blue led test successful!",true);
    return TRUE;
}
bool TestThread::DIAG_YELLOWLEDTEST(int testIndex)
{

    jrd_diag_led_req_type req;

    jrd_diag_led_rsp_type resp;

    short resp_len = sizeof(jrd_diag_led_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = 2 ;
    req.LedLight0 = 0;
    req.LedLight1 = 64;
    //req.LedLight1 = uiLedGreen;
    //req.LedLight2 = 0;


    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_led_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES - 1)
          {
                ERROR(FILE_LINE,"Yellow led send command error");
                emit signalDisplayTestResult(testIndex,"Yellow led send command error",false);
                return FALSE;
          }
          else
          {
                SLEEP(200);
          }

      }
      else
      {
          INFO(FILE_LINE,"Yellow led send command ok");
          break;
      }

    }


    if(resp.diag_errno!= 0)

    {
        ERROR(FILE_LINE,"Yellow led response error");
        emit signalDisplayTestResult(testIndex,"Yellow led response error",false);
        return FALSE;
    }


    return TRUE;
}


bool TestThread::DIAG_BLUELEDTEST(int testIndex)
{

    jrd_diag_led_req_type req;

    jrd_diag_led_rsp_type resp;

    short resp_len = sizeof(jrd_diag_led_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.event_type = 2 ;
    req.LedLight0 = 0;
    req.LedLight1 = 0;
    //req.LedLight1 = uiLedBlue;
    //req.LedLight2 = 1;


    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_led_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES - 1)
          {
                ERROR(FILE_LINE,"blue led send command error");
                emit signalDisplayTestResult(testIndex,"blue led send command error",false);
                return FALSE;
          }
          else
          {
                SLEEP(200);
          }

      }
      else
      {
          INFO(FILE_LINE,"blue led send command ok");
          break;
      }

    }


    if(resp.diag_errno!= 0)

    {
        ERROR(FILE_LINE,"blue led response error");
        emit signalDisplayTestResult(testIndex,"blue led response error",false);
        return FALSE;
    }

    //emit signalDisplayTestResult(testIndex,"blue led test successful",true);
    return TRUE;
}

bool TestThread::DIAG_WRITELEDTEST(int testIndex)
{

    jrd_leds_msg_general_req_type req;
    jrd_leds_msg_general_rsp_type resp;
    short resp_len = sizeof(jrd_leds_msg_general_req_type);
    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_LED;
    req.LedLight_Order = 6 ;
    req.leds_state=49152;



    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                  m_QHandle,
                  resp_len,
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES - 1)
          {
                ERROR(FILE_LINE,"WRITE led send command error");
                emit signalDisplayTestResult(testIndex,"WRITE led send command error",false);
                return FALSE;
          }
          else
          {
                SLEEP(200);
          }

      }
      else
      {
          INFO(FILE_LINE,"WRITE led send command ok");
          break;
      }

    }


    if(resp.diag_errno!= 0)

    {
        ERROR(FILE_LINE,"blue led response error");
        emit signalDisplayTestResult(testIndex,"blue led response error",false);
        return FALSE;
    }

    //emit signalDisplayTestResult(testIndex,"blue led test successful",true);
    return TRUE;
}


bool TestThread::DIAG_ENTRYKEYTEST(int testIndex)
{
    jrd_diag_key_req_type req;
    jrd_diag_key_rsp_type resp;
    short resp_len = sizeof(jrd_diag_key_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_KEY;
    req.event_type= ENTRY_KEY_TEST ;

    for(int i = 0; i < RETRY_TIMES;i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_key_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
         if(i == RETRY_TIMES - 1)
         {
             ERROR(FILE_LINE,"entry key send command error");
             emit signalDisplayTestResult(testIndex,"entry key send command error!",false);
             return FALSE;
         }
         else
         {
             SLEEP(200);
         }


      }
     else
     {
         INFO(FILE_LINE,"entry key send command OK");
         break;
     }
    }

    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"entry key response error");
        emit signalDisplayTestResult(testIndex,"entry key response error!",false);
        return FALSE;
    }

    return TRUE;
}


bool TestThread::DIAG_KEYTEST(int testIndex)
{
    emit signalPopupMsgWithOK("Key Test","Please press the side key!");
    while(true)
    {
        if(m_mainWindow->getIfUsrRsp())
        {
             break;
        }
        else
         SLEEP(100);
    }
    m_mainWindow->restUsrRsp();

    jrd_diag_key_req_type req;
    jrd_diag_key_rsp_type resp;

    short resp_len = sizeof(jrd_diag_key_rsp_type);
    QTime Startime = QTime::currentTime();

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_KEY;
    req.event_type= QUERY_KEY_TEST ;
    bool Test_result=true;
    QString Str_result;
    QSettings *configIni = new QSettings("TestItem.ini", QSettings::IniFormat);
    int itemNum = configIni->value("/SideKey/Key_Nums", QVariant("0")).toString().toUInt();
    QString PowerKey_Value = configIni->value("/SideKey/PowerKey").toString();
    QString ResetKey_Value = configIni->value("/SideKey/ResetKey").toString();

    //bool is_Power_key=false;
    //bool is_Reset_key=false;
    do
    {
        QCoreApplication::processEvents();

        QLIB_SendSync(
                    m_QHandle,
                    sizeof(jrd_diag_key_req_type),
                    (unsigned char *)(&req),
                    &resp_len,
                    (unsigned char *)&resp,
                    1000);
        int timeCount = Startime.msecsTo(QTime::currentTime());
        if( timeCount >= (10*1000))
        {
            ERROR(FILE_LINE,"key test out of time");
            break;
        }

        if(PowerKey_Value=="yes"&&resp.key_index == Power_key)
        {
            if(itemNum==1)
            {
                Test_result=true;
                Str_result="Power_key test successful!";
                break;
            }
            else
            {
                Test_result=false;
                Str_result="Power_key test successful!Other key test failed!";
                //is_Power_key=true;
                //is_Reset_key=false;
                SLEEP(500);
                continue;
            }

        }

        else if(ResetKey_Value=="yes"&&resp.key_index == Reset_key)
        {
            if(itemNum==1)
            {
                Test_result=true;
                Str_result="Reset_key test successful!";
                break;
            }
            else
            {
                Test_result=false;
                Str_result="Reset_key test successful, Other key test failed!";
                //is_Power_key=false;
                //is_Reset_key=true;
                SLEEP(500);
                continue;
            }

        }
        else if(itemNum!=1&&resp.key_index == All_key)
        {
            Test_result=true;
            //is_Power_key=true;
            //is_Reset_key=true;
            //emit signalDisplayTestResult(testIndex,"Power key and Reset Key test successful!",true);
            INFO(FILE_LINE,"key test rsp successful!");
            break;
            //return TRUE;
        }
        else
        {
            Test_result=false;
            Str_result="All key test failed!";
            //is_Power_key=false;
            //is_Reset_key=true;
            SLEEP(500);
            continue;

        }

        SLEEP(200);
    }while(1);

    emit signalDisplayTestResult(testIndex,Str_result,Test_result);
    return TRUE;
/*
    if(is_Reset_key==true&&is_Power_key==false)
    {
        emit signalDisplayTestResult(testIndex,"Reset_key test successful, Power_key test failed!",false);
        INFO(FILE_LINE,"Reset_key test successful, Power_key test failed!");
        //SLEEP(6000);
        //break;
        return FALSE;
    }
    else

        if(is_Reset_key==false&&is_Power_key==true)
    {


        emit signalDisplayTestResult(testIndex,"Reset_key test failed, Power_key test successful!",false);
        INFO(FILE_LINE,"Reset_key test failed, Power_key test successful");
        return FALSE;

    }
    else

        if(is_Reset_key==true&&is_Power_key==true)
    {


        emit signalDisplayTestResult(testIndex,"All Keys test successful!",false);
        INFO(FILE_LINE,"All Keys test successful");
        return FALSE;

    }

    else
    {
        emit signalDisplayTestResult(testIndex,"All Keys test failed!",false);
        INFO(FILE_LINE,"Keys test failed!");
        return TRUE;
    }
    */

}

bool TestThread::DIAG_EXITKEYTEST(int testIndex)
{
    jrd_diag_key_req_type req;
    jrd_diag_key_rsp_type resp;
    short resp_len = sizeof(jrd_diag_key_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_KEY;
    req.event_type= EXIT_KEY_TEST ;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                  m_QHandle,
                  sizeof(jrd_diag_key_req_type),
                  (unsigned char *)(&req),
                  &resp_len,
                  (unsigned char *)&resp,
                  10000) != TRUE)
      {
          if(i == RETRY_TIMES -1)
          {
              ERROR(FILE_LINE,"exit key send command error");
              emit signalDisplayTestResult(testIndex,"exit key send command error!",false);
              return FALSE;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"exit key send command OK");
          break;
      }

    }

    if(resp.diag_errno!= 0)
    {
        ERROR(FILE_LINE,"exit key response error");
        emit signalDisplayTestResult(testIndex,"exit key response error!",false);
        return FALSE;
    }

    return TRUE;
}

bool TestThread::DIAG_ReadFIRMWAREVersion(int testIndex,char* FirmVer)
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
        ERROR(FILE_LINE,"firmware send command error");
        emit signalDisplayTestResult(testIndex,"firmware send command error",false);
        return FALSE;
    }

    if(resp.version_len!= 0)
    {
        memcpy(FirmVer,resp.version,strlen(resp.version));
        FirmVer[strlen(FirmVer)] = '\0';
        QString Firmware_Version;
        Firmware_Version=QString::fromLocal8Bit(FirmVer);

        emit signalDisplayTestResult(testIndex,"Firmware version is "+Firmware_Version,true);
    }
    else
    {
        ERROR(FILE_LINE,"firmware response error");
        emit signalDisplayTestResult(testIndex,"firmware response error",false);
        return FALSE;
    }

    return TRUE;
}

bool TestThread::DIAG_CheckSD_Card(int testIndex)
{
    jrd_diag_sdcard_status_req_rsp_type req;
    jrd_diag_sdcard_status_req_rsp_type resp;
    short resp_len = sizeof(jrd_diag_sdcard_status_req_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SD_CARD_STATUS;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                   m_QHandle,
                   sizeof(jrd_diag_sdcard_status_req_rsp_type),
                   (unsigned char *)(&req),
                   &resp_len,
                   (unsigned char *)&resp,
                   10000) != TRUE)
       {
           if(i == RETRY_TIMES - 1)
           {
                ERROR(FILE_LINE,"check SD Card send command error!");
                emit signalDisplayTestResult(testIndex,"check SD Card send command error!",false);
                return FALSE;
           }
           else
           {
               SLEEP(200);
           }
       }
       else
       {
           INFO(FILE_LINE,"check SD Card send command OK!");
           break;
       }
    }

    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"check SD Card  read response error!");
        emit signalDisplayTestResult(testIndex,"check SD Card  read response error!",false);
        return FALSE;
    }
    if(resp.sdcard_status == 1)
    {
        ERROR(FILE_LINE,"Didn't find SD Card!");
        emit signalDisplayTestResult(testIndex,"Didn't find SD Card!",false);
        return FALSE;
    }

    emit signalDisplayTestResult(testIndex,"check SD Card successful!",true);
    return TRUE;
}

bool TestThread::DIAG_CheckSIM_Card(int testIndex)
{
    jrd_diag_sim_status_req_type req;
    jrd_diag_sim_status_rsp_type resp;
    short resp_len = sizeof(jrd_diag_sim_status_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_PERI_SIM_STATUS;

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                   m_QHandle,
                   sizeof(jrd_diag_sim_status_req_type),
                   (unsigned char *)(&req),
                   &resp_len,
                   (unsigned char *)&resp,
                   10000) != TRUE)
       {
           if( i == RETRY_TIMES - 1)
           {
                ERROR(FILE_LINE,"check SIM send command error!");
                emit signalDisplayTestResult(testIndex,"check SIM send command error!",false);
                return FALSE;
           }
           else
           {
                SLEEP(200);
           }
       }
       else
       {
           INFO(FILE_LINE,"check SIM send command OK!");
           break;
       }
    }

     if(resp.diag_errno != 0)
     {
        ERROR(FILE_LINE,"resp.sim_status response error!");
        emit signalDisplayTestResult(testIndex,"check sim status response error!",false);
        return FALSE;
     }

    if(resp.status_len != 0)
    {
        if(strstr(resp.sim_status,"READY") == NULL)
        {
            ERROR(FILE_LINE,"resp.sim_status response didn't have READY!");
            QString msg = QString("sim is not ready,%1").arg(resp.sim_status);
            emit signalDisplayTestResult(testIndex,msg,false);
            return FALSE;
        }
    }
    else
    {
        ERROR(FILE_LINE,"check SIM  read response error");
        emit signalDisplayTestResult(testIndex,"check SIM  read response error!",false);
        return FALSE;
    }

    emit signalDisplayTestResult(testIndex,"check SIM status successful!",true);
    return TRUE;
}

bool TestThread::DIAG_CheckNetWork(int testIndex)
{
    jrd_diag_nw_register_status_req_type req;
    jrd_diag_nw_register_status_rsp_type resp;
    short resp_len = sizeof(jrd_diag_nw_register_status_rsp_type);

    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_NETWORK;
    req.hdr.cmd_code = E_JRD_DIAG_NW_REG_STATUS;

    for(int i = 0 ; i < RETRY_TIMES; i++)
    {
        //QCoreApplication::processEvents();
        if(QLIB_SendSync(
                   m_QHandle,
                   sizeof(jrd_diag_nw_register_status_req_type),
                   (unsigned char *)(&req),
                   &resp_len,
                   (unsigned char *)&resp,
                   10000) != TRUE)
       {
           if(i == RETRY_TIMES - 1)
           {
               ERROR(FILE_LINE,"check network send command error");
               emit signalDisplayTestResult(testIndex,"check network send command error!",false);
               return FALSE;
           }
           else
           {
               SLEEP(200);
           }
           INFO(FILE_LINE,"check network send command OK");
       }
    }

     if(resp.diag_errno != 0)
     {
        ERROR(FILE_LINE,"check network read response error!");
        emit signalDisplayTestResult(testIndex,"check network read response error!",false);
        return FALSE;
     }

     int32 registas = resp.register_status;
     INFO(FILE_LINE,"check network resp.register_status = %d",resp.register_status);
     QString msg;
     bool isSuccess;
     if(registas == 1)
     {
          msg  = "register network successful!";
          isSuccess = true;
     }
     else
     {
          msg = "register network fail!";
          isSuccess = false;
     }

     emit signalDisplayTestResult(testIndex,msg,isSuccess);
     return TRUE;
}

bool TestThread::DIAG_TraceAbilityRead(int testIndex,TraceInfo *pTrace)
{
    jrd_diag_trace_info_req_type req;
    jrd_diag_trace_info_rsp_type resp;
    short resp_len = sizeof(jrd_diag_trace_info_rsp_type);

    //printf("resp len start is %d\n",resp_len);

    if(pTrace == NULL )
    {
        ERROR(FILE_LINE,"trace info is NULL");
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
        ERROR(FILE_LINE,"trace info read send command error");
        emit signalDisplayTestResult(testIndex,"trace info read send command error",false);
        return FALSE;
    }
    //printf("trace info len %d\n",resp.trace_info_len);

    if(resp.diag_errno == 0 && TRACE_INFO_LEN >= resp.trace_info_len)
    {
        memcpy(pTrace->All, resp.trace_info, resp.trace_info_len);
    }
    else
    {
        ERROR(FILE_LINE,"trace info read response error");
        emit signalDisplayTestResult(testIndex,"trace info read response error",false);
        return FALSE;
    }

    emit signalDisplayTestResult(testIndex,"get trace info successful",false);
    return TRUE;
}

bool TestThread::DIAG_TraceAbilityWrite(int testIndex,char *traceInfo)
{
    jrd_diag_trace_info_req_type req;
    jrd_diag_trace_info_rsp_type resp;
    short resp_len = sizeof(jrd_diag_trace_info_rsp_type);

    if(traceInfo == NULL )
    {
        ERROR(FILE_LINE,"trace info is NULL");
        emit signalDisplayTestResult(testIndex,"trace info is NULL",false);
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
        ERROR(FILE_LINE,"trace info write send command error");
        emit signalDisplayTestResult(testIndex,"trace info write send command error",false);
        return FALSE;
    }

    if(resp.diag_errno != 0 )
    {
        ERROR(FILE_LINE,"trace info write response error");
        emit signalDisplayTestResult(testIndex,"trace info write response error",false);
        return FALSE;
    }

    emit signalDisplayTestResult(testIndex,"write trace info successful ",false);
    return TRUE;
}

bool TestThread::DIAG_TraceAbilityLocal(int testIndex,TraceInfo *pTrace)
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

    strcpy(TraceSSID,pTrace->SSID);
    strcpy(TracePassword,pTrace->PasswordAT);

    return TRUE;
}

