QT += core gui
QT += sql
QT += serialport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = terminal
TEMPLATE = app

SOURCES += \
    arhivdialog.cpp \
    dtbase.cpp \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp \
    trparam.cpp

HEADERS += \
    arhivdialog.h \
    dtbase.h \
    mainwindow.h \
    settingsdialog.h \
    console.h \
    trparam.h

FORMS += \
    arhivdialog.ui \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    terminal.qrc

