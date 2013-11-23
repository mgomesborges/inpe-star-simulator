#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
    lsqnonlin                   = new LSqNonLin(this);
    longTermStability           = new LongTermStability(this);
    longTermStabilityAlarmClock = new LongTermStabilityAlarmClock(this);

    plotSMS500->setxLabel(tr("Wavelength (nm)"));
    plotSMS500->setyLabel(tr("uW/nm"));

    plotLSqNonLin->setxLabel(tr("Wavelength"));
    plotLSqNonLin->setyLabel(tr("Irradiance (uW/cm^2 nm)"));

    statusLabel  = new QLabel;
    statusBar()->addPermanentWidget( statusLabel );

    uiInputValidator();
    sms500SignalAndSlot();
    ledDriverSignalAndSlot();
    lsqNonLinSignalAndSlot();
    longTermStabilitySignalAndSlot();
    operationModeChanged();

    // Star Simulator :: Transference Function of Pinhole and Colimator
    if (starLoadTransferenceFunction() == false) {
        if (!QMessageBox::question(this, tr("Star transference function file not found!"),
                                   tr("For the correct generation of the star spectrum, "
                                      "load the transference function file."),
                                   tr("Load"), tr("Cancel") )) {
            if (starUpdateTransferenceFunction() == true) {
                starLoadTransferenceFunction();
            }
        }
    }

    lsqNonLinStarSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
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

        if (this->width() >= 1100) {
            ui->scanInfo->show();
            ui->scanInfo->setGeometry(950,20,230,23);
        }

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
        ui->scanInfo->hide();

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

void MainWindow::showInfo(const QString &text)
{
    if (text == QString::null) {
        if (plotSMS500->plotPicker->rubberBand()) {
            statusBar()->showMessage(tr("Cursor Pos: Press left mouse button in plot region"));
        } else {
            statusBar()->showMessage(tr("Zoom: Press mouse button and drag"));
        }
    }
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
        if (masterData[i] < 0) {
            data.append(tr("%1\t%2\n").arg(waveLength[i]).arg("0.000000000"));
        } else {
            data.append(tr("%1\t%2\n").arg(waveLength[i]).arg(masterData[i]));
        }
    }

    if (filePath.isEmpty()) {
        FileHandle file(this, data, tr("SMS500 - Save Data"));
    } else {
        FileHandle file(data, tr("SMS500 - Save Data"), filePath);
    }
    statusBar()->showMessage("File generated successfully!", 5000);
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
    if (sms500->isNoiseReductionEnabled()) {
        data.append(tr("; Noise Reduction.........: %1\n").arg(sms500->noiseReduction()));
    } else {
        data.append(tr("; Noise Reduction.........: 0\n"));
    }
    if (sms500->isDynamicDarkEnabled()) {
        data.append(tr("; Correct for Dynamic Dark: yes\n"));
    } else {
        data.append(tr("; Correct for Dynamic Dark: no\n"));
    }
    data.append(tr("; Purity..................: %1\n").arg(sms500->purity()));
    data.append(tr("; FWHM....................: %1\n").arg(sms500->fwhm()));

    return data;
}

void MainWindow::warningMessage(const QString &caption, const QString &message)
{
    QMessageBox::warning(this, caption, message);
}

void MainWindow::ledDriverConnectDisconnect()
{
    if (ui->btnConnectDisconnectLED->text().contains(tr("Connect"))) {
        ledDriverConnect();
    } else {
        ledDriverDisconnect();
    }
}

bool MainWindow::ledDriverLoadValuesForChannels()
{
    FileHandle file(this, tr("LED Driver - Load values for channels"));
    QVector< QVector<int> > values = file.data(tr("[ChannelLevel]"));

    if (file.isValidData(72, 2) == false) {
        return false;
    }

    // Update LED Driver GUI
    ledDriverGuiUpdate(Utils::matrix2vector(values, 1));

    // Update LED Driver
    ledDriverDac04Changed();
    ledDriverDac05Changed();
    ledDriverDac06Changed();
    ledDriverDac07Changed();
    ledDriverDac08Changed();
    ledDriverDac09Changed();
    ledDriverDac10Changed();
    ledDriverDac11Changed();
    ledDriverDac12Changed();

    return true;
}

void MainWindow::ledDriverGuiUpdate(QVector<int> level)
{
    ui->dac04channel25->setText(QString::number(level[0]));
    ui->dac04channel26->setText(QString::number(level[1]));
    ui->dac04channel27->setText(QString::number(level[2]));
    ui->dac04channel28->setText(QString::number(level[3]));
    ui->dac04channel29->setText(QString::number(level[4]));
    ui->dac04channel30->setText(QString::number(level[5]));
    ui->dac04channel31->setText(QString::number(level[6]));
    ui->dac04channel32->setText(QString::number(level[7]));
    ui->dac05channel33->setText(QString::number(level[8]));
    ui->dac05channel34->setText(QString::number(level[9]));
    ui->dac05channel35->setText(QString::number(level[10]));
    ui->dac05channel36->setText(QString::number(level[11]));
    ui->dac05channel37->setText(QString::number(level[12]));
    ui->dac05channel38->setText(QString::number(level[13]));
    ui->dac05channel39->setText(QString::number(level[14]));
    ui->dac05channel40->setText(QString::number(level[15]));
    ui->dac06channel41->setText(QString::number(level[16]));
    ui->dac06channel42->setText(QString::number(level[17]));
    ui->dac06channel43->setText(QString::number(level[18]));
    ui->dac06channel44->setText(QString::number(level[19]));
    ui->dac06channel45->setText(QString::number(level[20]));
    ui->dac06channel46->setText(QString::number(level[21]));
    ui->dac06channel47->setText(QString::number(level[22]));
    ui->dac06channel48->setText(QString::number(level[23]));
    ui->dac07channel49->setText(QString::number(level[24]));
    ui->dac07channel50->setText(QString::number(level[25]));
    ui->dac07channel51->setText(QString::number(level[26]));
    ui->dac07channel52->setText(QString::number(level[27]));
    ui->dac07channel53->setText(QString::number(level[28]));
    ui->dac07channel54->setText(QString::number(level[29]));
    ui->dac07channel55->setText(QString::number(level[30]));
    ui->dac07channel56->setText(QString::number(level[31]));
    ui->dac08channel57->setText(QString::number(level[32]));
    ui->dac08channel58->setText(QString::number(level[33]));
    ui->dac08channel59->setText(QString::number(level[34]));
    ui->dac08channel60->setText(QString::number(level[35]));
    ui->dac08channel61->setText(QString::number(level[36]));
    ui->dac08channel62->setText(QString::number(level[37]));
    ui->dac08channel63->setText(QString::number(level[38]));
    ui->dac08channel64->setText(QString::number(level[39]));
    ui->dac09channel65->setText(QString::number(level[40]));
    ui->dac09channel66->setText(QString::number(level[41]));
    ui->dac09channel67->setText(QString::number(level[42]));
    ui->dac09channel68->setText(QString::number(level[43]));
    ui->dac09channel69->setText(QString::number(level[44]));
    ui->dac09channel70->setText(QString::number(level[45]));
    ui->dac09channel71->setText(QString::number(level[46]));
    ui->dac09channel72->setText(QString::number(level[47]));
    ui->dac10channel73->setText(QString::number(level[48]));
    ui->dac10channel74->setText(QString::number(level[49]));
    ui->dac10channel75->setText(QString::number(level[50]));
    ui->dac10channel76->setText(QString::number(level[51]));
    ui->dac10channel77->setText(QString::number(level[52]));
    ui->dac10channel78->setText(QString::number(level[53]));
    ui->dac10channel79->setText(QString::number(level[54]));
    ui->dac10channel80->setText(QString::number(level[55]));
    ui->dac11channel81->setText(QString::number(level[56]));
    ui->dac11channel82->setText(QString::number(level[57]));
    ui->dac11channel83->setText(QString::number(level[58]));
    ui->dac11channel84->setText(QString::number(level[59]));
    ui->dac11channel85->setText(QString::number(level[60]));
    ui->dac11channel86->setText(QString::number(level[61]));
    ui->dac11channel87->setText(QString::number(level[62]));
    ui->dac11channel88->setText(QString::number(level[63]));
    ui->dac12channel89->setText(QString::number(level[64]));
    ui->dac12channel90->setText(QString::number(level[65]));
    ui->dac12channel91->setText(QString::number(level[66]));
    ui->dac12channel92->setText(QString::number(level[67]));
    ui->dac12channel93->setText(QString::number(level[68]));
    ui->dac12channel94->setText(QString::number(level[69]));
    ui->dac12channel95->setText(QString::number(level[70]));
    ui->dac12channel96->setText(QString::number(level[71]));
}

void MainWindow::ledDriverSetV2Ref(bool enable)
{
    ledDriver->setV2Ref(enable);
}

bool MainWindow::ledDriverConnect()
{
    // Prevents communnication errors
    ledDriver->closeConnection();

    if (ledDriver->openConnection() == true) {
        statusBar()->showMessage(tr("LED Driver successfully connected"),5000);
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

        return true;
    }

    QMessageBox::warning(this, tr("LED Driver Error"),
                         tr("LED Driver Open Comunication Error.\n\n"
                            "Check USB connection and try again..."));

    statusBar()->showMessage(tr("LED Driver not connected"));
    ui->btnConnectDisconnectLED->setText(tr("Connect"));
    ui->btnConnectDisconnectLED->setIcon(QIcon(":/pics/reconnect.png"));
    return false;
}

void MainWindow::ledDriverDisconnect()
{
    if (ledDriver->isConnected() == true) {
        ledDriver->closeConnection();
        statusBar()->showMessage(tr("LED Driver not connected"));
        ui->btnConnectDisconnectLED->setText(tr("Connect"));
        ui->btnConnectDisconnectLED->setIcon(QIcon(":/pics/reconnect.png"));
    }
}

void MainWindow::ledDriverDac04Changed()
{
    if (ledDriver->isConnected()) {
        ledDriverValueOfPort[0] = ui->dac04channel25->text().toUInt();
        ledDriverValueOfPort[1] = ui->dac04channel26->text().toUInt();
        ledDriverValueOfPort[2] = ui->dac04channel27->text().toUInt();
        ledDriverValueOfPort[3] = ui->dac04channel28->text().toUInt();
        ledDriverValueOfPort[4] = ui->dac04channel29->text().toUInt();
        ledDriverValueOfPort[5] = ui->dac04channel30->text().toUInt();
        ledDriverValueOfPort[6] = ui->dac04channel31->text().toUInt();
        ledDriverValueOfPort[7] = ui->dac04channel32->text().toUInt();
        ledDriverConfigureDac( 3 );
    }
}

void MainWindow::ledDriverDac05Changed()
{
    if (ledDriver->isConnected()) {
        ledDriverValueOfPort[0] = ui->dac05channel33->text().toUInt();
        ledDriverValueOfPort[1] = ui->dac05channel34->text().toUInt();
        ledDriverValueOfPort[2] = ui->dac05channel35->text().toUInt();
        ledDriverValueOfPort[3] = ui->dac05channel36->text().toUInt();
        ledDriverValueOfPort[4] = ui->dac05channel37->text().toUInt();
        ledDriverValueOfPort[5] = ui->dac05channel38->text().toUInt();
        ledDriverValueOfPort[6] = ui->dac05channel39->text().toUInt();
        ledDriverValueOfPort[7] = ui->dac05channel40->text().toUInt();
        ledDriverConfigureDac( 4 );
    }
}

void MainWindow::ledDriverDac06Changed()
{
    if (ledDriver->isConnected()) {
        ledDriverValueOfPort[0] = ui->dac06channel41->text().toUInt();
        ledDriverValueOfPort[1] = ui->dac06channel42->text().toUInt();
        ledDriverValueOfPort[2] = ui->dac06channel43->text().toUInt();
        ledDriverValueOfPort[3] = ui->dac06channel44->text().toUInt();
        ledDriverValueOfPort[4] = ui->dac06channel45->text().toUInt();
        ledDriverValueOfPort[5] = ui->dac06channel46->text().toUInt();
        ledDriverValueOfPort[6] = ui->dac06channel47->text().toUInt();
        ledDriverValueOfPort[7] = ui->dac06channel48->text().toUInt();

        ledDriverConfigureDac( 5 );
    }
}

