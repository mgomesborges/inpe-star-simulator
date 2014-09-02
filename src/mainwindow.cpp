#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->ledDriverWidget->setLayout(createLedDriverLayout());
    ui->starSimulatorWidget->setLayout(createX0GridLayout());
    ui->scanNumberLabel->hide();
    ui->scanNumberLabel_2->hide();
    ui->saturedLabel->hide();
    ui->saturedLabel_2->hide();
    ui->startLongTermStabilityLabel->hide();
    ui->startLongTermStabilityLabel_2->hide();
    ui->stopLongTermStabilityLabel->hide();
    ui->stopLongTermStabilityLabel_2->hide();

    plotSMS500                  = new Plot(ui->plotArea, tr("Amplitude"));
    plotLedDriver               = new Plot(ui->plotAreaLed, tr("Amplitude"));
    plotLSqNonLin               = new Plot(ui->plotStarSimulator, tr("Collimator Output"), tr("Star Irradiance"));
    plotLTS                     = new Plot(ui->plotAreaLongTermStability, tr("Amplitude"));
    sms500                      = new SMS500(this);
    ledDriver                   = new LedDriver(this);
    lsqNonLinStar               = new Star(this);
    lsqnonlin                   = new StarSimulator(this);
    longTermStability           = new LongTermStability(this);
    longTermStabilityAlarmClock = new LongTermStabilityAlarmClock(this);
    remoteControl               = new RemoteControl(this);

    outputIrradiance = 0;
    starIrradiance   = 0;

    lastDir = QDir::homePath();

    GeneralSettings settings;
    remoteControl->setPort(settings.remoteControlSettings().tcpPort);
    remoteControl->listen();

    plotSMS500->setxLabel(tr("Wavelength (nm)"));
    plotSMS500->setyLabel(tr("uW/nm"));

    plotLSqNonLin->setxLabel(tr("Wavelength"));
    plotLSqNonLin->setyLabel(tr("Irradiance (uW/cm^2 nm)"));

    statusLabel  = new QLabel;
    statusBar()->addPermanentWidget( statusLabel );

    uiInputValidator();
    sms500OperationModeChanged();

    // Star Simulator :: Transference Function of Pinhole and Colimator
    starLoadTransferenceFunction();
    lsqNonLinStarSettings();

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateConfigureMenu(int)));
    connect(ui->actionSetValues, SIGNAL(triggered()), this, SLOT(setDigitalLevelForChannels()));
    connect(ui->actionGeneralSettings, SIGNAL(triggered()), this, SLOT(setGeneralSettings()));

    /********************************************
    * SMS500 Signals And Slots
    ********************************************/
    connect(plotSMS500->plotPicker,      SIGNAL(moved(const QPoint&)),      this,       SLOT(plotMoved(const QPoint&)));
    connect(plotSMS500->plotPicker,      SIGNAL(selected(const QPolygon&)), this,       SLOT(plotSelected(const QPolygon&)));
    connect(ui->btnZoom,                 SIGNAL(toggled(bool)),             plotSMS500, SLOT(enableZoomMode(bool)));
    connect(plotSMS500,                  SIGNAL(showInfo()),                this,       SLOT(plotShowInfo()));
    connect(ui->btnStartScan,            SIGNAL(clicked()),                 this,       SLOT(sms500StartStopScan()));
    connect(ui->btnSaveScan,             SIGNAL(clicked()),                 this,       SLOT(sms500SaveScanData()));
    connect(ui->btnConnectDisconnect,    SIGNAL(clicked()),                 this,       SLOT(sms500ConnectDisconnect()));
    connect(ui->rbtnFlux,                SIGNAL(toggled(bool)),             this,       SLOT(sms500OperationModeChanged()));
    connect(ui->rbtnIntensity,           SIGNAL(toggled(bool)),             this,       SLOT(sms500OperationModeChanged()));
    connect(ui->rbtnIrradiance,          SIGNAL(toggled(bool)),             this,       SLOT(sms500OperationModeChanged()));
    connect(ui->rbtnRadiance,            SIGNAL(toggled(bool)),             this,       SLOT(sms500OperationModeChanged()));
    connect(ui->numberOfScansLineEdit,   SIGNAL(textChanged(QString)),      this,       SLOT(sms500NumberOfScansChanged(QString)));
    connect(ui->AutoRangeCheckBox,       SIGNAL(toggled(bool)),             this,       SLOT(sms500AutoRangeChanged(bool)));
    connect(ui->integrationTimeComboBox, SIGNAL(currentIndexChanged(int)),  this,       SLOT(sms500IntegrationTimeChanged(int)));
    connect(ui->samplesToAverageSpinBox, SIGNAL(valueChanged(int)),         this,       SLOT(sms500SamplesToAverageChanged(int)));
    connect(ui->smoothingSpinBox,        SIGNAL(valueChanged(int)),         this,       SLOT(sms500BoxcarSmoothingChanged(int)));
    connect(ui->startWaveLineEdit,       SIGNAL(textChanged(QString)),      this,       SLOT(sms500WavelengthStartChanged(QString)));
    connect(ui->stopWaveLineEdit,        SIGNAL(textChanged(QString)),      this,       SLOT(sms500WavelengthStopChanged(QString)));
    connect(ui->noiseReductionCheckBox,  SIGNAL(toggled(bool)),             this,       SLOT(sms500NoiseReductionChanged(bool)));
    connect(ui->noiseReductionLineEdit,  SIGNAL(textChanged(QString)),      this,       SLOT(sms500NoiseReductionFactorChanged(QString)));
    connect(ui->dynamicDarkCheckBox,     SIGNAL(toggled(bool)),             this,       SLOT(sms500CorrectForDynamicDarkChanged(bool)));
    connect(ui->actionAboutSMS500,       SIGNAL(triggered(bool)),           this,       SLOT(sms500About()));
    connect(ui->actionAboutThisSoftware, SIGNAL(triggered(bool)),           this,       SLOT(aboutThisSoftware()));
    connect(ui->actionSMS500SystemZero,      SIGNAL(triggered(bool)),       this,       SLOT(sms500SystemZero()));
    connect(ui->actionSMS500CalibrateSystem, SIGNAL(triggered(bool)),       this,       SLOT(sms500CalibrateSystem()));
    connect(sms500, SIGNAL(scanPerformed(int)),          this, SLOT(sms500ScanDataHandle(int)));
    connect(sms500, SIGNAL(scanFinished()),              this, SLOT(sms500StopScan()));
    connect(sms500, SIGNAL(saturedData(bool)),           this, SLOT(sms500SaturedDataHandle(bool)));
    connect(sms500, SIGNAL(integrationTimeChanged(int)), ui->integrationTimeComboBox, SLOT(setCurrentIndex(int)));

    /********************************************
    * LED Driver Signals And Slots
    ********************************************/
    connect(ui->actionLEDDriverTest,                SIGNAL(triggered()), this, SLOT(ledDriverTest()));
    connect(ui->actionConfigureLEDDriverconnection, SIGNAL(triggered()), this, SLOT(ledDriverConfigureConnection()));
    connect(ledDriver, SIGNAL(warningMessage(QString,QString)),          this, SLOT(warningMessage(QString,QString)));
    connect(ledDriver, SIGNAL(performScan()),         this,         SLOT(sms500NextScan()));
    connect(ledDriver, SIGNAL(saveData(QString)),     this,         SLOT(ledModelingSaveData(QString)));
    connect(ledDriver, SIGNAL(modelingFinished()),    this,         SLOT(ledModelingFinished()));
    connect(ledDriver, SIGNAL(testFinished()),        this,         SLOT(ledDriverTestFinished()));
    connect(sms500,    SIGNAL(scanPerformed(int)),    this,         SLOT(ledDriverDataHandle()));
    connect(ui->btnConnectDisconnectLED, SIGNAL(clicked()),          this,      SLOT(ledDriverConnectDisconnect()));
    connect(ui->btnLedModeling,          SIGNAL(clicked()),          this,      SLOT(ledModeling()));
    connect(ui->btnLoadValuesForChannels,SIGNAL(clicked()),          this,      SLOT(ledDriverLoadValuesForChannels()));
    connect(ui->setV2RefCheckBox,        SIGNAL(toggled(bool)),      ledDriver, SLOT(setV2Ref(bool)));
    connect(ui->startChannel,            SIGNAL(editingFinished()),  this,      SLOT(ledModelingSettings()));
    connect(ui->endChannel,              SIGNAL(editingFinished()),  this,      SLOT(ledModelingSettings()));
    connect(ui->levelIncDecValue,        SIGNAL(editingFinished()),  this,      SLOT(ledModelingSettings()));

    for (int i = 0; i < 96; i++)
        connect(ledDriverChannel[i], SIGNAL(editingFinished()), this, SLOT(ledDriverChannelChanged()));

    /********************************************
    * Star Simulator Signals And Slots
    ********************************************/
    connect(ui->actionLoadStarSimulatorDatabase, SIGNAL(triggered(bool)), this, SLOT(lsqNonLinLoadLedData()));
    connect(ui->actionLoadTransferenceFunction,  SIGNAL(triggered(bool)), this, SLOT(starUpdateTransferenceFunction()));
    connect(ui->btnLoadInitialSolution,          SIGNAL(clicked(bool)),   this, SLOT(lsqNonLinLoadInitialSolution()));
    connect(ui->btnSaveStarSimulatorData,        SIGNAL(clicked()),       this, SLOT(lsqNonLinSaveData()));

    connect(lsqnonlin, SIGNAL(ledDataNotFound()),       this, SLOT(lsqNonLinLoadLedData()));
    connect(lsqnonlin, SIGNAL(info(QString)),           this, SLOT(lsqNonLinLog(QString)));
    connect(lsqnonlin, SIGNAL(performScan()),           this, SLOT(sms500NextScan()));
    connect(lsqnonlin, SIGNAL(performScanWithUpdate()), this, SLOT(lsqNonLinPerformScanWithUpdate()));
    connect(lsqnonlin, SIGNAL(finished()),              this, SLOT(lsqNonLinFinished()));
    connect(sms500,    SIGNAL(scanPerformed(int)),      this, SLOT(lsqNonLinObjectiveFunction()));

    connect(ui->starMagnitude,   SIGNAL(editingFinished()), this, SLOT(lsqNonLinStarSettings()));
    connect(ui->starTemperature, SIGNAL(editingFinished()), this, SLOT(lsqNonLinStarSettings()));

    connect(ui->btnStartStopStarSimulator, SIGNAL(clicked()), this, SLOT(lsqNonLinStartStop()));

    connect(ui->x0Random,             SIGNAL(toggled(bool)), this, SLOT(lsqNonLinx0Handle()));
    connect(ui->x0UserDefined,        SIGNAL(toggled(bool)), this, SLOT(lsqNonLinx0Handle()));
    connect(ui->x0DefinedInLedDriver, SIGNAL(toggled(bool)), this, SLOT(lsqNonLinx0Handle()));

    /********************************************
    * long Term Stability Signals And Slots
    ********************************************/
    connect(ui->btnStartStopLongTermStability, SIGNAL(clicked()),  this, SLOT(longTermStabilityStartStop()));
    connect(longTermStabilityAlarmClock,       SIGNAL(finished()), this, SLOT(longTermStabilityStop()));
    connect(sms500, SIGNAL(scanPerformed(int)), this, SLOT(longTermStabilitySaveSMS500Data()));
    connect(ui->btnCreateDatabaseLongTermStability, SIGNAL(clicked()), this, SLOT(longTermStabilityCreateDB()));
    connect(ui->btnOpenDatabaseLongTermStability,   SIGNAL(clicked()), this, SLOT(longTermStabilityOpenDB()));
    connect(ui->btnExportAllLongTermStability,      SIGNAL(clicked()), this, SLOT(longTermStabilityExportAll()));
    connect(ui->tableView,                          SIGNAL(clicked(QModelIndex)), this, SLOT(longTermStabilityHandleTableSelection()));

    /********************************************
    * Remote Control Signals And Slots
    ********************************************/
    connect(remoteControl, SIGNAL(setSMS500AutoRange(bool)), this, SLOT(remoteSetSMS500AutoRange(bool)));
    connect(remoteControl, SIGNAL(SMS500Connect()), this, SLOT(sms500Connect()));
    connect(remoteControl, SIGNAL(SMS500Disconnect()), this, SLOT(sms500Disconnect()));
    connect(remoteControl, SIGNAL(SMS500StartScan()), this, SLOT(sms500StartScan()));
    connect(remoteControl, SIGNAL(SMS500StopScan()), this, SLOT(sms500StopScan()));
    connect(remoteControl, SIGNAL(setSMS500DefaultSettings(SMS500Parameters)), this, SLOT(sms500SetSettings(SMS500Parameters)));
    connect(remoteControl, SIGNAL(setSMS500NumberOfScans(QString)), this, SLOT(remoteSetSMS500NumberOfScans(QString)));
    connect(remoteControl, SIGNAL(setSMS500IntegrationTime(int)), this, SLOT(remoteSetSMS500IntegrationTime(int)));
    connect(remoteControl, SIGNAL(setSMS500SamplesToAverage(int)), this, SLOT(remoteSetSMS500SamplesToAverage(int)));
    connect(remoteControl, SIGNAL(setSMS500BoxcarSmothing(int)), this, SLOT(remoteSetSMS500BoxcarSmothing(int)));
    connect(remoteControl, SIGNAL(setSMS500NoiseReduction(bool)), this, SLOT(remoteSetSMS500NoiseReduction(bool)));
    connect(remoteControl, SIGNAL(setSMS500NoiseReductionFactor(QString)), this, SLOT(remoteSetSMS500NoiseReductionFactor(QString)));
    connect(remoteControl, SIGNAL(setSMS500CorrectForDynamicDark(bool)), this, SLOT(remoteSetSMS500CorrectForDynamicDark(bool)));
    connect(remoteControl, SIGNAL(LEDDriverConnect()), this, SLOT(ledDriverConnect()));
    connect(remoteControl, SIGNAL(LEDDriverDisconnect()), this, SLOT(ledDriverDisconnect()));
    connect(remoteControl, SIGNAL(loadLEDDriverValues(QVector<double>)), this, SLOT(ledDriverGuiUpdate(QVector<double>)));
    connect(remoteControl, SIGNAL(setLEDDriverV2Ref(bool)), ui->setV2RefCheckBox, SLOT(setChecked(bool)));
    connect(remoteControl, SIGNAL(startLEDDriverModeling()), this, SLOT(ledModelingStartPreprocessing()));
    connect(remoteControl, SIGNAL(stopLEDDriverModeling()), this, SLOT(ledModelingStop()));
    connect(remoteControl, SIGNAL(setStarMagnitude(QString)), this, SLOT(remoteSetStarMagnitude(QString)));
    connect(remoteControl, SIGNAL(setStarTemperature(QString)), this, SLOT(remoteSetStarTemperature(QString)));
    connect(remoteControl, SIGNAL(setStarSimulatorAlgorithmLM(bool)), ui->levenbergMarquardt, SLOT(setChecked(bool)));
    connect(remoteControl, SIGNAL(setStarSimulatorAlgorithmGD(bool)), ui->gradientDescent, SLOT(setChecked(bool)));
    connect(remoteControl, SIGNAL(setStarSimulatorX0random(bool)), ui->x0Random, SLOT(setChecked(bool)));
    connect(remoteControl, SIGNAL(setStarSimulatorX0userDefined(bool)), ui->x0UserDefined, SLOT(setChecked(bool)));
    connect(remoteControl, SIGNAL(setStarSimulatorX0ledDriver(bool)), ui->x0DefinedInLedDriver, SLOT(setChecked(bool)));
    connect(remoteControl, SIGNAL(starSimulatorLoadInitialSolution(QVector<double>)), this, SLOT(lsqNonLinX0GuiUpdate(QVector<double>)));
    connect(remoteControl, SIGNAL(startStarSimulator()), this, SLOT(lsqNonLinStart()));
    connect(remoteControl, SIGNAL(stopStarSimulator()), this, SLOT(lsqNonLinStop()));
    connect(remoteControl, SIGNAL(starSimulatorStatus()), this, SLOT(remoteStarSimulatorStatus()));
    connect(remoteControl, SIGNAL(starSimulatorIrradiances()), this, SLOT(remoteStarSimulatorIrradiances()));
    connect(remoteControl, SIGNAL(warningMessage(QString,QString)), this, SLOT(warningMessage(QString,QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

QGridLayout *MainWindow::createLedDriverLayout()
{
    QGridLayout *grid = new QGridLayout;
    int channel  = 0;

    for (int dac = 0; dac < 12; dac++) {
        QGridLayout *subgrid = new QGridLayout;
        QGroupBox *groupBox  = new QGroupBox(tr("DAC %1").arg(dac + 1));
        QLabel *label[8];

        groupBox->setMaximumSize(170, 150);

        for (int i = 0; i < 8; i++) {
            ledDriverChannel[channel] = new QLineEdit("0");
            ledDriverChannel[channel]->setObjectName(tr("%1").arg(channel + 1));
            ledDriverChannel[channel]->setAlignment(Qt::AlignCenter);
            ledDriverChannel[channel]->setMinimumWidth(40);
            label[channel] = new QLabel(tr("C%1").arg(channel + 1));
            subgrid->addWidget(label[channel], i % 4, i / 4 * 2);
            subgrid->addWidget(ledDriverChannel[channel], i % 4, i / 4 * 2 + 1);
            channel++;
        }

        groupBox->setLayout(subgrid);
        grid->addWidget(groupBox, dac % 3, dac / 3);
    }

    return grid;
}

QGridLayout *MainWindow::createX0GridLayout()
{
    QGridLayout *grid = new QGridLayout;

    for (int i = 0; i < 96; i++) {
        starSimulatorX0[i] = new QLineEdit;
        starSimulatorX0[i]->setEnabled(false);
        starSimulatorX0[i]->setMaximumWidth(32);
        starSimulatorX0[i]->setAlignment(Qt::AlignCenter);
        starSimulatorX0[i]->setPlaceholderText(tr("ch%1").arg(i + 1));
        grid->addWidget(starSimulatorX0[i], i / 9, i % 9);
    }

    return grid;
}

void MainWindow::aboutThisSoftware()
{
    QMessageBox::about(this, tr("About this Software"),
                       tr("<font color='#800000'><b>Star Simulator</b> %1, %2</font>"
                          "<p>Based on Qt 5.2.1 (32 bit), Qwt 6.1.0-rc3 and Eigen 3.2.</p>"
                          "<p>This software was developed to simulates stars required to calibrate the "
                          "Autonomous Star Tracker (AST).</p>"
                          "<p><b>Engineering:</b><br>"
                          "Marcos Eduardo Gomes Borges<br>"
                          "mail: <a href='mailto:marcosegborges@gmail.com?Subject=About%20StarSimulator'>marcosegborges@gmail.com</a></p>"
                          "<p><b>With special thanks to:</b><br>"
                          "Braulio Fonseca Carneiro de Albuquerque<br>"
                          "Enrico Spinetta<br>"
                          "Marcio Afonso Arimura Fialho<br>"
                          "Mario Luiz Selingardi</p>"
                          "%3")
                       .arg(VER_FILEVERSION_STR)
                       .arg(VER_BUILD_DATE_STR)
                       .arg(VER_LEGALCOPYRIGHT2_STR));
}

void MainWindow::warningMessage(const QString &caption, const QString &message)
{
    if (remoteControl->isConnected() == false)
        QMessageBox::warning(this, caption, message);
}

void MainWindow::updateConfigureMenu(int selectedTab)
{
    if (selectedTab == 2) {
        ui->actionSetValues->setText("Set Star Simulator x0");
        ui->actionSetValues->setToolTip("Set Star Simulator x0");
    } else {
        ui->actionSetValues->setText("Set LED Driver channels");
        ui->actionSetValues->setToolTip("Set LED Driver channels");
    }
}

void MainWindow::setDigitalLevelForChannels()
{
    ConfigureChannelsDialog *dialog = new ConfigureChannelsDialog(this);

    if (ui->tabWidget->currentIndex() == 1) {
        connect(dialog, SIGNAL(loadValues()), this, SLOT(ledDriverLoadValuesForChannels()));
        connect(dialog, SIGNAL(setValues(QVector<double>)), this, SLOT(ledDriverGuiUpdate(QVector<double>)));
    } else if (ui->tabWidget->currentIndex() == 2) {
        connect(dialog, SIGNAL(loadValues()), this, SLOT(lsqNonLinLoadInitialSolution()));
        connect(dialog, SIGNAL(setValues(QVector<double>)), this, SLOT(lsqNonLinX0GuiUpdate(QVector<double>)));
    }

    dialog->exec();
}

void MainWindow::setGeneralSettings()
{
    GeneralSettings *settings = new GeneralSettings(this);
    settings->exec();

    remoteControl->setPort(settings->remoteControlSettings().tcpPort);
}

void MainWindow::updateLastDir(QString path)
{
    lastDir = path;
}

void MainWindow::plotMoved( const QPoint &pos )
{
    plotShowInfo(tr("Wavelength=%1, Amplitude=%2").arg(plotSMS500->invTransform(QwtPlot::xBottom, pos.x())).arg(plotSMS500->invTransform(QwtPlot::yLeft, pos.y())));
}

void MainWindow::plotSelected( const QPolygon & )
{
    plotShowInfo();
}

void MainWindow::plotShowInfo(const QString &text)
{
    if (text == QString::null) {
        if (plotSMS500->plotPicker->rubberBand())
            statusBar()->showMessage(tr("Cursor Pos: Press left mouse button in plot region"));
        else
            statusBar()->showMessage(tr("Zoom: Press mouse button and drag"));
    }
}

SMS500Parameters MainWindow::sms500Settings()
{
    SMS500Parameters parameters;

    if (ui->rbtnFlux->isChecked())
        parameters.operationMode = Flux;
    else if (ui->rbtnIntensity->isChecked())
        parameters.operationMode = Intensity;
    else if (ui->rbtnIrradiance->isChecked())
        parameters.operationMode = Lux;
    else if (ui->rbtnRadiance->isChecked())
        parameters.operationMode = Cd_m2;

    parameters.numberOfScans        = ui->numberOfScansLineEdit->text().toInt();
    parameters.autoRange            = ui->AutoRangeCheckBox->isChecked();
    parameters.integrationTime      = ui->integrationTimeComboBox->currentIndex();
    parameters.samplesToAverage     = ui->samplesToAverageSpinBox->value();
    parameters.boxCarSmoothing      = ui->smoothingSpinBox->value();
    parameters.startWave            = ui->startWaveLineEdit->text().toInt();
    parameters.stopWave             = ui->stopWaveLineEdit->text().toInt();
    parameters.noiseReduction       = ui->noiseReductionCheckBox->isChecked();
    parameters.noiseReductionFactor = ui->noiseReductionLineEdit->text().toInt();
    parameters.dynamicDark          = ui->dynamicDarkCheckBox->isChecked();

    return parameters;
}

void MainWindow::sms500SetSettings(SMS500Parameters parameters)
{
    // backup previous parameters
    sms500parameters = sms500Settings();

    switch (parameters.operationMode) {
    case Flux:
        ui->rbtnFlux->setChecked(true);
        sms500->setOperationMode(SMS500::Flux);
        plotSMS500->setTitle("Operation Mode: Flux");
        plotLedDriver->setTitle("Operation Mode: Flux");
        break;
    case Intensity:
        ui->rbtnIntensity->setChecked(true);
        sms500->setOperationMode(SMS500::Intensity);
        plotSMS500->setTitle("Operation Mode: Intensity");
        plotLedDriver->setTitle("Operation Mode: Intensity");
        break;
    case Lux:
        ui->rbtnIrradiance->setChecked(true);
        sms500->setOperationMode(SMS500::lux);
        plotSMS500->setTitle("Operation Mode: Irradiance (lux)");
        plotLedDriver->setTitle("Operation Mode: Irradiance (lux)");
        break;
    case Cd_m2:
        ui->rbtnRadiance->setChecked(true);
        sms500->setOperationMode(SMS500::cd_m2);
        plotSMS500->setTitle("Operation Mode: Radiance (cd/m2)");
        plotLedDriver->setTitle("Operation Mode: Radiance (cd/m2)");
        break;
    }

    ui->numberOfScansLineEdit->setText(QString::number(parameters.numberOfScans));
    sms500->setNumberOfScans(parameters.numberOfScans);

    ui->AutoRangeCheckBox->setChecked(parameters.autoRange);
    sms500->setAutoRange(parameters.autoRange);

    ui->integrationTimeComboBox->setCurrentIndex(parameters.integrationTime);
    sms500->setRange(parameters.integrationTime);

    ui->samplesToAverageSpinBox->setValue(parameters.samplesToAverage);
    sms500->setAverage(parameters.samplesToAverage);

    ui->smoothingSpinBox->setValue(parameters.boxCarSmoothing);
    sms500->setBoxCarSmoothing(parameters.boxCarSmoothing);

    ui->startWaveLineEdit->setText(QString::number(parameters.startWave));
    sms500->setStartWave(parameters.startWave);

    ui->stopWaveLineEdit->setText(QString::number(parameters.stopWave));
    sms500->setStopWave(parameters.stopWave);

    ui->noiseReductionCheckBox->setChecked(parameters.noiseReduction);
    ui->noiseReductionLineEdit->setText(QString::number(parameters.noiseReductionFactor));
    sms500->setNoiseReduction(parameters.noiseReduction, parameters.noiseReductionFactor);

    ui->dynamicDarkCheckBox->setChecked(parameters.dynamicDark);
    sms500->setCorrecDarkCurrent(parameters.dynamicDark);
}

void MainWindow::sms500About()
{
    if (sms500->isConnected() == false)
        if (sms500Connect() == false)
            return;

    AboutSMSDialog *dialog = new AboutSMSDialog(this);
    dialog->setDLL(QString::number(sms500->dllVersion()));
    dialog->setSerialNumber(sms500->serialNumber());
    dialog->setFirstCoefficient(QString::number(sms500->coefficient1()));
    dialog->setSecondCoefficient(QString::number(sms500->coefficient2()));
    dialog->setThirdCoefficient(QString::number(sms500->coefficient3()));
    dialog->setIntercept(QString::number(sms500->intercept()));
    dialog->exec();
}

void MainWindow::sms500OperationModeChanged()
{
    if (ui->rbtnFlux->isChecked()) {
        sms500->setOperationMode(SMS500::Flux);
        plotSMS500->setTitle("Operation Mode: Flux");
        plotLedDriver->setTitle("Operation Mode: Flux");
    } else if (ui->rbtnIntensity->isChecked()) {
        sms500->setOperationMode(SMS500::Intensity);
        plotSMS500->setTitle("Operation Mode: Intensity");
        plotLedDriver->setTitle("Operation Mode: Intensity");
    } else if (ui->rbtnIrradiance->isChecked()) {
        sms500->setOperationMode(SMS500::lux);
        plotSMS500->setTitle("Operation Mode: Irradiance (lux)");
        plotLedDriver->setTitle("Operation Mode: Irradiance (lux)");
    } else if (ui->rbtnRadiance->isChecked()) {
        sms500->setOperationMode(SMS500::cd_m2);
        plotSMS500->setTitle("Operation Mode: Radiance (cd/m2)");
        plotLedDriver->setTitle("Operation Mode: Radiance (cd/m2)");
    }
}

void MainWindow::sms500AutoRangeChanged(bool enable)
{
    ui->integrationTimeComboBox->setEnabled(!enable);
    sms500->setAutoRange(enable);
}

void MainWindow::sms500NumberOfScansChanged(QString value)
{
    sms500->setNumberOfScans(value.toInt());
}

void MainWindow::sms500IntegrationTimeChanged(int index)
{
    sms500->setRange(index);
}

void MainWindow::sms500SamplesToAverageChanged(int value)
{
    sms500->setAverage(value);
}

void MainWindow::sms500BoxcarSmoothingChanged(int value)
{
    sms500->setBoxCarSmoothing(value);
}

void MainWindow::sms500WavelengthStartChanged(QString value)
{
    sms500->setStartWave(value.toInt());
}

void MainWindow::sms500WavelengthStopChanged(QString value)
{
    sms500->setStopWave(value.toInt());
}

void MainWindow::sms500NoiseReductionChanged(bool enable)
{
    ui->noiseReductionLineEdit->setEnabled(enable);
    sms500->setNoiseReduction(enable, ui->noiseReductionLineEdit->text().toDouble());
}

void MainWindow::sms500NoiseReductionFactorChanged(QString value)
{
    sms500->setNoiseReduction(ui->noiseReductionCheckBox->isChecked(), value.toDouble());
}

void MainWindow::sms500CorrectForDynamicDarkChanged(bool enable)
{
    sms500->setCorrecDarkCurrent(enable);
}

void MainWindow::sms500ConnectDisconnect()
{
    if (ui->btnConnectDisconnect->text().contains(tr("Reconnect SMS")))
        sms500Connect();
    else
        sms500Disconnect();
}

bool MainWindow::sms500Connect()
{
    if (sms500->isConnected() == true) {
        remoteControl->sendAnswer(RemoteControl::SUCCESS);
        return true;
    }

    sms500Configure();

    if (sms500->openConnection() == true) {
        statusBar()->showMessage(tr("SMS500 successfully connected"),5000);
        ui->btnConnectDisconnect->setIcon(QIcon(":/pics/disconnect.png"));
        ui->btnConnectDisconnect->setText(tr("Disconnect SMS"));
        remoteControl->sendAnswer(RemoteControl::SUCCESS);
        return true;
    }

    warningMessage(tr("Connection Error"), tr("Spectrometer Hardware not found.\n\nCheck USB connection and try again..."));
    statusBar()->showMessage(tr("SMS500 not connected"));
    remoteControl->sendAnswer(RemoteControl::CONNECTION_REFUSED);
    return false;
}

void MainWindow::sms500Disconnect()
{
    if (sms500->isConnected() == true) {
        sms500StopScan();
        sms500->closeConnection();
        statusBar()->showMessage(tr("SMS500 not connected"));
        ui->btnConnectDisconnect->setIcon(QIcon(":/pics/reconnect.png"));
        ui->btnConnectDisconnect->setText(tr("Reconnect SMS"));
    }
}

void MainWindow::sms500StartStopScan()
{
    if (ui->btnStartScan->text().contains("Start Scan"))
        sms500StartScan();
    else
        sms500StopScan();
}

void MainWindow::sms500StartScan()
{
    if (sms500->isConnected() == false)
        if (sms500Connect() == false)
            return;

    if (sms500->isRunning())
        return;

    sms500Configure();
    sms500->start();
    ui->btnStartScan->setIcon(QIcon(":/pics/stop.png"));
    ui->btnStartScan->setText("Stop Scan");
    ui->scanNumberLabel->setText(tr("    Scan number: %1").arg(0));
    ui->scanNumberLabel->show();
    ui->scanNumberLabel_2->setText(tr("    Scan number: %1").arg(0));
    ui->btnZoom->setEnabled(false);
    ui->btnSaveScan->setEnabled(false);
    ui->operationMode->setEnabled(false);
    ui->actionLEDDriverTest->setEnabled(false);
    ui->actionSMS500SystemZero->setEnabled(false);
    ui->actionSMS500CalibrateSystem->setEnabled(false);

    remoteControl->sendAnswer(RemoteControl::SUCCESS);
}

void MainWindow::sms500StopScan()
{
    sms500->stop();
    ui->btnStartScan->setIcon(QIcon(":/pics/start.png"));
    ui->btnStartScan->setText("Start Scan");
    ui->btnZoom->setEnabled(true);
    ui->btnSaveScan->setEnabled(true);
    ui->operationMode->setEnabled(true);
    ui->actionLEDDriverTest->setEnabled(true);
    ui->actionSMS500SystemZero->setEnabled(true);
    ui->actionSMS500CalibrateSystem->setEnabled(true);
}

void MainWindow::sms500NextScan()
{
    if (sms500->isRunning() == true)
        sms500->enableNextScan();
    else
        sms500StartScan();
}

void MainWindow::sms500Configure()
{
    if (ui->rbtnFlux->isChecked())
        sms500->setOperationMode(SMS500::Flux);
    else if (ui->rbtnIntensity->isChecked())
        sms500->setOperationMode(SMS500::Intensity);
    else if (ui->rbtnIrradiance->isChecked())
        sms500->setOperationMode(SMS500::lux);
    else if (ui->rbtnRadiance->isChecked())
        sms500->setOperationMode(SMS500::cd_m2);

    sms500->setAutoRange(ui->AutoRangeCheckBox->isChecked());
    sms500->setRange(ui->integrationTimeComboBox->currentIndex());
    sms500->setAverage(ui->samplesToAverageSpinBox->value());
    sms500->setBoxCarSmoothing(ui->smoothingSpinBox->value());
    sms500->setStartWave(ui->startWaveLineEdit->text().toInt());
    sms500->setStopWave(ui->stopWaveLineEdit->text().toInt());
    sms500->setCorrecDarkCurrent(ui->dynamicDarkCheckBox->isChecked());
    sms500->setNoiseReduction(ui->noiseReductionCheckBox->isChecked(), ui->noiseReductionLineEdit->text().toDouble());
    sms500->setNumberOfScans(ui->numberOfScansLineEdit->text().toInt());
}

void MainWindow::sms500SaturedDataHandle(bool satured)
{
    if (satured == true) {
        ui->saturedLabel->show();
        if (ui->plotArea->width() > 700)
            ui->saturedLabel_2->show();
    } else {
        ui->saturedLabel->hide();
        ui->saturedLabel_2->hide();
    }
}

void MainWindow::sms500ScanDataHandle(int scanNumber)
{
    QPolygonF points;
    QPolygonF pointsWithCorrection;
    int peakWavelength = sms500->peakWavelength();
    double amplitude   = sms500->maxMasterData();
    double *masterData = sms500->masterData();
    int *wavelength    = sms500->wavelength();

    for(int i = 0; i < sms500->points(); i++) {
        if (masterData[i] < 0)
            masterData[i] = 0;

        points << QPointF(wavelength[i], masterData[i]);
        pointsWithCorrection << QPointF(wavelength[i], masterData[i] * transferenceFunction[i]);
    }

    if (!(ledDriver->isRunning() || lsqnonlin->isRunning()))
        sms500->enableNextScan();

    plotSMS500->setPlotLimits(350, 1000, 0, amplitude);
    plotSMS500->showPeak(peakWavelength, amplitude);
    plotSMS500->showData(points, amplitude);

    plotLedDriver->setPlotLimits(350, 1000, 0, amplitude);
    plotLedDriver->showPeak(peakWavelength, amplitude);
    plotLedDriver->showData(points, amplitude);

    if (lsqnonlin->enableUpdatePlot()) {
        plotLSqNonLin->showPeak(peakWavelength, amplitude);
        plotLSqNonLin->showData(pointsWithCorrection, lsqNonLinStar->peak() * 1.2);
    }

    outputIrradiance = trapezoidalNumInteg(pointsWithCorrection);
    ui->outputIrradianceLabel->setText(QString::number(outputIrradiance, 'e', 2));

    ui->scanNumberLabel->setText(tr("    Scan number: %1").arg(scanNumber));
    ui->scanNumberLabel_2->setText(tr("    Scan number: %1").arg(scanNumber));
}

void MainWindow::sms500SystemZero()
{
    if (sms500->isConnected() == false)
        if (sms500Connect() == false)
            return;

    if (QMessageBox::question(this, tr("Sytem Zero"),
                              tr("Set the SMS-500 for Dark Current Reading.\n\nThis can take several seconds."),
                              tr("Yes"), tr("No") )) {
        return;
    }

    statusBar()->showMessage(tr("Running System Zero :: Please wait!"));
    sms500Configure();
    sms500->creatDarkRatioArray();
    QMessageBox::information(this, tr("SMS500 Info"), tr("System Zero Completed."));
}

void MainWindow::sms500CalibrateSystem()
{
    if (QMessageBox::question(this, tr("Master Spectrometer Calibration"),
                              tr("Spectrometer Calibration Procedure.\n\n"
                                 "This requires the user to follow the procedure that is outlined in the software.\n"
                                 "The user needs to have the Standard Lamp with Spectral data, correct lamp holder\n"
                                 "and lamps to be calibrated. Also, the correct 'Mode of Operation' must be selected.\n\n"
                                 "To continue with calibration, click on the 'YES' button, 'No' to cancel"),
                              tr("Yes"), tr("No") )) {
        return;
    }


    QString lampFile = QFileDialog::getOpenFileName(this);

    if (lampFile.isEmpty())
        return;

    if (sms500->readCalibratedLamp(lampFile) == false) {
        statusBar()->showMessage("Error trying to open calibration file", 10000);
        return;
    }

    if (QMessageBox::question(this, tr("Calibration Setup"),
                              tr("Please Setup the SMS500 for data acquisition...\n\n"
                                 "Turn on the calibration lamp and wait approximately 3 minutes\n"
                                 "for the lamp to stabilize.\n\nTo continue with calibration,\n"
                                 "click on the 'YES' button, 'No' to cancel"),
                              tr("Yes"), tr("No") )) {
        return;
    }

    statusBar()->showMessage("Running calibration :: Please wait!", 5000);
    sms500->startLampScan();

    if (QMessageBox::question(this, tr("Sytem Zero"),
                              tr("Set the SMS-500 for Dark Current Reading. To continue with calibration,\n"
                                 "turn off the calibrated lamp, then click on the 'YES' button, 'No' to cancel"),
                              tr("Yes"), tr("No") )) {
        return;
    }

    sms500->finishLampScan();
    QMessageBox::information(this, tr("SMS500 Info"), tr("System calibration completed."));
}

void MainWindow::sms500SaveScanData(const QString &filePath)
{
    QString data;
    QString currentTime(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate));

    data.append(tr("; Star Simulator file, Platform: Windows, Created on: %1\n\n").arg(currentTime));
    data.append(sms500MainData());
    data.append(tr("\n[SMS500Data]\n"));
    data.append(tr("; Holds the Wavelength Data:\n"));
    data.append(tr("; nm\tuW/nm\n"));

    double *masterData;
    int    *waveLength;

    masterData = sms500->masterData();
    waveLength = sms500->wavelength();

    for (int i = 0; i < sms500->points(); i++) {
        if (masterData[i] < 0)
            data.append(tr("%1\t%2\n").arg(waveLength[i]).arg("0.0"));
        else
            data.append(tr("%1\t%2\n").arg(waveLength[i]).arg(masterData[i]));
    }

    if (filePath.isEmpty()) {
        FileHandle file(this, data, tr("SMS500 - Save Data"), &lastDir);
    } else
        FileHandle file(data, tr("SMS500 - Save Data"), filePath);
}

QString MainWindow::sms500MainData()
{
    QString data;

    data.append(tr("; SMS-500 Data\n"));
    data.append(tr("; Start wavelength........: %1\n").arg(sms500->startWavelength()));
    data.append(tr("; Stop wavelength.........: %1\n").arg(sms500->stopWavelength()));
    data.append(tr("; Dominate wavelength.....: %1\n").arg(sms500->dominanteWavelength()));
    data.append(tr("; Peak wavelength.........: %1\n").arg(sms500->peakWavelength()));
    data.append(tr("; Power (W)...............: %1\n").arg(sms500->power()));
    data.append(tr("; Integration time (ms)...: %1\n").arg(sms500->integrationTime()));
    data.append(tr("; Samples to Average......: %1\n").arg(sms500->samplesToAverage()));
    data.append(tr("; Boxcar Smoothing........: %1\n").arg(sms500->boxCarSmoothing()));
    if (sms500->isNoiseReductionEnabled())
        data.append(tr("; Noise Reduction.........: %1\n").arg(sms500->noiseReduction()));
    else
        data.append(tr("; Noise Reduction.........: 0\n"));

    if (sms500->isDynamicDarkEnabled())
        data.append(tr("; Correct for Dynamic Dark: yes\n"));
    else
        data.append(tr("; Correct for Dynamic Dark: no\n"));

    data.append(tr("; Purity..................: %1\n").arg(sms500->purity()));
    data.append(tr("; FWHM....................: %1\n").arg(sms500->fwhm()));

    return data;
}

void MainWindow::ledDriverConfigureConnection()
{
    FTDIDeviceChooserDialog ftdiDevice;
    ftdiDevice.exec();
}

void MainWindow::ledDriverConnectDisconnect()
{
    if (ui->btnConnectDisconnectLED->text().contains(tr("Connect")))
        ledDriverConnect();
    else
        ledDriverDisconnect();
}

bool MainWindow::ledDriverConnect()
{
    // Prevents communnication errors
    ledDriver->closeConnection();

    if (ledDriver->openConnection() == true) {
        statusBar()->showMessage(tr("LED Driver successfully connected"),5000);
        ui->btnConnectDisconnectLED->setText(tr("Disconnect"));
        ui->btnConnectDisconnectLED->setIcon(QIcon(":/pics/disconnect.png"));

        // Update LED Driver channels settings
        for (int i = 0; i < 96; i++)
            ledDriver->updateChannel(i, ledDriverChannel[i]->text().toInt());

        remoteControl->sendAnswer(RemoteControl::SUCCESS);

        return true;
    }

    warningMessage(tr("LED Driver Error"), tr("LED Driver Open Comunication Error.\n\nCheck USB connection and try again..."));
    remoteControl->sendAnswer(RemoteControl::CONNECTION_REFUSED);

    return false;
}

void MainWindow::ledDriverDisconnect()
{
    if (ledDriver->isConnected() == true) {
        ledDriver->closeConnection();
        ui->btnConnectDisconnectLED->setText(tr("Connect"));
        ui->btnConnectDisconnectLED->setIcon(QIcon(":/pics/reconnect.png"));
    }
}

void MainWindow::ledDriverDataHandle()
{
    if (ledDriver->isRunning() == true) {
        double *masterData = sms500->masterData();

        if (ledDriver->operationMode() == LedDriver::ledModeling) {
            ledDriverChannel[ledDriver->currentChannel() - 1]->setStyleSheet(tr("background:#2bb24c;"));
            ledDriverChannel[ledDriver->currentChannel() - 1]->setText(tr("%1").arg(ledDriver->currentLevel()));

            // Stop Condition = maxMasterData <= 0 and Level Decrement
            if ((sms500->maxMasterData() <= 0) && (ui->levelIncDecComboBox->currentIndex() == 0))
                ledDriver->modelingNextChannel();

            // Adds data in LED Modeling Data
            for (int i = 0; i < sms500->points(); i++) {
                if (masterData[i] < 0)
                    masterData[i] = 0;

                ledModelingData[i].resize(ledModelingData[i].size() + 1);
                ledModelingData[i][ledModelingData[i].size() - 1] = masterData[i];
            }

            ledDriver->enabledModelingContinue();
        }

        else if (ledDriver->operationMode() == LedDriver::channelTest) {
            bool ok = false;

            for(int i = 0; i < sms500->points(); i++)
                if (masterData[i] > 0) {
                    ok = true;
                    break;
                }

            if (ok) {
                if (currentStatus == GD_ALGORITHM)
                    ui->starSimulatorLog->appendHtml(tr("Channel %1: <font color='#2bb24c'>ok</font>").arg(ledDriver->currentChannel()));

                activeChannels.append(ledDriver->currentChannel() - 1);
                activeChannelsToLedModeling.append(1);
                ledDriverChannel[ledDriver->currentChannel() - 1]->setStyleSheet(tr("background:#2bb24c;"));
            } else {
                if (currentStatus == GD_ALGORITHM)
                    ui->starSimulatorLog->appendHtml(tr("Channel %1: <font color='#ff0000'>not working</font>").arg(ledDriver->currentChannel()));

                activeChannelsToLedModeling.append(0);
                ledDriverChannel[ledDriver->currentChannel() - 1]->setStyleSheet(tr("background:#ff0000;"));
            }

            ledDriver->enabledModelingContinue();
        }
    }
}

bool MainWindow::ledDriverLoadValuesForChannels()
{
    FileHandle file(this, tr("LED Driver - Load values for channels"), &lastDir);
    QVector< QVector<double> > values = file.data(tr("[ChannelLevel]"));

    if (file.isValidData(96, 2) == false)
        return false;

    // Update LED Driver GUI
    ledDriverGuiUpdate(Utils::matrix2vector(values, 1));

    // V2REF
    QString v2ref = file.readSection(tr("[LedDriver]"));
    if (v2ref.isEmpty() == false) {
        if (v2ref.contains("on"))
            ui->setV2RefCheckBox->setChecked(true);
        else
            ui->setV2RefCheckBox->setChecked(false);
    }

    return true;
}

void MainWindow::ledDriverGuiUpdate(QVector<double> level)
{
    // Backup values
    ledDriverPreviousChannelValues = ledDriverChannelValues();

    for (int i = 0; i < 96; i++) {
        ledDriverChannel[i]->setText(QString::number(level[i]));
        ledDriverChannel[i]->editingFinished();
    }
}

void MainWindow::ledDriverChannelChanged()
{
    QLineEdit *lineEdit = (QLineEdit *)sender();
    ledDriver->updateChannel(lineEdit->objectName().toInt(), lineEdit->text().toInt());
}

QVector<int> MainWindow::ledDriverChannelValues()
{
    QVector<int> channelValue;

    for (int i = 0; i < 96; i++)
        channelValue.append(ledDriverChannel[i]->text().toInt());

    return channelValue;
}

void MainWindow::ledModeling()
{
    if (ui->btnLedModeling->text().contains("LED\nModeling") == true)
        ledModelingStartPreprocessing();
    else
        ledModelingStop();
}

void MainWindow::ledModelingStartPreprocessing()
{
    // SMS500 Configure
    if (sms500->isConnected() == false)
        if (sms500Connect() == false)
            return;

    // Prevents errors with SMS500
    if (sms500->isRunning())
        sms500StopScan();

    // LED Driver Connection
    if (ledDriver->isConnected() == false)
        if(ledDriverConnect() == false)
            return;

    // Choice directory to save data
    ledModelingFilePath = QFileDialog::getExistingDirectory(this,tr("Choose the directory to save LED Modeling Data"), lastDir);

    if (ledModelingFilePath.isEmpty())
        return;

    lastDir = ledModelingFilePath;

    ledModelingGuiConfig(false);

    currentStatus = LED_MODELING;

    connect(ledDriver, SIGNAL(testFinished()), this, SLOT(ledModelingStart()));
    ledDriverTest();
}

void MainWindow::ledModelingStart()
{
    ledModelingGuiConfig(false);

    ledDriver->setActiveChannels(activeChannelsToLedModeling);

    // Clear LED Driver GUI
    ledDriverGuiUpdate(QVector<double>(96, 0));

    // Set Operation Mode to Flux and number of scans
    ui->rbtnFlux->setChecked(true);
    ui->numberOfScansLineEdit->setText("-1");

    // Led Modeling Setup
    int modelingType;
    ledModelingConfiguration.clear();
    if (ui->levelIncDecComboBox->currentIndex() == 0)
        modelingType = LedDriver::levelDecrement;
    else
        modelingType = LedDriver::levelIncrement;

    ledDriver->setModelingParameters(ui->startChannel->text().toInt(), ui->endChannel->text().toInt(), modelingType, ui->levelIncDecValue->text().toInt());
    ledModelingConfiguration.append(tr("%1\t%2").arg(modelingType).arg(ui->levelIncDecValue->text().toInt()));
    ledDriver->setOperationMode(LedDriver::ledModeling);
    ledDriver->start();

    // Inits LED Modeling Data structure
    ledModelingData.resize(641);
    for (int i = 0; i < 641; i++) {
        ledModelingData[i].resize(1);
        ledModelingData[i][0] = i + 360;
    }

    ui->btnLedModeling->setText("STOP\nModeling");
}

void MainWindow::ledModelingStop()
{
    ledDriver->stop();
    sms500StopScan();
}

void MainWindow::ledModelingSettings()
{
    // Check values of start/end channel
    if (ui->startChannel->text().isEmpty())
        ui->startChannel->setText("1");

    if (ui->endChannel->text().isEmpty())
        ui->endChannel->setText("96");

    if (ui->startChannel->text().toInt() > ui->endChannel->text().toInt())
        ui->endChannel->setText(ui->startChannel->text());

    if (ui->levelIncDecValue->text().isEmpty())
        ui->levelIncDecValue->setText("50");
}

void MainWindow::ledModelingGuiConfig(bool enable)
{
    // LED Driver
    ui->ledDriverWidget->setEnabled(enable);
    ui->btnConnectDisconnectLED->setEnabled(enable);
    ui->btnLoadValuesForChannels->setEnabled(enable);
    ui->LEDDriverParametersGroupBox->setEnabled(enable);
    ui->ledModelingParametersGoupBox->setEnabled(enable);

    // SMS500
    ui->sms500Tab->setEnabled(enable);

    // Star Simulator
    ui->starSimulatorTab->setEnabled(enable);

    // Long Term Stability
    ui->longTermStabilityTab->setEnabled(enable);

    // Menu
    ui->actionSMS500SystemZero->setEnabled(enable);
    ui->actionSMS500CalibrateSystem->setEnabled(enable);
    ui->actionLoadStarSimulatorDatabase->setEnabled(enable);
    ui->actionLoadTransferenceFunction->setEnabled(enable);
    ui->actionConfigureLEDDriverconnection->setEnabled(enable);
    ui->actionSetValues->setEnabled(enable);
    ui->actionGeneralSettings->setEnabled(enable);
}

void MainWindow::ledModelingSaveData(QString channel)
{
    QString data;
    QString currentTime(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate));

    data.append(tr("; Star Simulator file, Platform: Windows, Created on: %1\n\n").arg(currentTime));
    data.append(sms500MainData());
    data.append(tr("\n[LEDModelingConfiguration]\n"));
    data.append(tr("; #1 [0] Increment\t[1] Decrement\n"));
    data.append(tr("; #2 value\n"));
    data.append(tr("%1\n").arg(ledModelingConfiguration));

    data.append(tr("\n[LedDriver]\n"));
    if (ui->setV2RefCheckBox->isChecked())
        data.append(tr("V2REF: on\n"));
    else
        data.append(tr("V2REF: off\n"));

    data.append(tr("\n[SMS500Data]\n"));
    data.append(tr("; Holds the Wavelength Data:\n"));
    data.append(tr("; nm\tuW/nm DigitalLevel\n"));

    // Header
    QVector<int> index = ledDriver->digitalLevelIndex();
    data.append(";\t");

    for (int i = 0; i < index.size(); i++)
        data.append(tr("%1\t").arg(index[i]));

    data.append("\n");

    // Data
    for (int i = 0; i < ledModelingData.size(); i++) {
        for (uint j = 0; j < ledModelingData[0].size(); j++) {
            if (ledModelingData[i][j] <= 0)
                data.append(tr("%1\t").arg("0"));
            else
                data.append(tr("%1\t").arg(ledModelingData[i][j]));
        }
        data.append("\n");
    }

    FileHandle file(data, tr("LED Driver - Save Data"), tr("%1/ch%2.txt").arg(ledModelingFilePath).arg(channel));

    ledModelingSaveDerivatives(channel);

    // Reset LED Modeling Data structure
    ledModelingData.clear();
    ledModelingData.resize(641);
    for (int i = 0; i < 641; i++) {
        ledModelingData[i].resize(1);
        ledModelingData[i][0] = i + 360;
    }

    ui->integrationTimeComboBox->setCurrentIndex(5);
    sms500->resetScanNumber();
}

void MainWindow::ledModelingSaveDerivatives(QString channel)
{
    // Led Modeling Setup
    int modelingType;
    if (ui->levelIncDecComboBox->currentIndex() == 0)
        modelingType = LedDriver::levelDecrement;
    else
        modelingType = LedDriver::levelIncrement;

    ledModelingBinaryData.levelIncDec = modelingType;
    ledModelingBinaryData.step        = ui->levelIncDecValue->text().toInt();


    MatrixXd channelData = Utils::qvector2eigen(ledModelingData);
    MatrixXd derivatives;

    // Prevents errors with columns == 2 OR channels without data
    if (channelData.cols() <= 2)
        derivatives.resize(channelData.rows(), 2);
    else
        derivatives.resize(channelData.rows(), channelData.cols() - 1);

    derivatives.col(0) = channelData.col(0); // Column 0 contains wavelengths values
    derivatives.col(1) = MatrixXd::Constant(channelData.rows(), 1, 1); // Column of ones

    // Computing Derivatives
    for (int column = 1; column < channelData.cols() - 1; column++)
        derivatives.col(column) = (channelData.col(column + 1) - channelData.col(column)) / ledModelingBinaryData.step;

    // Save Binary Data (*.led)
    FileHandle *file = new FileHandle();
    ledModelingBinaryData.matrix = Utils::eigen2QVector(derivatives);
    file->save(ledModelingBinaryData, tr("Load LED Data"), tr("%1/led_database/ch%2.led").arg(ledModelingFilePath).arg(channel));
}

void MainWindow::ledModelingFinished()
{
    QMessageBox::information(this, tr("LED Modeling finished"), tr("Press Ok to continue.\t\t"));
    ui->btnLedModeling->setText("LED\nModeling");
    ledModelingGuiConfig(true);
    sms500StopScan();

    currentStatus = STOPED;
}

void MainWindow::ledDriverTest()
{
    // Checks connection with SMS500
    if (sms500->isConnected() == false)
        if (sms500Connect() == false)
            return;

    // Prevents errors with SMS500
    if (sms500->isRunning())
        sms500StopScan();

    // Checks connection with LED Driver
    if (ledDriver->isConnected() == false)
        if (ledDriverConnect() == false)
            return;

    // SMS500 settings
    GeneralSettings settings;
    sms500SetSettings(settings.ledDriverTestSettings());

    // Reset all channels of LED Driver
    ledDriverGuiUpdate(QVector<double>(96, 0));

    for (int channel = 0; channel < 96; channel++)
        ledDriverChannel[channel]->setStyleSheet(tr("background:white;"));

    activeChannels.clear();
    activeChannelsToLedModeling.clear();

    ledDriver->setOperationMode(LedDriver::channelTest);
    ledDriver->start();

    // Configure GUI
    if (currentStatus != GD_ALGORITHM) {
        ui->tabWidget->setCurrentIndex(1);
        ui->starSimulatorTab->setEnabled(false);
    }

    ui->sms500Tab->setEnabled(false);
    ui->longTermStabilityTab->setEnabled(false);
    ui->setV2RefCheckBox->setEnabled(false);
    ui->startChannel->setEnabled(false);
    ui->endChannel->setEnabled(false);
    ui->levelIncDecValue->setEnabled(false);
    ui->levelIncDecComboBox->setEnabled(false);
    ui->btnConnectDisconnectLED->setEnabled(false);
    ui->btnLoadValuesForChannels->setEnabled(false);
    ui->btnLedModeling->setEnabled(false);
}

void MainWindow::ledDriverTestFinished()
{
    sms500StopScan();
    plotSMS500->showData(QPolygon(641), 0);
    plotLedDriver->showData(QPolygon(641), 0);
    plotLSqNonLin->showData(QPolygon(641), 0);
    ui->saturedLabel_2->setVisible(false);

    if (currentStatus == GD_ALGORITHM)
        disconnect(ledDriver, SIGNAL(testFinished()), this, SLOT(lsqNonLinStartGD()));
    else if (currentStatus == LED_MODELING)
        disconnect(ledDriver, SIGNAL(testFinished()), this, SLOT(ledModelingStart()));

    // Restore previous settings
    sms500SetSettings(sms500parameters);

    QVector<double> temp(ledDriverPreviousChannelValues.size());
    for (int i = 0; i < ledDriverPreviousChannelValues.size(); i++)
        temp[i] = ledDriverPreviousChannelValues[i];
    ledDriverGuiUpdate(temp); // Need a cast QVector<int> to QVector<double>

    // Configure GUI
    ui->saturedLabel->hide();
    ui->saturedLabel_2->hide();
    ui->sms500Tab->setEnabled(true);
    ui->starSimulatorTab->setEnabled(true);
    ui->longTermStabilityTab->setEnabled(true);
    ui->setV2RefCheckBox->setEnabled(true);
    ui->startChannel->setEnabled(true);
    ui->endChannel->setEnabled(true);
    ui->levelIncDecValue->setEnabled(true);
    ui->levelIncDecComboBox->setEnabled(true);
    ui->btnConnectDisconnectLED->setEnabled(true);
    ui->btnLoadValuesForChannels->setEnabled(true);
    ui->btnLedModeling->setEnabled(true);
}

void MainWindow::lsqNonLinStartStop()
{
    if (ui->btnStartStopStarSimulator->text().contains("Start\nSimulator"))
        lsqNonLinStart();
    else
        lsqNonLinStop();
}

void MainWindow::lsqNonLinStart()
{
    if (lsqnonlin->isRunning())
        return;

    // Checks connection with SMS500
    if (sms500->isConnected() == false)
        if (sms500Connect() == false)
            return;

    // Prevents errors with SMS500
    if (sms500->isRunning())
        sms500StopScan();

    // Checks connection with LED Driver
    if (ledDriver->isConnected() == false)
        if (ledDriverConnect() == false)
            return;

    // Set Operation Mode to Flux and number of scans
    ui->rbtnFlux->setChecked(true);
    ui->numberOfScansLineEdit->setText("-1");

    ui->btnStartStopStarSimulator->setIcon(QIcon(":/pics/stop.png"));
    ui->btnStartStopStarSimulator->setText("Stop\nSimulator ");

    lsqNonLinGuiConfig(false);

    // x0 type
    if (ui->x0Random->isChecked())
        lsqnonlin->setx0Type(StarSimulator::x0Random);
    else if (ui->x0UserDefined->isChecked())
        lsqnonlin->setx0Type(StarSimulator::x0UserDefined, lsqNonLinX0());
    else if (ui->x0DefinedInLedDriver->isChecked()) {
        MatrixXi matrix(96, 1);
        QVector<int> qvector = ledDriverChannelValues();

        for (int i = 0; i < qvector.size(); i++)
            matrix(i) = qvector[i];

        lsqnonlin->setx0Type(StarSimulator::x0Current, matrix);
    }

    // Transference Function of Pinhole and Colimator
    starLoadTransferenceFunction();

    GeneralSettings settings;
    lsqnonlin->setSettings(settings.starSimulatorSettings());

    lsqNonLinTime.start();
    lsqNonLinLog(tr("====================== Star Simulator Start ======================="
                    "<pre>\n%1\n</pre>"
                    "==========================================================")
                 .arg(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate)));

    if (ui->levenbergMarquardt->isChecked()) {
        lsqnonlin->setAlgorithm(StarSimulator::leastSquareNonLinear);
        lsqnonlin->start();
        remoteControl->sendAnswer(RemoteControl::SUCCESS);
    }
    else {
        currentStatus = GD_ALGORITHM;
        lsqnonlin->setAlgorithm(StarSimulator::gradientDescent);

        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Star Simulator :: Gradient Descent"));
        msgBox.setText("To improve the performance of the Gradient Descent algorithm,\nis recommended evaluating which channels are working.");
        msgBox.setInformativeText("Do you want perform evaluation of channels?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setIcon(QMessageBox::Question);

        switch (msgBox.exec()) {
            case QMessageBox::Yes:
                connect(ledDriver, SIGNAL(testFinished()), this, SLOT(lsqNonLinStartGD()));
                ledDriverTest();
                break;

            case QMessageBox::No:
                activeChannels.clear();

                for (int i = 0; i < 96; i++)
                    activeChannels.append(i);

                lsqNonLinStartGD();
                break;
        }
    }
}

void MainWindow::lsqNonLinStop()
{
    if (lsqnonlin->isRunning()) {
        ui->btnStartStopStarSimulator->setEnabled(false);
        lsqnonlin->stop();
        //        lsqnonlin->wait(5000);
        //        sms500StopScan();
    }
}

void MainWindow::lsqNonLinStartGD()
{
    ui->AutoRangeCheckBox->setChecked(true);

    lsqnonlin->setActiveChannels(Utils::qvector2eigen(activeChannels));
    lsqnonlin->start();
    lsqNonLinTime.start();
    remoteControl->sendAnswer(RemoteControl::SUCCESS);
}

void MainWindow::lsqNonLinFinished()
{
    sms500StopScan();
    ui->btnStartStopStarSimulator->setIcon(QIcon(":/pics/start.png"));
    ui->btnStartStopStarSimulator->setText("Start\nSimulator");
    ui->btnStartStopStarSimulator->setEnabled(true);
    ui->x0DefinedInLedDriver->setChecked(true);
    lsqNonLinGuiConfig(true);

    lsqNonLinLog(tr("==================== Star Simulator Finished ====================="
                    "<pre>\n%1\tElapsed time: %2 seconds\n</pre>"
                    "==========================================================")
                 .arg(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate))
                 .arg((lsqNonLinTime.elapsed())));

    currentStatus = STOPED;
}

