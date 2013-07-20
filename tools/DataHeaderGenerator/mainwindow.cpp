#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnRun, SIGNAL(clicked()), this, SLOT(headerGenerator()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::headerGenerator()
{
    inputPath = QFileDialog::getExistingDirectory(this, tr("Choose the directory have LED Modeling Data"), QDir::homePath());
    if (inputPath.isEmpty()) {
        return;
    }

    outputPath = QFileDialog::getExistingDirectory(this, tr("Choose the directory to save LED Modeling Data"), QDir::homePath());
    if (outputPath.isEmpty()) {
        return;
    }

    for (int channel = 25; channel <= 96; channel++) {
        QFile inFile( inputPath + "/ch" + QString::number(channel) + ".txt" );
        if (!inFile.exists()) {
            statusBar()->showMessage(tr("LEDs Data does not exist."));
            return;
        }
        inFile.open(QIODevice::ReadOnly);
        QTextStream in(&inFile);

        QFile outFile( outputPath + "/ch" + QString::number(channel) + ".txt");
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&outFile);

        QString line        = in.readLine();
        QStringList fields  = line.split("\t");
        int numberOfRows    = 641;
        int numberOfColumns = fields.size();

        out << numberOfRows << "\t" << numberOfColumns << "\n";
        out << line << "\n";

        for (int i = 1; i < 641; i++) {
            out << in.readLine() << "\n";
        }

        inFile.close();
        outFile.close();
    }

    statusBar()->showMessage(tr("Process completed successfully!"));
}