void MainWindow::ledDriverDac07Changed()
{
    if (ledDriver->isConnected()) {
        ledDriverValueOfPort[0] = ui->dac07channel49->text().toUInt();
        ledDriverValueOfPort[1] = ui->dac07channel50->text().toUInt();
        ledDriverValueOfPort[2] = ui->dac07channel51->text().toUInt();
        ledDriverValueOfPort[3] = ui->dac07channel52->text().toUInt();
        ledDriverValueOfPort[4] = ui->dac07channel53->text().toUInt();
        ledDriverValueOfPort[5] = ui->dac07channel54->text().toUInt();
        ledDriverValueOfPort[6] = ui->dac07channel55->text().toUInt();
        ledDriverValueOfPort[7] = ui->dac07channel56->text().toUInt();
        ledDriverConfigureDac( 6 );
    }
}

void MainWindow::ledDriverDac08Changed()
{
    if (ledDriver->isConnected()) {
        ledDriverValueOfPort[0] = ui->dac08channel57->text().toUInt();
        ledDriverValueOfPort[1] = ui->dac08channel58->text().toUInt();
        ledDriverValueOfPort[2] = ui->dac08channel59->text().toUInt();
        ledDriverValueOfPort[3] = ui->dac08channel60->text().toUInt();
        ledDriverValueOfPort[4] = ui->dac08channel61->text().toUInt();
        ledDriverValueOfPort[5] = ui->dac08channel62->text().toUInt();
        ledDriverValueOfPort[6] = ui->dac08channel63->text().toUInt();
        ledDriverValueOfPort[7] = ui->dac08channel64->text().toUInt();
        ledDriverConfigureDac( 7 );
    }
}

void MainWindow::ledDriverDac09Changed()
{
    if (ledDriver->isConnected()) {
        ledDriverValueOfPort[0] = ui->dac09channel65->text().toUInt();
        ledDriverValueOfPort[1] = ui->dac09channel66->text().toUInt();
        ledDriverValueOfPort[2] = ui->dac09channel67->text().toUInt();
        ledDriverValueOfPort[3] = ui->dac09channel68->text().toUInt();
        ledDriverValueOfPort[4] = ui->dac09channel69->text().toUInt();
        ledDriverValueOfPort[5] = ui->dac09channel70->text().toUInt();
        ledDriverValueOfPort[6] = ui->dac09channel71->text().toUInt();
        ledDriverValueOfPort[7] = ui->dac09channel72->text().toUInt();
        ledDriverConfigureDac( 8 );
    }
}

void MainWindow::ledDriverDac10Changed()
{
    if (ledDriver->isConnected()) {
        ledDriverValueOfPort[0] = ui->dac10channel73->text().toUInt();
        ledDriverValueOfPort[1] = ui->dac10channel74->text().toUInt();
        ledDriverValueOfPort[2] = ui->dac10channel75->text().toUInt();
        ledDriverValueOfPort[3] = ui->dac10channel76->text().toUInt();
        ledDriverValueOfPort[4] = ui->dac10channel77->text().toUInt();
        ledDriverValueOfPort[5] = ui->dac10channel78->text().toUInt();
        ledDriverValueOfPort[6] = ui->dac10channel79->text().toUInt();
        ledDriverValueOfPort[7] = ui->dac10channel80->text().toUInt();
        ledDriverConfigureDac( 9 );
    }
}

void MainWindow::ledDriverDac11Changed()
{
    if (ledDriver->isConnected()) {
        ledDriverValueOfPort[0] = ui->dac11channel81->text().toUInt();
        ledDriverValueOfPort[1] = ui->dac11channel82->text().toUInt();
        ledDriverValueOfPort[2] = ui->dac11channel83->text().toUInt();
        ledDriverValueOfPort[3] = ui->dac11channel84->text().toUInt();
        ledDriverValueOfPort[4] = ui->dac11channel85->text().toUInt();
        ledDriverValueOfPort[5] = ui->dac11channel86->text().toUInt();
        ledDriverValueOfPort[6] = ui->dac11channel87->text().toUInt();
        ledDriverValueOfPort[7] = ui->dac11channel88->text().toUInt();
        ledDriverConfigureDac( 10 );
    }
}

void MainWindow::ledDriverDac12Changed()
{
    if (ledDriver->isConnected()) {
        ledDriverValueOfPort[0] = ui->dac12channel89->text().toUInt();
        ledDriverValueOfPort[1] = ui->dac12channel90->text().toUInt();
        ledDriverValueOfPort[2] = ui->dac12channel91->text().toUInt();
        ledDriverValueOfPort[3] = ui->dac12channel92->text().toUInt();
        ledDriverValueOfPort[4] = ui->dac12channel93->text().toUInt();
        ledDriverValueOfPort[5] = ui->dac12channel94->text().toUInt();
        ledDriverValueOfPort[6] = ui->dac12channel95->text().toUInt();
        ledDriverValueOfPort[7] = ui->dac12channel96->text().toUInt();
        ledDriverConfigureDac( 11 );
    }
}

void MainWindow::ledDriverConfigureDac(char dac)
{
    for (int port = 0; port < 8; port++) {
        ledDriverTxBuffer[0] = 0x0C;
        ledDriverTxBuffer[1] = 0x40 |  (ledDriverValueOfPort[port] & 0x0000000F);
        ledDriverTxBuffer[2] = 0x80 | ((ledDriverValueOfPort[port] & 0x000000F0) >> 4);
        ledDriverTxBuffer[3] = 0x0D;
        ledDriverTxBuffer[4] = 0x40 | ((ledDriverValueOfPort[port] & 0x00000F00) >> 8);
        ledDriverTxBuffer[5] = 0x80 | port;
        ledDriverTxBuffer[6] =  dac;
        ledDriverTxBuffer[7] = 0x40;
        ledDriverTxBuffer[8] = 0x80;
        ledDriverTxBuffer[9] = '\0';

        if (ledDriver->writeData(ledDriverTxBuffer) == false) {
            statusBar()->showMessage("Transmission Error: check USB connection and try again!", 5000);
            break;
        }
    }
}

QVector<int> MainWindow::ledDriverChannelValues()
{
    QVector<int> channelValue;
    channelValue.append(ui->dac04channel25->text().toInt());
    channelValue.append(ui->dac04channel26->text().toInt());
    channelValue.append(ui->dac04channel27->text().toInt());
    channelValue.append(ui->dac04channel28->text().toInt());
    channelValue.append(ui->dac04channel29->text().toInt());
    channelValue.append(ui->dac04channel30->text().toInt());
    channelValue.append(ui->dac04channel31->text().toInt());
    channelValue.append(ui->dac04channel32->text().toInt());

    channelValue.append(ui->dac05channel33->text().toInt());
    channelValue.append(ui->dac05channel34->text().toInt());
    channelValue.append(ui->dac05channel35->text().toInt());
    channelValue.append(ui->dac05channel36->text().toInt());
    channelValue.append(ui->dac05channel37->text().toInt());
    channelValue.append(ui->dac05channel38->text().toInt());
    channelValue.append(ui->dac05channel39->text().toInt());
    channelValue.append(ui->dac05channel40->text().toInt());

    channelValue.append(ui->dac06channel41->text().toInt());
    channelValue.append(ui->dac06channel42->text().toInt());
    channelValue.append(ui->dac06channel43->text().toInt());
    channelValue.append(ui->dac06channel44->text().toInt());
    channelValue.append(ui->dac06channel45->text().toInt());
    channelValue.append(ui->dac06channel46->text().toInt());
    channelValue.append(ui->dac06channel47->text().toInt());
    channelValue.append(ui->dac06channel48->text().toInt());

    channelValue.append(ui->dac07channel49->text().toInt());
    channelValue.append(ui->dac07channel50->text().toInt());
    channelValue.append(ui->dac07channel51->text().toInt());
    channelValue.append(ui->dac07channel52->text().toInt());
    channelValue.append(ui->dac07channel53->text().toInt());
    channelValue.append(ui->dac07channel54->text().toInt());
    channelValue.append(ui->dac07channel55->text().toInt());
    channelValue.append(ui->dac07channel56->text().toInt());

    channelValue.append(ui->dac08channel57->text().toInt());
    channelValue.append(ui->dac08channel58->text().toInt());
    channelValue.append(ui->dac08channel59->text().toInt());
    channelValue.append(ui->dac08channel60->text().toInt());
    channelValue.append(ui->dac08channel61->text().toInt());
    channelValue.append(ui->dac08channel62->text().toInt());
    channelValue.append(ui->dac08channel63->text().toInt());
    channelValue.append(ui->dac08channel64->text().toInt());

    channelValue.append(ui->dac09channel65->text().toInt());
    channelValue.append(ui->dac09channel66->text().toInt());
    channelValue.append(ui->dac09channel67->text().toInt());
    channelValue.append(ui->dac09channel68->text().toInt());
    channelValue.append(ui->dac09channel69->text().toInt());
    channelValue.append(ui->dac09channel70->text().toInt());
    channelValue.append(ui->dac09channel71->text().toInt());
    channelValue.append(ui->dac09channel72->text().toInt());

    channelValue.append(ui->dac10channel73->text().toInt());
    channelValue.append(ui->dac10channel74->text().toInt());
    channelValue.append(ui->dac10channel75->text().toInt());
    channelValue.append(ui->dac10channel76->text().toInt());
    channelValue.append(ui->dac10channel77->text().toInt());
    channelValue.append(ui->dac10channel78->text().toInt());
    channelValue.append(ui->dac10channel79->text().toInt());
    channelValue.append(ui->dac10channel80->text().toInt());

    channelValue.append(ui->dac11channel81->text().toInt());
    channelValue.append(ui->dac11channel82->text().toInt());
    channelValue.append(ui->dac11channel83->text().toInt());
    channelValue.append(ui->dac11channel84->text().toInt());
    channelValue.append(ui->dac11channel85->text().toInt());
    channelValue.append(ui->dac11channel86->text().toInt());
    channelValue.append(ui->dac11channel87->text().toInt());
    channelValue.append(ui->dac11channel88->text().toInt());

    channelValue.append(ui->dac12channel89->text().toInt());
    channelValue.append(ui->dac12channel90->text().toInt());
    channelValue.append(ui->dac12channel91->text().toInt());
    channelValue.append(ui->dac12channel92->text().toInt());
    channelValue.append(ui->dac12channel93->text().toInt());
    channelValue.append(ui->dac12channel94->text().toInt());
    channelValue.append(ui->dac12channel95->text().toInt());
    channelValue.append(ui->dac12channel96->text().toInt());

    return channelValue;
}

void MainWindow::statusBarMessage(QString message)
{
    statusBar()->showMessage(message);
}

