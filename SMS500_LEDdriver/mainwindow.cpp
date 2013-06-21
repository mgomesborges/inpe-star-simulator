#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <fstream>

#include "aboutsmsdialog.h"
#include "leastsquarenonlin.h"
#include "star.h"
#include "sms500.h"
#include "plot.h"

#include "leddriver.h"

#include <iostream>
#include <Eigen/Dense>
using namespace Eigen;

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
    statusBar()->addPermanentWidget( mStatusLabel );

    // Star
    star = new Star(this);
    star->setMagnitude("0");
    star->setTemperature("7500");

    connect(ui->actionStar_Settings, SIGNAL(triggered(bool)), this, SLOT(starSettings()));
    connect(ui->actionLeast_Square, SIGNAL(triggered(bool)), this, SLOT(leastSquare()));

    lsqnonlin = new LeastSquareNonLin(this);
//    connect(lsqnonlin, SIGNAL(info(QString)), this, SLOT(statusBarMessage(QString)));
//    connect(lsqnonlin, SIGNAL(performScan()), this, SLOT(leastSquarePerformScan()));
//    connect(sms500, SIGNAL(scanFinished()), this, SLOT(leastSquareObjectiveFunction()));


    sms500SignalAndSlot();
    ledDriverSignalAndSlot();
}

MainWindow::~MainWindow()
{
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

        if (this->width() >= 1100) {
            ui->scanInfo->show();
            ui->scanInfo->setGeometry(950,20,230,23);
        }

        ui->saturedLabel_2->setGeometry(720, 60, 82, 23);
    } else {
        ui->plotAreaLed->resize(5, 457);
        plotLedDriver->resize( ui->plotAreaLed->width(), ui->plotAreaLed->height());
        ui->scanNumberLabel_2->hide();
        ui->scanInfo->hide();
        ui->saturedLabel_2->hide();
    }
}

void MainWindow::showInfo(const QString &text)
{
    if (text == QString::null) {
        if (plot->plotPicker->rubberBand()) {
            statusBar()->showMessage(tr("Cursor Pos: Press left mouse button in plot region"));
        } else {
            statusBar()->showMessage(tr("Zoom: Press mouse button and drag"));
        }
    }
}

void MainWindow::warningMessage(QString title, QString message)
{
    QMessageBox::warning(this, title, message);
}

void MainWindow::ledDriverConnectDisconnect()
{
    if (ui->btnConnectDisconnectLED->text().contains(tr("Connect"))) {

        // Prevents communnication errors
        ledDriver->closeConnection();

        if (ledDriver->openConnection() == true) {
            ui->btnConnectDisconnectLED->setText(tr("Disconnect"));
            ui->btnConnectDisconnectLED->setIcon(QIcon(":/pics/disconnect.png"));

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
        } else {
            QMessageBox::warning(this, tr("LED Driver Error"), tr("LED Driver Open Comunication Error"));
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
    statusBar()->showMessage(statusBarMsg);
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
            statusBar()->showMessage("Transmission Error: check USB connection and try again!", 5000);
            break;
        }
    }
}

void MainWindow::statusBarMessage(QString message)
{
    statusBar()->showMessage(message);
}

void MainWindow::ledModeling()
{
    if (ui->btnLedModeling->text().contains("LED Modeling") == true) {
        ledModelingFilePath = QFileDialog::getExistingDirectory(this, tr("Choose the directory to save Save Led Modeling Data"), QDir::homePath());

        if (ledModelingFilePath.isEmpty()) {
            return;
        }

        // SMS500 Configure
        if (sms500->isConnected() == false) {
            if (sms500Connect() == false) {
                return;
            }
        }

        // Set Operation Mode to Flux
        ui->rbtnFlux->setChecked(true);

        // Parameters
        ui->AutoRangeCheckBox->setChecked(true);
        ui->numberOfScansLineEdit->setText("1");
//        ui->dynamicDarkCheckBox->setChecked(true);
//        ui->smoothingSpinBox->setValue(2);

        sms500Configure();

        // LED Driver Connection
        if (ledDriver->isConnected() == false) {
            ledDriverConnectDisconnect();
        }

        ledDriver->setModelingParameters(ui->startChannel->text().toInt(),
                                         ui->endChannel->text().toInt(),
                                         ui->levelDecrement->text().toInt());

        scanNumberOfLedModeling = 0;

        ui->btnLedModeling->setText("STOP Modeling");
        ui->btnLedModeling->setIcon(QIcon(":/pics/led.jpg"));

        ledDriver->start();

        // Inits LED Modeling Data structure
        ledModelingData.resize( 641 ); // Size = 1000nm - 360nm = 640
        for (int i = 0; i <= 640; i++) {
            ledModelingData[i].resize( 1 );
            ledModelingData[i][0] = i + 360;
        }

        // Disable LED Driver GUI Interface
//        ui->groupBox_5->setEnabled(false);
//        ui->groupBox_10->setEnabled(false);
//        ui->groupBox_11->setEnabled(false);
//        ui->groupBox_12->setEnabled(false);
//        ui->groupBox_13->setEnabled(false);
//        ui->groupBox_14->setEnabled(false);
//        ui->groupBox_15->setEnabled(false);
//        ui->groupBox_16->setEnabled(false);
//        ui->groupBox_17->setEnabled(false);
//        ui->groupBox_18->setEnabled(false);
    } else {
        ledDriver->stop();
        ledDriver->wait(3000);
        ui->btnLedModeling->setText("LED Modeling");
        ui->btnLedModeling->setIcon(QIcon(":/pics/led.jpg"));
    }
}

