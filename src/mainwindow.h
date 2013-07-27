#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QVector>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QDateTime>

#include "plot.h"
#include "sms500.h"
#include "leddriver.h"
#include "aboutsmsdialog.h"
#include "lsqnonlin.h"
#include "lsqloadleddatadialog.h"
#include "lsqstardatadialog.h"
#include "longtermstability.h"
#include "longtermstabilityalarmclock.h"
#include "longtermstabilityexportdialog.h"

#include <iostream>

#include <Eigen/Dense>
using namespace Eigen;
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
    void uiInputValidator();
    void resizeEvent(QResizeEvent *);
    bool sms500Connect();
    void sms500Disconnect();
    void sms500Configure();
    void sms500SignalAndSlot();
    void ledDriverSignalAndSlot();
    void ledDriverConfigureDac(char dac);
    QStringList ledDriverChannelValues();
    void lsqNonLinSignalAndSlot();
    void longTermStabilitySignalAndSlot();

    Ui::MainWindow *ui;
    QLabel *statusLabel;
    Plot *plotSMS500;

    SMS500 *sms500;

    LedDriver *ledDriver;
    Plot *plotLedDriver;
    int  ledDriverValueOfPort[8];
    char ledDriverTxBuffer[256];
    int ledModelingScanNumber;
    QString ledModelingFilePath;
    vector< vector<double> > ledModelingData;

    LSqNonLin *lsqnonlin;
    LSqStarDataDialog *lsqStarDialog;

    LongTermStability *longTermStability;
    LongTermStabilityAlarmClock *longTermStabilityAlarmClock;
    Plot *plotLTS;
    int longTermStabilityScanNumber;

private slots:
    void statusBarMessage(QString message);
    void warningMessage(QString title, QString message);
    void aboutThisSoftware();
    void aboutSMS500();

    void operationModeChanged();
    void autoRangeChanged(bool enable);
    void noiseReductionChanged(bool enable);

    void connectDisconnect();
    void sms500StartStopScan();
    void sms500StartScan();
    void sms500StopScan();
    void performScan();
    void plotScanResult(QPolygonF points, int peakWavelength, double amplitude,
                        int scanNumber, int integrationTimeIndex, bool satured);
    void scanFinished();
    void systemZero();
    void calibrateSystem();

    void moved(const QPoint&);
    void selected(const QPolygon&);
    void showInfo(const QString &text = QString::null);

    void sms500SaveScanData();
    void sms500SaveScanData(const QString &filePath);

    bool ledDriverConnect();
    void ledDriverDisconnect();
    void ledDriverConnectDisconnect();
    void ledDriverDac04Changed();
    void ledDriverDac05Changed();
    void ledDriverDac06Changed();
    void ledDriverDac07Changed();
    void ledDriverDac08Changed();
    void ledDriverDac09Changed();
    void ledDriverDac10Changed();
    void ledDriverDac11Changed();
    void ledDriverDac12Changed();

    void ledModeling();
    void ledModelingPerformScan();
    void ledModelingSaveData(QString fileName);
    void ledModelingSaveChannelData(QString channel);
    void ledModelingFinished();
    void ledModelingInfo(QString message);

    void lsqNonLinStart();
    void lsqNonLinStarSettings();
    void lsqNonLinPerformScan();
    void lsqNonLinObjectiveFunction();
    void lsqNonLinLoadLedData();
    void lsqNonLinFinished();

    void longTermStabilityCreateDB();
    void longTermStabilityOpenDB();
    void longTermStabilityExportAll();
    void longTermStabilityStartStop();
    void longTermStabilityStart();
    void longTermStabilityStop();
    void longTermStabilitySaveSMS500Data();
    void longTermStabilityHandleTableSelection();
    void longTermStabilityUpdateView();
};

#endif // MAINWINDOW_H
