#include "generalsettings.h"
#include "ui_generalsettings.h"

GeneralSettings::GeneralSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralSettings)
{
    ui->setupUi(this);
    ui->noiseReductionLineEdit->setValidator(new QRegExpValidator(QRegExp("^$|^[1-9][0-9]{0,1}$|^100$"), this));
    ui->remoteControlTCPPort->setValidator(new QRegExpValidator(QRegExp("^$|^[1-9][0-9]{0,5}$"), this));
    ui->lmDampingFactor->setValidator(new QRegExpValidator(QRegExp("^$|^-[1-9][.][0-9]{1,2}|-0[.][1-9]{1,2}|[0-9][.][0-9]{1,2}|-[1-9][0-9]{0,1}|[1-9][0-9]{0,1}$"), this));
    ui->lmMaxIteration->setValidator(new QRegExpValidator(QRegExp("^$|^[1-9][0-9]{0,1}$|^100$"), this));
    ui->gdDampingFactor->setValidator(new QRegExpValidator(QRegExp("^$|^-[1-9][.][0-9]{1,2}|-0[.][1-9]{1,2}|[0-9][.][0-9]{1,2}|-[1-9][0-9]{0,1}|[1-9][0-9]{0,1}$"), this));
    ui->gdMaxIteration->setValidator(new QRegExpValidator(QRegExp("^$|^[1-9][0-9]{0,1}$|^100$"), this));

    connect(ui->autoRangeCheckBox, SIGNAL(toggled(bool)), this, SLOT(sms500AutoRangeChanged(bool)));
    connect(ui->noiseReductionCheckBox, SIGNAL(toggled(bool)), this, SLOT(sms500NoiseReductionChanged(bool)));
    connect(ui->noiseReductionLineEdit, SIGNAL(editingFinished()), this, SLOT(sms500NoiseReductionLineEditChanged()));
    connect(ui->remoteControlTCPPort, SIGNAL(editingFinished()), this, SLOT(remoteControlTCPPortChanged()));
    connect(ui->lmDampingFactor, SIGNAL(editingFinished()), this, SLOT(lmDampingFactorChanged()));
    connect(ui->lmMaxIteration, SIGNAL(editingFinished()), this, SLOT(lmMaxIterationChanged()));
    connect(ui->gdDampingFactor, SIGNAL(editingFinished()), this, SLOT(gdDampingFactorChanged()));
    connect(ui->gdMaxIteration, SIGNAL(editingFinished()), this, SLOT(gdMaxIterationChanged()));
    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(saveSettings()));

    filePath = QDir::currentPath() + "/config.txt";
    file.open(tr("Load Config file"), filePath);

    // Get default settings
    settings.ledDriverTest = ledDriverTestSettings();
    settings.remoteControl = remoteControlSettings();
    settings.starSimulator = starSimulatorSettings();
    updateGui();
}

GeneralSettings::~GeneralSettings()
{
    delete ui;
}

void GeneralSettings::updateGui()
{
    // LED Driver Test
    ui->autoRangeCheckBox->setChecked(settings.ledDriverTest.autoRange);
    ui->integrationTimeComboBox->setCurrentIndex(settings.ledDriverTest.integrationTime);
    ui->samplesToAverageSpinBox->setValue(settings.ledDriverTest.samplesToAverage);
    ui->smoothingSpinBox->setValue(settings.ledDriverTest.boxCarSmoothing);
    ui->noiseReductionCheckBox->setChecked(settings.ledDriverTest.noiseReduction);
    ui->noiseReductionLineEdit->setText(tr("%1").arg(settings.ledDriverTest.noiseReductionFactor));
    ui->dynamicDarkCheckBox->setChecked(settings.ledDriverTest.dynamicDark);

    // Remote Control
    ui->remoteControlTCPPort->setText(tr("%1").arg(settings.remoteControl.tcpPort));

    // Star Simulator
    ui->lmDampingFactor->setText(tr("%1").arg(settings.starSimulator.lmDampingFactor));
    ui->lmMaxIteration->setText(tr("%1").arg(settings.starSimulator.lmMaxIteration));
    ui->gdDampingFactor->setText(tr("%1").arg(settings.starSimulator.gdDampingFactor));
    ui->gdMaxIteration->setText(tr("%1").arg(settings.starSimulator.gdMaxIteration));
}