void MainWindow::lsqNonLinStarSettings()
{
    if(ui->starTemperature->text().toInt() < 100)
        ui->starTemperature->setText("100");

    lsqNonLinStar->setMagnitude(ui->starMagnitude->text().toDouble());
    lsqNonLinStar->setTemperature(ui->starTemperature->text().toInt());

    starIrradiance = trapezoidalNumInteg(lsqNonLinStar->spectralDataToPlot());
    ui->starIrradianceLabel->setText(QString::number(starIrradiance, 'e', 2));

    plotLSqNonLin->setPlotLimits(350, 1000, 0, lsqNonLinStar->peak() * 1.2);
    plotLSqNonLin->showData(lsqNonLinStar->spectralDataToPlot(), lsqNonLinStar->peak() * 1.2, 1);
    plotLSqNonLin->showData(lsqNonLinStar->spectralDataToPlot(), lsqNonLinStar->peak() * 1.2, 0);
}

void MainWindow::lsqNonLinPerformScanWithUpdate()
{
    MatrixXi level = lsqnonlin->getSolution();

    plotLSqNonLin->setPlotLimits(350, 1000, 0, lsqNonLinStar->peak() * 1.2);
    plotLSqNonLin->showData(lsqNonLinStar->spectralDataToPlot(), lsqNonLinStar->peak() * 1.2, 1);

    ledDriverGuiUpdate(Utils::eigen2QVector(level));

    sms500NextScan();
}

