#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/log/log.h"
#include <dbt.h>
#include "src/device/device.h"
#include "src/shiftdevice/shiftdevice.h"
#include "src/device/ziUsbDisk.h"
#include "jrd_diag.h"
#include "src/log/log.h"
#include "dialog_write.h"


#include <QMessageBox>

#define width  677

int             itemNum = 0;


QVector<QString> vecColorsValue;
QVector<uint32>    vecUiLedDiagOrder;
MainWindow::MainWindow(QWidget *parent,MainApp *mainApp):
    QMainWindow(parent),m_mainApp(mainApp),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    isUserRsp = false;
    isOK = false;
    UserRsp_Yes=false;
    UserRsp_No=false;

    QSettings *configIni = new QSettings("TestItem.ini", QSettings::IniFormat);
    version = configIni->value("/Version/version").toString();
    bool ok;
    itemNum = configIni->value("/LedColors/Nums", QVariant("0")).toString().toUInt();
    for(int i = 0;i<itemNum;i++)
    {
        QString strItemValue = configIni->value(QString("/LedColors/item%1").arg(i)).toString();
        vecUiLedDiagOrder.push_back(configIni->value(QString("/LedColors/%1").arg(strItemValue), QVariant("0")).toString().toUInt(&ok,16));
        vecColorsValue.push_back(strItemValue);
    }
    //itemNum = configIni->value("/SideKey/Nums", QVariant("0")).toString().toUInt();


//    uiLedRed = configIni->value("/LedColors/red", QVariant("0")).toString().toUInt(&ok,16);
//    uiLedGreen = configIni->value("/LedColors/green", QVariant("0")).toString().toUInt(&ok,16);
//    uiLedBlue = configIni->value("/LedColors/blue", QVariant("0")).toString().toUInt(&ok,16);

    this->setWindowTitle(QString("Reliability Test Tool(%1)").arg(version));

    //window in the center of screen
    QRect position = frameGeometry();
    position.moveCenter(QDesktopWidget().availableGeometry().center());
    move(position.topLeft());

    this->setFixedSize(width,490);
    this->removeToolBar(ui->mainToolBar);
    ui->textComport->setReadOnly(true);


    actionEditTestItem = new QAction("Edit Test Items",this);
    ui->menuBar->addAction(actionEditTestItem);
    testItemView = new TestItemsView(this);

    actionAbout = new QAction("Help",this);
    ui->menuBar->addAction(actionAbout);
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(OnMenuHelp()));


    connect(actionEditTestItem, SIGNAL(triggered()), this, SLOT(OnEditTestItem()));
    connect(testItemView,SIGNAL(signalTestItemsChanged()),this,SLOT(OnTestItemChanged()));
    connect(ui->btnStart,SIGNAL(clicked()),this,SLOT(OnStartTest()));


    connect(ui->pushButton_Read,SIGNAL(clicked()),this,SLOT(OnStartDiag()));
    //connect(ui->pushButton_NV_Write,SIGNAL(clicked()),this,SLOT(OnStartWrite_NV()));

    connect(m_mainApp,SIGNAL(signalDeviceArrive(int)),this,SLOT(OnDeviceArrive(int)));
    connect(m_mainApp,SIGNAL(signalDeviceRemove()),this,SLOT(OnDeviceRemove()));
    connect(ui->btnReload,SIGNAL(clicked()),this,SLOT(OnTestItemChanged()));
    connect(ui->toolButton_sensor,SIGNAL(clicked()),this,SLOT(OnDisableSensor()));
    ui->toolButton_sensor->hide();

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(OnSwitchNDIS()));
    timer->start(1000);

    bPost = false;
    netManager = new QNetworkAccessManager(this);
    connect(netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));


    QStringList header;
    header << "Test Item" << "Test Result";

    QStringList header_read;
    header_read << "Item" << "Result"<< "Item" << "Result"<< "Item" << "Result";
    ui->tableWidget->setHorizontalHeaderLabels(header_read);


    ui->tableWidget_2->setColumnCount(2);
    ui->tableWidget_2->setHorizontalHeaderLabels(header);

    ui->tableWidget_2->setFixedWidth(width);
    ui->tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget_2->setColumnWidth(0,width/2 - 140);
    ui->tableWidget_2->setColumnWidth(1,width/2 + 139);
    ui->tableWidget_2->horizontalHeader()->setResizeMode(0,QHeaderView::Fixed);
    ui->tableWidget_2->horizontalHeader()->setResizeMode(1,QHeaderView::Stretch);


    loadTestItems();
    initTestItems();

    m_testThread = new TestThread(this,this);
    m_DiagThread  = new DiagThread(this,this);

    connect(m_testThread,SIGNAL(signalPopupMsgWithYesNo(QString,QString)),this,SLOT(OnPopupMsgwithYesNO(QString,QString)));
    connect(ui->pushButton_Imei_Write,SIGNAL(clicked()),this,SLOT(IMEIWrite()));
    connect(ui->pushButton_Mac_Write,SIGNAL(clicked()),this,SLOT(MAC_Write()));
    connect(ui->pushButton_Wifi_PS_R,SIGNAL(clicked()),this,SLOT(GetWIFI_PASSWORD()));

    connect(ui->pushButton_SSID_Write,SIGNAL(clicked()),this,SLOT(WriteSSID()));
    connect(ui->pushButton_Activate_Sim_Lock_2,SIGNAL(clicked()),this,SLOT(Activate_Sim_Lock()));
    connect(ui->pushButton_Erase_Sim_Lock_2,SIGNAL(clicked()),this,SLOT(Erase_Sim_Lock()));
    //connect(ui->pushButton_MEID_Write,SIGNAL(clicked()),this,SLOT(MEIDWrite()));
   // connect(ui->pushButton_SPC_Write,SIGNAL(clicked()),this,SLOT(SPCWrite()));
   // connect(ui->pushButton_OKTSL_Write,SIGNAL(clicked()),this,SLOT(OKTSLWrite()));

    connect(m_testThread,SIGNAL(signalOnPopupMsgwithMacWrite(QString,QString)),this,SLOT(OnPopupMsgwithMacWrite(QString,QString)));
    connect(m_testThread,SIGNAL(signalOnPopupMsgwithIMEIWrite(QString,QString)),this,SLOT(IMEIWrite(QString,QString)));
    connect(m_testThread,SIGNAL(signalOnPopupMsgwithWriteWIFI_PassWord(QString,QString)),this,SLOT(OnPopupMsgwithWriteWIFI_PassWord(QString,QString)));


    connect(m_testThread,SIGNAL(signalPopupMsgWithOK(QString,QString)),this,SLOT(OnPopupMsgWithOK(QString,QString)));

    connect(m_testThread,SIGNAL(signalTestOver()),this,SLOT(OnTestOver()));
    connect(m_testThread,SIGNAL(signalDisplayTestResult(int,QString,bool)),this,SLOT(OnDisplayTestResult(int,QString,bool)));
    connect(m_testThread,SIGNAL(signalHighlightTestItems(int)),this,SLOT(OnhighLightTestItem(int)));

    connect(m_DiagThread,SIGNAL(signalPopupMsgWithOK(QString,QString)),this,SLOT(OnPopupMsgWithOK(QString,QString)));
    connect(m_DiagThread,SIGNAL(signalDiagOver()),this,SLOT(OnDiagOver()));
    connect(m_DiagThread,SIGNAL(signalDisplayDiagResult(QString,QString,bool)),this,SLOT(OnDisplayDiagResult(QString,QString,bool)));


    //SwitchCDROM();
    //ConnectMS_NV();

    uint16 ComId[32] = {0};
    uint16 count = 0;
    int comport = -1;
    CDeviceList devlist;
    count = devlist.GetComPortList(PORTTYPE_DIAG, ComId);
    testPort = ComId[0];
    if(count > 0)
    {
        comport = ComId[0];
        ui->textComport->setText(QString("COM%1").arg(comport));
        ConnectMS_NV();
        Get_Device  = true;
    }

    isTesting = false;
    isOK = false;
    line=0;
    column=0;
    Init_Config_Ui();

    QRegExp rx("[a-zA-Z0-9\-\.\\_]{32}");
    ui->lineEdit_SSID->setValidator(new QRegExpValidator(rx,this));
}

