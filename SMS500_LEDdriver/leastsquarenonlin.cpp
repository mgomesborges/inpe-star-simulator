#include "leastsquarenonlin.h"

LeastSquareNonLin::LeastSquareNonLin(QObject *parent) :
    QThread(parent)
{
    initialized = false;
    stopThread  = false;
}

LeastSquareNonLin::~LeastSquareNonLin()
{
    stop();
}

void LeastSquareNonLin::run()
{
    if (initialized == false) {
        // Load LED Data
        int index;
        int numberOfRows;
        int numberOfColumns;
        Matrix<double, Dynamic, Dynamic> matrix;

        for (int channel = 25; channel <= 96; channel++) {
            if (channel != 71) {
                emit info(tr("Loading channel %1 of 96").arg(channel));

                QString filePath = QDir::currentPath() + "/channel_data/ch" + QString::number(channel) + ".txt";

                QFile file(filePath);
                file.open(QIODevice::ReadOnly);

                QTextStream in(&file);

                QString line       = in.readLine();
                QStringList fields = line.split("\t");
                numberOfRows       = fields.at(0).toInt();
                numberOfColumns    = fields.at(1).toInt();

                // corrects missing channel
                if (channel < 71) {
                    index = channel - 25;
                } else {
                    index = channel - 26;
                }

                derivatives( index ).resize(numberOfRows, numberOfColumns);
                matrix.resize(numberOfRows, numberOfColumns);

                for (int i = 0; i < numberOfRows; i++) {
                    line   = in.readLine();
                    fields = line.split("\t");

                    for (int j = 0; j < numberOfColumns; j++) {
                        matrix(i,j) = fields.at(j).toDouble();
                    }
                }

                derivatives( index ) = matrix;
                file.close();

                // Minimum Digital Level
                minimumDigitalLevel( index ) = 4095 - matrix.cols() -1;
            }

            if (stopThread == true) {
                return;
            }
        }

        initialized = true;
    }

    // Initial Solution
    for (int i = 0; i < 71; i++) {
        x(i) = 2000;
    }

    Matrix<double, Dynamic, Dynamic> I;
    Matrix<double, Dynamic, Dynamic> temp;

    double alpha = 0.000001;

    for (int i = 0; i < 500; i++) {
        jacobian(x);

        // Diagonal Matrix
        I = J.transpose() * J;
        temp = I.diagonal();
        I = temp.asDiagonal();

        // Objective Function
        emit performScan();
        enabledToContinue = false;
        while ( enabledToContinue == false ) {
            qDebug() << "Waiting";
        }

        temp = (J.transpose() * J + alpha * I);
        temp = temp.inverse() * J.transpose() * objectiveFunction;

        for (int row = 0; row < x.rows(); row++) {
            x(row) = x(row) + round(temp(row));

            if (x(row) > 4095) {
                x(row) = 4095;
            }

            if (x(row) < 0) {
                x(row) = 0;
            }
        }

        if (stopThread == true) {
            return;
        }
    }
}

void LeastSquareNonLin::jacobian(Matrix<int, 71, 1> x0)
{
    Matrix<double, Dynamic, Dynamic> dx;
    double levelValue;

    for (int channel = 0; channel < 71; channel++) {
        dx = derivatives( channel );
        levelValue = round( x0(channel) );

        if (levelValue > 4095) {
            levelValue = 4095;
        }

        if (levelValue < minimumDigitalLevel(channel)){
            levelValue = minimumDigitalLevel(channel);
        }

        // Checks if there power for digital level of the LED
        // There are power for digital level of the LED
        if(dx.cols() > (4095 - levelValue + 1)) {
            J.col(channel) = dx.col(4095 - levelValue + 1);
        } else { // There are not power for digital level of the LED
            J.col(channel) = dx.col(dx.cols() - 1);
        }
    }

//    QString filePath = QDir::currentPath() + "/J.txt";
//    ofstream outFile( filePath.toAscii().data(), ios::out );

//    for (int i = 0; i < J.rows(); i++) {
//        for (int j = 0; j < J.cols(); j++) {
//            outFile << J(i,j) << "\t";
//        }
//        outFile << "\n";
//    }
}

void LeastSquareNonLin::setObjectiveFunction(Matrix<double, Dynamic, Dynamic> value)
{
    objectiveFunction = value;
    enabledToContinue = true;
}

Matrix<int, 71, 1> LeastSquareNonLin::getSolution()
{
    return x;
}

void LeastSquareNonLin::stop()
{
    stopThread = true;
}
