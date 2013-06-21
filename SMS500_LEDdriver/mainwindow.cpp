#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <fstream>

#include "aboutsmsdialog.h"
#include "sms500.h"
#include "plot.h"

#include "leddriver.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->scanNumberLabel->hide();
    ui->scanNumberLabel_2->hide();
    ui->saturedLabel->hide();
    ui->saturedLabel_2->hide();

    plot = new Plot( ui->plotArea );
    plotLedDriver = new Plot( ui->plotAreaLed );

    sms500 = new SMS500(this);
    operationModeChanged();
    //    sms500Connect();

    // LedDriver
    ledDriver = new LedDriver(this);

    mStatusLabel = new QLabel;
    QMainWindow::statusBar()->addPermanentWidget( mStatusLabel );

    sms500SignalAndSlot();
    ledDriverSignalAndSlot();
}

MainWindow::~MainWindow()
{
    //    sms500->closeConnection();
    //    ledDriver->closeConnection();
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    ui->plotArea->resize(this->width() - 280, this->height() - 170);
    plot->resize( ui->plotArea->width(), ui->plotArea->height() );

    if (ui->plotArea->width() > 700) {
        ui->plotAreaLed->resize(ui->plotArea->width() - 450, 457);
        plotLedDriver->resize( ui->plotAreaLed->width(), ui->plotAreaLed->height());

        ui->scanNumberLabel_2->show();
        ui->scanNumberLabel_2->setGeometry(720,20, 230, 23);
        ui->saturedLabel_2->setGeometry(720, 60, 82, 23);
    } else {
        ui->plotAreaLed->resize(5, 457);
        plotLedDriver->resize( ui->plotAreaLed->width(), ui->plotAreaLed->height());
        ui->scanNumberLabel_2->hide();
        ui->saturedLabel_2->hide();
    }
}

void MainWindow::showInfo(const QString &text)
{
    if (text == QString::null) {
        if (plot->plotPicker->rubberBand())
            statusBar()->showMessage(tr("Cursor Pos: Press left mouse button in plot region"));
        else
            statusBar()->showMessage(tr("Zoom: Press mouse button and drag"));
    }
}

void MainWindow::ledDriverConnectDisconnect()
{
    if (ui->btnConnectDisconnectLED->text().contains(tr("Connect"))) {
        ui->btnConnectDisconnectLED->setText(tr("Disconnect"));
        ui->btnConnectDisconnectLED->setIcon(QIcon(":/pics/disconnect.png"));
        if (ledDriver->openConnection() == true) {
            // Configure DACs
            ledDriverDac04Changed();
            ledDriverDac05Changed();
            ledDriverDac06Changed();
            ledDriverDac07Changed();
            ledDriverDac08Changed();
            ledDriverDac09Changed();
            ledDriverDac10Changed();
            ledDriverDac11Changed();
            ledDriverDac12Changed();
        }
    } else {
        ledDriver->closeConnection();
        ui->btnConnectDisconnectLED->setText(tr("Connect"));
        ui->btnConnectDisconnectLED->setIcon(QIcon(":/pics/reconnect.png"));
    }
}

void MainWindow::ledDriverConnectionError(QString serialError, QString statusBarMsg)
{
    ui->btnConnectDisconnectLED->setText(tr("Connect"));
    ui->btnConnectDisconnectLED->setIcon(QIcon(":/pics/reconnect.png"));
    ui->statusBar->showMessage(statusBarMsg);
    QMessageBox::warning(this, tr("Error"), serialError);
}

void MainWindow::ledDriverDac04Changed()
{
    valueOfPort[0] = ui->dac04channel25->text().toUInt();
    valueOfPort[1] = ui->dac04channel26->text().toUInt();
    valueOfPort[2] = ui->dac04channel27->text().toUInt();
    valueOfPort[3] = ui->dac04channel28->text().toUInt();
    valueOfPort[4] = ui->dac04channel29->text().toUInt();
    valueOfPort[5] = ui->dac04channel30->text().toUInt();
    valueOfPort[6] = ui->dac04channel31->text().toUInt();
    valueOfPort[7] = ui->dac04channel32->text().toUInt();

    ledDriverConfigureDac( 3 );
}

