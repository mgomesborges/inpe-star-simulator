#-------------------------------------------------
#
# Project created by QtCreator 2013-06-24T14:47:51
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = StarSimulator
TEMPLATE = app

SOURCES += main.cpp\
    mainwindow.cpp \
    aboutsmsdialog.cpp \
    plot.cpp \
    sms500.cpp \
    leddriver.cpp \
    longtermstabilityexportdialog.cpp \
    longtermstability.cpp \
    longtermstabilityalarmclock.cpp \
    star.cpp \
    filehandle.cpp \
    utils.cpp \
    ftdidevicechooserdialog.cpp \
    remotecontrol.cpp \
    configurechannelsdialog.cpp \
    starsimulator.cpp \
    starsimulatorloadleddata.cpp

HEADERS  += mainwindow.h \
    aboutsmsdialog.h \
    SpecData.h \
    plot.h \
    sms500.h \
    leddriver.h \
    ftd2xx.h \
    longtermstabilityexportdialog.h \
    longtermstability.h \
    longtermstabilityalarmclock.h \
    star.h \
    version.h \
    filehandle.h \
    utils.h \
    ftdidevicechooserdialog.h \
    remotecontrol.h \
    configurechannelsdialog.h \
    starsimulator.h \
    starsimulatorloadleddata.h

FORMS    += mainwindow.ui \
    aboutsmsdialog.ui \
    longtermstabilityexportdialog.ui \
    ftdidevicechooserdialog.ui \
    remotecontrol.ui \
    configurechannelsdialog.ui \
    starsimulatorloadleddata.ui

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
