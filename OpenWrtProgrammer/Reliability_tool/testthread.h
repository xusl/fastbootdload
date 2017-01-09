#ifndef TESTTHREAD_H
#define TESTTHREAD_H

#include <QThread>
#include <QVector>
#include <QMap>
#include <QFile>
#include "jrd_diag.h"
#include "mainwindow.h"
#include "src/QMSL/QLib.h"




class MainWindow;

extern HANDLE  	m2_QHandle ;
extern bool  	Get_Device;

extern bool Is_Wifi_Password_Read;
extern bool Is_Wifi_Password_Write;
extern bool Is_Wifi_Password_Quit;
extern HANDLE  	m_QHandle  ;
#define RETRY_TIMES 5
class TestThread : public QThread
{
    Q_OBJECT
public:
    explicit TestThread(QObject *parent = 0, MainWindow *window = NULL);
    ~TestThread();


    int            ConnectMS_NV(void);
    int            DisConnectMS_NV(void);
    bool           DIAG_ReadSSID(int testIndex,char *ssid);
    bool           DIAG_ReadIMEI(int testIndex,char *IMEI);
    bool           DIAG_WriteSSID(int testIndex,char *ssid,sec_info_rw_enum RW);
    bool           DIAG_RestartWiFi(int testIndex);
    bool           DIAG_DisableSecurity(int testIndex,sec_info_rw_enum Disable_Password_flag);
    bool           DIAG_CheckStaConnect(int testIndex);
    bool           DIAG_Charger_TEST(int testIndex);
    bool           DIAG_EnterLCDScreenTEST(int testIndex);
    bool           DIAG_Activate_Sim_Lock(int testIndex);
    bool           DIAG_Erase_Sim_Lock(int testIndex);
    bool           DIAG_LCDScreen_Turn_On(int testIndex);
    bool           DIAG_LCDScreen_Turn_Off(int testIndex);
    bool           DIAG_EXITLCDScreenTEST(int testIndex);
    bool           DIAG_EnterLEDTEST(int testIndex);
    bool           DIAG_EnterLEDLight(int testIndex);
    bool           DIAG_EXITLEDTEST(int testIndex);
    bool           DIAG_REDLEDTEST(int testIndex);    //LED
    bool           DIAG_GREENLEDTEST(int testIndex);  //OLED
    bool           DIAG_BLUELEDTEST(int testIndex);
    bool           DIAG_WRITELEDTEST(int testIndex);
    bool           DIAG_ENTRYKEYTEST(int testIndex);
    bool           DIAG_KEYTEST(int testIndex);
    bool           DIAG_EXITKEYTEST(int testIndex);
    bool           DIAG_ReadFIRMWAREVersion(int testIndex ,char* FirmVer);
    bool           DIAG_CheckSD_Card(int testIndex);
    bool           DIAG_CheckSIM_Card(int testIndex);
    bool           DIAG_CheckNetWork(int testIndex);
    bool           DIAG_TraceAbilityRead(int testIndex,TraceInfo *pTrace);
    bool           DIAG_TraceAbilityWrite(int testIndex ,char *traceInfo);
    bool           DIAG_TraceAbilityLocal(int testIndex ,TraceInfo *pTrace);
    bool           DIAG_MAC_READ(int testIndex,char *ssid);
    bool           DIAG_WriteMac(int testIndex, char *MAC);
    bool           DIAG_WriteImei(int testIndex, char *Imei);
    bool           DIAG_WriteWIFI_PassWord(int testIndex,char *PassWord);
    bool           DIAG_PassWord_Read(int testIndex,char *PassWord);
    bool           Change_Wifi_mode(int testIndex,Wifi_mode_flag_enum WIFI_mode);
    bool           Enter_test_mode(int testIndex);
    bool           Write_Wifi_Power_level(int testIndex,jrd_diag_wifi_dB_gain_adjust req);
    bool           Read_Power_level_File(int testIndex);
    bool           save_wifi_power(int testIndex);
    bool           DIAG_Screen_TEST(screen_msg_id which_item);
    bool           DIAG_YELLOWLEDTEST(int testIndex);
    bool           DIAG_LEDTEST(int testIndex,int ledNo);



protected:
    void run();

signals:
    void  signalPopupMsgWithYesNo(QString title,QString msg);
    void  signalOnPopupMsgwithWrite(QString,QString);
    void  signalOnPopupMsgwithIMEIWrite(QString,QString);
    void  signalOnPopupMsgwithMacWrite(QString,QString);
    void  signalPopupMsgWithOK(QString title,QString msg);
    void signalTestOver();
    void signalDisplayTestResult(int TestItemIndex,QString msg,bool isSuccess);
    void signalHighlightTestItems(int TestItemIndex);
    void signalStartTest();
    void signalOnPopupMsgwithWriteWIFI_PassWord(QString,QString);


public slots:

private :
       MainWindow        *m_mainWindow;
       QVector<QString>   testItems;
       QMap<QString,int>  mapTestItems;
       char           TraceSSID[33];
       char           TracePassword[16];
       uint16		   testPort;
};

#endif // TESTTHREAD_H
