#ifndef LONGTERMSTABILITY_H
#define LONGTERMSTABILITY_H

#include <QThread>
#include <QVector>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDateTime>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class LongTermStability : public QThread
{
    Q_OBJECT
public:
    explicit LongTermStability(QObject *parent = 0);
    ~LongTermStability();

    bool createDB(const QString &filePath);
    bool openDB(const QString &filePath);
    QSqlTableModel *scanInfoTableModel();
    QPolygonF selectedData(QModelIndexList indexList, QVector<double> &maximumAmplitude);
    QString lastError();
    void exportData(const QString &filePath);
    void stop();
    bool status();

    bool saveSMS500andLedDriverParameters(int id,
            int startWavelength,
            int stopWavelength,
            int integrationTime,
            int samplesToAverage,
            int boxcarSmoothing,
            int noiseReduction,
            bool dynamicDark,
            bool v2ref,
            int star_magnitude,
            int star_temperature,
            int fiting_algorithm,
            const QVector<int> &channelValue);

    bool saveScanData(int id,
            int scanNumber,
            int dominateWavelength,
            int peakWavelength,
            int fwhm,
            double power,
            double purity,
            int startWavelength,
            int stopWavelength,
            const double *masterData);

signals:
    void progressMinimumInfo(int value);
    void progressMaximumInfo(int value);
    void progressInfo(int value);
    void error(QString message);

private:
    void run();

    bool currentStatus;
    bool stopThread;
    QSqlDatabase db;
    QString exportFilePath;
    QString lastErrorMessage;
    QSqlTableModel *scanInfoModel;
    QSqlTableModel *parametersModel;
    QSqlTableModel *ledDriverConfigurationModel;
    QSqlTableModel *scanDataModel;
};

#endif // LONGTERMSTABILITY_H
