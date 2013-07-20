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
    void setMagnitude(const QString& magnitude) const;
    void setTemperature(const QString& temperature) const;

    vector< vector<double> > getData();

private:
    Ui::LSqStarDataDialog *ui;
};

#endif // LSQSTARDATA_H
