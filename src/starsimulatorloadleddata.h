#ifndef STARSIMULATORLOADLEDDATA_H
#define STARSIMULATORLOADLEDDATA_H

#include <QDialog>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrentRun>

#include "filehandle.h"
#include "utils.h"

#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

namespace Ui {
class StarSimulatorLoadLedData;
}

class StarSimulatorLoadLedData : public QDialog
{
    Q_OBJECT

public:
    explicit StarSimulatorLoadLedData(QWidget *parent = 0);
    ~StarSimulatorLoadLedData();

signals:
    void progressInfo(int value);
    void updateLastDir(QString lastDir);
    void warningMessage(QString caption, QString message);

public slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();
    void showMessage(const QString &caption, const QString &message);
    void setLastDir(const QString &lastDir);
    void loadLedData();

private:
    Ui::StarSimulatorLoadLedData *ui;
    QString inputPath;
    QString lastDir;
    bool stopThread;
};

#endif // STARSIMULATORLOADLEDDATA_H