void MainWindow::ledModeling()
{
    if (ui->btnLedModeling->text().contains("LED\nModeling") == true) {
        // Choice directory to save data
        ledModelingFilePath = QFileDialog::getExistingDirectory(this,tr("Choose the directory to save LED Modeling Data"), QDir::homePath());

        if (ledModelingFilePath.isEmpty()) {
            return;
        }

        // Creates directory for LEDs Database
        QDir().mkdir(ledModelingFilePath + "/led_database");

        // SMS500 Configure
        if (sms500->isConnected() == false) {
            if (sms500Connect() == false) {
                return;
            }
        }

        // Set Operation Mode to Flux and number of scans
        ui->rbtnFlux->setChecked(true);
        ui->numberOfScansLineEdit->setText("1");
        sms500Configure();

        // LED Driver Connection
        if (ledDriver->isConnected() == false) {
            if(ledDriverConnect() == false) {
                return;
            }
        }

        // GUI Update :: LED Driver
        ui->ledModelingParametersGoupBox->setEnabled(false);
        ui->dac4GroupBox->setEnabled(false);
        ui->dac5GroupBox->setEnabled(false);
        ui->dac6GroupBox->setEnabled(false);
        ui->dac7GroupBox->setEnabled(false);
        ui->dac8GroupBox->setEnabled(false);
        ui->dac9GroupBox->setEnabled(false);
        ui->dac10GroupBox->setEnabled(false);
        ui->dac11GroupBox->setEnabled(false);
        ui->dac12GroupBox->setEnabled(false);
        ui->btnLoadValuesForChannels->setEnabled(false);
        ui->LEDDriverParametersGroupBox->setEnabled(false);

        // GUI Update :: SMS500
        ui->sms500Tab->setEnabled(false);

        // GUI Update :: Star Simulator
        ui->starSimulatorTab->setEnabled(false);

        // GUI Update :: Long Term Stability
        ui->longTermStabilityTab->setEnabled(false);

        // GUI Update :: Menu
        ui->actionSystemZero->setEnabled(false);
        ui->actionCalibrateSystem->setEnabled(false);
        ui->actionLoadInitialSolution->setEnabled(false);
        ui->actionLoadTransferenceFunction->setEnabled(false);

        // Led Modeling Setup
        ledModelingConfiguration.clear();
        if (ui->levelIncDecComboBox->currentIndex() == 0) {
            // Level Decrement
            ledDriver->setModelingParameters(ui->startChannel->text().toInt(),
                                             ui->endChannel->text().toInt(),
                                             LedDriver::levelDecrement,
                                             ui->levelIncDecValue->text().toInt());

            ledModelingConfiguration.append(tr("%1\t%2")
                                            .arg(LedDriver::levelDecrement)
                                            .arg(ui->levelIncDecValue->text().toInt()));
        } else {
            // Level Increment
            ledDriver->setModelingParameters(ui->startChannel->text().toInt(),
                                             ui->endChannel->text().toInt(),
                                             LedDriver::levelIncrement,
                                             ui->levelIncDecValue->text().toInt());

            ledModelingConfiguration.append(tr("%1\t%2")
                                            .arg(LedDriver::levelIncrement)
                                            .arg(ui->levelIncDecValue->text().toInt()));
        }

        ledDriver->start();

        // Inits LED Modeling Data structure
        ledModelingData.resize( 641 ); // Size = 1000nm - 360nm = 640
        for (int i = 0; i <= 640; i++) {
            ledModelingData[i].resize( 1 );
            ledModelingData[i][0] = i + 360;
        }

        ledModelingScanNumber = 0;
        ui->btnLedModeling->setText("STOP\nModeling");
    } else {
        ledDriver->stop();
        ledDriver->wait(3000);
        ui->btnLedModeling->setText("LED\nModeling");
    }
}

void MainWindow::ledModelingPerformScan()
{
    sms500->start();
}

void MainWindow::ledModelingSaveData(QString fileName)
{
    ledModelingScanNumber++;
    ui->scanNumberLabel->setText(tr("    Scan number: %1").arg(ledModelingScanNumber));
    ui->scanNumberLabel_2->setText(tr("    Scan number: %1").arg(ledModelingScanNumber));

    // Stop Condition = maxMasterData <= 0 and Level Decrement
    if ((sms500->maxMasterData() <= 0) && (ui->levelIncDecComboBox->currentIndex() == 0)) {
        ledDriver->modelingNextChannel();
    }

    QString filePath(ledModelingFilePath + fileName);

    sms500SaveScanData(filePath);

    double *masterData;
    masterData = sms500->masterData();

    // Adds Data in LED Modeling Data
    for (int i = 0; i < sms500->points(); i++) {
        ledModelingData[i].resize(ledModelingScanNumber + 1);

        if (masterData[i] < 0) {
            ledModelingData[i][ledModelingScanNumber] = 0;
        } else {
            ledModelingData[i][ledModelingScanNumber] = masterData[i];
        }
    }

    ledDriver->enabledModelingContinue();
}

void MainWindow::ledModelingSaveChannelData(QString channel)
{
    QString data;
    QString currentTime(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate));

    data.append(tr("; Star Simulator file, Platform: Windows, Created on: %1\n\n").arg(currentTime));
    data.append(sms500MainData());
    data.append(tr("\n[LEDModelingConfiguration]\n"));
    data.append(tr("; #1 [0] Increment\t[1] Decrement\n"));
    data.append(tr("; #2 value\n"));
    data.append(tr("%1\n").arg(ledModelingConfiguration));
    data.append(tr("\n[SMS500Data]\n"));
    data.append(tr("; Holds the Wavelength Data:\n"));
    data.append(tr("; nm\tuW/nm DigitalLevel\n"));

    // Header
    QVector<int> index = ledDriver->digitalLevelIndex();
    data.append(";\t");
    for (int i = 0; i < index.size(); i++) {
        data.append(tr("%1\t").arg(index[i]));
    }
    data.append("\n");

    // Data
    for (int i = 0; i <= 640; i++) {
        for (unsigned int j = 0; j < ledModelingData[0].size(); j++) {
            if (ledModelingData[i][j] <= 0) {
                data.append(tr("%1\t").arg("0"));
            } else {
                data.append(tr("%1\t").arg(ledModelingData[i][j]));
            }
        }
        data.append("\n");
    }

    FileHandle file(data,
                    tr("LED Driver - Save Data"),
                    ledModelingFilePath + QString(tr("/led_database/ch%1.txt").arg(channel)));

    // Inits LED Modeling Data structure
    ledModelingData.erase(ledModelingData.begin(), ledModelingData.end());
    ledModelingData.resize( 641 ); // Size = 1000nm - 360nm = 641
    for (int i = 0; i <= 640; i++) {
        ledModelingData[i].resize( 1 );
        ledModelingData[i][0] = i + 360;
    }

    // Resets ledModelingScanNumber and Scan Info
    ledModelingScanNumber = 0;
    ui->scanInfo->setText(tr("Next Channel... Wait!"));
}

void MainWindow::ledModelingFinished()
{
    ui->btnLedModeling->setText("LED\nModeling");
    QMessageBox::information(this, tr("LED Modeling finished"), tr("Press Ok to continue.\t\t"));

    // GUI Update :: LED Driver
    ui->ledModelingParametersGoupBox->setEnabled(true);
    ui->dac4GroupBox->setEnabled(true);
    ui->dac5GroupBox->setEnabled(true);
    ui->dac6GroupBox->setEnabled(true);
    ui->dac7GroupBox->setEnabled(true);
    ui->dac8GroupBox->setEnabled(true);
    ui->dac9GroupBox->setEnabled(true);
    ui->dac10GroupBox->setEnabled(true);
    ui->dac11GroupBox->setEnabled(true);
    ui->dac12GroupBox->setEnabled(true);
    ui->btnLoadValuesForChannels->setEnabled(true);
    ui->LEDDriverParametersGroupBox->setEnabled(true);
    ui->scanInfo->setText(tr(""));

    // GUI Update :: SMS500
    ui->sms500Tab->setEnabled(true);

    // GUI Update :: Star Simulator
    ui->starSimulatorTab->setEnabled(true);

    // GUI Update :: Long Term Stability
    ui->longTermStabilityTab->setEnabled(true);

    // GUI Update :: Menu
    ui->actionSystemZero->setEnabled(true);
    ui->actionCalibrateSystem->setEnabled(true);
    ui->actionLoadInitialSolution->setEnabled(true);
    ui->actionLoadTransferenceFunction->setEnabled(true);
}

void MainWindow::ledModelingInfo(QString message)
{
    ui->scanInfo->setText(message);
}

void MainWindow::lsqNonLinStartStop()
{
    if (ui->btnStartStopStarSimulator->text().contains("Start\nSimulator")) {
        lsqNonLinStart();
    } else {
        lsqNonLinStop();
    }
}

void MainWindow::lsqNonLinStart()
{
    // Checks connection with SMS500
    if (sms500->isConnected() == false) {
        if (sms500Connect() == false) {
            return;
        }
    }

    // Prevents errors with SMS500
    if (sms500->isRunning()) {
        sms500->stop();
        sms500->wait();
    }

    // Checks connection with LED Driver
    if (ledDriver->isConnected() == false) {
        if (ledDriverConnect() == false) {
            return;
        }
    }

    // GUI Update :: SMS500
    ui->rbtnFlux->setChecked(true);
    ui->btnStartScan->setEnabled(false);
    ui->btnSaveScan->setEnabled(false);
    ui->numberOfScansLineEdit->setText("1");
    ui->numberOfScansLineEdit->setEnabled(false);
    ui->AutoRangeCheckBox->setChecked(true);
    ui->sms500WavelengthGroupBox->setEnabled(false);
//    ui->samplesToAverageSpinBox->setValue(20);
    ui->smoothingSpinBox->setValue(1);
    ui->dynamicDarkCheckBox->setChecked(true);
    ui->modesOfOperation->setEnabled(true);

    // GUI Update :: LED Driver
    ui->ledDriverTab->setEnabled(false);

    // GUI Update :: Star Simulator
    ui->btnStartStopStarSimulator->setIcon(QIcon(":/pics/stop.png"));
    ui->btnStartStopStarSimulator->setText("Stop\nSimulator ");
    ui->btnSaveStarSimulatorData->setEnabled(false);
    ui->btnLoadInitialSolution->setEnabled(false);
    ui->starSettingsGroupBox->setEnabled(false);
    ui->fitingAlgorithm->setEnabled(false);
    ui->x0GroupBox->setEnabled(false);

    // GUI Update :: Long Term Stability
    ui->longTermStabilityTab->setEnabled(false);

    // GUI Update :: Menu
    ui->actionSystemZero->setEnabled(false);
    ui->actionCalibrateSystem->setEnabled(false);
    ui->actionLoadInitialSolution->setEnabled(false);
    ui->actionLoadTransferenceFunction->setEnabled(false);

    // Configures SMS500
    sms500Configure();

    // x0 type
    if (ui->x0Random->isChecked()) {
        lsqnonlin->setx0Type(LSqNonLin::x0Random);
    } else if (ui->x0UserDefined->isChecked()) {
        lsqnonlin->setx0Type(LSqNonLin::x0UserDefined, lsqNonLinx0());
    } else if (ui->x0DefinedInLedDriver->isChecked()) {
        MatrixXi matrix(72, 1);
        QVector<int> qvector = ledDriverChannelValues();

        for (int i = 0; i < qvector.size(); i++) {
            matrix(i) = qvector[i];
        }

        lsqnonlin->setx0Type(LSqNonLin::x0Current, matrix);
    }

    // Transference Function of Pinhole and Colimator
    if (starLoadTransferenceFunction() == false) {
        if (!QMessageBox::question(this, tr("Star transference function file not found!"),
                                   tr("For the correct generation of the star spectrum, "
                                      "load the transference function file."),
                                   tr("Load"), tr("Cancel") )) {
            if (starUpdateTransferenceFunction() == true) {
                starLoadTransferenceFunction();
            }
        }
    }

    if (ui->levenbergMarquardt->isChecked()) {
        lsqnonlin->setAlgorithm(LSqNonLin::leastSquareNonLinear);
    } else {
        lsqnonlin->setAlgorithm(LSqNonLin::gradientDescent);
    }

    lsqnonlin->start();
    lsqNonLinTime.start();

    lsqNonLinLog(tr("==================== Star Simulator Start ====================="
                    "\n%1\n"
                    "======================================================")
                 .arg(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate)));
}

void MainWindow::lsqNonLinStop()
{
    ui->btnStartStopStarSimulator->setIcon(QIcon(":/pics/start.png"));
    ui->btnStartStopStarSimulator->setText("Start\nSimulator");
    lsqnonlin->stop();
}