void MainWindow::Init_Config_Ui()
{

    QSettings *configIni = new QSettings("TestItem.ini", QSettings::IniFormat);
    QString TestInterface = configIni->value("/InterFace/TestInterFace").toString();
    QString ConfigInterFace = configIni->value("/InterFace/ConfigInterFace").toString();
    QString DiagInterFace = configIni->value("/InterFace/DiagInterFace").toString();
    QString SensofInterFace = configIni->value("/InterFace/SensofInterFace").toString();
    int removeItem=0;
    if(TestInterface!="yes")
    {
        this->ui->tabWidget->removeTab(removeItem);
    }
    else
        removeItem+=1;
    if(ConfigInterFace!="yes")
    {
        this->ui->tabWidget->removeTab(removeItem);
    }
    else
        removeItem+=1;
    if(DiagInterFace!="yes")
    {
        this->ui->tabWidget->removeTab(removeItem);
    }
    else
        removeItem+=1;
    if(SensofInterFace!="yes")
    {
        this->ui->tabWidget->removeTab(removeItem);
    }
    this->ui->tabWidget->setCurrentIndex(0);


    pushButton_Write=ui->pushButton_Write_2;
    Security_comboBox = ui->Security_comboBox_2;
    Password_lineEdit= ui->Password_lineEdit_2;
    Encryption_comboBox= ui->Encryption_comboBox_2;
    TKIP_radioButton= ui->TKIP_radioButton_2;
    AES_radioButton= ui->AES_radioButton_2;
    AUTO_radioButton= ui->AUTO_radioButton_2;
    QStringList Security_comlist ;
    Security_comlist<<"Disable"<<"WEP"<<"WPA PSK"<<"WPA2 PSK"<<"WPA/WPA2 PSK";
    ui->Security_comboBox_2->addItems(Security_comlist);
    QStringList Encryption_comlist ;
    Encryption_comlist<<"Open"<<"Share";
    ui->Encryption_comboBox_2->addItems(Encryption_comlist);
    btnGroup=new QButtonGroup(this);
    btnGroup->addButton(ui->TKIP_radioButton_2);
    btnGroup->addButton(ui->AES_radioButton_2);
    btnGroup->addButton(ui->AUTO_radioButton_2);
    btnGroup->setExclusive(true);
    btnGroup->setId(ui->TKIP_radioButton_2,0);
    btnGroup->setId(ui->AES_radioButton_2,1);
    btnGroup->setId(ui->AUTO_radioButton_2,2);
    connect(Security_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onChanged(int)));
    connect(ui->pushButton_Write_2,SIGNAL(clicked()),this,SLOT(PassWordWrite()));
    ui->Security_comboBox_2->setEnabled(true);
    ui->Encryption_comboBox_2->setEnabled(false);
    ui->Password_lineEdit_2->setEnabled(false);
    ui->TKIP_radioButton_2->setEnabled(false);
    ui->AES_radioButton_2->setEnabled(false);
    ui->AUTO_radioButton_2->setEnabled(false);
    ui->pushButton_Write_2->setEnabled(false);

}



void MainWindow::Erase_Sim_Lock()
{
    if(Get_Device==false)
    {
        OnPopupMsgWithOK("ReliabilityTest","please insert a device to test!");
        return;
    }
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
               OnPopupMsgWithOK("ReliabilityTest","Erase Sim Lock send command error!");
               //emit signalDisplayTestResult(testIndex,"Erase Sim Lock send command error",false);
               return ;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"Erase Sim Lock send command OK");
          OnPopupMsgWithOK("ReliabilityTest","Erase Sim Lock successful!");
          //emit signalDisplayTestResult(testIndex,"Erase Sim Lock successful",true);
          break;
      }

    }


    return ;

}

void MainWindow::Activate_Sim_Lock()
{
    if(Get_Device==false)
    {
        OnPopupMsgWithOK("ReliabilityTest","please insert a device to test!");
        return;
    }
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
               OnPopupMsgWithOK("ReliabilityTest","Activate Sim Lock send command error!");
               //emit signalDisplayTestResult(testIndex,"Activate Sim Lock send command error",false);
               return ;
          }
          else
          {
              SLEEP(200);
          }
      }
      else
      {
          INFO(FILE_LINE,"Activate Sim Lock send command OK");
          OnPopupMsgWithOK("ReliabilityTest","Activate Sim Lock successful!");
          //emit signalDisplayTestResult(testIndex,"Activate Sim Lock successful",true);
          break;
      }

    }

    return ;


}

MainWindow::~MainWindow()
{
    delete ui;

    if(NULL != testItemView)
    {
        delete testItemView;
    }

    if(NULL != actionEditTestItem)
    {
        delete actionEditTestItem;
    }

    if(NULL != netManager)
    {
        delete netManager;
    }

    if(m_testThread != NULL)
    {
        disconnect(m_testThread,0,this,0);
        delete m_testThread;
    }
}

