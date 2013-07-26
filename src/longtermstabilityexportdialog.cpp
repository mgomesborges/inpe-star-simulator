#include "longtermstabilityexportdialog.h"
#include "ui_longtermstabilityexportdialog.h"

LongTermStabilityExportDialog::LongTermStabilityExportDialog(QWidget *parent, LongTermStability *longTermStability) :
    QDialog(parent),
    ui(new Ui::LongTermStabilityExportDialog)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    ui->label->setText(tr("This may take several minutes to run if there is too much\n"
                          "data in the database.\n\n"
                          "To continue, click on the Ok button."));
    lts    = longTermStability;
    status = false;;
}

LongTermStabilityExportDialog::~LongTermStabilityExportDialog()
{
    delete ui;
}

void LongTermStabilityExportDialog::on_btnOk_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(
                this,
                tr("Long Term Stability :: Export Data"),
                QDir::homePath(),
                tr("Text File *.txt"));

    if (filePath.isEmpty()) {
        status = false;
        return;
    }

    // Checks if exist extension '.txt'
    if (filePath.contains(".txt")) {
        filePath.replace(".txt", "");
    }

    ui->progressBar->setVisible(true);
    ui->label->setText(tr("This may take several minutes to run.\n\n"
                          "To cancel, click on the Cancel button."));
    ui->btnOk->setVisible(false);

    connect(lts, SIGNAL(progressMinimumInfo(int)), this->ui->progressBar, SLOT(setMinimum(int)));
    connect(lts, SIGNAL(progressMaximumInfo(int)), this->ui->progressBar, SLOT(setMaximum(int)));
    connect(lts, SIGNAL(progressInfo(int)), this->ui->progressBar, SLOT(setValue(int)));
    connect(lts, SIGNAL(error(QString)), this, SLOT(error(QString)));
    connect(lts, SIGNAL(finished()), this, SLOT(processFinished()));
    lts->exportData(filePath);
}

void LongTermStabilityExportDialog::on_btnCancel_clicked()
{
    if (lts != NULL) {
        lts->stop(); // Stop thread
        lts->wait(); // Wait thread finishes to delete this
    }

    status = false;
    this->close();
}

void LongTermStabilityExportDialog::error(QString message)
{
    status = false;
    this->close();
    QMessageBox::warning(this, tr("Export Data"), message);
}

void LongTermStabilityExportDialog::processFinished()
{
    if (lts->status() == true) {
        status = true;
        this->close();
        QMessageBox::information(this, tr("Export Data"), tr("Procedure successfully completed."));

    } else {
        status = false;
        this->close();
    }
}

bool LongTermStabilityExportDialog::performed()
{
    return status;
}