void MainWindow::lsqNonLinObjectiveFunction()
{
    if (lsqnonlin->isRunning() == true) {
        MatrixXd f(641, 1);
        double *masterData                  = sms500->masterData();
        QVector< QVector<double> > starData = lsqNonLinStar->spectralData();

        for (int i = 0; i < sms500->points(); i++) {
            if (masterData[i] < 0)
                f(i) = - starData[i][1];
            else
                f(i) = (masterData[i] * transferenceFunction[i]) - (starData[i][1]);

            // Due to very small values ​​of the objective function was necessary to add this multiplier
            f(i) *= 1e11;;
        }

        lsqnonlin->setObjectiveFunction(f);
    }
}

void MainWindow::lsqNonLinLoadLedData()
{
    StarSimulatorLoadLedData *loadLedData = new StarSimulatorLoadLedData(this);
    connect(loadLedData, SIGNAL(updateLastDir(QString)), this, SLOT(updateLastDir(QString)));
    loadLedData->setLastDir(lastDir);
    loadLedData->exec();
}

void MainWindow::lsqNonLinx0Handle()
{
    bool enabled;
    if (ui->x0UserDefined->isChecked())
        enabled = true;
    else
        enabled = false;

    for (int i = 0; i < 96; i++)
        starSimulatorX0[i]->setEnabled(enabled);
}

