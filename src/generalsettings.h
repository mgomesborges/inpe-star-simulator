#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <QDialog>
#include "filehandle.h"
#include "datatype.h"

namespace Ui {
class GeneralSettings;
}

class GeneralSettings : public QDialog
{
    Q_OBJECT

public:
    struct Parameters {
        SMS500Parameters ledDriverTest;
        RemoteControlParameters remoteControl;
        StarSimulatorParameters starSimulator;
    };

    explicit GeneralSettings(QWidget *parent = 0);
    ~GeneralSettings();

public slots:
    void updateGui();

    void sms500AutoRangeChanged(bool enable);
    void sms500NoiseReductionChanged(bool enable);
    void sms500NoiseReductionLineEditChanged();
    SMS500Parameters ledDriverTestSettings();

    void remoteControlTCPPortChanged();
    RemoteControlParameters remoteControlSettings();

    void lmDampingFactorChanged();
    void lmMaxIterationChanged();
    void gdDampingFactorChanged();
    void gdMaxIterationChanged();
    void objectiveFunctionFactorChanged();
    StarSimulatorParameters starSimulatorSettings();

    void saveSettings();
    void saveSettings(const SMS500Parameters &parameters);
    void saveSettings(const RemoteControlParameters &parameters);
    void saveSettings(const StarSimulatorParameters &parameters);

private:
    Ui::GeneralSettings *ui;
    QString filePath;
    FileHandle file;
    Parameters settings;

    void saveLedDriverTestSettings();
    void saveRemoteControlSettings();
    void saveStarSimulatorSettings();
};

#endif // GENERALSETTINGS_H
