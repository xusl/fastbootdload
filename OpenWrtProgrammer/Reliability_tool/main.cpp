#include <QApplication>
#include "mainwindow.h"
#include "mainapp.h"
#include <QSystemSemaphore>
#include <QSharedMemory>

int main(int argc, char *argv[])
{

    QSharedMemory *shareMem = new QSharedMemory(QString("Reliability"));
    volatile short i = 2;
    while (i--)
    {
        if (shareMem->attach(QSharedMemory::ReadOnly))
        {
            shareMem->detach();
        }
    }
    if (shareMem->create(1))
    {
        MainApp a(argc, argv);
        MainWindow w(0,&a);
        w.show();
        return a.exec();

        if (shareMem->isAttached())
            shareMem->detach();

        delete shareMem;
    }



}
