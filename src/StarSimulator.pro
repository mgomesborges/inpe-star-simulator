#-------------------------------------------------
#
# Project created by QtCreator 2013-06-24T14:47:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = StarSimulator
TEMPLATE = app

SOURCES += main.cpp\
    mainwindow.cpp \
    aboutsmsdialog.cpp \
    plot.cpp \
    zoomer.cpp \
    sms500.cpp \
    leddriver.cpp \
    longtermstability.cpp \
    lsqloadleddata.cpp \
    lsqloadleddatadialog.cpp \
    lsqnonlin.cpp \
    lsqstardatadialog.cpp

HEADERS  += mainwindow.h \
    aboutsmsdialog.h \
    SpecData.h \
    plot.h \
    zoomer.h \
    sms500.h \
    leddriver.h \
    ftd2xx.h \
    longtermstability.h \
    lsqloadleddata.h \
    lsqloadleddatadialog.h \
    lsqnonlin.h \
    lsqstardatadialog.h

FORMS    += mainwindow.ui \
    aboutsmsdialog.ui \
    longtermstabilitydialog.ui \
    lsqloadleddatadialog.ui \
    lsqstardatadialog.ui

RESOURCES += \
    Pics.qrc

CONFIG   += qwt

unix:!macx:!symbian|win32: LIBS += -L$$PWD/ -lftd2xx

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

win32: PRE_TARGETDEPS += $$PWD/ftd2xx.lib
else:unix:!macx:!symbian: PRE_TARGETDEPS += $$PWD/libftd2xx.a

unix:!macx:!symbian|win32: LIBS += -L$$PWD/ -lSpecData

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

win32: PRE_TARGETDEPS += $$PWD/SpecData.lib
else:unix:!macx:!symbian: PRE_TARGETDEPS += $$PWD/libSpecData.a
