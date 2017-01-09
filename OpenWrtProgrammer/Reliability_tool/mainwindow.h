#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "src/define/stdafx.h"
#include "testitemsview.h"
#include <QVector>
#include "mainapp.h"
#include "testthread.h"
#include "diagthread.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkInterface>
#include<QNetworkAddressEntry>
#include <iphlpapi.h>



class TestThread;
class DiagThread;

#define RETRY_TIMES 5

class JrdDiag;
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0,MainApp *mainApp = NULL);
    ~MainWindow();

    QLabel *label;
    QLabel *label_3;
    QComboBox *Security_comboBox;
    QRadioButton *AUTO_radioButton;
    QLineEdit *Password_lineEdit;
    QRadioButton *AES_radioButton;
    QLabel *label_2;
    QComboBox *Encryption_comboBox;
    QRadioButton *TKIP_radioButton;
    QLabel *label_4;
    QPushButton *pushButton_Write;
    QPushButton *pushButton_Quit;


public:
    void              loadTestItems();
    void              initTestItems();
    void              clearTestResult();
    QMap<QString,int> getMapTestItems();
    QVector<QString>  getTestItems();
    bool              getIfUsrRsp();
    bool              getIfOK();
    void              restUsrRsp();
    bool              getUsrRsp_Yes();
    bool              getUsrRsp_No();
    QString           get_value_write();
    void              setTestOver();
    void              PostHttpRequest();
    int               ConnectMS_NV(void);

    wlan_security_mode_enum_type security_mode;
    wlan_wpa_encryption_enum_type encryption_Cipher_Type;
    wlan_wep_enum_type             EncryptionWep;
    e_jrd_rw_flag         Read_Write;
    bool DIAG_Wifi_PassWord_RW(QString ,e_jrd_rw_flag );

public slots:
    bool        GetWIFI_PASSWORD();
    void        OnEditTestItem();
    void        OnStartTest();
    //void        OnStartWrite_NV();
    void        OnStartDiag();
    void        OnTestItemChanged();
    void        OnDeviceArrive(int Port);
    void        OnDeviceRemove();
    void        OnPopupMsgwithYesNO(QString title,QString msg);
    void        OnPopupMsgwithWrite();
    void        OnPopupMsgwithMacWrite(QString title,QString msg);
    void        MAC_Write();
    void        IMEIWrite();
    void        OnDisableSensor();
    void        OnPopupMsgWithOK(QString title,QString msg);
    bool        Enter_test_mode();

    void        OnTestOver();
    void        OnDiagOver();
    void        OnDisplayTestResult(int TestItemIndex,QString result,bool isSuccess);
    void        OnDisplayDiagResult(QString TestItemIndex,QString result,bool isSuccess);
    void        OnhighLightTestItem(int TestItemIndex);
    void        OnMenuHelp();
    //bool        DIAG_WriteSSID(char *ssid);
    bool        DIAG_WriteImei( char *IMEI);
    void        WriteSSID();
    void        Erase_Sim_Lock();
    void        Activate_Sim_Lock();

    void        OnSwitchNDIS();
    void        finished(QNetworkReply *reply);
    void        onChanged(int);
    void        PassWordWrite();
    void         OTKSLWrite();
    void         MEIDWrite();
    void         SPCWrite();



private:
    void                    Init_Config_Ui();
    bool                    CheckMac(QString &Mac);
    bool                    IMEICheck(QString &IMEI);
    bool                    GetAdapter();

    Ui::MainWindow          *ui;
    uint16                  testPort;

    TestItemsView           *testItemView;
    //QMenu                 *menuEditTestItem;
    QAction                 *actionEditTestItem;
    QVector<QString>        testItems;
    QMap<QString,int>       mapTestItems;


    bool                    isTesting;
    MainApp                 *m_mainApp;
    TestThread              *m_testThread;
    DiagThread              *m_DiagThread;
    bool                    isUserRsp;
    bool                    UserRsp_Yes;
    bool                    UserRsp_No;
    bool                    isOK;
    QString                 m_strGateWayAddr;
    //wlan_security_mode_enum_type security_mode;
    //wlan_wpa_encryption_enum_type encryption_Cipher_Type;
    //wlan_wep_enum_type             EncryptionWep;

    QString                 version;
    QString                 Str_Write;

    QNetworkAccessManager   *netManager;
    bool                    bPost;
    QTimer                  *timer;

    QMenu                   *aboutMenu;
    QAction                 *actionAbout;



    //config_Ui
    QButtonGroup *btnGroup;
    //QPushButton *pushButton_Write;
    //QComboBox   *Security_comboBox;
    //QComboBox   * Encryption_comboBox;
    //QLineEdit *Password_lineEdit;
    //QRadioButton *TKIP_radioButton;
    //QRadioButton *AES_radioButton;
    //QRadioButton *AUTO_radioButton;
    bool check_key(QString,wlan_security_mode_enum_type);
    QString key;

    int line;
    int column;


};

#endif // MAINWINDOW_H