void MainWindow::lsqNonLinFinished()
{
    // GUI Update :: Star Simulator
    ui->btnStartStopStarSimulator->setIcon(QIcon(":/pics/start.png"));
    ui->btnStartStopStarSimulator->setText("Start\nSimulator");
    ui->btnSaveStarSimulatorData->setEnabled(true);
    ui->btnLoadInitialSolution->setEnabled(true);
    ui->starSettingsGroupBox->setEnabled(true);
    ui->fitingAlgorithm->setEnabled(true);
    ui->x0GroupBox->setEnabled(true);
    ui->x0DefinedInLedDriver->setChecked(true);

    // GUI Update :: SMS500
    ui->btnStartScan->setEnabled(true);
    ui->btnSaveScan->setEnabled(true);
    ui->numberOfScansLineEdit->setText("-1");
    ui->numberOfScansLineEdit->setEnabled(true);
    ui->AutoRangeCheckBox->setEnabled(true);
    ui->sms500WavelengthGroupBox->setEnabled(true);
    ui->modesOfOperation->setEnabled(true);

    // GUI Update :: LED Driver
    ui->ledDriverTab->setEnabled(true);

    // GUI Update :: Long Term Stability
    ui->longTermStabilityTab->setEnabled(true);

    // GUI Update :: Menu
    ui->actionSystemZero->setEnabled(true);
    ui->actionCalibrateSystem->setEnabled(true);
    ui->actionLoadInitialSolution->setEnabled(true);
    ui->actionLoadTransferenceFunction->setEnabled(true);

    lsqNonLinLog(tr("\n================== Star Simulator Finished ==================="
                    "\n%1\tElapsed time: %2 seconds\n"
                    "====================================================\n")
                 .arg(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate))
                 .arg((lsqNonLinTime.elapsed())));
}

void MainWindow::lsqNonLinStarSettings()
{
    lsqNonLinStar->setMagnitude(ui->starMagnitude->text().toInt());
    lsqNonLinStar->setTemperature(ui->starTemperature->text().toInt());

    plotLSqNonLin->setPlotLimits(350, 1000, 0, lsqNonLinStar->peak() * 1.2);
    plotLSqNonLin->showData(lsqNonLinStar->spectralDataToPlot(), lsqNonLinStar->peak() * 1.2, 1);
    plotLSqNonLin->showData(lsqNonLinStar->spectralDataToPlot(), lsqNonLinStar->peak() * 1.2, 0);
}

void MainWindow::lsqNonLinPerformScan()
{
    plotLSqNonLin->setPlotLimits(350, 1000, 0, lsqNonLinStar->peak() * 1.2);
    plotLSqNonLin->showData(lsqNonLinStar->spectralDataToPlot(), lsqNonLinStar->peak() * 1.2, 1);

    int dac;
    int port;
    int levelValue;
    char ledDriverTxBuffer[10];

    MatrixXi level = lsqnonlin->getSolution();

    for (int channel = 25; channel <= 96; channel++) {
        // Find the value of DAC
        if ((channel % 8) != 0) {
            dac  = (channel / 8);
        } else {
            dac  = channel / 8 - 1;
        }

        // Find the value of Port
        port       = channel - (dac * 8) - 1;
        levelValue = level(channel - 25);

        // LED Driver configure: set DAC and Port value
        ledDriverTxBuffer[0] = 0x0C;
        ledDriverTxBuffer[1] = 0x40 |  (levelValue & 0x0000000F);
        ledDriverTxBuffer[2] = 0x80 | ((levelValue & 0x000000F0) >> 4);
        ledDriverTxBuffer[3] = 0x0D;
        ledDriverTxBuffer[4] = 0x40 | ((levelValue & 0x00000F00) >> 8);
        ledDriverTxBuffer[5] = 0x80 | port;
        ledDriverTxBuffer[6] = dac;
        ledDriverTxBuffer[7] = 0x40;
        ledDriverTxBuffer[8] = 0x80;
        ledDriverTxBuffer[9] = '\0';

        if (ledDriver->writeData(ledDriverTxBuffer) == false) {
            QMessageBox::warning(this, tr("Connection Error"),
                                 tr("Spectrometer Hardware not found.\n\n"
                                    "Check USB connection and try again..."));
        }
    }

    // GUI Updates
    ledDriverGuiUpdate(Utils::eigen2QVector(level));

    sms500->start();
}

void MainWindow::lsqNonLinObjectiveFunction()
{
    MatrixXd f(641, 1);
    double *masterData;
    QVector< QVector<double> > starData;
    double convergenceFactor;

    masterData = sms500->masterData();
    starData   = lsqNonLinStar->spectralData();

    // Due to very small values ​​of the objective function was necessary to add this multiplier
    convergenceFactor = 1e11;

    for (int i = 0; i < sms500->points(); i++) {
        if (masterData[i] < 0) {
            f(i) = - starData[i][1];
        } else {
            f(i) = (masterData[i] * transferenceFunction[i]) - (starData[i][1]);
        }

        f(i) *= convergenceFactor;
    }

    lsqnonlin->setObjectiveFunction( f );
}

void MainWindow::lsqNonLinLoadLedData()
{
    LSqLoadLedDataDialog loadLedData;
    loadLedData.exec();
}

void MainWindow::lsqNonLinx0Handle()
{
    if (ui->x0UserDefined->isChecked()) {
        ui->initialSolution_ch25->setEnabled(true);
        ui->initialSolution_ch26->setEnabled(true);
        ui->initialSolution_ch27->setEnabled(true);
        ui->initialSolution_ch28->setEnabled(true);
        ui->initialSolution_ch29->setEnabled(true);
        ui->initialSolution_ch30->setEnabled(true);
        ui->initialSolution_ch31->setEnabled(true);
        ui->initialSolution_ch32->setEnabled(true);
        ui->initialSolution_ch33->setEnabled(true);
        ui->initialSolution_ch34->setEnabled(true);
        ui->initialSolution_ch35->setEnabled(true);
        ui->initialSolution_ch36->setEnabled(true);
        ui->initialSolution_ch37->setEnabled(true);
        ui->initialSolution_ch38->setEnabled(true);
        ui->initialSolution_ch39->setEnabled(true);
        ui->initialSolution_ch40->setEnabled(true);
        ui->initialSolution_ch41->setEnabled(true);
        ui->initialSolution_ch42->setEnabled(true);
        ui->initialSolution_ch43->setEnabled(true);
        ui->initialSolution_ch44->setEnabled(true);
        ui->initialSolution_ch45->setEnabled(true);
        ui->initialSolution_ch46->setEnabled(true);
        ui->initialSolution_ch47->setEnabled(true);
        ui->initialSolution_ch48->setEnabled(true);
        ui->initialSolution_ch49->setEnabled(true);
        ui->initialSolution_ch50->setEnabled(true);
        ui->initialSolution_ch51->setEnabled(true);
        ui->initialSolution_ch52->setEnabled(true);
        ui->initialSolution_ch53->setEnabled(true);
        ui->initialSolution_ch54->setEnabled(true);
        ui->initialSolution_ch55->setEnabled(true);
        ui->initialSolution_ch56->setEnabled(true);
        ui->initialSolution_ch57->setEnabled(true);
        ui->initialSolution_ch58->setEnabled(true);
        ui->initialSolution_ch59->setEnabled(true);
        ui->initialSolution_ch60->setEnabled(true);
        ui->initialSolution_ch61->setEnabled(true);
        ui->initialSolution_ch62->setEnabled(true);
        ui->initialSolution_ch63->setEnabled(true);
        ui->initialSolution_ch64->setEnabled(true);
        ui->initialSolution_ch65->setEnabled(true);
        ui->initialSolution_ch66->setEnabled(true);
        ui->initialSolution_ch67->setEnabled(true);
        ui->initialSolution_ch68->setEnabled(true);
        ui->initialSolution_ch69->setEnabled(true);
        ui->initialSolution_ch70->setEnabled(true);
        ui->initialSolution_ch71->setEnabled(true);
        ui->initialSolution_ch72->setEnabled(true);
        ui->initialSolution_ch73->setEnabled(true);
        ui->initialSolution_ch74->setEnabled(true);
        ui->initialSolution_ch75->setEnabled(true);
        ui->initialSolution_ch76->setEnabled(true);
        ui->initialSolution_ch77->setEnabled(true);
        ui->initialSolution_ch78->setEnabled(true);
        ui->initialSolution_ch79->setEnabled(true);
        ui->initialSolution_ch80->setEnabled(true);
        ui->initialSolution_ch81->setEnabled(true);
        ui->initialSolution_ch82->setEnabled(true);
        ui->initialSolution_ch83->setEnabled(true);
        ui->initialSolution_ch84->setEnabled(true);
        ui->initialSolution_ch85->setEnabled(true);
        ui->initialSolution_ch86->setEnabled(true);
        ui->initialSolution_ch87->setEnabled(true);
        ui->initialSolution_ch88->setEnabled(true);
        ui->initialSolution_ch89->setEnabled(true);
        ui->initialSolution_ch90->setEnabled(true);
        ui->initialSolution_ch91->setEnabled(true);
        ui->initialSolution_ch92->setEnabled(true);
        ui->initialSolution_ch93->setEnabled(true);
        ui->initialSolution_ch94->setEnabled(true);
        ui->initialSolution_ch95->setEnabled(true);
        ui->initialSolution_ch96->setEnabled(true);
    } else {
        ui->initialSolution_ch25->setEnabled(false);
        ui->initialSolution_ch26->setEnabled(false);
        ui->initialSolution_ch27->setEnabled(false);
        ui->initialSolution_ch28->setEnabled(false);
        ui->initialSolution_ch29->setEnabled(false);
        ui->initialSolution_ch30->setEnabled(false);
        ui->initialSolution_ch31->setEnabled(false);
        ui->initialSolution_ch32->setEnabled(false);
        ui->initialSolution_ch33->setEnabled(false);
        ui->initialSolution_ch34->setEnabled(false);
        ui->initialSolution_ch35->setEnabled(false);
        ui->initialSolution_ch36->setEnabled(false);
        ui->initialSolution_ch37->setEnabled(false);
        ui->initialSolution_ch38->setEnabled(false);
        ui->initialSolution_ch39->setEnabled(false);
        ui->initialSolution_ch40->setEnabled(false);
        ui->initialSolution_ch41->setEnabled(false);
        ui->initialSolution_ch42->setEnabled(false);
        ui->initialSolution_ch43->setEnabled(false);
        ui->initialSolution_ch44->setEnabled(false);
        ui->initialSolution_ch45->setEnabled(false);
        ui->initialSolution_ch46->setEnabled(false);
        ui->initialSolution_ch47->setEnabled(false);
        ui->initialSolution_ch48->setEnabled(false);
        ui->initialSolution_ch49->setEnabled(false);
        ui->initialSolution_ch50->setEnabled(false);
        ui->initialSolution_ch51->setEnabled(false);
        ui->initialSolution_ch52->setEnabled(false);
        ui->initialSolution_ch53->setEnabled(false);
        ui->initialSolution_ch54->setEnabled(false);
        ui->initialSolution_ch55->setEnabled(false);
        ui->initialSolution_ch56->setEnabled(false);
        ui->initialSolution_ch57->setEnabled(false);
        ui->initialSolution_ch58->setEnabled(false);
        ui->initialSolution_ch59->setEnabled(false);
        ui->initialSolution_ch60->setEnabled(false);
        ui->initialSolution_ch61->setEnabled(false);
        ui->initialSolution_ch62->setEnabled(false);
        ui->initialSolution_ch63->setEnabled(false);
        ui->initialSolution_ch64->setEnabled(false);
        ui->initialSolution_ch65->setEnabled(false);
        ui->initialSolution_ch66->setEnabled(false);
        ui->initialSolution_ch67->setEnabled(false);
        ui->initialSolution_ch68->setEnabled(false);
        ui->initialSolution_ch69->setEnabled(false);
        ui->initialSolution_ch70->setEnabled(false);
        ui->initialSolution_ch71->setEnabled(false);
        ui->initialSolution_ch72->setEnabled(false);
        ui->initialSolution_ch73->setEnabled(false);
        ui->initialSolution_ch74->setEnabled(false);
        ui->initialSolution_ch75->setEnabled(false);
        ui->initialSolution_ch76->setEnabled(false);
        ui->initialSolution_ch77->setEnabled(false);
        ui->initialSolution_ch78->setEnabled(false);
        ui->initialSolution_ch79->setEnabled(false);
        ui->initialSolution_ch80->setEnabled(false);
        ui->initialSolution_ch81->setEnabled(false);
        ui->initialSolution_ch82->setEnabled(false);
        ui->initialSolution_ch83->setEnabled(false);
        ui->initialSolution_ch84->setEnabled(false);
        ui->initialSolution_ch85->setEnabled(false);
        ui->initialSolution_ch86->setEnabled(false);
        ui->initialSolution_ch87->setEnabled(false);
        ui->initialSolution_ch88->setEnabled(false);
        ui->initialSolution_ch89->setEnabled(false);
        ui->initialSolution_ch90->setEnabled(false);
        ui->initialSolution_ch91->setEnabled(false);
        ui->initialSolution_ch92->setEnabled(false);
        ui->initialSolution_ch93->setEnabled(false);
        ui->initialSolution_ch94->setEnabled(false);
        ui->initialSolution_ch95->setEnabled(false);
        ui->initialSolution_ch96->setEnabled(false);
    }
}