void GeneralSettings::sms500AutoRangeChanged(bool enable)
{
    ui->integrationTimeComboBox->setEnabled(!enable);
}

void GeneralSettings::sms500NoiseReductionChanged(bool enable)
{
    ui->noiseReductionLineEdit->setEnabled(enable);
}

void GeneralSettings::sms500NoiseReductionLineEditChanged()
{
    if (ui->noiseReductionLineEdit->text().isEmpty())
        ui->noiseReductionLineEdit->setText("5");
}

SMS500Parameters GeneralSettings::ledDriverTestSettings()
{
    SMS500Parameters settings;
    QStringList list = (file.readSection(tr("[LedDriverTest]"))).split("\n");

    settings.operationMode = Flux;
    settings.numberOfScans = -1;
    settings.startWave     = 360;
    settings.stopWave      = 1000;

    if (!list.isEmpty()) {
        QStringList temp;

        if (!(temp = list.filter(tr("AutoRange"))).isEmpty())
            settings.autoRange = temp.at(0).mid(10, 1).toInt();

        if (!(temp = list.filter(tr("IntegrationTime"))).isEmpty())
            settings.integrationTime = temp.at(0).mid(16, 2).toInt();

        if (!(temp = list.filter(tr("SamplesToAverage"))).isEmpty())
            settings.samplesToAverage = temp.at(0).mid(17, 2).toInt();

        if (!(temp = list.filter(tr("BoxCarSmoothing"))).isEmpty())
            settings.boxCarSmoothing = temp.at(0).mid(16, 2).toInt();

        if (!(temp = list.filter(tr("NoiseReduction"))).isEmpty())
            settings.noiseReduction = temp.at(0).mid(15, 1).toInt();

        if (!(temp = list.filter(tr("NoiseReductionFactor"))).isEmpty())
            settings.noiseReductionFactor = temp.at(0).mid(21, 3).toInt();

        if (!(temp = list.filter(tr("DynamicDark"))).isEmpty())
            settings.dynamicDark = temp.at(0).mid(12, 1).toInt();
    }

    return settings;
}

void GeneralSettings::remoteControlTCPPortChanged()
{
    if (ui->remoteControlTCPPort->text().isEmpty())
        ui->remoteControlTCPPort->setText("6000");
}

RemoteControlParameters GeneralSettings::remoteControlSettings()
{
    RemoteControlParameters settings;
    QStringList list = (file.readSection(tr("[RemoteControl]"))).split("\n");

    if (!list.isEmpty()) {
        QStringList temp;

        if (!(temp = list.filter(tr("TCPPort"))).isEmpty())
            settings.tcpPort = temp.at(0).mid(8, 6).toInt();
    }

    return settings;
}

void GeneralSettings::lmDampingFactorChanged()
{
    if (ui->lmDampingFactor->text().isEmpty())
        ui->lmDampingFactor->setText("6.5");
}

void GeneralSettings::lmMaxIterationChanged()
{
    if (ui->lmMaxIteration->text().isEmpty())
        ui->lmMaxIteration->setText("20");
}

void GeneralSettings::gdDampingFactorChanged()
{
    if (ui->gdDampingFactor->text().isEmpty())
        ui->gdDampingFactor->setText("0.05");
}

void GeneralSettings::gdMaxIterationChanged()
{
    if (ui->gdMaxIteration->text().isEmpty())
        ui->gdMaxIteration->setText("10");
}