void MainWindow::OnEditTestItem()
{
    testItemView->setWindowFlags(Qt::Window |Qt::WindowTitleHint);
    testItemView->setModal(true);
    testItemView->exec();
}

void MainWindow::OnStartTest()
{
    isTesting = true;
    ui->btnStart->setEnabled(false);

    ui->btnStart->setText("Testing");
    clearTestResult();

    m_testThread->start();
}

void MainWindow::OnStartDiag()
{
    ui->tableWidget->clear();

    ui->pushButton_Read->setEnabled(false);
    m_DiagThread->start();
}


void MainWindow::OnTestItemChanged()
{
    if(!isTesting)
    {
        loadTestItems();
        clearTestResult();
    }
}

void MainWindow::OnDeviceArrive(int Port)
{
    INFO(FILE_LINE, "OnDeviceArrive,arrive Port = %d",Port);
    INFO(FILE_LINE, "OnDeviceArrive,testPort Port = %d",testPort);

    if(testPort != Port)
    {
        bPost = true;
        ui->textComport->setText(QString("COM%1").arg(Port));
        testPort = Port;
        ConnectMS_NV();
        Get_Device  = true;
    }
}

int MainWindow::ConnectMS_NV(void)
{
    int i=0;
    int status = 0;

    char resultBuf[256]="";
    unsigned char _bUserSelectQPST = FALSE;//TRUE;//FALSE;

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
void MainWindow::OnDeviceRemove()
{
    INFO(FILE_LINE, "---------in OnDeviceRemove-------");

    uint16 ComId[32] = {0};
    uint16 count = 0;
    CDeviceList devlist;
    count = devlist.GetComPortList(PORTTYPE_DIAG, ComId);
    INFO(FILE_LINE, "---------in OnDeviceRemove-------,ComId[0] = %d",ComId[0]);
    INFO(FILE_LINE, "---------in OnDeviceRemove-------,testPort = %d",testPort);

    if((count <= 0 || ComId[0] != testPort) )//&& !isTesting)
    {
        bPost = false;
       testPort = 0;
       ui->textComport->setText("");
       Get_Device  = false;
       if(!isTesting)
           clearTestResult();
    }
}

void MainWindow::OnPopupMsgwithYesNO(QString title,QString msg)
{
    QMessageBox::StandardButton rb = QMessageBox::information(NULL, title, msg,
                                        QMessageBox::Yes, QMessageBox::No);

    if(rb == QMessageBox::Yes)
    {
        isUserRsp = true;
        isOK = true;
    }
    else if(rb == QMessageBox::No)
    {
        isUserRsp = true;
        isOK = false;
    }

}

bool MainWindow::getUsrRsp_Yes()
{
    return UserRsp_Yes;
}
bool MainWindow::getUsrRsp_No()
{
    return UserRsp_No;
}
QString MainWindow::get_value_write()
{
    return Str_Write;
}

void MainWindow::OnPopupMsgwithWrite()
{
    if(Get_Device==false)
    {
        OnPopupMsgWithOK("ReliabilityTest","please insert a device to test!");
        return;
    }
    Str_Write = ui->lineEdit_IMEI->text();
    if (Str_Write.isEmpty())
    {

        QMessageBox::information(this, "title", "Please enter the value!", QMessageBox::Yes, QMessageBox::NoButton);
        return;
    }
    QByteArray temp=get_value_write().toLatin1();
    char *SSID=temp.data();
    //IsWrite=true;
    DIAG_WriteImei( SSID);

}


void MainWindow::WriteSSID()
{
    char *SSID="";
    if(Get_Device==false)
    {
        OnPopupMsgWithOK("ReliabilityTest","please insert a device to test!");
        return;
    }
    Str_Write = ui->lineEdit_SSID->text();

    if(Str_Write == NULL )
    {
        ERROR(FILE_LINE,"SSID string is NULL");
        OnPopupMsgWithOK("ReliabilityTest","SSID string is NULL!");
        return ;
    }

    QByteArray temp=Str_Write.toLatin1();
    SSID=temp.data();


    jrd_diag_wifi_ssid_wr_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_ssid_wr_req_rsp_type);
    if(SSID == NULL || strlen(SSID) > 32)
    {
        ERROR(FILE_LINE,"SSID string is NULL or the strlen is too long");

        OnPopupMsgWithOK("ReliabilityTest","SSID string is NULL or the strlen is too long!");
        return ;
    }



    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_SSID_RW;

    req.rw_flag = AP1_SSID_WRITE;
    memcpy(req.ssid,SSID,MAX_SSID_LEN+1);

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
                OnPopupMsgWithOK("ReliabilityTest","ssid write send command error!");
                restUsrRsp();
                return ;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            INFO(FILE_LINE,"ssid write send command ok,the ssid = %s",SSID);
            OnPopupMsgWithOK("ReliabilityTest","ssid write send successfully!");

            restUsrRsp();

            break;
        }
    }

    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"ssid write response error!");
        OnPopupMsgWithOK("ReliabilityTest","ssid write response error!");

        restUsrRsp();
        return ;
    }

    return ;
}

void MainWindow::OnPopupMsgwithMacWrite(QString title,QString msg)
{
    bool isOK;
    do
    {
        Str_Write = QInputDialog::getText(NULL, title,msg,QLineEdit::Normal,"",&isOK);
        if(isOK)
        {
            if(CheckMac(Str_Write)==false)
                QMessageBox::information(this, "title", "Wrong Mac Input!\n For example: AA:BB:CC:DD:EE:FF", QMessageBox::Yes, QMessageBox::NoButton);
             else
            {
                UserRsp_Yes=true;
                break;
            }
        }
        else
        {
            UserRsp_No=true;
            break;
        }
    }while(CheckMac(Str_Write)==false);

}
void   MainWindow::OnDisableSensor()
{
    if(Get_Device==false)
    {
        OnPopupMsgWithOK("ReliabilityTest","please insert a device to test!");
        return;
    }
    jrd_diag_get_psensor_status_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_get_psensor_status_rsp_type);
    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_PERIPHERAL;
    req.hdr.cmd_code = E_JRD_DIAG_SYS_USB_SWITCH;
    req.diag_errno = P_Sonser_Close;

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
                ERROR(FILE_LINE,"Disable P_sonser send command error!");
                QMessageBox::information(this, "P_sonser", "Disable P_sonser send command error!", QMessageBox::Yes, QMessageBox::NoButton);
                restUsrRsp();
                return ;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            //INFO(FILE_LINE,"IMEI write send command ok,the IMEI = %s",IMEI);
            //QMessageBox::information(this, "IMEI", "Write IMEI successfully!", QMessageBox::Yes, QMessageBox::NoButton);

           restUsrRsp();
            break;
        }
    }
    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"Disable P_sonser response error!");
        QMessageBox::information(this, "P_sonser", "Disable P_sonser response error!", QMessageBox::Yes, QMessageBox::NoButton);
        restUsrRsp();
        return ;
    }
    req.diag_errno = Disable_power_saving;

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
                ERROR(FILE_LINE,"Disable power_saving send command error!");
                QMessageBox::information(this, "power", "Disable power_saving send command error!", QMessageBox::Yes, QMessageBox::NoButton);
                restUsrRsp();
                return ;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            //INFO(FILE_LINE,"IMEI write send command ok,the IMEI = %s",IMEI);
            QMessageBox::information(this, "Disable", "Disable power_saving and P_soner successfully! please restart the dievice", QMessageBox::Yes, QMessageBox::NoButton);

           restUsrRsp();
            break;
        }
    }
    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"Disable power_saving response error!");
        QMessageBox::information(this, "power_saving", "Disable power_saving response error!", QMessageBox::Yes, QMessageBox::NoButton);
        restUsrRsp();
        return ;
    }
    return ;

}

