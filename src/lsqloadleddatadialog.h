#ifndef LSQLOADLEDDATADIALOG_H
#define LSQLOADLEDDATADIALOG_H

#include <QDialog>
#include <QMessageBox>
#include "lsqloadleddata.h"

namespace Ui {
class LSqLoadLedDataDialog;
}

class LSqLoadLedDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LSqLoadLedDataDialog(QWidget *parent = 0);
    ~LSqLoadLedDataDialog();

public slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();
    void error(QString message);
    void processFinished();
    bool performed();

private:
    Ui::LSqLoadLedDataDialog *ui;
    LSqLoadLedData *loadData;
    bool status;
};

#endif // LSQLOADLEDDATADIALOG_H
