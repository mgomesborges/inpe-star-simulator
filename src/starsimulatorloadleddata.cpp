#include "starsimulatorloadleddata.h"
#include "ui_starsimulatorloadleddata.h"

StarSimulatorLoadLedData::StarSimulatorLoadLedData(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StarSimulatorLoadLedData)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    ui->label->setText(tr("LED Data not found: you need inform the path to the measured LED Data.\n\n"
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

    // Derivative Step is negative because the digital level is from 4095 to 0
    signed int derivativeStep = -1;

    for (int channel = 1; channel <= 96; channel++) {
        if (stopThread == true)
            return;

        emit progressInfo(channel);

        if (!file->open(tr("Load LED Data"), inputPath + "/ch" + QString::number(channel) + ".txt"))
            return;

        QVector< QVector<double> > matrix = file->data("[SMS500InterpolatedData]");

        if (matrix.isEmpty())
            return;

        MatrixXd channelData = Utils::qvector2eigen(matrix);
        MatrixXd derivatives;

        // Prevents errors with columns == 2 OR channels without data
        if (channelData.cols() <= 2)
            derivatives.resize(channelData.rows(), 2);
        else
            derivatives.resize(channelData.rows(), channelData.cols() - 1);

        derivatives.col(0) = channelData.col(0); // Column 0 contains wavelengths values
        derivatives.col(1) = MatrixXd::Constant(channelData.rows(), 1, 1); // Column of ones

        // Computing Derivatives
        for (int column = 1; column < channelData.cols() - 1; column++)
            derivatives.col(column) = (channelData.col(column + 1) - channelData.col(column)) / derivativeStep;

        file->save(Utils::eigen2QVector(derivatives), tr("Load LED Data"), QDir::currentPath() + "/led_database/ch" + QString::number(channel) + ".led");
    }

    emit warningMessage(tr("Load LED Data"), tr("Procedure successfully completed."));
}

void StarSimulatorLoadLedData::setLastDir(const QString &lastDir)
{
    this->lastDir = lastDir;
    emit updateLastDir(lastDir);
}