void MainWindow::IMEIWrite()
{
    if(Get_Device==false)
    {
        OnPopupMsgWithOK("ReliabilityTest","please insert a device to test!");
        return;
    }
    Str_Write = ui->lineEdit_IMEI->text();
    if(IMEICheck(Str_Write)==false)
    {
        QMessageBox::information(this, "title", "Wrong IMEI Input!\n For example: 863459020000414", QMessageBox::Yes, QMessageBox::NoButton);
        return;
    }
    QByteArray temp=Str_Write.toLatin1();
    char *IMEI=temp.data();

    jrd_diag_sys_imei_write_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_sys_imei_write_req_rsp_type);

    if(IMEI == NULL )
    {
        ERROR(FILE_LINE,"IMEI string is NULL");
        OnPopupMsgWithOK("ReliabilityTest","IMEI string is NULL!");
        return ;
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
                QMessageBox::information(this, "IMEI", "IMEI write send command error!", QMessageBox::Yes, QMessageBox::NoButton);
                restUsrRsp();
                return ;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            INFO(FILE_LINE,"IMEI write send command ok,the IMEI = %s",IMEI);
            QMessageBox::information(this, "IMEI", "Write IMEI successfully!", QMessageBox::Yes, QMessageBox::NoButton);

           restUsrRsp();
            break;
        }
    }
    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"IMEI write response error!");
        QMessageBox::information(this, "IMEI", "IMEI write response error!", QMessageBox::Yes, QMessageBox::NoButton);
        restUsrRsp();
        return ;
    }
    return ;
}
void MainWindow::OTKSLWrite()
{
    if(Get_Device==false)
    {
        OnPopupMsgWithOK("ReliabilityTest","please insert a device to test!");
        return;
    }
    Str_Write = ui->lineEdit_IMEI->text();
    if(IMEICheck(Str_Write)==false)
    {
        QMessageBox::information(this, "title", "Wrong IMEI Input!\n For example: 863459020000414", QMessageBox::Yes, QMessageBox::NoButton);
        return;
    }
    QByteArray temp=Str_Write.toLatin1();
    char *IMEI=temp.data();

    jrd_diag_sys_imei_write_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_sys_imei_write_req_rsp_type);

    if(IMEI == NULL )
    {
        ERROR(FILE_LINE,"IMEI string is NULL");
        OnPopupMsgWithOK("ReliabilityTest","IMEI string is NULL!");
        return ;
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
                QMessageBox::information(this, "IMEI", "IMEI write send command error!", QMessageBox::Yes, QMessageBox::NoButton);
                restUsrRsp();
                return ;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            INFO(FILE_LINE,"IMEI write send command ok,the IMEI = %s",IMEI);
            QMessageBox::information(this, "IMEI", "Write IMEI successfully!", QMessageBox::Yes, QMessageBox::NoButton);

           restUsrRsp();
            break;
        }
    }
    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"IMEI write response error!");
        QMessageBox::information(this, "IMEI", "IMEI write response error!", QMessageBox::Yes, QMessageBox::NoButton);
        restUsrRsp();
        return ;
    }
    return ;
}
void MainWindow::MEIDWrite()
{
    if(Get_Device==false)
    {
        OnPopupMsgWithOK("ReliabilityTest","please insert a device to test!");
        return;
    }
    Str_Write = ui->lineEdit_IMEI->text();
    if(IMEICheck(Str_Write)==false)
    {
        QMessageBox::information(this, "title", "Wrong IMEI Input!\n For example: 863459020000414", QMessageBox::Yes, QMessageBox::NoButton);
        return;
    }
    QByteArray temp=Str_Write.toLatin1();
    char *IMEI=temp.data();

    jrd_diag_sys_imei_write_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_sys_imei_write_req_rsp_type);

    if(IMEI == NULL )
    {
        ERROR(FILE_LINE,"IMEI string is NULL");
        OnPopupMsgWithOK("ReliabilityTest","IMEI string is NULL!");
        return ;
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
                QMessageBox::information(this, "IMEI", "IMEI write send command error!", QMessageBox::Yes, QMessageBox::NoButton);
                restUsrRsp();
                return ;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            INFO(FILE_LINE,"IMEI write send command ok,the IMEI = %s",IMEI);
            QMessageBox::information(this, "IMEI", "Write IMEI successfully!", QMessageBox::Yes, QMessageBox::NoButton);

           restUsrRsp();
            break;
        }
    }
    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"IMEI write response error!");
        QMessageBox::information(this, "IMEI", "IMEI write response error!", QMessageBox::Yes, QMessageBox::NoButton);
        restUsrRsp();
        return ;
    }
    return ;
}
void MainWindow::SPCWrite()
{
    if(Get_Device==false)
    {
        OnPopupMsgWithOK("ReliabilityTest","please insert a device to test!");
        return;
    }
    Str_Write = ui->lineEdit_IMEI->text();
    if(IMEICheck(Str_Write)==false)
    {
        QMessageBox::information(this, "title", "Wrong IMEI Input!\n For example: 863459020000414", QMessageBox::Yes, QMessageBox::NoButton);
        return;
    }
    QByteArray temp=Str_Write.toLatin1();
    char *IMEI=temp.data();

    jrd_diag_sys_imei_write_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_sys_imei_write_req_rsp_type);

    if(IMEI == NULL )
    {
        ERROR(FILE_LINE,"IMEI string is NULL");
        OnPopupMsgWithOK("ReliabilityTest","IMEI string is NULL!");
        return ;
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
                QMessageBox::information(this, "IMEI", "IMEI write send command error!", QMessageBox::Yes, QMessageBox::NoButton);
                restUsrRsp();
                return ;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            INFO(FILE_LINE,"IMEI write send command ok,the IMEI = %s",IMEI);
            QMessageBox::information(this, "IMEI", "Write IMEI successfully!", QMessageBox::Yes, QMessageBox::NoButton);

           restUsrRsp();
            break;
        }
    }
    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"IMEI write response error!");
        QMessageBox::information(this, "IMEI", "IMEI write response error!", QMessageBox::Yes, QMessageBox::NoButton);
        restUsrRsp();
        return ;
    }
    return ;
}