void MainWindow::ledModelingPerformScan()
{
    sms500->start();
}

void MainWindow::ledModelingSaveData(QString fileName)
{
    scanNumberOfLedModeling++;
    ui->scanNumberLabel->setText(tr("    Scan number: %1").arg(scanNumberOfLedModeling));
    ui->scanNumberLabel_2->setText(tr("    Scan number: %1").arg(scanNumberOfLedModeling));

    // Stop Condition
    if (sms500->maxMasterData() < 5) {
        ledDriver->modelingNextChannel();
    }

    QString filePath(ledModelingFilePath + fileName);

    // ofstream constructor opens file
    std::ofstream outFile( filePath.toAscii().data(), std::ios::out );

    // exit program if unable to create file
    if ( !outFile ) // overloaded ! operator
    {
        QMessageBox::warning(this, tr("Save Modeling Data Error"), tr("File could not be create"));
        return;
    }

    outFile << "Start wavelength:\t" << sms500->startWavelength() << std::endl;
    outFile << "Stop wavelength:\t" << sms500->stopWavelength() << std::endl;
    outFile << "Dominate wavelength:\t" << sms500->dominanteWavelength() << std::endl;
    outFile << "Peak wavelength:\t" << sms500->peakWavelength() << std::endl;
    outFile << "Power (W):\t\t" << sms500->power() << std::endl;
    outFile << "Integration time (ms):\t" << sms500->integrationTime() << std::endl;
    outFile << "Samples to Average:\t" << sms500->samplesToAverage() << std::endl;
    outFile << "Boxcar Smoothing: \t" << sms500->boxCarSmoothing() << std::endl;
    outFile << "Purity:\t\t\t" << sms500->purity() << std::endl;
    outFile << "FWHM:\t\t\t" << sms500->fwhm() << std::endl;
    outFile << "\nHolds the Wavelength Data:" << std::endl;
    outFile << "nm\tuW/nm" << std::endl;

    double *masterData;
    int    *waveLength;

    masterData = sms500->masterData();
    waveLength = sms500->wavelength();

    for (int i = 0; i < sms500->points(); i++) {
        if (masterData[i] < 0) {
            outFile << waveLength[ i ] << "\t" << "0.0" << std::endl;
        } else {
            outFile << waveLength[ i ] << "\t" << masterData[i] << std::endl;
        }
    }

    outFile.close();

    // Adds Data in LED Modeling Data
    for (int i = 0; i < sms500->points(); i++) {
        ledModelingData[i].resize(scanNumberOfLedModeling + 1);

        if (masterData[i] < 0) {
            ledModelingData[i][scanNumberOfLedModeling] = 0;
        } else {
            ledModelingData[i][scanNumberOfLedModeling] = masterData[i];
        }
    }

    ledDriver->enabledModelingContinue();
}

