#ifndef LSQSTARDATADIALOG_H
#define LSQSTARDATADIALOG_H

#include <QDialog>
#include <cmath>

using namespace std;

namespace Ui {
class LSqStarDataDialog;
}

class LSqStarDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LSqStarDataDialog(QWidget *parent = 0);
    ~LSqStarDataDialog();

    int magnitude();
    int temperature();
    double peak();
    void setMagnitude(const QString& magnitude) const;
    void setTemperature(const QString& temperature) const;

    QVector< QVector<double> > spectralData();
    QPolygonF spectralDataToPlot();

private:
    Ui::LSqStarDataDialog *ui;
    double starPeak;
};

#endif // LSQSTARDATA_H