bool MainWindow::IMEICheck(QString &IMEI)
{
    if(IMEI.size()!=15)
        return false;
    for(int i=0;i<IMEI.size();i++)
    {
        if(IMEI.mid(i,1)=="0"||IMEI.mid(i,1)=="1"||IMEI.mid(i,1)=="2"||IMEI.mid(i,1)=="3"||
           IMEI.mid(i,1)=="4"||IMEI.mid(i,1)=="5"||IMEI.mid(i,1)=="6"||IMEI.mid(i,1)=="7"||
           IMEI.mid(i,1)=="8"||IMEI.mid(i,1)=="9")
            ;
        else
            return false;
    }
    QString temp="08";
    temp.append(IMEI.mid(0,1)).append("a").append(IMEI.mid(2,1)).append(IMEI.mid(1,1)).append(IMEI.mid(4,1))
          .append(IMEI.mid(3,1)).append(IMEI.mid(6,1)).append(IMEI.mid(5,1)).append(IMEI.mid(8,1)).append(IMEI.mid(7,1)).append(IMEI.mid(10,1))
          .append(IMEI.mid(9,1)).append(IMEI.mid(12,1)).append(IMEI.mid(11,1)).append(IMEI.mid(14,1)).append(IMEI.mid(13,1));
    IMEI=temp;
    return true;
}



bool MainWindow::CheckMac(QString &Mac)
{
    if(Mac.isEmpty())
        return false;
    qDebug()<<Mac.size()<<Mac.mid(2,1)<<Mac.mid(5,1)<<Mac.mid(8,1)<<Mac.mid(11,1)<<Mac.mid(14,1);
    if(Mac.size()!=17||Mac.mid(2,1)!=":" ||Mac.mid(5,1)!=":"
       ||Mac.mid(8,1)!=":"||Mac.mid(11,1)!=":"||Mac.mid(14,1)!=":")
        return false;
    QString temp;
    for(int i=0;i<Mac.size();i++)
        if(Mac.mid(i,1)!=":")
            temp.append(Mac.mid(i,1));

    for(int i=0;i<temp.size();i++)
    {
        if(temp.mid(i,1)=="0"||temp.mid(i,1)=="1"||temp.mid(i,1)=="2"||temp.mid(i,1)=="3"||temp.mid(i,1)=="4"||temp.mid(i,1)=="5"
           ||temp.mid(i,1)=="6"||temp.mid(i,1)=="7"||temp.mid(i,1)=="8"||temp.mid(i,1)=="9"||temp.mid(i,1)=="a"||temp.mid(i,1)=="b"
           ||temp.mid(i,1)=="c"||temp.mid(i,1)=="d"||temp.mid(i,1)=="e"||temp.mid(i,1)=="f"||temp.mid(i,1)=="A"||temp.mid(i,1)=="B"
           ||temp.mid(i,1)=="C"||temp.mid(i,1)=="D"||temp.mid(i,1)=="E"||temp.mid(i,1)=="F")
            ;
        else
            return false;
    }

    return true;
}

void MainWindow::OnPopupMsgWithOK(QString title,QString msg)
{
    QMessageBox::StandardButton rb = QMessageBox::information(NULL, title, msg,
                                                              QMessageBox::Ok);

    if(rb == QMessageBox::Ok)
    {
        isUserRsp = true;
    }
}

void MainWindow::OnTestOver()
{
    isTesting = false;
    ui->btnStart->setEnabled(true);
    ui->btnStart->setText("Start");
}


void MainWindow::OnDiagOver()
{

    ui->pushButton_Read->setEnabled(true);
    line=0;
    column=0;

}
void MainWindow::loadTestItems()
{
    testItems.clear();

    QSettings *configIni = new QSettings("TestItem.ini", QSettings::IniFormat);
    int allTestItems = configIni->value("/AllItems/Nums").toInt();
    for(int i = 0; i < allTestItems; i++)
    {
        QString prefix = QString("/selectItems/item%1").arg(i);
        QString aTestitem = QString("item%1").arg(i);
        QString selItem = configIni->value(prefix).toString();
        if(selItem == "yes")
        {
            for(int  j = 0; j < allTestItems; j++)
            {
                QString aPrefix = QString("/AllItems/item%1").arg(j);
                QString Testitem = QString("item%1").arg(j);
                if(aTestitem == Testitem)
                {
                    QString aTestItem = configIni->value(aPrefix).toString();
                    testItems.push_back(aTestItem);
                }
            }
        }
    }

    ui->tableWidget_2->setRowCount(testItems.size());
    for(int k = 0; k < testItems.size(); k++)
    {
        QString aTestItem = testItems.at(k);
        QTableWidgetItem *item = new QTableWidgetItem(aTestItem);
        ui->tableWidget_2->setItem(k,0,item);
    }

}