void MainWindow::ledModelingSaveChannelData(QString channel)
{
    // Saves LED Model Data
    QString filename(tr("/ch%1.txt").arg(channel));

    QString ledFilePath(ledModelingFilePath + filename);

    // ofstream constructor opens file
    std::ofstream ledModelingFile( ledFilePath.toAscii().data(), std::ios::out );

    // exit program if unable to create file
    if ( !ledModelingFile ) // overloaded ! operator
    {
        QMessageBox::warning(this, tr("Save Modeling Data Error"), tr("File could not be create"));
        return;
    }

    for (int i = 0; i <= 640; i++) {
        for (unsigned int j = 0; j < ledModelingData[0].size(); j++) {
            ledModelingFile << ledModelingData[i][j] << '\t';
        }
        ledModelingFile << std::endl;
    }

    ledModelingFile.close();


    // Inits LED Modeling Data structure
    ledModelingData.erase(ledModelingData.begin(), ledModelingData.end());
    ledModelingData.resize( 641 ); // Size = 1000nm - 360nm = 640
    for (int i = 0; i <= 640; i++) {
        ledModelingData[i].resize( 1 );
        ledModelingData[i][0] = i + 360;
    }

    // Resets scanNumberOfLedModeling and Scan Info
    scanNumberOfLedModeling = 0;
    ui->scanInfo->setText(tr("Next Channel... Wait!"));
}

void MainWindow::ledModelingFinished()
{
    ui->btnLedModeling->setText("LED Modeling");
    ui->btnLedModeling->setIcon(QIcon(":/pics/led.jpg"));
    QMessageBox::warning(this, tr("LED Modeling finished"), tr("Press Ok to continue"));

    // Enable LED Driver GUI Interface
//    ui->groupBox_5->setEnabled(true);
//    ui->groupBox_10->setEnabled(true);
//    ui->groupBox_11->setEnabled(true);
//    ui->groupBox_12->setEnabled(true);
//    ui->groupBox_13->setEnabled(true);
//    ui->groupBox_14->setEnabled(true);
//    ui->groupBox_15->setEnabled(true);
//    ui->groupBox_16->setEnabled(true);
//    ui->groupBox_17->setEnabled(true);
    //    ui->groupBox_18->setEnabled(true);
}

void MainWindow::ledModelingInfo(QString message)
{
    ui->scanInfo->setText(message);
}

void MainWindow::starSettings()
{
    if (star->exec() == true) {
        int starMagnitude   = star->magnitude();
        int starTemperature = star->temperature();

        starData = star->getData(starMagnitude, starTemperature);

        QPolygonF points;

        for (int i = 0; i <= 640; i++) {
            points << QPointF(starData[i][0], starData[i][1]);
        }

        const bool doReplot = plot->autoReplot();
        plot->setAutoReplot( false );
        plot->showData(points, 1000);
        plot->setAutoReplot( doReplot );
        plot->replot();

        // Plot Led Driver
        plotLedDriver->setAutoReplot( false );
        plotLedDriver->showData(points, 1000);
        plotLedDriver->setAutoReplot( doReplot );
        plotLedDriver->replot();
    }
}

void MainWindow::leastSquare()
{
    connect(lsqnonlin, SIGNAL(info(QString)), this, SLOT(statusBarMessage(QString)));
    connect(lsqnonlin, SIGNAL(performScan()), this, SLOT(leastSquarePerformScan()));
    connect(sms500, SIGNAL(scanFinished()), this, SLOT(leastSquareObjectiveFunction()));

    lsqnonlin->start();
}

