#-------------------------------------------------
#
# Project created by QtCreator 2013-06-24T14:47:51
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = StarSimulator
TEMPLATE = app

SOURCES += main.cpp\
    mainwindow.cpp \
    aboutsmsdialog.cpp \
    plot.cpp \
    sms500.cpp \
    leddriver.cpp \
    lsqloadleddata.cpp \
    lsqloadleddatadialog.cpp \
    lsqnonlin.cpp \
    longtermstabilityexportdialog.cpp \
    longtermstability.cpp \
    longtermstabilityalarmclock.cpp \
    star.cpp \
    filehandle.cpp \
    utils.cpp

HEADERS  += mainwindow.h \
    aboutsmsdialog.h \
    SpecData.h \
    plot.h \
    sms500.h \
    leddriver.h \
    ftd2xx.h \
    lsqloadleddata.h \
    lsqloadleddatadialog.h \
    lsqnonlin.h \
    longtermstabilityexportdialog.h \
    longtermstability.h \
    longtermstabilityalarmclock.h \
    star.h \
    version.h \
    filehandle.h \
    utils.h

FORMS    += mainwindow.ui \
    aboutsmsdialog.ui \
    lsqloadleddatadialog.ui \
    longtermstabilityexportdialog.ui

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

OTHER_FILES += \
    project.rc

win32:RC_FILE = project.rc