void MainWindow::initTestItems()
{
    mapTestItems.insert("Wifi_Connecting_2G",Wifi_Connecting_2G);
    mapTestItems.insert("Wifi_Connecting",Wifi_Connecting);
    mapTestItems.insert("Wifi_Connecting_5G",Wifi_Connecting_5G);
    mapTestItems.insert("Wifi_Connecting_5G_and_2G",Wifi_Connecting_5G_and_2G);
    mapTestItems.insert("Side_Key_Checking",Side_Key_Checking);
    mapTestItems.insert("Sd_Card",Sd_Card);
    mapTestItems.insert("SIM_Card",SIM_Card);
    mapTestItems.insert("Network_Register_Checking",Network_Register_Checking);
    mapTestItems.insert("OLED_Screen",OLED_Screen);
    mapTestItems.insert("Single_LED_Test",Single_LED_Test);
    mapTestItems.insert("Charger_Test",Charger_Test);
    mapTestItems.insert("Three_LED_Test",Three_LED_Test);
    mapTestItems.insert("Screen_Test",Screen_Test);
    mapTestItems.insert("LED_Test",LED_Test);
    mapTestItems.insert("Firmware_Version",Firmware_Version);
    mapTestItems.insert("SSID_Read",SSID_Read);
    mapTestItems.insert("SSID_Write",SSID_Write);
    mapTestItems.insert("MAC_Read",MAC_Read);
    //mapTestItems.insert("MAC_Write",MAC_Write);
    mapTestItems.insert("IMEI_Read",IMEI_Read);
    mapTestItems.insert("IMEI_Write",IMEI_Write);
    //mapTestItems.insert("Activate_Sim_Lock",Activate_Sim_Lock);
    //mapTestItems.insert("Erase_Sim_Lock",Erase_Sim_Lock);
    mapTestItems.insert("WIFI_PassWord_Write",WIFI_PassWord_Write);
    mapTestItems.insert("WIFI_PassWord_Read",WIFI_PassWord_Read);
    mapTestItems.insert("Wifi_Calibration",Wifi_Calibration);


}


void MainWindow::OnDisplayTestResult(int TestItemIndex,QString result,bool isSuccess)
{
    QTableWidgetItem *item = new QTableWidgetItem(result);
    ui->tableWidget_2->setItem(TestItemIndex,1,item);
    if(!isSuccess)
    {
        item->setTextColor(QColor("red"));
    }
}


void MainWindow::OnDisplayDiagResult(QString TestItem,QString result,bool isSuccess)
{

    QTableWidgetItem *DiagItem = new QTableWidgetItem(TestItem);
    ui->tableWidget->setItem(line,column,DiagItem);
    if(!isSuccess)
    {
        DiagItem->setTextColor(QColor("red"));
    }
    QTableWidgetItem *DiagResult = new QTableWidgetItem(result);
    ui->tableWidget->setItem(line,column+1,DiagResult);
    if(!isSuccess)
    {
        DiagResult->setTextColor(QColor("red"));
    }
    line++;
    if(line==8)
    {
        line=0;
        column=column+2;
    }

}

void MainWindow::OnhighLightTestItem(int TestItemIndex)
{
    for(int k = 0; k < testItems.size(); k++)
    {
        if( k == TestItemIndex)
        {
            ui->tableWidget_2->item(TestItemIndex,0)->setBackgroundColor(QColor("green"));
            ui->tableWidget_2->item(TestItemIndex,1)->setBackground(QColor("green"));
        }
        else
        {
            ui->tableWidget_2->item(TestItemIndex,0)->setBackgroundColor(QColor("white"));
            ui->tableWidget_2->item(TestItemIndex,1)->setBackground(QColor("white"));
        }
    }
}

void MainWindow::OnMenuHelp()
{


    HINSTANCE hNewExe=ShellExecuteA(NULL, "open", "Help.pdf", NULL, NULL, SW_NORMAL);
    if ((DWORD)hNewExe <= 32)
    {
        QMessageBox::information(this, "Reliability Tool", "Open help document file failed!", QMessageBox::Ok, QMessageBox::NoButton);
    }
    return;
}

void MainWindow::clearTestResult()
{
    for(int k = 0; k < testItems.size(); k++)
    {
        QTableWidgetItem *item = new QTableWidgetItem("");
        ui->tableWidget_2->setItem(k,1,item);
    }
}


QMap<QString,int> MainWindow::getMapTestItems()
{
    return mapTestItems;
}

QVector<QString> MainWindow::getTestItems()
{
    return testItems;
}

bool MainWindow::getIfUsrRsp()
{
    return isUserRsp;
}

bool MainWindow::getIfOK()
{
    return isOK;
}

void  MainWindow::restUsrRsp()
{
    isUserRsp = false;
    isOK = false;
    UserRsp_Yes=false;
    UserRsp_No=false;
    Is_Wifi_Password_Read=false;
    Is_Wifi_Password_Quit=false;
    Is_Wifi_Password_Write=false;

}

void  MainWindow::setTestOver()
{
    isTesting = false;
    ui->btnStart->setEnabled(true);
    ui->btnStart->setText("Start");
}

bool MainWindow::Enter_test_mode()
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
               QMessageBox::information(NULL, "ReliabilityTool", "Send Command Failed!",QMessageBox::Ok);
               //emit signalDisplayTestResult(testIndex,"wifi restart send command error!",false);
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
        QMessageBox::information(NULL, "ReliabilityTool", "Device response failed!",QMessageBox::Ok);
        //emit signalDisplayTestResult(testIndex,"wifi restart read response error!",false);
        return FALSE;
    }

    INFO(FILE_LINE,"enter test mode read response ok!");
    QMessageBox::information(NULL, "ReliabilityTool", "Enter test mode successfully!",QMessageBox::Ok);
    return TRUE;
}


void MainWindow::OnSwitchNDIS()
{
    if (!bPost)
    {
        //connect(netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
        PostHttpRequest();
        bPost = true;
    }
}


bool MainWindow::GetAdapter()
{
    const char* pAdapterName = "Windows Mobile-based Internet Sharing Device";  //XP
    const char* pAdapterName2 = "Remote NDIS based Internet Sharing Device";	//vista win7 win8
    if (pAdapterName == NULL)
    {
        return false;
    }
    unsigned long dwIndex = -1;
    unsigned long ulOutBufLen = 0;
    unsigned long dwRet =GetAdaptersInfo(NULL, &ulOutBufLen);
    if (dwRet == ERROR_BUFFER_OVERFLOW)
    {
        PIP_ADAPTER_INFO pAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(new BYTE[ulOutBufLen]);
        dwRet = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
        if (dwRet == ERROR_SUCCESS)
        {
            PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
            while (pAdapter != NULL)
            {
                //qDebug()<<QString(pAdapter->Description);
                //qDebug()<<QString(pAdapter->GatewayList.IpAddress.String);
                INFO(FILE_LINE,"pAdapter->Description = %s",pAdapter->Description);
                INFO(FILE_LINE,"pAdapter->IpAddress = %s",pAdapter->GatewayList.IpAddress.String);
                if ((_strnicmp(pAdapter->Description, pAdapterName, strlen(pAdapterName)) == 0)
                    ||(_strnicmp(pAdapter->Description, pAdapterName2, strlen(pAdapterName2)) == 0))
                    {
                        //qDebug()<<QString(pAdapter->GatewayList.IpAddress.String);
                        INFO(FILE_LINE,"find IP = %s",pAdapter->GatewayList.IpAddress.String);
                        m_strGateWayAddr = pAdapter->GatewayList.IpAddress.String;
                        //qDebug()<<m_strGateWayAddr;
                        dwIndex = pAdapter->Index;
                        //JRD_LOG("Interface description==> %s, IP addr ==> %s\r\n", pAdapter->Description, pAdapter->IpAddressList.IpAddress.String);
                        LOG(FILE_LINE,"Get Adapter success.");
                        if(m_strGateWayAddr!="")
                        {
                            QByteArray ba = m_strGateWayAddr.toLatin1();
                            INFO(FILE_LINE,"the URL = %s",ba.data());
                        }
                        delete pAdapterInfo;
                        return true;
                    }
                        pAdapter = pAdapter->Next;
            }
        }

        delete pAdapterInfo;
    }

    //qDebug()<<"Get Adapter error";
    LOG(FILE_LINE,"Get Adapter error.");
    return false;
}

