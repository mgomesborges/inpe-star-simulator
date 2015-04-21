#include "starsimulatorloadleddata.h"
#include "ui_starsimulatorloadleddata.h"

StarSimulatorLoadLedData::StarSimulatorLoadLedData(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StarSimulatorLoadLedData)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    ui->label->setText(tr("You need inform the path to the measured LED Data.\n\n"
                          "This procedure is necessary to load the measured LED Data and compute the\n"
                          "derivatives for all channels. This may take several minutes to run.\n\n"
                          "To continue, click on the Ok button."));

    lastDir = QDir::homePath();
}

StarSimulatorLoadLedData::~StarSimulatorLoadLedData()
{
    delete ui;
}

void StarSimulatorLoadLedData::on_btnOk_clicked()
{
    inputPath = QFileDialog::getExistingDirectory(this, tr("Choose the directory have LED Modeling Data"), lastDir);

    if (inputPath.isEmpty())
        return;

    setLastDir(inputPath);

    ui->progressBar->setMinimum(1);
    ui->progressBar->setMaximum(96);
    ui->progressBar->setVisible(true);
    ui->btnOk->setVisible(false);
    ui->label->setText(tr("This procedure is necessary to load the measured LED Data and compute the\n"
                          "derivatives for all channels. This may take several minutes to run.\n\n"
                          "To cancel, click on the Cancel button."));

    connect(this, SIGNAL(progressInfo(int)), ui->progressBar, SLOT(setValue(int)));
    connect(this, SIGNAL(warningMessage(QString,QString)), this, SLOT(showMessage(QString,QString)));

    QtConcurrent::run(this, &StarSimulatorLoadLedData::loadLedData);
}

void StarSimulatorLoadLedData::on_btnCancel_clicked()
{
    stopThread = true;
    this->close();
}

void StarSimulatorLoadLedData::showMessage(const QString &caption, const QString &message)
{
    this->close();
    QMessageBox::warning(0, caption, message);
}

void StarSimulatorLoadLedData::loadLedData()
{
    FileHandle *file = new FileHandle();
    connect(file, SIGNAL(warningMessage(QString,QString)), this, SLOT(showMessage(QString,QString)));
    connect(file, SIGNAL(updateLastDir(QString)), this, SLOT(setLastDir(QString)));

    stopThread = false;

    for (int channel = 1; channel <= 96; channel++) {
        if (stopThread == true)
            return;

        emit progressInfo(channel);

        QString inputFile(inputPath + "/ch" + QString::number(channel) + ".txt");
        QString outputFile(QDir::currentPath() + "/led_database/ch" + QString::number(channel) + ".txt");

        if (!file->open(tr("Load LED Data"), inputFile))
            return;

        QVector< QVector<double> > matrix = file->data("[SMS500Data]");

        if (matrix.isEmpty())
            return;

        // Guarantees copy with replacement
        if (QFile::exists(outputFile))
            QFile::remove(outputFile);

        QFile::copy(inputFile, outputFile);
    }

    emit warningMessage(tr("Load LED Data"), tr("Procedure successfully completed."));
}

void StarSimulatorLoadLedData::setLastDir(const QString &lastDir)
{
    this->lastDir = lastDir;
    emit updateLastDir(lastDir);
}
