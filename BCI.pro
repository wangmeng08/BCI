QT       += core gui serialport sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    basedialog.cpp \
    basewindow.cpp \
    choosetype.cpp \
    constvalue.cpp \
    datamanager.cpp \
    db.cpp \
    eventmanager.cpp \
    logmanager.cpp \
    main.cpp \
    mainwindow.cpp \
    mainwindowhifu.cpp \
    mainwindowlifu.cpp \
    messageinfo.cpp \
    option.cpp \
    patientcreate.cpp \
    patientmanager.cpp \
    profileload.cpp \
    savedialog.cpp \
    serialmanager.cpp \
    txitem.cpp

HEADERS += \
    basedialog.h \
    basewindow.h \
    choosetype.h \
    constvalue.h \
    datamanager.h \
    db.h \
    enuminfo.h \
    eventmanager.h \
    logmanager.h \
    mainwindow.h \
    mainwindowhifu.h \
    mainwindowlifu.h \
    messageinfo.h \
    option.h \
    patientcreate.h \
    patientmanager.h \
    profileload.h \
    savedialog.h \
    serialmanager.h \
    txitem.h

FORMS += \
    choosetype.ui \
    mainwindow.ui \
    mainwindowhifu.ui \
    mainwindowlifu.ui \
    messageinfo.ui \
    option.ui \
    patientcreate.ui \
    patientmanager.ui \
    profileload.ui \
    savedialog.ui \
    txitem.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