void MainWindow::PostHttpRequest()
{
    qDebug()<<"112233";
    if((!GetAdapter())&&(m_strGateWayAddr==""))
    {
        //QMessageBox::information(this, "Reliablity", "Get Ip Failed!", QMessageBox::Yes, QMessageBox::NoButton);
        return;
    }
    //bPost = true;
    INFO(FILE_LINE,"----------PostHttpRequest-------");
    QString url = "http://"+m_strGateWayAddr+"/vodafoneapi/process.cgi";
    //INFO(FILE_LINE,"the URL = %s",url);
    //m_strGateWayAddr
    QByteArray postData;
    postData.append(QString("<?xml  version=\"1.0\"  encoding=\"utf-8\"?>\n").toAscii());
    postData.append(QString("<QuickStart>\n").toAscii());
    postData.append(QString("  <Device>\n").toAscii());
    postData.append(QString("    <Request>\n").toAscii());
    postData.append(QString("      <Diag>\n").toAscii());
    postData.append(QString("        <Adsu>Debug</Adsu>\n").toAscii());
    postData.append(QString("      </Diag>\n").toAscii());
    postData.append(QString("    </Request>\n").toAscii());
    postData.append(QString("  </Device>\n").toAscii());
    postData.append(QString("</QuickStart>\n").toAscii());

    netManager->post(QNetworkRequest(QUrl(url)),postData);
}


void MainWindow::finished(QNetworkReply *reply)
{
    qDebug()<<"1122!";
    if(reply->error() == QNetworkReply::NoError)
    {
         QByteArray bytes = reply->readAll();
         QString string = QString::fromUtf8(bytes);
         //INFO(FILE_LINE, "answer http requset no error,res = %s",string.toLatin1().data());
         QXmlStreamReader xReader(string);
         while (!xReader.atEnd())
         {
            xReader.readNext();

            if (xReader.isStartElement() && (xReader.name().toString() == "errorCode"))
            {
                QString strValue = xReader.readElementText();
                if (strValue == "0")
                {
                    qDebug()<<"successful!";

                    //bPost = true;
                    break;
                }
                else
                {
                    //bPost = false;
                }
            }
         }

         //bPost = false;
    }
    else
    {
        QString strTest = reply->errorString();
        qDebug()<<"failed!  "<<strTest;
        //bPost = false;
    }
}

bool MainWindow::DIAG_WriteImei( char *IMEI)
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

                restUsrRsp();

                return FALSE;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            INFO(FILE_LINE,"IMEI write send command ok,the IMEI = %s",IMEI);
            QMessageBox::information(this, "IMEI", "Write IMEI successfully!", QMessageBox::Yes, QMessageBox::NoButton);

           restUsrRsp();
            break;
        }
    }
    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"IMEI write response error!");

        restUsrRsp();
        return FALSE;
    }
    return TRUE;
}

void MainWindow::MAC_Write()
{
    char *MAC="";

    if(Get_Device==false)
    {
        OnPopupMsgWithOK("ReliabilityTest","please insert a device to test!");
        return;
    }
    Str_Write = ui->lineEdit_MAC->text();

    if(CheckMac(Str_Write)==false)
    {
        QMessageBox::information(this, "title", "Wrong Mac Input!\n For example: AA:BB:CC:DD:EE:FF", QMessageBox::Yes, QMessageBox::NoButton);
        return;
    }
    QByteArray temp=Str_Write.toLatin1();
    MAC=temp.data();


    jrd_diag_wifi_mac_addr_wr_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_mac_addr_wr_req_rsp_type);

    if(MAC == NULL )
    {
        ERROR(FILE_LINE,"MAC string is NULL");
        return ;
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
                QMessageBox::information(this, "MAC", "MAC write send command error!", QMessageBox::Yes, QMessageBox::NoButton);

                restUsrRsp();

                return ;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            INFO(FILE_LINE,"MAC write send command ok,the ssid = %s",MAC);

            QMessageBox::information(this, "MAC", "MAC write successfully!", QMessageBox::Yes, QMessageBox::NoButton);
            restUsrRsp();
            break;
        }
    }
    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"MAC write response error!");
        QMessageBox::information(this, "MAC", "MAC write response error!", QMessageBox::Yes, QMessageBox::NoButton);

        restUsrRsp();
        return ;
    }
    return ;

}
void MainWindow::onChanged(int)
{
    QString value;
    value=Security_comboBox->currentText ();
    if(value=="Disable")
    {
        ui->Encryption_comboBox_2->setEnabled(false);
        ui->Password_lineEdit_2->setEnabled(false);
        ui->TKIP_radioButton_2->setEnabled(false);
        ui->AES_radioButton_2->setEnabled(false);
        ui->AUTO_radioButton_2->setEnabled(false);

        ui->pushButton_Write_2->setEnabled(false);

    }
    else if(value=="WEP")
    {
        ui->Encryption_comboBox_2->setEnabled(true);
        ui->Password_lineEdit_2->setEnabled(true);
        ui->TKIP_radioButton_2->setEnabled(false);
        ui->AES_radioButton_2->setEnabled(false);
        ui->AUTO_radioButton_2->setEnabled(false);

        ui->pushButton_Write_2->setEnabled(true);

    }
    else
    {
        ui->Encryption_comboBox_2->setEnabled(false);
        ui->Password_lineEdit_2->setEnabled(true);
        ui->TKIP_radioButton_2->setEnabled(true);
        ui->AES_radioButton_2->setEnabled(true);
        ui->AUTO_radioButton_2->setEnabled(true);

        ui->pushButton_Write_2->setEnabled(true);
    }

}