void MainWindow::ledDriverDac05Changed()
{
    valueOfPort[0] = ui->dac05channel33->text().toUInt();
    valueOfPort[1] = ui->dac05channel34->text().toUInt();
    valueOfPort[2] = ui->dac05channel35->text().toUInt();
    valueOfPort[3] = ui->dac05channel36->text().toUInt();
    valueOfPort[4] = ui->dac05channel37->text().toUInt();
    valueOfPort[5] = ui->dac05channel38->text().toUInt();
    valueOfPort[6] = ui->dac05channel39->text().toUInt();
    valueOfPort[7] = ui->dac05channel40->text().toUInt();

    ledDriverConfigureDac( 4 );
}

void MainWindow::ledDriverDac06Changed()
{
    valueOfPort[0] = ui->dac06channel41->text().toUInt();
    valueOfPort[1] = ui->dac06channel42->text().toUInt();
    valueOfPort[2] = ui->dac06channel43->text().toUInt();
    valueOfPort[3] = ui->dac06channel44->text().toUInt();
    valueOfPort[4] = ui->dac06channel45->text().toUInt();
    valueOfPort[5] = ui->dac06channel46->text().toUInt();
    valueOfPort[6] = ui->dac06channel47->text().toUInt();
    valueOfPort[7] = ui->dac06channel48->text().toUInt();

    ledDriverConfigureDac( 5 );
}

void MainWindow::ledDriverDac07Changed()
{
    valueOfPort[0] = ui->dac07channel49->text().toUInt();
    valueOfPort[1] = ui->dac07channel50->text().toUInt();
    valueOfPort[2] = ui->dac07channel51->text().toUInt();
    valueOfPort[3] = ui->dac07channel52->text().toUInt();
    valueOfPort[4] = ui->dac07channel53->text().toUInt();
    valueOfPort[5] = ui->dac07channel54->text().toUInt();
    valueOfPort[6] = ui->dac07channel55->text().toUInt();
    valueOfPort[7] = ui->dac07channel56->text().toUInt();

    ledDriverConfigureDac( 6 );
}

void MainWindow::ledDriverDac08Changed()
{
    valueOfPort[0] = ui->dac08channel57->text().toUInt();
    valueOfPort[1] = ui->dac08channel58->text().toUInt();
    valueOfPort[2] = ui->dac08channel59->text().toUInt();
    valueOfPort[3] = ui->dac08channel60->text().toUInt();
    valueOfPort[4] = ui->dac08channel61->text().toUInt();
    valueOfPort[5] = ui->dac08channel62->text().toUInt();
    valueOfPort[6] = ui->dac08channel63->text().toUInt();
    valueOfPort[7] = ui->dac08channel64->text().toUInt();

    ledDriverConfigureDac( 7 );
}

void MainWindow::ledDriverDac09Changed()
{
    valueOfPort[0] = ui->dac09channel65->text().toUInt();
    valueOfPort[1] = ui->dac09channel66->text().toUInt();
    valueOfPort[2] = ui->dac09channel67->text().toUInt();
    valueOfPort[3] = ui->dac09channel68->text().toUInt();
    valueOfPort[4] = ui->dac09channel69->text().toUInt();
    valueOfPort[5] = ui->dac09channel70->text().toUInt();
    valueOfPort[6] = ui->dac09channel71->text().toUInt();
    valueOfPort[7] = ui->dac09channel72->text().toUInt();

    ledDriverConfigureDac( 8 );
}

void MainWindow::ledDriverDac10Changed()
{
    valueOfPort[0] = ui->dac10channel73->text().toUInt();
    valueOfPort[1] = ui->dac10channel74->text().toUInt();
    valueOfPort[2] = ui->dac10channel75->text().toUInt();
    valueOfPort[3] = ui->dac10channel76->text().toUInt();
    valueOfPort[4] = ui->dac10channel77->text().toUInt();
    valueOfPort[5] = ui->dac10channel78->text().toUInt();
    valueOfPort[6] = ui->dac10channel79->text().toUInt();
    valueOfPort[7] = ui->dac10channel80->text().toUInt();

    ledDriverConfigureDac( 9 );
}

