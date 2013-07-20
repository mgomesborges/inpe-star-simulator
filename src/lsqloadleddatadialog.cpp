#include "lsqloadleddatadialog.h"
#include "ui_lsqloadleddatadialog.h"

LSqLoadLedDataDialog::LSqLoadLedDataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LSqLoadLedDataDialog)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    ui->label->setText(tr("LED Data no found: you need inform the path to the measured LED Data.\n\n"
                          "This procedure is necessary to load the measured LED Data and compute the\n"
                          "derivatives for all channels. This may take several minutes to run.\n\n"
                          "To continue, click on the Ok button."));
    status   = false;
    loadData = NULL;
}

LSqLoadLedDataDialog::~LSqLoadLedDataDialog()
{
    delete ui;
}

void LSqLoadLedDataDialog::on_btnOk_clicked()
{
    QString inputPath = QFileDialog::getExistingDirectory(this, tr("Choose the directory have LED Modeling Data"), QDir::homePath());
    if (inputPath.isEmpty()) {
        status = false;
        return;
    }
    ui->progressBar->setMinimum(25);
    ui->progressBar->setMaximum(96);
    ui->progressBar->setVisible(true);
    ui->btnOk->setVisible(false);
    ui->label->setText(tr("This procedure is necessary to load the measured LED Data and compute the\n"
                          "derivatives for all channels. This may take several minutes to run.\n\n"
                          "To cancel, click on the Cancel button."));
    loadData = new LSqLoadLedData(this, inputPath); // Instantiates LSqLoadLedData thread
    connect(loadData, SIGNAL(progressInfo(int)), this->ui->progressBar, SLOT(setValue(int)));
    connect(loadData, SIGNAL(finished()), this, SLOT(processFinished()));
    connect(loadData, SIGNAL(error(QString)), this, SLOT(error(QString)));
    loadData->start();
}

void LSqLoadLedDataDialog::on_btnCancel_clicked()
{
    if (loadData != NULL) {
        loadData->stop(); // Stop thread
        loadData->wait(); // Wait loadData thread finishes to delete this
    }
    status = false;
    this->close();
}

void LSqLoadLedDataDialog::error(QString message)
{
    status = false;
    this->close();
    QMessageBox::warning(this, tr("Load LED Data"), message);
}

void LSqLoadLedDataDialog::processFinished()
{
    status = true;
    this->close();
    QMessageBox::information(this, tr("Load LED Data"), tr("Procedure successfully completed."));
}

bool LSqLoadLedDataDialog::performed()
{
    return status;
}
