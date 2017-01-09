#ifndef DIAGTHREAD_H
#define DIAGTHREAD_H

#include <QThread>
#include "jrd_diag.h"
#include "mainwindow.h"
#include "src/QMSL/QLib.h"
#include "src/log/log.h"

class MainWindow;

class DiagThread : public QThread
{
    Q_OBJECT
public:

    explicit DiagThread(QObject *parent = 0, MainWindow *window = NULL);

    ~DiagThread();

signals:
    void  signalPopupMsgWithOK(QString title,QString msg);
    void  signalDisplayDiagResult(QString,QString,bool);
    void  signalDiagOver();

public slots:

protected:
    void run();

private :
        bool             DIAG_ReadFIRMWAREVersion();
        bool             DIAG_PassWord_Read();
        bool             DIAG_ReadSSID();
        bool             DIAG_ReadIMEI();
        bool             DIAG_MAC_READ();
        MainWindow        *m_mainWindow;





};

#endif // DIAGTHREAD_H