void MainWindow::lsqNonLinLog(QString info)
{
    ui->starSimulatorLog->appendPlainText(info);
}

void MainWindow::lsqNonLinSaveData()
{
    QString data;
    QString currentTime(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate));

    data.append(tr("; Star Simulator file, Platform: Windows, Created on: %1\n\n").arg(currentTime));
    data.append(sms500MainData());
    data.append(tr("; Output irradiance.......: %1\n").arg(outputIrradiance));
    data.append(tr("; Star irradiance.........: %1\n\n").arg(starIrradiance));
    data.append(tr("[ChannelLevel]\n"));

    QVector<int> channelValues = ledDriverChannelValues();
    for (int i = 0; i < channelValues.size(); i++) {
        data.append(tr("%1\t%2\n").arg(i + 25).arg(channelValues[i]));
    }

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
        if (masterData[i] < 0) {
            data.append(tr("%1\t%2\t%3\n")
                       .arg(waveLength[i])
                       .arg("0.000000000")
                       .arg(starData[i][1]));
        } else {
            data.append(tr("%1\t%2\t%3\n")
                       .arg(waveLength[i])
                       .arg(masterData[i] * transferenceFunction[i])
                       .arg(starData[i][1]));
        }
    }

    FileHandle file(this, data, tr("Star Simulator - Save Data"));
    statusBar()->showMessage("File generated successfully!", 5000);
}

bool MainWindow::lsqNonLinLoadInitialSolution()
{
    FileHandle file(this, tr("Star Simulator - Initial Solution"));
    QVector< QVector<int> > initialSolution = file.data(tr("[ChannelLevel]"));

    if (file.isValidData(72, 2) == false) {
        return false;
    }

    // GUI update
    lsqNonLinInitialSolutionGuiUpdate(Utils::matrix2vector(initialSolution, 1));
    ui->x0UserDefined->setChecked(true);

    return true;
}

bool MainWindow::starLoadTransferenceFunction()
{
    transferenceFunction.resize(641);

    // Opens input data in txt file
    QString filePath = QDir::currentPath() + "/transferenceFunction.txt";
    QFile file(filePath);
    if (!file.exists()) {
        for (int i = 0; i < 641; i++) {
            transferenceFunction[i] = 1;
        }
        return false;
    }
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);
    QString line;
    QStringList fields;

    in.seek(0); // Reposition to beginning of file

    for (int i = 0; i < 641; i++) {
        line   = in.readLine();
        fields = line.split("\t");
        transferenceFunction[i] = fields.at(1).toDouble();
    }
    file.close();

    return true;
}

bool MainWindow::starUpdateTransferenceFunction()
{
    QString filePath = QFileDialog::getOpenFileName(this);

    if (filePath.isEmpty()) {
        return false;
    }

    QFile inFile(filePath);
    inFile.open(QIODevice::ReadOnly);
    QTextStream in(&inFile);

    // Saves output data
    QString outFilePath = QDir::currentPath() + "/transferenceFunction.txt";
    QFile outFile(outFilePath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        statusBar()->showMessage(tr("Transference function: an error occurred while loading file."));
        return false;
    }
    QTextStream out(&outFile);
    out << in.readAll();

    outFile.close();
    inFile.close();

    statusBar()->showMessage(tr("Transference function successfully loaded"));
    return true;
}

void MainWindow::lsqNonLinInitialSolutionGuiUpdate(QVector<int> initialSolution)
{
    ui->initialSolution_ch25->setText(QString::number(initialSolution[0]));
    ui->initialSolution_ch26->setText(QString::number(initialSolution[1]));
    ui->initialSolution_ch27->setText(QString::number(initialSolution[2]));
    ui->initialSolution_ch28->setText(QString::number(initialSolution[3]));
    ui->initialSolution_ch29->setText(QString::number(initialSolution[4]));
    ui->initialSolution_ch30->setText(QString::number(initialSolution[5]));
    ui->initialSolution_ch31->setText(QString::number(initialSolution[6]));
    ui->initialSolution_ch32->setText(QString::number(initialSolution[7]));
    ui->initialSolution_ch33->setText(QString::number(initialSolution[8]));
    ui->initialSolution_ch34->setText(QString::number(initialSolution[9]));
    ui->initialSolution_ch35->setText(QString::number(initialSolution[10]));
    ui->initialSolution_ch36->setText(QString::number(initialSolution[11]));
    ui->initialSolution_ch37->setText(QString::number(initialSolution[12]));
    ui->initialSolution_ch38->setText(QString::number(initialSolution[13]));
    ui->initialSolution_ch39->setText(QString::number(initialSolution[14]));
    ui->initialSolution_ch40->setText(QString::number(initialSolution[15]));
    ui->initialSolution_ch41->setText(QString::number(initialSolution[16]));
    ui->initialSolution_ch42->setText(QString::number(initialSolution[17]));
    ui->initialSolution_ch43->setText(QString::number(initialSolution[18]));
    ui->initialSolution_ch44->setText(QString::number(initialSolution[19]));
    ui->initialSolution_ch45->setText(QString::number(initialSolution[20]));
    ui->initialSolution_ch46->setText(QString::number(initialSolution[21]));
    ui->initialSolution_ch47->setText(QString::number(initialSolution[22]));
    ui->initialSolution_ch48->setText(QString::number(initialSolution[23]));
    ui->initialSolution_ch49->setText(QString::number(initialSolution[24]));
    ui->initialSolution_ch50->setText(QString::number(initialSolution[25]));
    ui->initialSolution_ch51->setText(QString::number(initialSolution[26]));
    ui->initialSolution_ch52->setText(QString::number(initialSolution[27]));
    ui->initialSolution_ch53->setText(QString::number(initialSolution[28]));
    ui->initialSolution_ch54->setText(QString::number(initialSolution[29]));
    ui->initialSolution_ch55->setText(QString::number(initialSolution[30]));
    ui->initialSolution_ch56->setText(QString::number(initialSolution[31]));
    ui->initialSolution_ch57->setText(QString::number(initialSolution[32]));
    ui->initialSolution_ch58->setText(QString::number(initialSolution[33]));
    ui->initialSolution_ch59->setText(QString::number(initialSolution[34]));
    ui->initialSolution_ch60->setText(QString::number(initialSolution[35]));
    ui->initialSolution_ch61->setText(QString::number(initialSolution[36]));
    ui->initialSolution_ch62->setText(QString::number(initialSolution[37]));
    ui->initialSolution_ch63->setText(QString::number(initialSolution[38]));
    ui->initialSolution_ch64->setText(QString::number(initialSolution[39]));
    ui->initialSolution_ch65->setText(QString::number(initialSolution[40]));
    ui->initialSolution_ch66->setText(QString::number(initialSolution[41]));
    ui->initialSolution_ch67->setText(QString::number(initialSolution[42]));
    ui->initialSolution_ch68->setText(QString::number(initialSolution[43]));
    ui->initialSolution_ch69->setText(QString::number(initialSolution[44]));
    ui->initialSolution_ch70->setText(QString::number(initialSolution[45]));
    ui->initialSolution_ch71->setText(QString::number(initialSolution[46]));
    ui->initialSolution_ch72->setText(QString::number(initialSolution[47]));
    ui->initialSolution_ch73->setText(QString::number(initialSolution[48]));
    ui->initialSolution_ch74->setText(QString::number(initialSolution[49]));
    ui->initialSolution_ch75->setText(QString::number(initialSolution[50]));
    ui->initialSolution_ch76->setText(QString::number(initialSolution[51]));
    ui->initialSolution_ch77->setText(QString::number(initialSolution[52]));
    ui->initialSolution_ch78->setText(QString::number(initialSolution[53]));
    ui->initialSolution_ch79->setText(QString::number(initialSolution[54]));
    ui->initialSolution_ch80->setText(QString::number(initialSolution[55]));
    ui->initialSolution_ch81->setText(QString::number(initialSolution[56]));
    ui->initialSolution_ch82->setText(QString::number(initialSolution[57]));
    ui->initialSolution_ch83->setText(QString::number(initialSolution[58]));
    ui->initialSolution_ch84->setText(QString::number(initialSolution[59]));
    ui->initialSolution_ch85->setText(QString::number(initialSolution[60]));
    ui->initialSolution_ch86->setText(QString::number(initialSolution[61]));
    ui->initialSolution_ch87->setText(QString::number(initialSolution[62]));
    ui->initialSolution_ch88->setText(QString::number(initialSolution[63]));
    ui->initialSolution_ch89->setText(QString::number(initialSolution[64]));
    ui->initialSolution_ch90->setText(QString::number(initialSolution[65]));
    ui->initialSolution_ch91->setText(QString::number(initialSolution[66]));
    ui->initialSolution_ch92->setText(QString::number(initialSolution[67]));
    ui->initialSolution_ch93->setText(QString::number(initialSolution[68]));
    ui->initialSolution_ch94->setText(QString::number(initialSolution[69]));
    ui->initialSolution_ch95->setText(QString::number(initialSolution[70]));
    ui->initialSolution_ch96->setText(QString::number(initialSolution[71]));
}