void MainWindow::lsqNonLinLog(QString info)
{
    ui->starSimulatorLog->appendHtml(info);
}

void MainWindow::lsqNonLinSaveData()
{
    QString data;
    QString currentTime(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate));

    data.append(tr("; Star Simulator file, Platform: Windows, Created on: %1\n\n").arg(currentTime));
    data.append(sms500MainData());
    data.append(tr("; Output irradiance.......: %1\n").arg(outputIrradiance));
    data.append(tr("; Star irradiance.........: %1\n").arg(starIrradiance));

    data.append(tr("\n[LedDriver]\n"));
    if (ui->setV2RefCheckBox->isChecked())
        data.append(tr("V2REF: on\n"));
    else
        data.append(tr("V2REF: off\n"));

    data.append(tr("\n[ChannelLevel]\n"));

    QVector<int> channelValues = ledDriverChannelValues();
    for (int i = 0; i < channelValues.size(); i++)
        data.append(tr("%1\t%2\n").arg(i + 1).arg(channelValues[i]));

    data.append(tr("\n[StarSimulatorSettings]\n"));
    data.append(tr("; #1: Magnitude\n"));
    data.append(tr("; #2: Temperature\n"));
    data.append(tr("; #3: Fiting Algorithm [0] LM, [1] GD\n"));
    data.append(tr("%1\n").arg(ui->starMagnitude->text()));
    data.append(tr("%1\n").arg(ui->starTemperature->text()));
    data.append(tr("%1\n").arg(ui->gradientDescent->isChecked()));

    data.append(tr("\n[StarSimulatorData]\n"));
    data.append(tr("; #1: wavelength (nm)\n"));
    data.append(tr("; #2: irradiance in the collimator output (uW/cm2 nm)\n"));
    data.append(tr("; #3: star irradiance (uW/cm2 nm)\n"));
    data.append(tr("; #1\t#2\t\t#3\n"));

    QVector< QVector<double> > starData = lsqNonLinStar->spectralData();
    double *masterData;
    int    *waveLength;

    masterData = sms500->masterData();
    waveLength = sms500->wavelength();

    for (int i = 0; i < sms500->points(); i++) {
        if (masterData[i] < 0)
            data.append(tr("%1\t%2\t%3\n")
                        .arg(waveLength[i])
                        .arg("0.000000000")
                        .arg(starData[i][1]));
        else
            data.append(tr("%1\t%2\t%3\n")
                        .arg(waveLength[i])
                        .arg(masterData[i] * transferenceFunction[i])
                        .arg(starData[i][1]));
    }

    FileHandle file(this, data, tr("Star Simulator - Save Data"), &lastDir);
}

