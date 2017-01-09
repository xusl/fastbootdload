#-------------------------------------------------
#
# Project created by QtCreator 2013-12-03T08:57:00
#
#-------------------------------------------------

QT       += core gui
QT += widgets
QT += network

TARGET = ReliabilityTool
TEMPLATE = app

CONFIG += -staticlib
CONFIG(debug, debug_and_release)

LIBS += -lsetupapi QMSL_MSVC6R.dll
    LIBS += -lsetupapi
    LIBS += -liphlpapi


SOURCES += main.cpp\
        mainwindow.cpp \
    src/define/stdafx.cpp \
    src/device/ziUsbDevice.cpp \
    src/device/ziBasic.cpp \
    src/device/device.cpp \
    src/device/ziUsbDisk.cpp \
    src/log/log.cpp \
    src/scsi/scsicmd.cpp \
    src/serialport/serialport.cpp \
    src/shiftdevice/shiftdevice.cpp \
    src/utils/utils.cpp \
    src/packet/pkt.cpp \
    testitemsview.cpp \
    mainapp.cpp \
    testthread.cpp \
    diagthread.cpp \
    src/Diag/diagcmd.cpp \
    src/Diag/jrddiagcmd.cpp

HEADERS  += mainwindow.h \
    src/define/define.h \
    src/define/stdafx.h \
    src/device/ziUsbDevice.h \
    src/device/ziBasic.h \
    src/device/device.h \
    src/device/ziUsbDisk.h \
    src/log/log.h \
    src/scsi/scsicmd.h \
    src/serialport/serialport.h \
    src/shiftdevice/shiftdevice.h \
    src/utils/utils.h \
    src/packet/pkt.h \
    jrd_diag.h \
    src/QMSL/QLibb.h \
    src/QMSL/QLib_SoftwareDownloadDefines.h \
    src/QMSL/QLib_Defines.h \
    src/QMSL/QLib.h \
    src/QMSL/QMSL_Linear_Predistortion_Refwaveform.h \
    testitemsview.h \
    mainapp.h \
    testthread.h \
    diagthread.h \
    src/Diag/diagcmd.h \
    src/Diag/jrddiagcmd.h

FORMS    += mainwindow.ui \
    testitemsview.ui

RC_FILE += \
    ReliabilityTool.rc \
    Icon.ico