void MainWindow::ledDriverDac11Changed()
{
    valueOfPort[0] = ui->dac11channel81->text().toUInt();
    valueOfPort[1] = ui->dac11channel82->text().toUInt();
    valueOfPort[2] = ui->dac11channel83->text().toUInt();
    valueOfPort[3] = ui->dac11channel84->text().toUInt();
    valueOfPort[4] = ui->dac11channel85->text().toUInt();
    valueOfPort[5] = ui->dac11channel86->text().toUInt();
    valueOfPort[6] = ui->dac11channel87->text().toUInt();
    valueOfPort[7] = ui->dac11channel88->text().toUInt();

    ledDriverConfigureDac( 10 );
}

void MainWindow::ledDriverDac12Changed()
{
    valueOfPort[0] = ui->dac12channel89->text().toUInt();
    valueOfPort[1] = ui->dac12channel90->text().toUInt();
    valueOfPort[2] = ui->dac12channel91->text().toUInt();
    valueOfPort[3] = ui->dac12channel92->text().toUInt();
    valueOfPort[4] = ui->dac12channel93->text().toUInt();
    valueOfPort[5] = ui->dac12channel94->text().toUInt();
    valueOfPort[6] = ui->dac12channel95->text().toUInt();
    valueOfPort[7] = ui->dac12channel96->text().toUInt();

    ledDriverConfigureDac( 11 );
}

void MainWindow::ledDriverConfigureDac(char dac)
{
    for (int port = 0; port < 8; port++) {
        txBuffer[0] = 0x0C;
        txBuffer[1] = 0x40 |  (valueOfPort[port] & 0x0000000F);
        txBuffer[2] = 0x80 | ((valueOfPort[port] & 0x000000F0) >> 4);
        txBuffer[3] = 0x0D;
        txBuffer[4] = 0x40 | ((valueOfPort[port] & 0x00000F00) >> 8);
        txBuffer[5] = 0x80 | port;
        txBuffer[6] =  dac;
        txBuffer[7] = 0x40;
        txBuffer[8] = 0x80;
        txBuffer[9] = '\0';

        if (ledDriver->writeData(txBuffer) == false) {
            ui->statusBar->showMessage("Transmission Error: check USB connection and try again!", 5000);
            break;
        }
        ledDriver->wait();
    }
}

void MainWindow::sms500Configure()
{
    QString operationMode;
    QString calibratedDataPath;

    // Operation Mode
    if (ui->rbtnFlux->isChecked()) {
        operationMode      = "Flux";
        calibratedDataPath = "Flux.dat";
    } else if (ui->rbtnIntensity->isChecked()) {
        operationMode      = "Intensity";
        calibratedDataPath = "Irradiance.dat";
    } else if (ui->rbtnIrradiance->isChecked()) {
        operationMode      = "lux";
        calibratedDataPath = "Irradiance.dat";
    } else if (ui->rbtnRadiance->isChecked()) {
        operationMode      = "cd/m2";
        calibratedDataPath = "Radiance.dat";
    }

    sms500->setOperationMode(operationMode);
    sms500->setCalibratedDataPath(calibratedDataPath);

    // Parameters
    sms500->setAutoRange(ui->AutoRangeCheckBox->isChecked());
    sms500->setRange(ui->integrationTimeComboBox->currentIndex());
    sms500->setBoxCarSmoothing(ui->smoothingSpinBox->value());
    sms500->setAverage(ui->samplesToAverageSpinBox->value());
    sms500->setStartWave(ui->startWaveLineEdit->text().toInt());
    sms500->setStopWave(ui->stopWaveLineEdit->text().toInt());
    sms500->setCorrecDarkCurrent(ui->dynamicDarkCheckBox->isChecked());
    sms500->setNoiseReduction(ui->noiseReductionCheckBox->isChecked(), ui->noiseReductionLineEdit->text().toDouble());
    sms500->setNumberOfScans(ui->numberOfScansLineEdit->text().toInt());
}

bool MainWindow::sms500Connect()
{
    sms500Configure();

    if (sms500->openConnection() == true) {
        statusBar()->showMessage(tr("SMS500 successfully connected"));
        ui->btnConnectDisconnect->setIcon(QIcon(":/pics/disconnect.png"));
        ui->btnConnectDisconnect->setText(tr("Disconnect SMS"));
        return true;
    }

    QMessageBox::warning(this, tr("Connection Error"),
                         tr("Spectrometer Hardware not found.\n\n"
                            "Check USB connection and try again..."));

    statusBar()->showMessage(tr("SMS500 not connected"));
    ui->btnConnectDisconnect->setIcon(QIcon(":/pics/reconnect.png"));
    ui->btnConnectDisconnect->setText(tr("Reconnect SMS"));
    return false;
}