void MainWindow::lsqNonLinGuiConfig(bool enable)
{
    // LED Driver
    ui->ledDriverTab->setEnabled(enable);

    // SMS500
    ui->btnConnectDisconnect->setEnabled(enable);
    ui->btnStartScan->setEnabled(enable);
    ui->AutoRangeCheckBox->setChecked(true);
    ui->numberOfScansLineEdit->setEnabled(enable);
    ui->sms500WavelengthGroupBox->setEnabled(enable);

    // Star Simulator
    ui->btnSaveStarSimulatorData->setEnabled(enable);
    ui->btnLoadInitialSolution->setEnabled(enable);
    ui->starSettingsGroupBox->setEnabled(enable);
    ui->fitingAlgorithm->setEnabled(enable);
    ui->x0GroupBox->setEnabled(enable);

    // Menu
    ui->actionSMS500SystemZero->setEnabled(enable);
    ui->actionSMS500CalibrateSystem->setEnabled(enable);
    ui->actionLoadStarSimulatorDatabase->setEnabled(enable);
    ui->actionLoadTransferenceFunction->setEnabled(enable);
    ui->actionConfigureLEDDriverconnection->setEnabled(enable);
    ui->actionSetValues->setEnabled(enable);
    ui->actionGeneralSettings->setEnabled(enable);
}