StarSimulatorParameters GeneralSettings::starSimulatorSettings()
{
    StarSimulatorParameters settings;
    QStringList list = (file.readSection(tr("[StarSimulator]"))).split("\n");

    if (!list.isEmpty()) {
        QStringList temp;

        if (!(temp = list.filter(tr("LMDampingFactor"))).isEmpty())
            settings.lmDampingFactor = temp.at(0).mid(16, 10).toDouble();

        if (!(temp = list.filter(tr("LMMaxIteration"))).isEmpty())
            settings.lmMaxIteration = temp.at(0).mid(15, 10).toInt();

        if (!(temp = list.filter(tr("GDDampingFactor"))).isEmpty())
            settings.gdDampingFactor = temp.at(0).mid(16, 10).toDouble();

        if (!(temp = list.filter(tr("GDMaxIteration"))).isEmpty())
            settings.gdMaxIteration = temp.at(0).mid(15, 10).toInt();
    }

    return settings;
}

void GeneralSettings::saveSettings()
{
    saveLedDriverTestSettings();
    saveRemoteControlSettings();
    saveStarSimulatorSettings();
    this->close();
}

void GeneralSettings::saveSettings(const SMS500Parameters &parameters)
{
    QString data;
    data.append(tr("[LedDriverTest]\n"));
    data.append(tr("AutoRange=%1\n").arg(parameters.autoRange));
    data.append(tr("IntegrationTime=%1\n").arg(parameters.integrationTime));
    data.append(tr("SamplesToAverage=%1\n").arg(parameters.samplesToAverage));
    data.append(tr("BoxCarSmoothing=%1\n").arg(parameters.boxCarSmoothing));
    data.append(tr("NoiseReduction=%1\n").arg(parameters.noiseReduction));
    data.append(tr("NoiseReductionFactor=%1\n").arg(parameters.noiseReductionFactor));
    data.append(tr("DynamicDark=%1\n").arg(parameters.dynamicDark));

    file.save(data, tr("Save Config file"), tr("[LedDriverTest]"), filePath);
}

void GeneralSettings::saveSettings(const RemoteControlParameters &parameters)
{
    QString data;
    data.append(tr("[RemoteControl]\n"));
    data.append(tr("TCPPort=%1\n").arg(parameters.tcpPort));

    file.save(data, tr("Save Config file"), tr("[RemoteControl]"), filePath);
}

void GeneralSettings::saveSettings(const StarSimulatorParameters &parameters)
{
    QString data;
    data.append(tr("[StarSimulator]\n"));
    data.append(tr("LMDampingFactor=%1\n").arg(parameters.lmDampingFactor));
    data.append(tr("LMMaxIteration=%1\n").arg(parameters.lmMaxIteration));
    data.append(tr("GDDampingFactor=%1\n").arg(parameters.gdDampingFactor));
    data.append(tr("GDMaxIteration=%1\n").arg(parameters.gdMaxIteration));

    file.save(data, tr("Save Config file"), tr("[StarSimulator]"), filePath);
}

void GeneralSettings::saveLedDriverTestSettings()
{
    settings.ledDriverTest.autoRange            = ui->autoRangeCheckBox->isChecked();
    settings.ledDriverTest.integrationTime      = ui->integrationTimeComboBox->currentIndex();
    settings.ledDriverTest.samplesToAverage     = ui->samplesToAverageSpinBox->value();
    settings.ledDriverTest.boxCarSmoothing      = ui->smoothingSpinBox->value();
    settings.ledDriverTest.noiseReduction       = ui->noiseReductionCheckBox->isChecked();
    settings.ledDriverTest.noiseReductionFactor = ui->noiseReductionLineEdit->text().toInt();
    settings.ledDriverTest.dynamicDark          = ui->dynamicDarkCheckBox->isChecked();

    saveSettings(settings.ledDriverTest);
}

void GeneralSettings::saveRemoteControlSettings()
{
    settings.remoteControl.tcpPort = ui->remoteControlTCPPort->text().toInt();

    saveSettings(settings.remoteControl);
}

void GeneralSettings::saveStarSimulatorSettings()
{
    settings.starSimulator.lmDampingFactor = ui->lmDampingFactor->text().toDouble();
    settings.starSimulator.lmMaxIteration  = ui->lmMaxIteration->text().toInt();
    settings.starSimulator.gdDampingFactor = ui->gdDampingFactor->text().toDouble();
    settings.starSimulator.gdMaxIteration  = ui->gdMaxIteration->text().toInt();

    saveSettings(settings.starSimulator);
}