void MainWindow::PassWordWrite()
{
    if(Get_Device==false)
    {
        OnPopupMsgWithOK("ReliabilityTest","please insert a device to test!");
        return;
    }
   if(ui->Security_comboBox_2->currentText()=="Disable")
       security_mode=SEC_DISABLED;
   else
       if(ui->Security_comboBox_2->currentText ()=="WEP")
           security_mode=SEC_STATIC_WEP;
   else
       if(ui->Security_comboBox_2->currentText ()=="WPA PSK")
           security_mode=SEC_WPA_PSK;
   else
       if(ui->Security_comboBox_2->currentText ()=="WPA2 PSK")
           security_mode=SEC_WPA2_PSK;
   else
       if(ui->Security_comboBox_2->currentText ()=="WPA/WPA2 PSK")
           security_mode=SEC_MIXED_WPA_PSK;

   if(ui->Encryption_comboBox_2->currentText()=="Open")
       EncryptionWep=OPEN_SYSTEM;
   else
       if(ui->Encryption_comboBox_2->currentText()=="Share")
           EncryptionWep=SHARED_KEY;

   int a = btnGroup->checkedId();           //get the button which one is pressed
   if(a==0)
       encryption_Cipher_Type=WPA_TKIP;
   else if(a==1)
       encryption_Cipher_Type=WPA_CCMP;
   else if(a==2)
       encryption_Cipher_Type=WPA_AUTO;
   key=ui->Password_lineEdit_2->text();
   bool Check_Result=check_key(key,security_mode);
   if(Check_Result==false&&security_mode==SEC_STATIC_WEP)
       QMessageBox::information(this, "PassWord", "Wep Key is 5 or 13 ASCII characters,or 10 or 26 hexadecimal digits!", QMessageBox::Yes, QMessageBox::NoButton);
   else
       if(Check_Result==false)
       QMessageBox::information(this, "PassWord", "The WPA Pre-Shared Key (PSK) should be 8-63 ASCII characters!", QMessageBox::Yes, QMessageBox::NoButton);
   else
   {

       e_jrd_rw_flag Read_Write=E_JRD_WRITE;
       if(DIAG_Wifi_PassWord_RW(key, Read_Write))
       {
           QMessageBox::information(this, "PassWord", "Write password successfully!", QMessageBox::Yes, QMessageBox::NoButton);
           Is_Wifi_Password_Write=true;
       }
       else
           QMessageBox::information(this, "PassWord", "Write password failed!", QMessageBox::Yes, QMessageBox::NoButton);
   }

}

bool MainWindow::check_key(QString key,wlan_security_mode_enum_type security_mode)
{
    int length=key.size();
    if(security_mode==SEC_DISABLED)
        return false;
    else
        if(security_mode==SEC_STATIC_WEP)
        {
            if(length==5||length==13||length==10||length==26)
                ;
            else
                return false;
            if(length==10||length==26)
            {
                for(int i=0;i<length;i++)
                {
                    if(key.mid(i,1)=="0"||key.mid(i,1)=="1"||key.mid(i,1)=="2"||key.mid(i,1)=="3"||key.mid(i,1)=="4"||key.mid(i,1)=="5"
                    ||key.mid(i,1)=="6"||key.mid(i,1)=="7"||key.mid(i,1)=="8"||key.mid(i,1)=="9"||key.mid(i,1)=="a"||key.mid(i,1)=="b"
                    ||key.mid(i,1)=="c"||key.mid(i,1)=="d"||key.mid(i,1)=="e"||key.mid(i,1)=="f"||key.mid(i,1)=="A"||key.mid(i,1)=="B"
                    ||key.mid(i,1)=="C"||key.mid(i,1)=="D"||key.mid(i,1)=="E"||key.mid(i,1)=="F")
                       ;
                    else
                        return false;
                }
            }
        }
    else
        if(length<8||length>63)
            return false;
    return true;
}
bool MainWindow::DIAG_Wifi_PassWord_RW(QString key,e_jrd_rw_flag Read_Write)
{
    jrd_diag_wifi_sec_wr_req_rsp_type req,resp;
    short resp_len = sizeof(jrd_diag_wifi_sec_wr_req_rsp_type);


    req.hdr.cmd_entry = JRD_DIAG_CMD_F;
    req.hdr.class_code = E_JRD_DIAG_WIFI;
    req.hdr.cmd_code = E_JRD_DIAG_WIFI_SECURITY_RW;
    req.rw_flag = E_JRD_WRITE;
    req.security_mode=security_mode;
    if(security_mode==SEC_STATIC_WEP)
        req.sub_mode=EncryptionWep;
    else
        req.sub_mode=encryption_Cipher_Type;

    char *temp;
    QByteArray ba=key.toLatin1();
    temp=ba.data();

    memcpy(req.password,temp,MAX_PASSWORD_LEN+1);

    for(int i = 0; i < RETRY_TIMES; i++)
    {
        if(QLIB_SendSync(
                    m_QHandle,
                    sizeof(jrd_diag_wifi_sec_wr_req_rsp_type),
                    (unsigned char *)&req,
                    &resp_len,
                    (unsigned char *)&resp,
                    10000) != TRUE)
        {
            if(i == RETRY_TIMES - 1)
            {
                ERROR(FILE_LINE,"WIFI PassWord write send command error!");


                return FALSE;
            }
            else
            {
                SLEEP(200);
            }
        }
        else
        {
            INFO(FILE_LINE,"WIFI PassWord write send command ok,the WIFI PassWord = %s",temp);


            break;
        }
    }

    if(resp.diag_errno != 0)
    {
        ERROR(FILE_LINE,"WIFI PassWord write response error!");

        return FALSE;
    }

    return TRUE;
}
bool MainWindow::GetWIFI_PASSWORD()
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
        QMessageBox::information(this, "PassWord", "Get Imei failed!", QMessageBox::Yes, QMessageBox::NoButton);
        //emit signalDisplayDiagResult("IMEI","IMEI read send command error",false);
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
        QString Main_wifi=temp2.mid(8,7);
        Main_wifi+="a";
        QString Guest_wifi=temp2.mid(11,4);
        Guest_wifi+="b";
        ui->lineEdit_Guset_wifi->setText(Guest_wifi);
        ui->lineEdit_Main_wifi->setText(Main_wifi);

        //emit signalDisplayDiagResult("IMEI",temp2,true);
    }
    else
    {
        ERROR(FILE_LINE,"IMEI read response error");
        QMessageBox::information(this, "PassWord", "Get Imei failed!", QMessageBox::Yes, QMessageBox::NoButton);
        return FALSE;
    }


    return TRUE;
}

