#ifndef MAINAPP_H
#define MAINAPP_H

#include <QApplication>
#include "src/define/stdafx.h"

class MainApp : public QApplication
{
    Q_OBJECT
public:
    MainApp(int &argc, char **argv);
    ~MainApp();
    //MainApp(int &argc, char **argv);

    void		    SwitchCDROM(void);
    void		    EnumDiagPort(void);
    void            SwitchDisk(void);

protected:
    virtual bool    winEventFilter(MSG* msg, long* result );

signals:
    void            signalDeviceArrive(int port);
    void            signalDeviceRemove();

private:
    uint16		   testPort;

};

#endif // MAINAPP_H