void MainWindow::sms500Disconnect()
{
    if (sms500->isConnected() == true) {
        sms500->closeConnection();
        statusBar()->showMessage(tr("SMS500 not connected"));
        ui->btnConnectDisconnect->setIcon(QIcon(":/pics/reconnect.png"));
        ui->btnConnectDisconnect->setText(tr("Reconnect SMS"));
        ui->btnStartScan->setIcon(QIcon(":/pics/start.png"));
        ui->btnStartScan->setText("Start Scan");
    }
}

void MainWindow::aboutThisSoftware()
{
    QMessageBox::about(this, tr("About this Software"),
                       tr("This software was developed to control the SMS500 SphereOptics Spectroradiometer\n"
                          "(Spectral Measurement System - SMS500).\n\n"
                          "Author: Marcos Eduardo Gomes Borges <marcoseborges@gmail.com>\n"
                          "Contributor: Braulio Fonseca Carneiro de Albuquerque <brauliofca@gmail.com>.\n\n"
                          "This program is free software\n\n"
                          "National Institute For Space Research (INPE), Brazil."));
}

void MainWindow::aboutSMS500()
{
    if (sms500->isConnected() == true) {
        AboutSMSDialog *dlg = new AboutSMSDialog(this);
        dlg->setDLL(QString::number(sms500->dllVersion()));
        dlg->setSerialNumber(sms500->serialNumber());
        dlg->setFirstCoefficient(QString::number(sms500->coefficient1()));
        dlg->setSecondCoefficient(QString::number(sms500->coefficient2()));
        dlg->setThirdCoefficient(QString::number(sms500->coefficient3()));
        dlg->setIntercept(QString::number(sms500->intercept()));
        dlg->exec();
    }
}

void MainWindow::operationModeChanged()
{
    if (ui->rbtnFlux->isChecked()) {
        plot->setTitle("Operation Mode: Flux");
        plotLedDriver->setTitle("Operation Mode: Flux");
    } else if (ui->rbtnIntensity->isChecked()) {
        plot->setTitle("Operation Mode: Intensity");
        plotLedDriver->setTitle("Operation Mode: Intensity");
    } else if (ui->rbtnIrradiance->isChecked()) {
        plot->setTitle("Operation Mode: Irradiance (lux)");
        plotLedDriver->setTitle("Operation Mode: Irradiance (lux)");
    } else if (ui->rbtnRadiance->isChecked()) {
        plot->setTitle("Operation Mode: Radiance (cd/m2)");
        plotLedDriver->setTitle("Operation Mode: Radiance (cd/m2)");
    }
}

void MainWindow::autoRangeChanged(bool enable)
{
    ui->integrationTimeComboBox->setEnabled(!enable);
}

void MainWindow::noiseReductionChanged(bool enable)
{
    ui->noiseReductionLineEdit->setEnabled(enable);
}

void MainWindow::connectDisconnect()
{
    if (ui->btnConnectDisconnect->text().contains(tr("Reconnect SMS"))) {
        sms500Connect();
    } else {
        sms500Disconnect();
    }
}

void MainWindow::startStopScan()
{
    if (sms500->isConnected() == false) {
        if (sms500Connect() == false) {
            return;
        }
    }

    if (ui->btnStartScan->text().contains("Start Scan")) {
        ui->btnStartScan->setIcon(QIcon(":/pics/stop.png"));
        ui->btnStartScan->setText("Stop Scan");
        ui->scanNumberLabel->setText(tr("    Scan number: %1").arg(0));
        ui->scanNumberLabel->show();
        ui->scanNumberLabel_2->setText(tr("    Scan number: %1").arg(0));
        ui->scanNumberLabel_2->show();
        ui->btnZoom->setEnabled(false);
        performScan();
    } else {
        ui->btnStartScan->setIcon(QIcon(":/pics/start.png"));
        ui->btnStartScan->setText("Start Scan");
        ui->btnSaveScan->setEnabled(true);
        sms500->stop();
    }
}

void MainWindow::performScan()
{
    if (sms500->isConnected() == false) {
        if (sms500Connect() == false) {
            return;
        }
    }

    statusBar()->showMessage(tr("Geting Spectral Data"), 5000);
    sms500Configure();
    sms500->start();
}