void MainWindow::leastSquarePerformScan()
{
    // STAR
    int starMagnitude   = star->magnitude();
    int starTemperature = star->temperature();
    starData            = star->getData(starMagnitude, starTemperature);

    QPolygonF points;

    for (int i = 0; i <= 640; i++) {
        points << QPointF(starData[i][0], starData[i][1]);
    }

    const bool doReplot = plot->autoReplot();
    plot->setAutoReplot( false );
    plot->showData(points, 1000);
    plot->setAutoReplot( doReplot );
    plot->replot();

    // Plot Led Driver
    plotLedDriver->setAutoReplot( false );
    plotLedDriver->showData(points, 1000);
    plotLedDriver->setAutoReplot( doReplot );
    plotLedDriver->replot();

    // SMS500 Configure
    if (sms500->isConnected() == false) {
        if (sms500Connect() == false) {
            return;
        }
    }

    // Set Operation Mode to Flux
    ui->rbtnFlux->setChecked(true);

    // Parameters
//    ui->AutoRangeCheckBox->setChecked(false);
    ui->AutoRangeCheckBox->setChecked(true);
    ui->integrationTimeComboBox->setCurrentIndex(7);
    ui->numberOfScansLineEdit->setText("1");
    ui->dynamicDarkCheckBox->setChecked(true);
//    ui->smoothingSpinBox->setValue(2);

    sms500Configure();

    // LED Driver Connection
    if (ledDriver->isConnected() == false) {
        ledDriverConnectDisconnect();
    }

//    ledDriver->resetDACs();

    int index;
    int dac;
    int port;
    int levelValue;
    char txBuffer[10];

    Matrix<int, 71, 1> level = lsqnonlin->getSolution();

    for (int channel = 25; channel <= 96; channel++) {
        if (channel != 71) {
            // Find the value of DAC
            if ((channel % 8) != 0) {
                dac  = (channel / 8);
            } else {
                dac  = channel / 8 - 1;
            }

            // Find the value of Port
            port = channel - (dac * 8) - 1;

            // corrects missing channel
            if (channel < 71) {
                index = channel - 25;
            } else {
                index = channel - 26;
            }

            levelValue = level(index);

            // LED Driver configure: set DAC and Port value
            txBuffer[0] = 0x0C;
            txBuffer[1] = 0x40 |  (levelValue & 0x0000000F);
            txBuffer[2] = 0x80 | ((levelValue & 0x000000F0) >> 4);
            txBuffer[3] = 0x0D;
            txBuffer[4] = 0x40 | ((levelValue & 0x00000F00) >> 8);
            txBuffer[5] = 0x80 | port;
            txBuffer[6] = dac;
            txBuffer[7] = 0x40;
            txBuffer[8] = 0x80;
            txBuffer[9] = '\0';

            if (ledDriver->writeData(txBuffer) == false) {
                QMessageBox::warning(this, tr("Connection Error"),
                                     tr("Spectrometer Hardware not found.\n\n"
                                        "Check USB connection and try again..."));
            }
        }
    }

    // GUI Updates
    ui->dac04channel25->setText(QString::number(level(0)));
    ui->dac04channel26->setText(QString::number(level(1)));
    ui->dac04channel27->setText(QString::number(level(2)));
    ui->dac04channel28->setText(QString::number(level(3)));
    ui->dac04channel29->setText(QString::number(level(4)));
    ui->dac04channel30->setText(QString::number(level(5)));
    ui->dac04channel31->setText(QString::number(level(6)));
    ui->dac04channel32->setText(QString::number(level(7)));
    ui->dac05channel33->setText(QString::number(level(8)));
    ui->dac05channel34->setText(QString::number(level(9)));
    ui->dac05channel35->setText(QString::number(level(10)));
    ui->dac05channel36->setText(QString::number(level(11)));
    ui->dac05channel37->setText(QString::number(level(12)));
    ui->dac05channel38->setText(QString::number(level(13)));
    ui->dac05channel39->setText(QString::number(level(14)));
    ui->dac05channel40->setText(QString::number(level(15)));
    ui->dac06channel41->setText(QString::number(level(16)));
    ui->dac06channel42->setText(QString::number(level(17)));
    ui->dac06channel43->setText(QString::number(level(18)));
    ui->dac06channel44->setText(QString::number(level(19)));
    ui->dac06channel45->setText(QString::number(level(20)));
    ui->dac06channel46->setText(QString::number(level(21)));
    ui->dac06channel47->setText(QString::number(level(22)));
    ui->dac06channel48->setText(QString::number(level(23)));
    ui->dac07channel49->setText(QString::number(level(24)));
    ui->dac07channel50->setText(QString::number(level(25)));
    ui->dac07channel51->setText(QString::number(level(26)));
    ui->dac07channel52->setText(QString::number(level(27)));
    ui->dac07channel53->setText(QString::number(level(28)));
    ui->dac07channel54->setText(QString::number(level(29)));
    ui->dac07channel55->setText(QString::number(level(30)));
    ui->dac07channel56->setText(QString::number(level(31)));
    ui->dac08channel57->setText(QString::number(level(32)));
    ui->dac08channel58->setText(QString::number(level(33)));
    ui->dac08channel59->setText(QString::number(level(34)));
    ui->dac08channel60->setText(QString::number(level(35)));
    ui->dac08channel61->setText(QString::number(level(36)));
    ui->dac08channel62->setText(QString::number(level(37)));
    ui->dac08channel63->setText(QString::number(level(38)));
    ui->dac08channel64->setText(QString::number(level(39)));
    ui->dac09channel65->setText(QString::number(level(40)));
    ui->dac09channel66->setText(QString::number(level(41)));
    ui->dac09channel67->setText(QString::number(level(42)));
    ui->dac09channel68->setText(QString::number(level(43)));
    ui->dac09channel69->setText(QString::number(level(44)));
    ui->dac09channel70->setText(QString::number(level(45)));
//    ui->dac09channel71->setText(QString::number(level(46)));
    ui->dac09channel72->setText(QString::number(level(46)));
    ui->dac10channel73->setText(QString::number(level(47)));
    ui->dac10channel74->setText(QString::number(level(48)));
    ui->dac10channel75->setText(QString::number(level(59)));
    ui->dac10channel76->setText(QString::number(level(50)));
    ui->dac10channel77->setText(QString::number(level(51)));
    ui->dac10channel78->setText(QString::number(level(52)));
    ui->dac10channel79->setText(QString::number(level(53)));
    ui->dac10channel80->setText(QString::number(level(54)));
    ui->dac11channel81->setText(QString::number(level(55)));
    ui->dac11channel82->setText(QString::number(level(56)));
    ui->dac11channel83->setText(QString::number(level(57)));
    ui->dac11channel84->setText(QString::number(level(58)));
    ui->dac11channel85->setText(QString::number(level(59)));
    ui->dac11channel86->setText(QString::number(level(60)));
    ui->dac11channel87->setText(QString::number(level(61)));
    ui->dac11channel88->setText(QString::number(level(62)));
    ui->dac12channel89->setText(QString::number(level(63)));
    ui->dac12channel90->setText(QString::number(level(64)));
    ui->dac12channel91->setText(QString::number(level(65)));
    ui->dac12channel92->setText(QString::number(level(66)));
    ui->dac12channel93->setText(QString::number(level(67)));
    ui->dac12channel94->setText(QString::number(level(68)));
    ui->dac12channel95->setText(QString::number(level(69)));
    ui->dac12channel96->setText(QString::number(level(70)));

    sms500->start();
}

