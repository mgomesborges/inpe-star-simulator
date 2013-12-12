#ifndef LSQLOADLEDDATA_H
#define LSQLOADLEDDATA_H

#include <QThread>

#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QStringList>
#include <QString>
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
    bool status();
    void stop();

signals:
    void progressInfo(int value);
    void error(QString message);

private:
    void run();

    QString inputPath;
    bool stopThread;
    bool isLoadCompleted;
};

#endif // LSQLOADLEDDATA_H