bool MainWindow::lsqNonLinLoadInitialSolution()
{
    FileHandle file(this, tr("Star Simulator - Initial Solution"), &lastDir);
    QVector< QVector<double> > initialSolution = file.data(tr("[ChannelLevel]"));

    if (file.isValidData(96, 2) == false)
        return false;

    // GUI update
    lsqNonLinX0GuiUpdate(Utils::matrix2vector(initialSolution, 1));
    ui->x0UserDefined->setChecked(true);

    // V2REF
    QString v2ref = file.readSection(tr("[LedDriver]"));
    if (v2ref.isEmpty() == false) {
        if (v2ref.contains("on"))
            ui->setV2RefCheckBox->setChecked(true);
        else
            ui->setV2RefCheckBox->setChecked(false);
    }

    // Star Simulator Settings
    QVector< QVector<double> > settings = file.data(tr("[StarSimulatorSettings]"));
    if (settings.isEmpty() == false && settings.size() == 3) {
        ui->starMagnitude->setText(tr("%1").arg(settings[0][0]));
        ui->starTemperature->setText(tr("%1").arg(settings[1][0]));
        if (settings[2][0] == 0)
            ui->levenbergMarquardt->setChecked(true);
        else
            ui->gradientDescent->setChecked(true);

        // Update plot
        lsqNonLinStarSettings();
    }

    return true;
}

