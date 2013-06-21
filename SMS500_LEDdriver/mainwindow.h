#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

class SMS500;
class LedDriver;
class QLabel;
class Plot;
class LeastSquareNonLin;
class Star;

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
    
private:
    Ui::MainWindow *ui;
    Plot *plot;
    Plot *plotLedDriver;
    SMS500 *sms500;
    QLabel *mStatusLabel;
    LedDriver *ledDriver;
    int  valueOfPort[8];
    char txBuffer[256];
    QString ledModelingFilePath;
    int scanNumberOfLedModeling;

    std::vector< std::vector<double> > ledModelingData;
    std::vector< std::vector<double> > starData;

    Star *star;

    LeastSquareNonLin *lsqnonlin;

    void resizeEvent(QResizeEvent *);

    void sms500SignalAndSlot();
    void sms500Configure();
    bool sms500Connect();
    void sms500Disconnect();

    void ledDriverSignalAndSlot();
    void ledDriverConfigureDac(char dac);

private slots:
    void statusBarMessage(QString message);
    void warningMessage(QString title, QString message);
    void aboutThisSoftware();
    void aboutSMS500();

    void operationModeChanged();
    void autoRangeChanged(bool enable);
    void noiseReductionChanged(bool enable);

    void connectDisconnect();
    void startStopScan();
    void performScan();
    void plotScanResult(const double *masterData, const int *wavelegth,int peakWavelength,
                        double amplitude, int numberOfPoints, int scanNumber, int integrationTimeIndex, bool satured);
    void scanFinished();
    void saveScanData();
    void systemZero();
    void calibrateSystem();

    void moved(const QPoint&);
    void selected(const QPolygon&);
    void showInfo(const QString &text = QString::null);

    void ledDriverConnectDisconnect();
    void ledDriverConnectionError(QString serialError, QString statusBarMsg);
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

    void starSettings();
    void leastSquare();

    void leastSquarePerformScan();
    void leastSquareObjectiveFunction();
    void leastSquareStop();
};

#endif // MAINWINDOW_H
