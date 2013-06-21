#include "loadleddatathread.h"

#include <QDebug>
#include <QFileDialog>
#include <fstream>

LoadLedDataThread::LoadLedDataThread(QObject *parent) :
    QThread(parent)
{
}

const Matrix<double, Dynamic, Dynamic> &LoadLedDataThread::channelData(int channel)
{
    return vectorXM( channel );
}

void LoadLedDataThread::run()
{
    using namespace std;

    int numberOfRows;
    int numberOfColumns;
    Matrix<double, Dynamic, Dynamic> matrix;

    for (int channel = 25; channel <= 96; channel++) {
        if (channel != 71) {
            emit info(tr("Loading channel %1 of 96").arg(channel));

            QString filePath = QDir::currentPath() + "/led_data/ch" + QString::number(channel) + "_interpolado.txt";

            QFile file(filePath);
            file.open(QIODevice::ReadOnly);

            QTextStream in(&file);

            QString line       = in.readLine();
            QStringList fields = line.split("\t");
            numberOfRows       = fields.at(0).toInt();
            numberOfColumns    = fields.at(1).toInt();

            vectorXM( channel - 25 ).resize(numberOfRows, numberOfColumns);
            matrix.resize(numberOfRows, numberOfColumns);

            for (int i = 0; i < numberOfRows; i++) {
                line   = in.readLine();
                fields = line.split("\t");

                for (int j = 0; j < numberOfColumns; j++) {
                    matrix(i,j) = fields.at(j).toDouble();
                }
            }

            vectorXM( channel - 25 ) = matrix;
            file.close();
        }
    }
}