void MainWindow::plotScanResult(const double *masterData, const int *wavelegth, int peakWavelength,
                                double amplitude,int numberOfPoints, int scanNumber, int integrationTimeIndex, bool satured)
{
    QPolygonF points;

    plot->setPlotLimits(300, 1200, 0, amplitude);
    plotLedDriver->setPlotLimits(300, 1200, 0, amplitude);

    for (int i = 0; i < numberOfPoints; i++) {
        if (masterData[i] < 0) {
            points << QPointF(wavelegth[i], 0);
        } else {
            points << QPointF(wavelegth[i], masterData[i]);
        }
    }

    const bool doReplot = plot->autoReplot();
    plot->setAutoReplot( false );
    plot->showPeak(peakWavelength, amplitude);
    plot->showData(points, amplitude);
    plot->setAutoReplot( doReplot );
    plot->replot();

    // Plot Led Driver
    plotLedDriver->setAutoReplot( false );
    plotLedDriver->showPeak(peakWavelength, amplitude);
    plotLedDriver->showData(points, amplitude);
    plotLedDriver->setAutoReplot( doReplot );
    plotLedDriver->replot();

    if (ui->AutoRangeCheckBox->isChecked()) {
        ui->integrationTimeComboBox->setCurrentIndex(integrationTimeIndex);
        sms500->setRange(integrationTimeIndex);
    }

    ui->scanNumberLabel->setText(tr("    Scan number: %1").arg(scanNumber));
    ui->scanNumberLabel_2->setText(tr("    Scan number: %1").arg(scanNumber));

    if (satured == true) {
        ui->saturedLabel->show();
        ui->saturedLabel_2->show();
    } else {
        ui->saturedLabel->hide();
        ui->saturedLabel_2->hide();
    }

    statusBar()->showMessage(tr("Geting Spectral Data"), 5000);
    sms500Configure();
    sms500->enableNextScan();
}

void MainWindow::scanFinished()
{
    ui->btnSaveScan->setEnabled(true);
    ui->btnZoom->setEnabled(true);
    ui->btnStartScan->setIcon(QIcon(":/pics/start.png"));
    ui->btnStartScan->setText("Start Scan");

    sms500->stop();
    sms500->wait();
}

void MainWindow::saveScanData()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save SMS Scan Data"), QDir::homePath(), tr("Text File *.txt"));

    if (filePath.isEmpty()) {
        return;
    }

    using namespace std;
    // ofstream constructor opens file
    ofstream outFile( filePath.toAscii().data(), ios::out );

    // exit program if unable to create file
    if ( !outFile ) // overloaded ! operator
    {
        statusBar()->showMessage( tr("File could not be create") );
        exit(1);
    }

    outFile << "Start wavelength:\t" << sms500->startWavelength() << endl;
    outFile << "Stop wavelength:\t" << sms500->stopWavelength() << endl;
    outFile << "Dominate wavelength:\t" << sms500->dominanteWavelength() << endl;
    outFile << "Peak wavelength:\t" << sms500->peakWavelength() << endl;
    outFile << "Power (W):\t\t" << sms500->power() << endl;
    outFile << "Integration time (ms):\t" << sms500->integrationTime() << endl;
    outFile << "Purity:\t\t\t" << sms500->purity() << endl;
    outFile << "FWHM:\t\t\t" << sms500->fwhm() << endl;
    outFile << "\nHolds the Wavelength Data:" << endl;
    outFile << "nm\tuW/nm" << endl;

    QString number;

    double *masterData;
    int    *waveLength;

    masterData = sms500->masterData();
    waveLength = sms500->wavelength();

    for (int i = 0; i < sms500->points(); i++) {
        number = QString::number(masterData[i]);
        number.replace(".", ",");

        outFile << waveLength[ i ] << "\t" << number.toAscii().data() << endl;
    }

    QMainWindow::statusBar()->showMessage("Log gerado com sucess!", 5000);
}

void MainWindow::systemZero()
{
    if (sms500->isConnected() == false) {
        if (sms500Connect() == false) {
            QMessageBox::warning(this, tr("Connection Error"),
                                 tr("Spectrometer Hardware not found.\n\n"
                                    "Check USB connection and try again..."));
            return;
        }
    }

    sms500Configure();

    if (QMessageBox::question(this, tr("Sytem Zero"),
                              tr("Set the SMS-500 for Dark Current Reading."),
                              tr("Yes"), tr("No") )) {
        return;
    }

    sms500->creatDarkRatioArray();
}