MatrixXi MainWindow::lsqNonLinx0()
{
    MatrixXi matrix(72, 1);

    matrix(0)  = ui->initialSolution_ch25->text().toInt();
    matrix(1)  = ui->initialSolution_ch26->text().toInt();
    matrix(2)  = ui->initialSolution_ch27->text().toInt();
    matrix(3)  = ui->initialSolution_ch28->text().toInt();
    matrix(4)  = ui->initialSolution_ch29->text().toInt();
    matrix(5)  = ui->initialSolution_ch30->text().toInt();
    matrix(6)  = ui->initialSolution_ch31->text().toInt();
    matrix(7)  = ui->initialSolution_ch32->text().toInt();
    matrix(8)  = ui->initialSolution_ch33->text().toInt();
    matrix(9)  = ui->initialSolution_ch34->text().toInt();
    matrix(10) = ui->initialSolution_ch35->text().toInt();
    matrix(11) = ui->initialSolution_ch36->text().toInt();
    matrix(12) = ui->initialSolution_ch37->text().toInt();
    matrix(13) = ui->initialSolution_ch38->text().toInt();
    matrix(14) = ui->initialSolution_ch39->text().toInt();
    matrix(15) = ui->initialSolution_ch40->text().toInt();
    matrix(16) = ui->initialSolution_ch41->text().toInt();
    matrix(17) = ui->initialSolution_ch42->text().toInt();
    matrix(18) = ui->initialSolution_ch43->text().toInt();
    matrix(19) = ui->initialSolution_ch44->text().toInt();
    matrix(20) = ui->initialSolution_ch45->text().toInt();
    matrix(21) = ui->initialSolution_ch46->text().toInt();
    matrix(22) = ui->initialSolution_ch47->text().toInt();
    matrix(23) = ui->initialSolution_ch48->text().toInt();
    matrix(24) = ui->initialSolution_ch49->text().toInt();
    matrix(25) = ui->initialSolution_ch50->text().toInt();
    matrix(26) = ui->initialSolution_ch51->text().toInt();
    matrix(27) = ui->initialSolution_ch52->text().toInt();
    matrix(28) = ui->initialSolution_ch53->text().toInt();
    matrix(29) = ui->initialSolution_ch54->text().toInt();
    matrix(30) = ui->initialSolution_ch55->text().toInt();
    matrix(31) = ui->initialSolution_ch56->text().toInt();
    matrix(32) = ui->initialSolution_ch57->text().toInt();
    matrix(33) = ui->initialSolution_ch58->text().toInt();
    matrix(34) = ui->initialSolution_ch59->text().toInt();
    matrix(35) = ui->initialSolution_ch60->text().toInt();
    matrix(36) = ui->initialSolution_ch61->text().toInt();
    matrix(37) = ui->initialSolution_ch62->text().toInt();
    matrix(38) = ui->initialSolution_ch63->text().toInt();
    matrix(39) = ui->initialSolution_ch64->text().toInt();
    matrix(40) = ui->initialSolution_ch65->text().toInt();
    matrix(41) = ui->initialSolution_ch66->text().toInt();
    matrix(42) = ui->initialSolution_ch67->text().toInt();
    matrix(43) = ui->initialSolution_ch68->text().toInt();
    matrix(44) = ui->initialSolution_ch69->text().toInt();
    matrix(45) = ui->initialSolution_ch70->text().toInt();
    matrix(46) = ui->initialSolution_ch71->text().toInt();
    matrix(47) = ui->initialSolution_ch72->text().toInt();
    matrix(48) = ui->initialSolution_ch73->text().toInt();
    matrix(49) = ui->initialSolution_ch74->text().toInt();
    matrix(50) = ui->initialSolution_ch75->text().toInt();
    matrix(51) = ui->initialSolution_ch76->text().toInt();
    matrix(52) = ui->initialSolution_ch77->text().toInt();
    matrix(53) = ui->initialSolution_ch78->text().toInt();
    matrix(54) = ui->initialSolution_ch79->text().toInt();
    matrix(55) = ui->initialSolution_ch80->text().toInt();
    matrix(56) = ui->initialSolution_ch81->text().toInt();
    matrix(57) = ui->initialSolution_ch82->text().toInt();
    matrix(58) = ui->initialSolution_ch83->text().toInt();
    matrix(59) = ui->initialSolution_ch84->text().toInt();
    matrix(60) = ui->initialSolution_ch85->text().toInt();
    matrix(61) = ui->initialSolution_ch86->text().toInt();
    matrix(62) = ui->initialSolution_ch87->text().toInt();
    matrix(63) = ui->initialSolution_ch88->text().toInt();
    matrix(64) = ui->initialSolution_ch89->text().toInt();
    matrix(65) = ui->initialSolution_ch90->text().toInt();
    matrix(66) = ui->initialSolution_ch91->text().toInt();
    matrix(67) = ui->initialSolution_ch92->text().toInt();
    matrix(68) = ui->initialSolution_ch93->text().toInt();
    matrix(69) = ui->initialSolution_ch94->text().toInt();
    matrix(70) = ui->initialSolution_ch95->text().toInt();
    matrix(71) = ui->initialSolution_ch96->text().toInt();

    return matrix;
}

void MainWindow::longTermStabilityCreateDB()
{
    QString filePath = QFileDialog::getSaveFileName(
                           this,
                           tr("Long Term Stability :: Create Database"),
                           QDir::homePath(),
                           tr("Star Simulator DB *.db"));

    if (filePath.isEmpty()) {
        return;
    }

    // Checks if exist extension '.db'
    if (filePath.contains(".db") == false) {
        filePath.append(".db");
    }

    // Prevents errors
    if (QFile::exists(filePath)) {
        QFile::remove(filePath);
    }

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
                           QDir::currentPath(),
                           tr("Star Simulator Database (*.db);;All files (*.*)"));

    if (filePath.isNull()) {
        return;
    }

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
    if (ui->btnStartStopLongTermStability->text().contains("Start Timer")) {
        longTermStabilityStart();
    } else {
        longTermStabilityStop();
    }
}

void MainWindow::longTermStabilityStart()
{
    int hour = ui->longTermStabilityHour->text().toInt();
    int min  = ui->longTermStabilityMin->text().toInt();
    int sec  = ui->longTermStabilitySec->text().toInt();
    int secTimeInterval;

    if (ui->timeIntervalTypeComboBox->currentIndex() == 0) {
        secTimeInterval = ui->longTermStabilityTimeInterval->text().toInt();
    } else {
        secTimeInterval = ui->longTermStabilityTimeInterval->text().toInt() * 60;
    }

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
    sms500StopScan();
    ui->numberOfScansLineEdit->setText("1");

    ui->btnStartStopLongTermStability->setIcon(QIcon(":/pics/stop.png"));
    ui->btnStartStopLongTermStability->setText("Stop Timer");
    ui->btnOpenDatabaseLongTermStability->setEnabled(false);
    ui->btnCreateDatabaseLongTermStability->setEnabled(false);
    ui->btnExportAllLongTermStability->setEnabled(false);
    ui->btnExportSelectedLongTermStability->setEnabled(false);
    ui->groupBoxTimeTorun->setEnabled(false);
    ui->groupBoxTimeInterval->setEnabled(false);
    ui->sms500Tab->setEnabled(false);
    ui->starSimulatorTab->setEnabled(false);
    ui->ledDriverTab->setEnabled(false);
    ui->actionSystemZero->setEnabled(false);
    ui->actionCalibrateSystem->setEnabled(false);
    ui->actionLoadInitialSolution->setEnabled(false);
    ui->actionLoadTransferenceFunction->setEnabled(false);

    // Adjusts textbox values
    if (ui->longTermStabilityHour->text().isEmpty()) {
        ui->longTermStabilityHour->setText("0");
    }
    if (ui->longTermStabilityMin->text().isEmpty()) {
        ui->longTermStabilityMin->setText("0");
    }
    if (ui->longTermStabilitySec->text().isEmpty()) {
        if (ui->longTermStabilityMin->text().compare("0") == 0) {
            ui->longTermStabilitySec->setText("1");
        } else {
            ui->longTermStabilitySec->setText("0");
        }
    }
    if (ui->longTermStabilityTimeInterval->text().isEmpty()) {
        ui->longTermStabilityTimeInterval->setText("1");
    }

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
    if (ui->noiseReductionCheckBox->isChecked()) {
        noiseReduction = ui->noiseReductionLineEdit->text().toInt();
    }

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
            channelValues);

    longTermStabilityAlarmClock->setAlarmClock(hour, min, sec, secTimeInterval);
    longTermStabilityAlarmClock->start();
}

void MainWindow::longTermStabilityStop()
{
    longTermStabilityAlarmClock->stop();

    ui->btnStartStopLongTermStability->setIcon(QIcon(":/pics/start.png"));
    ui->btnStartStopLongTermStability->setText("Start Timer");
    ui->btnOpenDatabaseLongTermStability->setEnabled(true);
    ui->btnCreateDatabaseLongTermStability->setEnabled(true);
    ui->btnExportAllLongTermStability->setEnabled(true);
    ui->groupBoxTimeTorun->setEnabled(false);
    ui->groupBoxTimeInterval->setEnabled(false);
    ui->sms500Tab->setEnabled(true);
    ui->starSimulatorTab->setEnabled(true);
    ui->ledDriverTab->setEnabled(true);
    ui->actionSystemZero->setEnabled(true);
    ui->actionCalibrateSystem->setEnabled(true);
    ui->actionLoadInitialSolution->setEnabled(true);
    ui->actionLoadTransferenceFunction->setEnabled(true);

    // Prevents Database subscrition
    ui->btnStartStopLongTermStability->setEnabled(false);

    ui->startLongTermStabilityLabel->hide();
    ui->stopLongTermStabilityLabel->hide();
    ui->startLongTermStabilityLabel_2->hide();
    ui->stopLongTermStabilityLabel_2->hide();
}

