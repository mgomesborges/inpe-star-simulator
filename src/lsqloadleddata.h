#ifndef LSQLOADLEDDATA_H
#define LSQLOADLEDDATA_H

#include <QThread>

#include <QTextStream>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QTime>

#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

class LSqLoadLedData : public QThread
{
    Q_OBJECT
public:
    explicit LSqLoadLedData(QObject *parent = 0, QString path = QString::null);
    void stop();

signals:
    void progressInfo(int value);
    void error(QString message);

private:
    void run();

    QString inputPath;
    bool stopThread;
};

#endif // LSQLOADLEDDATA_H