void MainWindow::calibrateSystem()
{
    systemZero();

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

    if (lampFile.isEmpty()) {
        return;
    }

    if (sms500->readCalibratedLamp(lampFile) == false) {
        statusBar()->showMessage("Error trying to open calibration file");
        return;
    }

    statusBar()->showMessage("Performing calibration, please wait", 10);

    sms500->startLampScan();


    if (QMessageBox::question(this, tr("Sytem Zero"),
                              tr("Set the SMS-500 for Dark Current Reading. To continue with calibration,\n"
                                 "click on the 'YES' button, 'No' to cancel"),
                              tr("Yes"), tr("No") )) {
        return;
    }

    sms500->finishLampScan();

    QMessageBox::question(this, tr("Calibration Completed"),
                          tr("System calibration completed..."),
                          tr("Ok"));
}

void MainWindow::moved( const QPoint &pos )
{
    QString info;
    info.sprintf( "Wavelength=%g, Amplitude=%g",
                  plot->invTransform( QwtPlot::xBottom, pos.x() ),
                  plot->invTransform( QwtPlot::yLeft, pos.y() )
                  );
    showInfo( info );
}

void MainWindow::selected( const QPolygon & )
{
    showInfo();
}

void MainWindow::sms500SignalAndSlot()
{
    connect(ui->btnZoom, SIGNAL(toggled(bool)), plot, SLOT(enableZoomMode(bool)));
    connect(plot->plotPicker, SIGNAL(moved(const QPoint&)), this, SLOT(moved(const QPoint&)));
    connect(plot->plotPicker, SIGNAL(selected(const QPolygon&)), this, SLOT(selected(const QPolygon&)));
    connect(plot, SIGNAL(showInfo()), this, SLOT(showInfo()));

    connect(ui->btnStartScan, SIGNAL(clicked()), this, SLOT(startStopScan()));
    connect(ui->btnSaveScan, SIGNAL(clicked()), this, SLOT(saveScanData()));
    connect(ui->btnConnectDisconnect, SIGNAL(clicked()), this, SLOT(connectDisconnect()));

    connect(ui->AutoRangeCheckBox, SIGNAL(toggled(bool)), this, SLOT(autoRangeChanged(bool)));
    connect(ui->noiseReductionCheckBox, SIGNAL(toggled(bool)), this, SLOT(noiseReductionChanged(bool)));

    connect(ui->rbtnFlux, SIGNAL(toggled(bool)), this, SLOT(operationModeChanged()));
    connect(ui->rbtnIntensity, SIGNAL(toggled(bool)), this, SLOT(operationModeChanged()));
    connect(ui->rbtnIrradiance, SIGNAL(toggled(bool)), this, SLOT(operationModeChanged()));
    connect(ui->rbtnRadiance, SIGNAL(toggled(bool)), this, SLOT(operationModeChanged()));

    connect(ui->actionSystemZero, SIGNAL(triggered(bool)), this, SLOT(systemZero()));
    connect(ui->actionCalibrateSystem, SIGNAL(triggered(bool)), this, SLOT(calibrateSystem()));

    connect(ui->actionAboutSMS500, SIGNAL(triggered(bool)), this, SLOT(aboutSMS500()));
    connect(ui->actionAboutThisSoftware, SIGNAL(triggered(bool)), this, SLOT(aboutThisSoftware()));

    connect(sms500, SIGNAL(scanPerformed(const double*,const int*,int,double,int,int,int, bool)), this,
            SLOT(plotScanResult(const double*,const int*,int,double,int,int,int, bool)));
    connect(sms500, SIGNAL(scanFinished()), this, SLOT(scanFinished()));
}

