#-------------------------------------------------
#
# Project created by QtCreator 2013-01-07T01:07:18
#
#-------------------------------------------------

QT       += core gui

CONFIG += serialport

TARGET   = SMS500_LEDdriver
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    aboutsmsdialog.cpp \
    plot.cpp \
    zoomer.cpp \
    sms500.cpp \
    leddriver.cpp \
    star.cpp \
    leastsquarenonlin.cpp

HEADERS  += mainwindow.h \
    aboutsmsdialog.h \
    SpecData.h \
    plot.h \
    zoomer.h \
    sms500.h \
    leddriver.h \
    ftd2xx.h \
    star.h \
    leastsquarenonlin.h

FORMS    += mainwindow.ui \
    aboutsmsdialog.ui \
    star.ui

RESOURCES += \
    Pics.qrc

CONFIG += qwt

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