void MainWindow::leastSquareObjectiveFunction()
{    
    double *masterData;
    masterData = sms500->masterData();

    Matrix<double, 641, 1> f;

    for (int i = 0; i < sms500->points(); i++) {
        if (masterData[i] < 0) {
            f(i) = starData[i][1];
        } else {
            f(i) = starData[i][1] - masterData[i];
        }
    }

    lsqnonlin->setObjectiveFunction( f );
}

void MainWindow::leastSquareStop()
{
    lsqnonlin->stop();
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
        statusBar()->showMessage(tr("SMS500 successfully connected"),5000);
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
    statusBar()->showMessage(tr("Geting Spectral Data"), 5000);
    sms500Configure();
    sms500->start();
}

void MainWindow::plotScanResult(const double *masterData, const int *wavelegth, int peakWavelength,
                                double amplitude,int numberOfPoints, int scanNumber, int integrationTimeIndex, bool satured)
{
    // Corrigir isso depois
    amplitude = 1000;

    QPolygonF points;
    QPolygonF starPoints;

    plot->setPlotLimits(300, 1100, 0, amplitude);
    plotLedDriver->setPlotLimits(300, 1100, 0, amplitude);

    for (int i = 0; i < numberOfPoints; i++) {
        if (masterData[i] < 0) {
            points << QPointF(wavelegth[i], 0);
        } else {
            points << QPointF(wavelegth[i], masterData[i]);
        }
    }

    if (starData.size() == 641) {
        for (int i = 0; i <= 640; i++) {
            starPoints << QPointF(starData[i][0], starData[i][1]);
        }
    }

    const bool doReplot = plot->autoReplot();
    plot->setAutoReplot( false );
    plot->showPeak(peakWavelength, amplitude);
    if (starData.size() == 641) {
        plot->showData(points, starPoints, amplitude);
    } else {
        plot->showData(points, amplitude);
    }
    plot->setAutoReplot( doReplot );
    plot->replot();

    // Plot Led Driver
    plotLedDriver->setAutoReplot( false );
    plotLedDriver->showPeak(peakWavelength, amplitude);
    if (starData.size() == 641) {
        plotLedDriver->showData(points, starPoints, amplitude);
    } else {
        plotLedDriver->showData(points, amplitude);
    }
    plotLedDriver->setAutoReplot( doReplot );
    plotLedDriver->replot();

    if (ui->AutoRangeCheckBox->isChecked()) {
        ui->integrationTimeComboBox->setCurrentIndex(integrationTimeIndex);
        sms500->setRange(integrationTimeIndex);
    }

    ui->scanNumberLabel->setText(tr("    Scan number: %1").arg(scanNumber));

    if (satured == true) {
        ui->saturedLabel->show();
        ui->saturedLabel_2->show();
    } else {
        ui->saturedLabel->hide();
        ui->saturedLabel_2->hide();
    }

//    statusBar()->showMessage(tr("Geting Spectral Data"), 5000);
    sms500Configure();
    sms500->enableNextScan();
}

