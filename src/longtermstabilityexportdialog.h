#ifndef LONGTERMSTABILITYEXPORTDIALOG_H
#define LONGTERMSTABILITYEXPORTDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>

#include "longtermstability.h"

namespace Ui {
class LongTermStabilityExportDialog;
}

class LongTermStabilityExportDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LongTermStabilityExportDialog(QWidget *parent = 0,
                                           LongTermStability *longTermStability = NULL);
    ~LongTermStabilityExportDialog();

public slots:
    void on_btnOk_clicked();
    void on_btnCancel_clicked();
    void error(QString message);
    void processFinished();
    bool performed();
    
private:
    Ui::LongTermStabilityExportDialog *ui;
    LongTermStability *lts;
    bool status;
};

#endif // LONGTERMSTABILITYEXPORTDIALOG_H