void MainWindow::starLoadTransferenceFunction()
{
    QString filePath = QDir::currentPath() + "/transferenceFunction.txt";

    FileHandle file(tr("Load Transference Function"), filePath);
    QVector<double> data = Utils::matrix2vector(file.data(tr("[TransferenceFunction]")), 1);

    if (file.isValidData(641) == false) {
        if (!QMessageBox::question(this, tr("Star transference function file not found!"),
                                   tr("For the correct generation of the star spectrum, "
                                      "load the transference function file."),
                                   tr("Load"), tr("Cancel") )) {
            if (starUpdateTransferenceFunction() == true) {
                starLoadTransferenceFunction();
                return;
            }
        }
    }

    transferenceFunction = data;
}

bool MainWindow::starUpdateTransferenceFunction()
{
    FileHandle fileInput;

    if (fileInput.open(this, tr("Load Transference Function"), &lastDir) == false)
        return false;

    QString data = fileInput.readSection(tr("[TransferenceFunction]"));

    if (data.isEmpty())
        return false;

    QString outFilePath = QDir::currentPath() + "/transferenceFunction.txt";
    FileHandle fileOutput(data, tr("Load Transference Function"), outFilePath);

    statusBar()->showMessage(tr("Transference function successfully loaded"));
    return true;
}

void MainWindow::lsqNonLinX0GuiUpdate(QVector<double> x0)
{
    for (int i = 0; i < 96; i++)
        starSimulatorX0[i]->setText(QString::number(x0[i]));
}

MatrixXi MainWindow::lsqNonLinX0()
{
    MatrixXi matrix(96, 1);

    for (int i = 0; i < 96; i++)
        matrix(i) = starSimulatorX0[i]->text().toInt();

    return matrix;
}

void MainWindow::longTermStabilityCreateDB()
{
    QString filePath = QFileDialog::getSaveFileName(
                           this,
                           tr("Long Term Stability :: Create Database"),
                           lastDir,
                           tr("Star Simulator DB *.db"));

    if (filePath.isEmpty())
        return;

    lastDir = QFileInfo(filePath).path();

    // Checks if exist extension '.db'
    if (filePath.contains(".db") == false)
        filePath.append(".db");

    // Prevents errors
    if (QFile::exists(filePath))
        QFile::remove(filePath);

    if (longTermStability->createDB(filePath) == false) {
        QMessageBox::warning(this, tr("Long Term Stability"), longTermStability->lastError());
        return;
    }

    // Updates tableView and plot
    longTermStabilityUpdateView();

    ui->groupBoxTimeTorun->setEnabled(true);
    ui->groupBoxTimeInterval->setEnabled(true);
    ui->btnStartStopLongTermStability->setEnabled(true);
    ui->btnExportAllLongTermStability->setEnabled(true);
    ui->plotAreaLongTermStability->setEnabled(true);
    ui->tableView->setEnabled(true);
}

void MainWindow::longTermStabilityOpenDB()
{
    QString filePath = QFileDialog::getOpenFileName(
                           this,
                           tr("Long Term Stability :: Open Database"),
                           lastDir,
                           tr("Star Simulator Database (*.db);;All files (*.*)"));

    if (filePath.isNull())
        return;

    lastDir = QFileInfo(filePath).path();

    if (longTermStability->openDB(filePath) == false) {
        QMessageBox::warning(this, tr("Long Term Stability"), longTermStability->lastError());
        return;
    }

    // Updates tableView and plot
    longTermStabilityUpdateView();

    ui->groupBoxTimeTorun->setEnabled(false);
    ui->groupBoxTimeInterval->setEnabled(false);
    ui->btnStartStopLongTermStability->setEnabled(false);
    ui->btnExportAllLongTermStability->setEnabled(true);
    ui->plotAreaLongTermStability->setEnabled(true);
    ui->tableView->setEnabled(true);
}

void MainWindow::longTermStabilityExportAll()
{
    LongTermStabilityExportDialog ltsExportDialog(this, longTermStability);
    ltsExportDialog.exec();
}

void MainWindow::longTermStabilityStartStop()
{
    if (ui->btnStartStopLongTermStability->text().contains("Start Timer"))
        longTermStabilityStart();
    else
        longTermStabilityStop();
}

void MainWindow::longTermStabilityStart()
{
    int hour            = ui->longTermStabilityHour->text().toInt();
    int min             = ui->longTermStabilityMin->text().toInt();
    int sec             = ui->longTermStabilitySec->text().toInt();
    int secTimeInterval = ui->longTermStabilityTimeInterval->text().toInt();

    if (ui->timeIntervalTypeComboBox->currentIndex() == 1)
        secTimeInterval *= 60;

    // Prevents parameters errors
    if (secTimeInterval > ((hour * 60 * 60) + (min * 60) + sec)) {
        QMessageBox::information(this, tr("Long Term Stability"),
                                 tr("The time interval is great then time to run!\nPlease, check your time parameters."));
        return;
    }

    // Checks if SMS500 and LED Driver are connecteds and configureds
    if ((sms500->isConnected() == false) && (ledDriver->isConnected() == false)) {
        QMessageBox::warning(this, tr("Long Term Stability"), tr("SMS500 and LED Driver not configured."));
        return;
    } else {
        if (sms500->isConnected() == false) {
            QMessageBox::warning(this, tr("Long Term Stability"), tr("SMS500 not configured."));
            return;
        }
        if (ledDriver->isConnected() == false) {
            QMessageBox::warning(this, tr("Long Term Stability"), tr("LED Driver not configured."));
            return;
        }
    }

    // Configure SMS500's parameters
    ui->numberOfScansLineEdit->setText("-1");

    if (sms500->isRunning() == false)
        sms500StartScan();

    ui->btnStartStopLongTermStability->setIcon(QIcon(":/pics/stop.png"));
    ui->btnStartStopLongTermStability->setText("Stop Timer");

    longTermStabilityGuiConfig(false);

    // Adjusts textbox values
    if (ui->longTermStabilityHour->text().isEmpty())
        ui->longTermStabilityHour->setText("0");

    if (ui->longTermStabilityMin->text().isEmpty())
        ui->longTermStabilityMin->setText("0");

    if (ui->longTermStabilitySec->text().isEmpty()) {
        if (ui->longTermStabilityMin->text().compare("0") == 0)
            ui->longTermStabilitySec->setText("1");
        else
            ui->longTermStabilitySec->setText("0");
    }

    if (ui->longTermStabilityTimeInterval->text().isEmpty())
        ui->longTermStabilityTimeInterval->setText("1");

    longTermStabilityScanNumber = 0;

    QDateTime stopTime;
    stopTime.setTime_t((QDateTime::currentDateTime().currentMSecsSinceEpoch()/1000)
                       + (hour * 60 * 60)
                       + (min * 60)
                       + sec);
    ui->startLongTermStabilityLabel->setText(tr("Start: %1").arg(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate)));
    ui->stopLongTermStabilityLabel->setText(tr("Stop : %1").arg(stopTime.toString(Qt::SystemLocaleShortDate)));
    ui->startLongTermStabilityLabel_2->setText(ui->startLongTermStabilityLabel->text());
    ui->stopLongTermStabilityLabel_2->setText(ui->stopLongTermStabilityLabel->text());

    // Long Term Stability Labels
    if (ui->plotArea->width() > 700) {
        ui->startLongTermStabilityLabel_2->show();
        ui->stopLongTermStabilityLabel_2->show();
    } else {
        ui->startLongTermStabilityLabel->show();
        ui->stopLongTermStabilityLabel->show();
    }

    // Save SMS500 and LED Driver parameters into database
    int noiseReduction = 0;
    if (ui->noiseReductionCheckBox->isChecked())
        noiseReduction = ui->noiseReductionLineEdit->text().toInt();

    QVector<int> channelValues = ledDriverChannelValues();

    // Save SMS500 and LED Driver parameters into database
    longTermStability->saveSMS500andLedDriverParameters(
                1,
                ui->startWaveLineEdit->text().toInt(),
                ui->stopWaveLineEdit->text().toInt(),
                INTEGRATION_TIME[ui->integrationTimeComboBox->currentIndex()],
            ui->samplesToAverageSpinBox->value(),
            ui->smoothingSpinBox->value(),
            noiseReduction,
            ui->dynamicDarkCheckBox->isChecked(),
            ui->setV2RefCheckBox->isChecked(),
            ui->starMagnitude->text().toInt(),
            ui->starTemperature->text().toInt(),
            ui->gradientDescent->isChecked(),
            channelValues);

    longTermStabilityAlarmClock->setAlarmClock(hour, min, sec, secTimeInterval);
    longTermStabilityAlarmClock->start();
}

void MainWindow::longTermStabilityStop()
{
    longTermStabilityAlarmClock->stop();

    ui->btnStartStopLongTermStability->setIcon(QIcon(":/pics/start.png"));
    ui->btnStartStopLongTermStability->setText("Start Timer");

    longTermStabilityGuiConfig(true);

    // Prevents Database subscrition
    ui->btnStartStopLongTermStability->setEnabled(false);

    ui->startLongTermStabilityLabel->hide();
    ui->stopLongTermStabilityLabel->hide();
    ui->startLongTermStabilityLabel_2->hide();
    ui->stopLongTermStabilityLabel_2->hide();
}