void MainWindow::ledDriverSignalAndSlot()
{
    connect(ui->btnConnectDisconnectLED, SIGNAL(clicked()), this, SLOT(ledDriverConnectDisconnect()));
    connect(ledDriver, SIGNAL(connectionError(QString, QString)), this, SLOT(ledDriverConnectionError(QString, QString)));

    connect(ui->dac04channel25, SIGNAL(editingFinished()), this, SLOT(ledDriverDac04Changed()));
    connect(ui->dac04channel26, SIGNAL(editingFinished()), this, SLOT(ledDriverDac04Changed()));
    connect(ui->dac04channel27, SIGNAL(editingFinished()), this, SLOT(ledDriverDac04Changed()));
    connect(ui->dac04channel28, SIGNAL(editingFinished()), this, SLOT(ledDriverDac04Changed()));
    connect(ui->dac04channel29, SIGNAL(editingFinished()), this, SLOT(ledDriverDac04Changed()));
    connect(ui->dac04channel30, SIGNAL(editingFinished()), this, SLOT(ledDriverDac04Changed()));
    connect(ui->dac04channel31, SIGNAL(editingFinished()), this, SLOT(ledDriverDac04Changed()));
    connect(ui->dac04channel32, SIGNAL(editingFinished()), this, SLOT(ledDriverDac04Changed()));

    connect(ui->dac05channel33, SIGNAL(editingFinished()), this, SLOT(ledDriverDac05Changed()));
    connect(ui->dac05channel34, SIGNAL(editingFinished()), this, SLOT(ledDriverDac05Changed()));
    connect(ui->dac05channel35, SIGNAL(editingFinished()), this, SLOT(ledDriverDac05Changed()));
    connect(ui->dac05channel36, SIGNAL(editingFinished()), this, SLOT(ledDriverDac05Changed()));
    connect(ui->dac05channel37, SIGNAL(editingFinished()), this, SLOT(ledDriverDac05Changed()));
    connect(ui->dac05channel38, SIGNAL(editingFinished()), this, SLOT(ledDriverDac05Changed()));
    connect(ui->dac05channel39, SIGNAL(editingFinished()), this, SLOT(ledDriverDac05Changed()));
    connect(ui->dac05channel40, SIGNAL(editingFinished()), this, SLOT(ledDriverDac05Changed()));

    connect(ui->dac06channel41, SIGNAL(editingFinished()), this, SLOT(ledDriverDac06Changed()));
    connect(ui->dac06channel42, SIGNAL(editingFinished()), this, SLOT(ledDriverDac06Changed()));
    connect(ui->dac06channel43, SIGNAL(editingFinished()), this, SLOT(ledDriverDac06Changed()));
    connect(ui->dac06channel44, SIGNAL(editingFinished()), this, SLOT(ledDriverDac06Changed()));
    connect(ui->dac06channel45, SIGNAL(editingFinished()), this, SLOT(ledDriverDac06Changed()));
    connect(ui->dac06channel46, SIGNAL(editingFinished()), this, SLOT(ledDriverDac06Changed()));
    connect(ui->dac06channel47, SIGNAL(editingFinished()), this, SLOT(ledDriverDac06Changed()));
    connect(ui->dac06channel48, SIGNAL(editingFinished()), this, SLOT(ledDriverDac06Changed()));

    connect(ui->dac07channel49, SIGNAL(editingFinished()), this, SLOT(ledDriverDac07Changed()));
    connect(ui->dac07channel50, SIGNAL(editingFinished()), this, SLOT(ledDriverDac07Changed()));
    connect(ui->dac07channel51, SIGNAL(editingFinished()), this, SLOT(ledDriverDac07Changed()));
    connect(ui->dac07channel52, SIGNAL(editingFinished()), this, SLOT(ledDriverDac07Changed()));
    connect(ui->dac07channel53, SIGNAL(editingFinished()), this, SLOT(ledDriverDac07Changed()));
    connect(ui->dac07channel54, SIGNAL(editingFinished()), this, SLOT(ledDriverDac07Changed()));
    connect(ui->dac07channel55, SIGNAL(editingFinished()), this, SLOT(ledDriverDac07Changed()));
    connect(ui->dac07channel56, SIGNAL(editingFinished()), this, SLOT(ledDriverDac07Changed()));

    connect(ui->dac08channel57, SIGNAL(editingFinished()), this, SLOT(ledDriverDac08Changed()));
    connect(ui->dac08channel58, SIGNAL(editingFinished()), this, SLOT(ledDriverDac08Changed()));
    connect(ui->dac08channel59, SIGNAL(editingFinished()), this, SLOT(ledDriverDac08Changed()));
    connect(ui->dac08channel60, SIGNAL(editingFinished()), this, SLOT(ledDriverDac08Changed()));
    connect(ui->dac08channel61, SIGNAL(editingFinished()), this, SLOT(ledDriverDac08Changed()));
    connect(ui->dac08channel62, SIGNAL(editingFinished()), this, SLOT(ledDriverDac08Changed()));
    connect(ui->dac08channel63, SIGNAL(editingFinished()), this, SLOT(ledDriverDac08Changed()));
    connect(ui->dac08channel64, SIGNAL(editingFinished()), this, SLOT(ledDriverDac08Changed()));

    connect(ui->dac09channel65, SIGNAL(editingFinished()), this, SLOT(ledDriverDac09Changed()));
    connect(ui->dac09channel66, SIGNAL(editingFinished()), this, SLOT(ledDriverDac09Changed()));
    connect(ui->dac09channel67, SIGNAL(editingFinished()), this, SLOT(ledDriverDac09Changed()));
    connect(ui->dac09channel68, SIGNAL(editingFinished()), this, SLOT(ledDriverDac09Changed()));
    connect(ui->dac09channel69, SIGNAL(editingFinished()), this, SLOT(ledDriverDac09Changed()));
    connect(ui->dac09channel70, SIGNAL(editingFinished()), this, SLOT(ledDriverDac09Changed()));
    connect(ui->dac09channel71, SIGNAL(editingFinished()), this, SLOT(ledDriverDac09Changed()));
    connect(ui->dac09channel72, SIGNAL(editingFinished()), this, SLOT(ledDriverDac09Changed()));

    connect(ui->dac10channel73, SIGNAL(editingFinished()), this, SLOT(ledDriverDac10Changed()));
    connect(ui->dac10channel74, SIGNAL(editingFinished()), this, SLOT(ledDriverDac10Changed()));
    connect(ui->dac10channel75, SIGNAL(editingFinished()), this, SLOT(ledDriverDac10Changed()));
    connect(ui->dac10channel76, SIGNAL(editingFinished()), this, SLOT(ledDriverDac10Changed()));
    connect(ui->dac10channel77, SIGNAL(editingFinished()), this, SLOT(ledDriverDac10Changed()));
    connect(ui->dac10channel78, SIGNAL(editingFinished()), this, SLOT(ledDriverDac10Changed()));
    connect(ui->dac10channel79, SIGNAL(editingFinished()), this, SLOT(ledDriverDac10Changed()));
    connect(ui->dac10channel80, SIGNAL(editingFinished()), this, SLOT(ledDriverDac10Changed()));

    connect(ui->dac11channel81, SIGNAL(editingFinished()), this, SLOT(ledDriverDac11Changed()));
    connect(ui->dac11channel82, SIGNAL(editingFinished()), this, SLOT(ledDriverDac11Changed()));
    connect(ui->dac11channel83, SIGNAL(editingFinished()), this, SLOT(ledDriverDac11Changed()));
    connect(ui->dac11channel84, SIGNAL(editingFinished()), this, SLOT(ledDriverDac11Changed()));
    connect(ui->dac11channel85, SIGNAL(editingFinished()), this, SLOT(ledDriverDac11Changed()));
    connect(ui->dac11channel86, SIGNAL(editingFinished()), this, SLOT(ledDriverDac11Changed()));
    connect(ui->dac11channel87, SIGNAL(editingFinished()), this, SLOT(ledDriverDac11Changed()));
    connect(ui->dac11channel88, SIGNAL(editingFinished()), this, SLOT(ledDriverDac11Changed()));

    connect(ui->dac12channel89, SIGNAL(editingFinished()), this, SLOT(ledDriverDac12Changed()));
    connect(ui->dac12channel90, SIGNAL(editingFinished()), this, SLOT(ledDriverDac12Changed()));
    connect(ui->dac12channel91, SIGNAL(editingFinished()), this, SLOT(ledDriverDac12Changed()));
    connect(ui->dac12channel92, SIGNAL(editingFinished()), this, SLOT(ledDriverDac12Changed()));
    connect(ui->dac12channel93, SIGNAL(editingFinished()), this, SLOT(ledDriverDac12Changed()));
    connect(ui->dac12channel94, SIGNAL(editingFinished()), this, SLOT(ledDriverDac12Changed()));
    connect(ui->dac12channel95, SIGNAL(editingFinished()), this, SLOT(ledDriverDac12Changed()));
    connect(ui->dac12channel96, SIGNAL(editingFinished()), this, SLOT(ledDriverDac12Changed()));
}
