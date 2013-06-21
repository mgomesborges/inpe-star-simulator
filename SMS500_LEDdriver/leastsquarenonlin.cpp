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
        xCurrent(i) = 2000;
    }

    Matrix<double, Dynamic, Dynamic> I;
    Matrix<double, Dynamic, Dynamic> temp;
    Matrix<int, 71, 1> xPrevious;
    double fxPrevious;
    double fxCurrent;
    int stopCriteria = 0;
    double alpha = getAlpha( 0 );

    for (int i = 0; i < 500; i++) {
        // Objective Function
        emit performScan();
        enabledToContinue = false;
        while ( enabledToContinue == false ) {
            qDebug() << "Waiting";
        }

        temp       = objectiveFunction.array().pow(2);
        fxPrevious = sqrt( temp.sum() );

        jacobian(xCurrent);

        // Diagonal Matrix
        I = J.transpose() * J;
        temp = I.diagonal();
        I = temp.asDiagonal();

        temp = (J.transpose() * J + alpha * I);
        temp = temp.inverse() * J.transpose() * objectiveFunction;

        // Updates the value of x
        xPrevious  = xCurrent;

        for (int row = 0; row < xCurrent.rows(); row++) {
            xCurrent(row) = xCurrent(row) + round(temp(row));

            // Impose a bounds constraints
            if (xCurrent(row) > 4095) {
                xCurrent(row) = 4095;
            }

            if (xCurrent(row) < 0) {
                xCurrent(row) = 0;
            }
        }

        // Objective Function
        emit performScan();
        enabledToContinue = false;
        while ( enabledToContinue == false ) {
            qDebug() << "Waiting";
        }

        temp       = objectiveFunction.array().pow(2);
        fxCurrent  = sqrt( temp.sum() );

        if (fxCurrent >= fxPrevious) {
            xCurrent = xPrevious;
            alpha    = getAlpha( fxPrevious );
            stopCriteria++;

            if (stopCriteria == 3) {
                emit info(tr("Least Square Finished"));
                return;
            }
        } else {
            stopCriteria = 0;
            emit info(tr("f(x): %1").arg(fxCurrent));
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
    return xCurrent;
}

double LeastSquareNonLin::getAlpha(double bestFx)
{
    Matrix<double, Dynamic, Dynamic> I;
    Matrix<double, Dynamic, Dynamic> temp;
    Matrix<int, 71, 1> xBackup;
    Matrix<int, 71, 1> x0;
    Matrix<int, 71, 1> x1;
    double fx0;
    double fx1;
    double alpha = 1e-1;
    double alphaPrevious;

    xBackup = xCurrent;

    while (true) {
        // Objective Function
        emit performScan();
        enabledToContinue = false;
        while ( enabledToContinue == false ) {
            qDebug() << "Waiting";
        }

        // x0 --------------------------------
        jacobian(xCurrent);

        // Diagonal Matrix
        I = J.transpose() * J;
        temp = I.diagonal();
        I = temp.asDiagonal();

        temp = (J.transpose() * J + alpha * I);
        temp = temp.inverse() * J.transpose() * objectiveFunction;

        // Updates the value of x0
        for (int row = 0; row < x0.rows(); row++) {
            x0(row) = xCurrent(row) + round(temp(row));

            // Impose a bounds constraints
            if (x0(row) > 4095) {
                x0(row) = 4095;
            }

            if (x0(row) < 0) {
                x0(row) = 0;
            }
        }

        // New value for alpha
        alphaPrevious = alpha;
        alpha         = alpha + 1e-1;

        // x1 --------------------------------
        temp = (J.transpose() * J + alpha * I);
        temp = temp.inverse() * J.transpose() * objectiveFunction;

        // Updates the value of x1
        for (int row = 0; row < x1.rows(); row++) {
            x1(row) = xCurrent(row) + round(temp(row));

            // Impose a bounds constraints
            if (x1(row) > 4095) {
                x1(row) = 4095;
            }

            if (x1(row) < 0) {
                x1(row) = 0;
            }
        }

        // fx0 --------------------------------
        xCurrent = x0;

        // Objective Function
        emit performScan();
        enabledToContinue = false;
        while ( enabledToContinue == false ) {
            qDebug() << "Waiting";
        }

        temp = objectiveFunction.array().pow(2);
        fx0  = sqrt( temp.sum() );

        // fx1 --------------------------------
        xCurrent = x1;

        // Objective Function
        emit performScan();
        enabledToContinue = false;
        while ( enabledToContinue == false ) {
            qDebug() << "Waiting";
        }

        temp = objectiveFunction.array().pow(2);
        fx1  = sqrt( temp.sum() );

        // Reset xCurrent
        xCurrent = xBackup;

        // Stopping criteria
        if (fx1 > fx0) {
            return alphaPrevious;
        }

        emit info(tr("f(x): %1 :: Getting Alpha: f(x): %2, alpha: %3").arg(bestFx).arg(fx1).arg(alpha));
    }
}

void LeastSquareNonLin::stop()
{
    stopThread = true;
}