void MainWindow::scanFinished()
{
    ui->btnSaveScan->setEnabled(true);
    ui->btnZoom->setEnabled(true);
    ui->btnStartScan->setIcon(QIcon(":/pics/start.png"));
    ui->btnStartScan->setText("Start Scan");
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

    outFile << "Start wavelength:\t" << sms500->startWavelength() << std::endl;
    outFile << "Stop wavelength:\t" << sms500->stopWavelength() << std::endl;
    outFile << "Dominate wavelength:\t" << sms500->dominanteWavelength() << std::endl;
    outFile << "Peak wavelength:\t" << sms500->peakWavelength() << std::endl;
    outFile << "Power (W):\t\t" << sms500->power() << std::endl;
    outFile << "Integration time (ms):\t" << sms500->integrationTime() << std::endl;
    outFile << "Purity:\t\t\t" << sms500->purity() << std::endl;
    outFile << "FWHM:\t\t\t" << sms500->fwhm() << std::endl;
    outFile << "\nHolds the Wavelength Data:" << std::endl;
    outFile << "nm\tuW/nm" << std::endl;

    double *masterData;
    int    *waveLength;

    masterData = sms500->masterData();
    waveLength = sms500->wavelength();

    for (int i = 0; i < sms500->points(); i++) {
        if (masterData[i] < 0) {
            outFile << waveLength[ i ] << "\t" << "0.0" << std::endl;
        } else {
            outFile << waveLength[ i ] << "\t" << masterData[i] << std::endl;
        }
    }

    statusBar()->showMessage("Log gerado com sucess!", 5000);
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

    if (QMessageBox::question(this, tr("Sytem Zero"),
                              tr("Set the SMS-500 for Dark Current Reading.\n\nThis can take several seconds."),
                              tr("Yes"), tr("No") )) {
        return;
    }

    statusBar()->showMessage(tr("Running System Zero :: Please wait!"));
    sms500Configure();
    sms500->creatDarkRatioArray();

    QMessageBox::information( this, tr("SMS500 Info"), tr("System Zero Completed.") );
}

void MainWindow::calibrateSystem()
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

    if (lampFile.isEmpty()) {
        return;
    }

    if (sms500->readCalibratedLamp(lampFile) == false) {
        statusBar()->showMessage("Error trying to open calibration file", 10000);
        return;
    }

    if (QMessageBox::question(this, tr("Calibration Setup"),
                              tr("Please Setup the SMS500 for data acquisition...\n\n"
                                 "Turn on the calibration lamp and wait approximately 3 minutes\n"
                                 "for the lamp to stabilize.\n\n"
                                 "To continue with calibration,\n"
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

    statusBar()->showMessage("Running calibration :: Please wait!", 5000);

    sms500->finishLampScan();

    QMessageBox::information(this, tr("SMS500 Info"), tr("System calibration completed."));
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
    connect(ledDriver, SIGNAL(warningMessage(QString,QString)),   this, SLOT(warningMessage(QString,QString)));

    connect(ledDriver, SIGNAL(performScan()),        this, SLOT(ledModelingPerformScan()));
    connect(ledDriver, SIGNAL(saveData(QString)),    this, SLOT(ledModelingSaveData(QString)));
    connect(ledDriver, SIGNAL(saveAllData(QString)), this, SLOT(ledModelingSaveChannelData(QString)));
    connect(ledDriver, SIGNAL(modelingFinished()),   this, SLOT(ledModelingFinished()));
    connect(ledDriver, SIGNAL(info(QString)),        this, SLOT(ledModelingInfo(QString)));

    connect(ui->btnConnectDisconnectLED, SIGNAL(clicked()),      this,      SLOT(ledDriverConnectDisconnect()));
    connect(ui->btnLedModeling,          SIGNAL(clicked()),      this,      SLOT(ledModeling()));
    connect(sms500,                      SIGNAL(scanFinished()), ledDriver, SLOT(enabledModelingContinue()));


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
