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
#include <QSignalMapper>


#include <QLineEdit>
#include <QGridLayout>


#include "utils.h"
#include "filehandle.h"

#include "plot.h"
#include "star.h"
#include "sms500.h"
#include "leddriver.h"
#include "aboutsmsdialog.h"
#include "starsimulator.h"
#include "starsimulatorloadleddata.h"
#include "longtermstability.h"
#include "longtermstabilityalarmclock.h"
#include "longtermstabilityexportdialog.h"
#include "ftdidevicechooserdialog.h"
#include "configurechannelsdialog.h"
#include "remotecontrol.h"

#include "version.h"

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

    enum status {
        STOPED,
        GD_ALGORITHM
    };

private:
    QGridLayout *createLedDriverLayout();
    QGridLayout *createX0GridLayout();
    void uiInputValidator();
    void resizeEvent(QResizeEvent *);
    double trapezoidalNumInteg(QPolygonF points);

    MatrixXi lsqNonLinX0();
    QVector<int> ledDriverChannelValues();

    // GUI Components
    Ui::MainWindow *ui;
    QLabel *statusLabel;
    QLineEdit *starSimulatorX0[96];
    QLineEdit *ledDriverChannel[96];

    Plot *plotSMS500;
    SMS500 *sms500;
    LedDriver *ledDriver;
    Plot *plotLedDriver;
    QString ledModelingFilePath;
    QString ledModelingConfiguration;
    vector< vector<double> > ledModelingData;

    Star *lsqNonLinStar;
    StarSimulator *lsqnonlin;
    Plot *plotLSqNonLin;
    QTime lsqNonLinTime;
    QVector<double> transferenceFunction;
    double outputIrradiance; // Updated in plotScanResult()
    double starIrradiance;   // Updated in plotScanResult()

    LongTermStability *longTermStability;
    LongTermStabilityAlarmClock *longTermStabilityAlarmClock;
    Plot *plotLTS;
    int longTermStabilityScanNumber;

    RemoteControl *remoteControl;

    QVector<int> activeChannels;

    QString lastDir;

    int currentStatus;

private slots:
    void aboutThisSoftware();
    void warningMessage(const QString &caption, const QString &message);
    void updateConfigureMenu(int selectedTab);
    void setDigitalLevelForChannels();
    void updateLastDir(QString path);

    void plotMoved(const QPoint&);
    void plotSelected(const QPolygon&);
    void plotShowInfo(const QString &text = QString::null);

    void sms500About();
    void sms500OperationModeChanged();
    void sms500AutoRangeChanged(bool enable);
    void sms500NumberOfScansChanged(QString value);
    void sms500IntegrationTimeChanged(int index);
    void sms500SamplesToAverageChanged(int value);
    void sms500BoxcarSmoothingChanged(int value);
    void sms500WavelengthStartChanged(QString value);
    void sms500WavelengthStopChanged(QString value);
    void sms500NoiseReductionChanged(bool enable);
    void sms500NoiseReductionFactorChanged(QString value);
    void sms500CorrectForDynamicDarkChanged(bool enable);
    void sms500ConnectDisconnect();
    bool sms500Connect();
    void sms500Disconnect();
    void sms500StartStopScan();
    void sms500StartScan();
    void sms500StopScan();
    void sms500NextScan();
    void sms500Configure();
    void sms500SaturedDataHandle(bool satured);
    void sms500ScanDataHandle(int scanNumber);
    void sms500SystemZero();
    void sms500CalibrateSystem();
    void sms500SaveScanData(const QString &filePath = QString());
    QString sms500MainData();

    void ledDriverConfigureConnection();
    void ledDriverConnectDisconnect();
    bool ledDriverConnect();
    void ledDriverDisconnect();
    void ledDriverDataHandle();
    bool ledDriverLoadValuesForChannels();
    void ledDriverGuiUpdate(QVector<double> level);
    void ledDriverChannelChanged();
    void ledModeling();
    void ledModelingStart();
    void ledModelingStop();
    void ledModelingGuiConfig(bool enable);
    void ledModelingSaveData(QString channel);
    void ledModelingFinished();
    void ledDriverTest();
    void ledDriverTestFinished();

    void lsqNonLinStartStop();
    void lsqNonLinStart();
    void lsqNonLinStop();
    void lsqNonLinStartGD();
    void lsqNonLinFinished();
    void lsqNonLinStarSettings();
    void lsqNonLinPerformScanWithUpdate();
    void lsqNonLinObjectiveFunction();
    void lsqNonLinLoadLedData();
    void lsqNonLinx0Handle();
    void lsqNonLinLog(QString info);
    void lsqNonLinSaveData();
    void lsqNonLinGuiConfig(bool enable);
    bool lsqNonLinLoadInitialSolution();
    void starLoadTransferenceFunction();
    bool starUpdateTransferenceFunction();
    void lsqNonLinX0GuiUpdate(QVector<double> x0);

    void longTermStabilityCreateDB();
    void longTermStabilityOpenDB();
    void longTermStabilityExportAll();
    void longTermStabilityStartStop();
    void longTermStabilityStart();
    void longTermStabilityStop();
    void longTermStabilitySaveSMS500Data();
    void longTermStabilityHandleTableSelection();
    void longTermStabilityUpdateView();
    void longTermStabilityGuiConfig(bool enable);

    void remoteSetSMS500AutoRange(bool enable);
    void remoteSetSMS500NumberOfScans(QString value);
    void remoteSetSMS500IntegrationTime(int index);
    void remoteSetSMS500SamplesToAverage(int value);
    void remoteSetSMS500BoxcarSmothing(int value);
    void remoteSetSMS500NoiseReduction(bool enable);
    void remoteSetSMS500NoiseReductionFactor(QString value);
    void remoteSetSMS500CorrectForDynamicDark(bool enable);
    void remoteSetStarMagnitude(QString value);
    void remoteSetStarTemperature(QString value);
    void remoteStarSimulatorStatus();
    void remoteStarSimulatorIrradiances();
};

#endif // MAINWINDOW_H