void MainWindow::longTermStabilitySaveSMS500Data()
{
    if (longTermStabilityAlarmClock->isRunning() && longTermStabilityAlarmClock->isTimeout()) {
        longTermStabilityAlarmClock->clearTimeout();
        longTermStabilityScanNumber++;

        longTermStability->saveScanData(
                    1,
                    longTermStabilityScanNumber,
                    sms500->dominanteWavelength(),
                    sms500->peakWavelength(),
                    sms500->fwhm(),
                    sms500->power(),
                    sms500->purity(),
                    sms500->startWavelength(),
                    sms500->stopWavelength(),
                    sms500->masterData());

        longTermStabilityUpdateView();
    }
}

void MainWindow::longTermStabilityHandleTableSelection()
{
    QVector<double> amplitude;
    QPolygonF points = longTermStability->selectedData(ui->tableView->selectionModel()->selectedRows(), amplitude);

    plotLTS->setPlotLimits(350, 1000, 0, 1000);
    for (int i = 0; i < amplitude.size(); i++)
        plotLTS->showData(points, amplitude[i]);
}

void MainWindow::longTermStabilityUpdateView()
{
    QSqlTableModel *scanInfoModel = longTermStability->scanInfoTableModel();

    ui->tableView->setModel(scanInfoModel);
    ui->tableView->hideColumn(0);
    ui->tableView->verticalHeader()->hide();
    ui->tableView->resizeColumnsToContents();
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::longTermStabilityGuiConfig(bool enable)
{
    // LED Driver
    if (lsqnonlin->isRunning() == false)
        ui->ledDriverTab->setEnabled(enable);

    // SMS500
    ui->sms500Tab->setEnabled(enable);

    // Star Simulator
    ui->starSimulatorTab->setEnabled(enable);

    // Long Term Stability
    ui->btnOpenDatabaseLongTermStability->setEnabled(enable);
    ui->btnCreateDatabaseLongTermStability->setEnabled(enable);
    ui->btnExportAllLongTermStability->setEnabled(enable);
    ui->groupBoxTimeTorun->setEnabled(enable);
    ui->groupBoxTimeInterval->setEnabled(enable);

    // Menu
    if (lsqnonlin->isRunning() == false) {
        ui->actionLoadTransferenceFunction->setEnabled(enable);
        ui->actionConfigureLEDDriverconnection->setEnabled(enable);
    }
}

void MainWindow::remoteSetSMS500AutoRange(bool enable)
{
    ui->AutoRangeCheckBox->setChecked(enable);
}

void MainWindow::remoteSetSMS500NumberOfScans(QString value)
{
    ui->numberOfScansLineEdit->setText(value);
}

void MainWindow::remoteSetSMS500IntegrationTime(int index)
{
    ui->integrationTimeComboBox->setCurrentIndex(index);
}

void MainWindow::remoteSetSMS500SamplesToAverage(int value)
{
    ui->samplesToAverageSpinBox->setValue(value);
}

void MainWindow::remoteSetSMS500BoxcarSmothing(int value)
{
    ui->smoothingSpinBox->setValue(value);
}

void MainWindow::remoteSetSMS500NoiseReduction(bool enable)
{
    ui->noiseReductionCheckBox->setChecked(enable);
}

void MainWindow::remoteSetSMS500NoiseReductionFactor(QString value)
{
    ui->noiseReductionLineEdit->setText(value);
}

void MainWindow::remoteSetSMS500CorrectForDynamicDark(bool enable)
{
    ui->dynamicDarkCheckBox->setChecked(enable);
}

void MainWindow::remoteSetStarMagnitude(QString value)

{
    ui->starMagnitude->setText(value);
    lsqNonLinStarSettings();
}

void MainWindow::remoteSetStarTemperature(QString value)
{
    ui->starTemperature->setText(value);
    lsqNonLinStarSettings();
}

void MainWindow::remoteStarSimulatorStatus()
{
    QString data;
    data.append(tr("%1").arg(RemoteControl::SUCCESS));
    data.append(tr(",%1").arg(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate)));

    switch (lsqnonlin->algorithmStatus()) {
        case StarSimulator::FITING_OK:
            data.append(tr(",%1").arg(RemoteControl::FITING_OK));
            break;
        case StarSimulator::PERFORMING_FITING:
            data.append(tr(",%1").arg(RemoteControl::PERFORMING_FITING));
            break;
        case StarSimulator::STOPPED:
            data.append(tr(",%1").arg(RemoteControl::STOPPED));
            break;
        default:
            data.append(tr(",%1").arg(RemoteControl::ERROR_WRONG_PARAMETERS));
            break;
    }

    data.append(tr(",%1").arg(lsqnonlin->fx()));
    data.append(tr(",%1").arg(lsqnonlin->iterationNumber()));
    data.append(tr(",%1").arg(sms500->dominanteWavelength()));
    data.append(tr(",%1").arg(sms500->peakWavelength()));
    data.append(tr(",%1").arg(sms500->fwhm()));
    data.append(tr(",%1").arg(sms500->power()));
    data.append(tr(",%1").arg(sms500->purity()));
    data.append(tr(",%1").arg(starIrradiance));
    data.append(tr(",%1").arg(outputIrradiance));
    data.append(tr(",%1").arg(ui->starMagnitude->text()));
    data.append(tr(",%1").arg(ui->starTemperature->text()));

    remoteControl->sendAnswer(data);
}

void MainWindow::remoteStarSimulatorIrradiances()
{
    remoteControl->sendAnswer(tr("%1,%2,%3").arg(RemoteControl::SUCCESS).arg(outputIrradiance).arg(starIrradiance));
}

void MainWindow::uiInputValidator()
{
    QValidator *numberOfScansValidator = new QRegExpValidator(QRegExp("^-1$|^[1-9][0-9]{0,6}$"), this);
    ui->numberOfScansLineEdit->setValidator(numberOfScansValidator);

    QValidator *wavelengthValidator = new QRegExpValidator(QRegExp("^0$|^[1-9][0-9]{0,2}$|^1[0-9]{0,3}$|^2000$"), this);
    ui->startWaveLineEdit->setValidator(wavelengthValidator);
    ui->stopWaveLineEdit->setValidator(wavelengthValidator);

    QValidator *noiseReductionValidator = new QRegExpValidator(QRegExp("^[1-9][0-9]{0,1}$|^100$"), this);
    ui->noiseReductionLineEdit->setValidator(noiseReductionValidator);

    QValidator *ledModelingValidator = new QRegExpValidator(QRegExp("^$|^[1-8][0-9]{0,1}$|^9[0-6]{0,1}$"), this);
    ui->startChannel->setValidator(ledModelingValidator);
    ui->endChannel->setValidator(ledModelingValidator);

    QValidator *ledModelingIncDecValidator = new QRegExpValidator(QRegExp("^$|^[1-9][0-9]{0,2}$|^1000$"), this);
    ui->levelIncDecValue->setValidator(ledModelingIncDecValidator);

    QValidator *timeValidator = new QRegExpValidator(QRegExp("^([0-9]|[1-5][0-9]{1})$"), this);
    ui->longTermStabilityMin->setValidator(timeValidator);
    ui->longTermStabilitySec->setValidator(timeValidator);

    QValidator *hourValidator = new QRegExpValidator(QRegExp("^(0|[1-9][0-9]{0,2})$"), this);
    ui->longTermStabilityHour->setValidator(hourValidator);

    QValidator *timeInterval = new QRegExpValidator(QRegExp("^([1-9][0-9]{0,3})$"), this);
    ui->longTermStabilityTimeInterval->setValidator(timeInterval);

    QValidator *magnitudeValidator = new QRegExpValidator(QRegExp("^-[1-9][.][0-9]{1,2}|-0[.][1-9]{1,2}|[0-9][.][0-9]{1,2}|-[1-9]|[0-9]$"), this);
    ui->starMagnitude->setValidator(magnitudeValidator);

    QValidator *temperatureValidator = new QRegExpValidator(QRegExp("^[1-9][0-9]{0,5}$"), this);
    ui->starTemperature->setValidator(temperatureValidator);

    QValidator *channelValidator = new QRegExpValidator(QRegExp("^0$|^[1-9][0-9]{0,2}$|^[1-3][0-9]{0,3}$|^40([0-8][0-9]|[9][0-5])$"), this);

    for (int i = 0; i < 96; i++) {
        ledDriverChannel[i]->setValidator(channelValidator);
        starSimulatorX0[i]->setValidator(channelValidator);
    }
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    ui->plotArea->resize(this->width() - 280, this->height() - 170);
    plotSMS500->resize(ui->plotArea->width(), ui->plotArea->height());

    ui->plotAreaLongTermStability->resize(this->width() - 520, this->height() - 200);
    plotLTS->resize(ui->plotAreaLongTermStability->width(), ui->plotAreaLongTermStability->height() - 100);

    ui->plotStarSimulator->resize(this->width() - 650, this->height() * 0.60);
    plotLSqNonLin->resize(ui->plotStarSimulator->width(), ui->plotStarSimulator->height());

    plotLSqNonLin->setxLabel(tr("Wavelength"));
    plotLSqNonLin->setyLabel(tr("Irradiance"));

    if (ui->plotArea->width() > 900) {
        ui->plotAreaLongTermStability->resize(this->width() - 570, this->height() - 190);
        plotLTS->resize(ui->plotAreaLongTermStability->width(), ui->plotAreaLongTermStability->height() * 0.9);

        ui->plotAreaLed->resize(ui->plotArea->width() - 460, ui->plotArea->height() - 100);
        plotLedDriver->resize( ui->plotAreaLed->width(), ui->plotAreaLed->height() * 0.8);

        ui->plotStarSimulator->resize(this->width() - 650, this->height() * 0.60);
        plotLSqNonLin->resize(ui->plotStarSimulator->width(), ui->plotStarSimulator->height());

        ui->scanNumberLabel_2->show();
        ui->scanNumberLabel_2->setGeometry(720,20, 230, 23);
        ui->saturedLabel_2->setGeometry(720, 60, 82, 23);

        // Long Term Stability Labels
        if (ui->startLongTermStabilityLabel->isHidden() == false) {
            ui->startLongTermStabilityLabel->hide();
            ui->startLongTermStabilityLabel_2->show();
            ui->stopLongTermStabilityLabel->hide();
            ui->stopLongTermStabilityLabel_2->show();
        }

        plotLSqNonLin->setxLabel(tr("Wavelength (nm)"));
        plotLSqNonLin->setyLabel(tr("Irradiance (uW/cm^2 nm)"));
    } else {
        ui->plotAreaLed->resize(5, 457);
        plotLedDriver->resize( ui->plotAreaLed->width(), ui->plotAreaLed->height());
        ui->scanNumberLabel_2->hide();

        // Long Term Stability Labels
        if (ui->startLongTermStabilityLabel_2->isHidden() == false) {
            ui->startLongTermStabilityLabel->show();
            ui->startLongTermStabilityLabel_2->hide();
            ui->stopLongTermStabilityLabel->show();
            ui->stopLongTermStabilityLabel_2->hide();
        }
    }
}

double MainWindow::trapezoidalNumInteg(QPolygonF points)
{
    double h;
    double integral = 0;
    int nstep       = points.size();

    for (int i = 0; i < nstep - 1; i++) {
        h = points.at(i+1).x() - points.at(i).x();
        integral += h / 2 * (points.at(i).y() + points.at(i+1).y());
    }

    return integral;
}