void MainWindow::longTermStabilitySaveSMS500Data()
{
    if (longTermStabilityAlarmClock->isRunning()) {
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

/**
 *  Esta funcao necessita de melhorias
 */
void MainWindow::longTermStabilityHandleTableSelection()
{
    QVector<double> amplitude;
    QPolygonF points = longTermStability->selectedData(ui->tableView->selectionModel()->selectedRows(), amplitude);

    plotLTS->setPlotLimits(350, 1000, 0, 1000);
    for (int i = 0; i < amplitude.size(); i++) {
        plotLTS->showData(points, amplitude[i]);
    }
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
                       tr("<font color='#800000'><b>Star Simulator</b> %1, %2</font>"
                          "<p>Based on Qt 5.0.2 (32 bit) and Qwt 6.0.1.</p>"
                          "<p>This software was developed to simulates stars required to calibrate the "
                          "Autonomous Star Tracker (AST).</p>"
                          "<p><b>Engineering:</b><br>"
                          "Marcos Eduardo Gomes Borges<br>"
                          "mail: <a href='mailto:marcoseborges@gmail.com?Subject=About%20StarSimulator'>marcoseborges@gmail.com</a><br>"
                          "web site: <a href='http://www.borges-solutions.com/'>www.borges-solutions.com</a></p>"
                          "<p><b>With special thanks to:</b><br>"
                          "Braulio Fonseca Carneiro de Albuquerque</p>"
                          "Copyright 2013 National Institute For Space Research (INPE).")
                       .arg(VER_FILEVERSION_STR)
                       .arg(VER_BUILD_DATE_STR));
}

void MainWindow::aboutSMS500()
{
    // SMS500 Configure
    if (sms500->isConnected() == false) {
        if (sms500Connect() == false) {
            return;
        }
    }

    AboutSMSDialog *dlg = new AboutSMSDialog(this);
    dlg->setDLL(QString::number(sms500->dllVersion()));
    dlg->setSerialNumber(sms500->serialNumber());
    dlg->setFirstCoefficient(QString::number(sms500->coefficient1()));
    dlg->setSecondCoefficient(QString::number(sms500->coefficient2()));
    dlg->setThirdCoefficient(QString::number(sms500->coefficient3()));
    dlg->setIntercept(QString::number(sms500->intercept()));
    dlg->exec();
}


// APAGAR APAGAR APAGAR
template <class T>
QVector< QVector<T> > MainWindow::loadData(const QString &title, const QString &filter, const QString &section)
{
    //    QString filePath = QFileDialog::getOpenFileName(this, title, QDir::currentPath(), filter);
    //    MatrixXi matrix;

    //    if (filePath.isEmpty()) {
    //        return matrix; // size zero
    //    }

    //    QFile inFile(filePath);
    //    inFile.open(QIODevice::ReadOnly);
    //    QTextStream in(&inFile);
    //    QString line;
    //    QStringList fields;
    //    bool headerOk     = false;
    //    bool conversionOk = false;
    //    int numberOfColumns;

    //    // Find section
    //    do {
    //        line = in.readLine();
    //        if (line.contains(section)) {
    //            headerOk = true;
    //        }
    //    } while ((headerOk == false) && !in.atEnd());

    //    if (headerOk == false) {
    //        return matrix; // size zero
    //    }

    //    // Read data
    //    while (!in.atEnd()) {
    //        line   = in.readLine();

    //        // Is a comment?
    //        while (line.startsWith(';', Qt::CaseInsensitive) && !in.atEnd()) {
    //            line = in.readLine();
    //        }

    //        // Is other section or end of file?
    //        fields = line.split("\t");
    //        if (fields.empty()) {
    //            return matrix;
    //        }

    //        for (int i = 0; i < fields.length(); i++) {
    //            matrix.resize();
    //        }

    //        fields.at(1).toInt(&conversionOk);
    //        if (conversionOk == false) {
    //            return false;
    //        }

    //        values(i) = fields.at(1).toInt();
    //    }
    //    inFile.close();
}

void MainWindow::operationModeChanged()
{
    if (ui->rbtnFlux->isChecked()) {
        plotSMS500->setTitle("Operation Mode: Flux");
        plotLedDriver->setTitle("Operation Mode: Flux");
    } else if (ui->rbtnIntensity->isChecked()) {
        plotSMS500->setTitle("Operation Mode: Intensity");
        plotLedDriver->setTitle("Operation Mode: Intensity");
    } else if (ui->rbtnIrradiance->isChecked()) {
        plotSMS500->setTitle("Operation Mode: Irradiance (lux)");
        plotLedDriver->setTitle("Operation Mode: Irradiance (lux)");
    } else if (ui->rbtnRadiance->isChecked()) {
        plotSMS500->setTitle("Operation Mode: Radiance (cd/m2)");
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

void MainWindow::sms500StartStopScan()
{
    if (sms500->isConnected() == false) {
        if (sms500Connect() == false) {
            return;
        }
    }

    if (ui->btnStartScan->text().contains("Start Scan")) {
        sms500StartScan();
    } else {
        sms500StopScan();
    }
}

void MainWindow::sms500StartScan()
{
    ui->btnStartScan->setIcon(QIcon(":/pics/stop.png"));
    ui->btnStartScan->setText("Stop Scan");
    ui->scanNumberLabel->setText(tr("    Scan number: %1").arg(0));
    ui->scanNumberLabel->show();
    ui->scanNumberLabel_2->setText(tr("    Scan number: %1").arg(0));
    ui->btnZoom->setEnabled(false);
    ui->btnSaveScan->setEnabled(false);
    performScan();
}

void MainWindow::sms500StopScan()
{
    ui->btnStartScan->setIcon(QIcon(":/pics/start.png"));
    ui->btnStartScan->setText("Start Scan");
    sms500->stop();
}

void MainWindow::performScan()
{
    statusBar()->showMessage(tr("Geting Spectral Data"), 5000);
    sms500Configure();
    sms500->start();
}

void MainWindow::plotScanResult(QPolygonF points, int peakWavelength, double amplitude,
                                int scanNumber, int integrationTimeIndex, bool satured)
{
    plotSMS500->setPlotLimits(350, 1000, 0, amplitude);
    plotSMS500->showPeak(peakWavelength, amplitude);
    plotSMS500->showData(points, amplitude);

    plotLedDriver->setPlotLimits(350, 1000, 0, amplitude);
    plotLedDriver->showPeak(peakWavelength, amplitude);
    plotLedDriver->showData(points, amplitude);

    // Star Simulator Module
    QPolygonF newPoints;
    for (int i = 0; i < points.size(); i++) {
        newPoints << QPointF(points.at(i).x(), points.at(i).y() * transferenceFunction[i]);
    }

    plotLSqNonLin->showPeak(peakWavelength, amplitude);
    plotLSqNonLin->showData(newPoints, lsqNonLinStar->peak() * 1.2);

    // Updates irradiance labels
    outputIrradiance = trapezoidalNumInteg(newPoints);
    starIrradiance   = trapezoidalNumInteg(lsqNonLinStar->spectralDataToPlot());

    ui->outputIrradianceLabel->setText(QString::number(outputIrradiance, 'e', 2));
    ui->starIrradianceLabel->setText(QString::number(starIrradiance, 'e', 2));

    if (ui->AutoRangeCheckBox->isChecked()) {
        ui->integrationTimeComboBox->setCurrentIndex(integrationTimeIndex);
        sms500->setRange(integrationTimeIndex);
    }

    ui->scanNumberLabel->setText(tr("    Scan number: %1").arg(scanNumber));
    ui->scanNumberLabel_2->setText(tr("    Scan number: %1").arg(scanNumber));

    if (satured == true) {
        ui->saturedLabel->show();
        if (ui->plotArea->width() > 700) {
            ui->saturedLabel_2->show();
        }
    } else {
        ui->saturedLabel->hide();
        ui->saturedLabel_2->hide();
    }

    sms500Configure();
    sms500->enableNextScan();
}

void MainWindow::scanFinished()
{
    ui->btnZoom->setEnabled(true);
    ui->btnSaveScan->setEnabled(true);
    ui->btnStartScan->setIcon(QIcon(":/pics/start.png"));
    ui->btnStartScan->setText("Start Scan");
}

void MainWindow::systemZero()
{
    if (sms500->isConnected() == false) {
        if (sms500Connect() == false) {
            QMessageBox::warning(this, tr("Connection Error"),
                                 tr("Spectrometer Hardware not found.\n\nCheck USB connection and try again..."));
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

    statusBar()->showMessage("Running calibration :: Please wait!", 5000);

    sms500->finishLampScan();

    QMessageBox::information(this, tr("SMS500 Info"), tr("System calibration completed."));
}

void MainWindow::moved( const QPoint &pos )
{
    showInfo(tr("Wavelength=%1, Amplitude=%2")
             .arg(plotSMS500->invTransform(QwtPlot::xBottom, pos.x()))
             .arg(plotSMS500->invTransform(QwtPlot::yLeft, pos.y())));
}

void MainWindow::selected( const QPolygon & )
{
    showInfo();
}

void MainWindow::uiInputValidator()
{
    QValidator *numberOfScansValidator =
            new QRegExpValidator(QRegExp("^-1$|^[1-9][0-9]{0,6}$"), this);
    ui->numberOfScansLineEdit->setValidator(numberOfScansValidator);

    QValidator *wavelengthValidator =
            new QRegExpValidator(QRegExp("^0$|^[1-9][0-9]{0,2}$|^1[0-9]{0,3}$|^2000$"), this);
    ui->startWaveLineEdit->setValidator(wavelengthValidator);
    ui->stopWaveLineEdit->setValidator(wavelengthValidator);

    QValidator *noiseReductionValidator =
            new QRegExpValidator(QRegExp("^0$|^[1-9][0-9]{0,1}$|^100$"), this);
    ui->noiseReductionLineEdit->setValidator(noiseReductionValidator);

    QValidator *ledModelingValidator =
            new QRegExpValidator(QRegExp("^0$|^[1-8][0-9]{0,1}$|^9[0-6]{0,1}$"), this);
    ui->startChannel->setValidator(ledModelingValidator);
    ui->endChannel->setValidator(ledModelingValidator);

    QValidator *ledModelingIncDecValidator =
            new QRegExpValidator(QRegExp("^[1-9][0-9]{0,2}$|^1000$"), this);
    ui->levelIncDecValue->setValidator(ledModelingIncDecValidator);

    QValidator *timeValidator = new QRegExpValidator(QRegExp("^([0-9]|[1-5][0-9]{1})$"), this);
    ui->longTermStabilityMin->setValidator(timeValidator);
    ui->longTermStabilitySec->setValidator(timeValidator);

    QValidator *hourValidator =
            new QRegExpValidator(QRegExp("^(0|[1-9][0-9]{0,2})$"), this);
    ui->longTermStabilityHour->setValidator(hourValidator);

    QValidator *timeInterval =
            new QRegExpValidator(QRegExp("^([1-9][0-9]{0,3})$"), this);
    ui->longTermStabilityTimeInterval->setValidator(timeInterval);

    QValidator *magnitudeValidator = new QRegExpValidator(QRegExp("^-[1-9][.][0-9]|-0[.][1-9]|[0-9][.][0-9]|-[1-9]|[0-9]$"), this);
    ui->starMagnitude->setValidator(magnitudeValidator);

    QValidator *temperatureValidator = new QRegExpValidator(QRegExp("^[1-9][0-9]{0,5}$"), this);
    ui->starTemperature->setValidator(temperatureValidator);

    QValidator *channelValidator =
            new QRegExpValidator(QRegExp("^0$|^[1-9][0-9]{0,2}$|^[1-3][0-9]{0,3}$|^40([0-8][0-9]|[9][0-5])$"), this);
    ui->dac04channel25->setValidator(channelValidator);
    ui->dac04channel26->setValidator(channelValidator);
    ui->dac04channel27->setValidator(channelValidator);
    ui->dac04channel28->setValidator(channelValidator);
    ui->dac04channel29->setValidator(channelValidator);
    ui->dac04channel30->setValidator(channelValidator);
    ui->dac04channel31->setValidator(channelValidator);
    ui->dac04channel32->setValidator(channelValidator);
    ui->dac05channel33->setValidator(channelValidator);
    ui->dac05channel34->setValidator(channelValidator);
    ui->dac05channel35->setValidator(channelValidator);
    ui->dac05channel36->setValidator(channelValidator);
    ui->dac05channel37->setValidator(channelValidator);
    ui->dac05channel38->setValidator(channelValidator);
    ui->dac05channel39->setValidator(channelValidator);
    ui->dac05channel40->setValidator(channelValidator);
    ui->dac06channel41->setValidator(channelValidator);
    ui->dac06channel42->setValidator(channelValidator);
    ui->dac06channel43->setValidator(channelValidator);
    ui->dac06channel44->setValidator(channelValidator);
    ui->dac06channel45->setValidator(channelValidator);
    ui->dac06channel46->setValidator(channelValidator);
    ui->dac06channel47->setValidator(channelValidator);
    ui->dac06channel48->setValidator(channelValidator);
    ui->dac07channel49->setValidator(channelValidator);
    ui->dac07channel50->setValidator(channelValidator);
    ui->dac07channel51->setValidator(channelValidator);
    ui->dac07channel52->setValidator(channelValidator);
    ui->dac07channel53->setValidator(channelValidator);
    ui->dac07channel54->setValidator(channelValidator);
    ui->dac07channel55->setValidator(channelValidator);
    ui->dac07channel56->setValidator(channelValidator);
    ui->dac08channel57->setValidator(channelValidator);
    ui->dac08channel58->setValidator(channelValidator);
    ui->dac08channel59->setValidator(channelValidator);
    ui->dac08channel60->setValidator(channelValidator);
    ui->dac08channel61->setValidator(channelValidator);
    ui->dac08channel62->setValidator(channelValidator);
    ui->dac08channel63->setValidator(channelValidator);
    ui->dac08channel64->setValidator(channelValidator);
    ui->dac09channel65->setValidator(channelValidator);
    ui->dac09channel66->setValidator(channelValidator);
    ui->dac09channel67->setValidator(channelValidator);
    ui->dac09channel68->setValidator(channelValidator);
    ui->dac09channel69->setValidator(channelValidator);
    ui->dac09channel70->setValidator(channelValidator);
    ui->dac09channel71->setValidator(channelValidator);
    ui->dac09channel72->setValidator(channelValidator);
    ui->dac10channel73->setValidator(channelValidator);
    ui->dac10channel74->setValidator(channelValidator);
    ui->dac10channel75->setValidator(channelValidator);
    ui->dac10channel76->setValidator(channelValidator);
    ui->dac10channel77->setValidator(channelValidator);
    ui->dac10channel78->setValidator(channelValidator);
    ui->dac10channel79->setValidator(channelValidator);
    ui->dac10channel80->setValidator(channelValidator);
    ui->dac11channel81->setValidator(channelValidator);
    ui->dac11channel82->setValidator(channelValidator);
    ui->dac11channel83->setValidator(channelValidator);
    ui->dac11channel84->setValidator(channelValidator);
    ui->dac11channel85->setValidator(channelValidator);
    ui->dac11channel86->setValidator(channelValidator);
    ui->dac11channel87->setValidator(channelValidator);
    ui->dac11channel88->setValidator(channelValidator);
    ui->dac12channel89->setValidator(channelValidator);
    ui->dac12channel90->setValidator(channelValidator);
    ui->dac12channel91->setValidator(channelValidator);
    ui->dac12channel92->setValidator(channelValidator);
    ui->dac12channel93->setValidator(channelValidator);
    ui->dac12channel94->setValidator(channelValidator);
    ui->dac12channel95->setValidator(channelValidator);
    ui->dac12channel96->setValidator(channelValidator);

    // Star Simulator
    ui->initialSolution_ch01->setValidator(channelValidator);
    ui->initialSolution_ch02->setValidator(channelValidator);
    ui->initialSolution_ch03->setValidator(channelValidator);
    ui->initialSolution_ch04->setValidator(channelValidator);
    ui->initialSolution_ch05->setValidator(channelValidator);
    ui->initialSolution_ch06->setValidator(channelValidator);
    ui->initialSolution_ch07->setValidator(channelValidator);
    ui->initialSolution_ch08->setValidator(channelValidator);
    ui->initialSolution_ch09->setValidator(channelValidator);
    ui->initialSolution_ch10->setValidator(channelValidator);
    ui->initialSolution_ch11->setValidator(channelValidator);
    ui->initialSolution_ch12->setValidator(channelValidator);
    ui->initialSolution_ch13->setValidator(channelValidator);
    ui->initialSolution_ch14->setValidator(channelValidator);
    ui->initialSolution_ch15->setValidator(channelValidator);
    ui->initialSolution_ch16->setValidator(channelValidator);
    ui->initialSolution_ch17->setValidator(channelValidator);
    ui->initialSolution_ch18->setValidator(channelValidator);
    ui->initialSolution_ch19->setValidator(channelValidator);
    ui->initialSolution_ch20->setValidator(channelValidator);
    ui->initialSolution_ch21->setValidator(channelValidator);
    ui->initialSolution_ch22->setValidator(channelValidator);
    ui->initialSolution_ch23->setValidator(channelValidator);
    ui->initialSolution_ch24->setValidator(channelValidator);
    ui->initialSolution_ch25->setValidator(channelValidator);
    ui->initialSolution_ch26->setValidator(channelValidator);
    ui->initialSolution_ch27->setValidator(channelValidator);
    ui->initialSolution_ch28->setValidator(channelValidator);
    ui->initialSolution_ch29->setValidator(channelValidator);
    ui->initialSolution_ch30->setValidator(channelValidator);
    ui->initialSolution_ch31->setValidator(channelValidator);
    ui->initialSolution_ch32->setValidator(channelValidator);
    ui->initialSolution_ch33->setValidator(channelValidator);
    ui->initialSolution_ch34->setValidator(channelValidator);
    ui->initialSolution_ch35->setValidator(channelValidator);
    ui->initialSolution_ch36->setValidator(channelValidator);
    ui->initialSolution_ch37->setValidator(channelValidator);
    ui->initialSolution_ch38->setValidator(channelValidator);
    ui->initialSolution_ch39->setValidator(channelValidator);
    ui->initialSolution_ch40->setValidator(channelValidator);
    ui->initialSolution_ch41->setValidator(channelValidator);
    ui->initialSolution_ch42->setValidator(channelValidator);
    ui->initialSolution_ch43->setValidator(channelValidator);
    ui->initialSolution_ch44->setValidator(channelValidator);
    ui->initialSolution_ch45->setValidator(channelValidator);
    ui->initialSolution_ch46->setValidator(channelValidator);
    ui->initialSolution_ch47->setValidator(channelValidator);
    ui->initialSolution_ch48->setValidator(channelValidator);
    ui->initialSolution_ch49->setValidator(channelValidator);
    ui->initialSolution_ch50->setValidator(channelValidator);
    ui->initialSolution_ch51->setValidator(channelValidator);
    ui->initialSolution_ch52->setValidator(channelValidator);
    ui->initialSolution_ch53->setValidator(channelValidator);
    ui->initialSolution_ch54->setValidator(channelValidator);
    ui->initialSolution_ch55->setValidator(channelValidator);
    ui->initialSolution_ch56->setValidator(channelValidator);
    ui->initialSolution_ch57->setValidator(channelValidator);
    ui->initialSolution_ch58->setValidator(channelValidator);
    ui->initialSolution_ch59->setValidator(channelValidator);
    ui->initialSolution_ch60->setValidator(channelValidator);
    ui->initialSolution_ch61->setValidator(channelValidator);
    ui->initialSolution_ch62->setValidator(channelValidator);
    ui->initialSolution_ch63->setValidator(channelValidator);
    ui->initialSolution_ch64->setValidator(channelValidator);
    ui->initialSolution_ch65->setValidator(channelValidator);
    ui->initialSolution_ch66->setValidator(channelValidator);
    ui->initialSolution_ch67->setValidator(channelValidator);
    ui->initialSolution_ch68->setValidator(channelValidator);
    ui->initialSolution_ch69->setValidator(channelValidator);
    ui->initialSolution_ch70->setValidator(channelValidator);
    ui->initialSolution_ch71->setValidator(channelValidator);
    ui->initialSolution_ch72->setValidator(channelValidator);
    ui->initialSolution_ch73->setValidator(channelValidator);
    ui->initialSolution_ch74->setValidator(channelValidator);
    ui->initialSolution_ch75->setValidator(channelValidator);
    ui->initialSolution_ch76->setValidator(channelValidator);
    ui->initialSolution_ch77->setValidator(channelValidator);
    ui->initialSolution_ch78->setValidator(channelValidator);
    ui->initialSolution_ch79->setValidator(channelValidator);
    ui->initialSolution_ch80->setValidator(channelValidator);
    ui->initialSolution_ch81->setValidator(channelValidator);
    ui->initialSolution_ch82->setValidator(channelValidator);
    ui->initialSolution_ch83->setValidator(channelValidator);
    ui->initialSolution_ch84->setValidator(channelValidator);
    ui->initialSolution_ch85->setValidator(channelValidator);
    ui->initialSolution_ch86->setValidator(channelValidator);
    ui->initialSolution_ch87->setValidator(channelValidator);
    ui->initialSolution_ch88->setValidator(channelValidator);
    ui->initialSolution_ch89->setValidator(channelValidator);
    ui->initialSolution_ch90->setValidator(channelValidator);
    ui->initialSolution_ch91->setValidator(channelValidator);
    ui->initialSolution_ch92->setValidator(channelValidator);
    ui->initialSolution_ch93->setValidator(channelValidator);
    ui->initialSolution_ch94->setValidator(channelValidator);
    ui->initialSolution_ch95->setValidator(channelValidator);
    ui->initialSolution_ch96->setValidator(channelValidator);

}

void MainWindow::sms500SignalAndSlot()
{
    connect(ui->btnZoom,            SIGNAL(toggled(bool)),       plotSMS500, SLOT(enableZoomMode(bool)));
    connect(plotSMS500->plotPicker, SIGNAL(moved(const QPoint&)),      this, SLOT(moved(const QPoint&)));
    connect(plotSMS500->plotPicker, SIGNAL(selected(const QPolygon&)), this, SLOT(selected(const QPolygon&)));
    connect(plotSMS500,             SIGNAL(showInfo()),                this, SLOT(showInfo()));

    connect(ui->btnStartScan,         SIGNAL(clicked()), this, SLOT(sms500StartStopScan()));
    connect(ui->btnSaveScan,          SIGNAL(clicked()), this, SLOT(sms500SaveScanData()));
    connect(ui->btnConnectDisconnect, SIGNAL(clicked()), this, SLOT(connectDisconnect()));

    connect(ui->AutoRangeCheckBox,      SIGNAL(toggled(bool)), this, SLOT(autoRangeChanged(bool)));
    connect(ui->noiseReductionCheckBox, SIGNAL(toggled(bool)), this, SLOT(noiseReductionChanged(bool)));

    connect(ui->rbtnFlux,       SIGNAL(toggled(bool)), this, SLOT(operationModeChanged()));
    connect(ui->rbtnIntensity,  SIGNAL(toggled(bool)), this, SLOT(operationModeChanged()));
    connect(ui->rbtnIrradiance, SIGNAL(toggled(bool)), this, SLOT(operationModeChanged()));
    connect(ui->rbtnRadiance,   SIGNAL(toggled(bool)), this, SLOT(operationModeChanged()));

    connect(ui->actionSystemZero,      SIGNAL(triggered(bool)), this, SLOT(systemZero()));
    connect(ui->actionCalibrateSystem, SIGNAL(triggered(bool)), this, SLOT(calibrateSystem()));

    connect(ui->actionAboutSMS500,       SIGNAL(triggered(bool)), this, SLOT(aboutSMS500()));
    connect(ui->actionAboutThisSoftware, SIGNAL(triggered(bool)), this, SLOT(aboutThisSoftware()));

    connect(sms500, SIGNAL(scanPerformed(QPolygonF,int,double,int,int,bool)), this,
            SLOT(plotScanResult(QPolygonF,int,double,int,int,bool)));
    connect(sms500, SIGNAL(scanFinished()), this, SLOT(scanFinished()));
    connect(sms500, SIGNAL(info(QString)), this, SLOT(statusBarMessage(QString)));
}

void MainWindow::ledDriverSignalAndSlot()
{
    connect(ledDriver, SIGNAL(warningMessage(QString,QString)),   this, SLOT(warningMessage(QString,QString)));

    connect(ledDriver, SIGNAL(performScan()),        this, SLOT(ledModelingPerformScan()));
    connect(ledDriver, SIGNAL(saveData(QString)),    this, SLOT(ledModelingSaveData(QString)));
    connect(ledDriver, SIGNAL(saveAllData(QString)), this, SLOT(ledModelingSaveChannelData(QString)));
    connect(ledDriver, SIGNAL(modelingFinished()),   this, SLOT(ledModelingFinished()));
    connect(ledDriver, SIGNAL(info(QString)),        this, SLOT(ledModelingInfo(QString)));
    connect(ui->setV2RefCheckBox, SIGNAL(toggled(bool)), this, SLOT(ledDriverSetV2Ref(bool)));

    connect(sms500,                      SIGNAL(scanFinished()), ledDriver, SLOT(enabledModelingContinue()));
    connect(ui->btnConnectDisconnectLED, SIGNAL(clicked()),      this,      SLOT(ledDriverConnectDisconnect()));
    connect(ui->btnLedModeling,          SIGNAL(clicked()),      this,      SLOT(ledModeling()));
    connect(ui->btnLoadValuesForChannels,SIGNAL(clicked()),      this,      SLOT(ledDriverLoadValuesForChannels()));

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

void MainWindow::lsqNonLinSignalAndSlot()
{
    connect(ui->actionLoadTransferenceFunction, SIGNAL(triggered(bool)), this, SLOT(starUpdateTransferenceFunction()));
    connect(ui->actionLoadInitialSolution, SIGNAL(triggered(bool)), this, SLOT(lsqNonLinLoadInitialSolution()));
    connect(ui->btnLoadInitialSolution, SIGNAL(clicked(bool)), this, SLOT(lsqNonLinLoadInitialSolution()));
    connect(ui->btnSaveStarSimulatorData, SIGNAL(clicked()), SLOT(lsqNonLinSaveData()));

    connect(lsqnonlin, SIGNAL(ledDataNotFound()), this, SLOT(lsqNonLinLoadLedData()));
    connect(lsqnonlin, SIGNAL(info(QString)),     this, SLOT(lsqNonLinLog(QString)));
    connect(lsqnonlin, SIGNAL(performScan()),     this, SLOT(lsqNonLinPerformScan()));
    connect(lsqnonlin, SIGNAL(finished()),        this, SLOT(lsqNonLinFinished()));
    connect(sms500,    SIGNAL(scanFinished()),    this, SLOT(lsqNonLinObjectiveFunction()));

    connect(ui->starMagnitude, SIGNAL(editingFinished()), this, SLOT(lsqNonLinStarSettings()));
    connect(ui->starTemperature, SIGNAL(editingFinished()), this, SLOT(lsqNonLinStarSettings()));

    connect(ui->btnStartStopStarSimulator, SIGNAL(clicked()), this, SLOT(lsqNonLinStartStop()));

    connect(ui->x0Random, SIGNAL(toggled(bool)), this, SLOT(lsqNonLinx0Handle()));
    connect(ui->x0UserDefined, SIGNAL(toggled(bool)), this, SLOT(lsqNonLinx0Handle()));
    connect(ui->x0DefinedInLedDriver, SIGNAL(toggled(bool)), this, SLOT(lsqNonLinx0Handle()));
}

void MainWindow::longTermStabilitySignalAndSlot()
{
    connect(ui->btnStartStopLongTermStability, SIGNAL(clicked()), this, SLOT(longTermStabilityStartStop()));
    connect(longTermStabilityAlarmClock, SIGNAL(timeout()), this, SLOT(performScan()));
    connect(longTermStabilityAlarmClock, SIGNAL(finished()), this, SLOT(longTermStabilityStop()));

    connect(sms500, SIGNAL(scanFinished()), this, SLOT(longTermStabilitySaveSMS500Data()));

    connect(ui->btnCreateDatabaseLongTermStability, SIGNAL(clicked()), this, SLOT(longTermStabilityCreateDB()));
    connect(ui->btnOpenDatabaseLongTermStability, SIGNAL(clicked()), this, SLOT(longTermStabilityOpenDB()));
    connect(ui->btnExportAllLongTermStability, SIGNAL(clicked()), this, SLOT(longTermStabilityExportAll()));
    connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(longTermStabilityHandleTableSelection()));
}

