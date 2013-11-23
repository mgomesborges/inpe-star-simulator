#include "lsqnonlin.h"

LSqNonLin::LSqNonLin(QObject *parent) :
    QThread(parent)
{
    solution.resize(72,1);
    jacobianMatrix.resize(641,72);
    objectiveFunction.resize(641,1);
    minimumDigitalLevelByChannel.resize(1,72);

    initialized           = false;
    stopThread            = false;
    enabledToContinue     = false;
    x0Type                = x0Random;

    // Create seed for the random
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
}

LSqNonLin::~LSqNonLin()
{
    stop();
}

int LSqNonLin::randomInt(int low, int high)
{
    return qrand() % ((high + 1) - low) + low;
}

void LSqNonLin::stop()
{
    stopThread = true;
}

bool LSqNonLin::loadDerivates()
{
    if (initialized == false) {
        // Load LED Data
        int numberOfRows;
        int numberOfColumns;
        MatrixXd matrixWithChannelData;

        for (int channel = 25; channel <= 96; channel++) {
            msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
            if (stopThread == true) {
                emit info("");
                return false;
            }

            QString inFilePath = QDir::currentPath() + "/led_database/ch" + QString::number(channel) + ".led";
            QFile inFile(inFilePath);
            if (!inFile.exists()) {
                emit ledDataNotFound();
                emit info("");
                return false;
            }
            inFile.open(QIODevice::ReadOnly);
            QDataStream in(&inFile);
            in.setVersion(QDataStream::Qt_5_0);

            in >> numberOfRows;
            in >> numberOfColumns;

            matrixWithChannelData.resize(numberOfRows, numberOfColumns);

            for (int i = 0; i < numberOfRows; i++) {
                for (int j = 0; j < numberOfColumns; j++) {
                    in >> matrixWithChannelData(i,j);
                }
            }

            derivatives3DMatrix( channel - 25 ).resize(numberOfRows, numberOfColumns);
            derivatives3DMatrix( channel - 25 ) = matrixWithChannelData;
            inFile.close();

            // Record the Minimum Digital Level
            minimumDigitalLevelByChannel( channel - 25 ) = 4095 - (numberOfColumns -2);

            emit info(tr("Loading LED Data from channel %1 of 96").arg(channel));
        }
    }

    return true;
}

void LSqNonLin::setAlgorithm(int algorithm)
{
    chosenAlgorithm = algorithm;
}

void LSqNonLin::setx0Type(int x0SearchType, MatrixXi x)
{
    x0Type = x0SearchType;
    x0     = x;
}

void LSqNonLin::run()
{
    stopThread = false;

    if (chosenAlgorithm == leastSquareNonLinear) {
        if (loadDerivates() == true) {
            initialized = true;
        } else {
            emit finished();
            return;
        }
    }

    int stopCriteria;
    double alpha;
    double fxCurrent;
    double fxBest;
    MatrixXd I;
    MatrixXd temp;
    MatrixXd deltaND;
    MatrixXi xCurrent(72,1);
    MatrixXi xBest(72,1);

    // Initial Solution
    if (x0Type == x0Random) {
        for (int row = 0; row < xCurrent.rows(); row++) {
            xCurrent(row) = randomInt(0, 4095);
        }
    } else if ((x0Type == x0UserDefined) || (x0Type == x0Current)) {
        xCurrent = x0;
    }

    getObjectiveFunction(xCurrent);

    alpha        = 6.5;  // Empirical value
    fxBest       = 1e15; // Big number
    stopCriteria = 0;

    if (chosenAlgorithm == leastSquareNonLinear) {
        // Impose a bound constraint for minimum value
        for (int row = 0; row < xCurrent.rows(); row++) {
            if (xCurrent(row) < minimumDigitalLevelByChannel(row)){
                xCurrent(row) = minimumDigitalLevelByChannel(row);
            }
        }

        for (int i = 0; i < 1000; i++) {
            // getObjectiveFunction(xCurrent);
            jacobian(xCurrent);

            // Diagonal Matrix :: I = diag(diag((J'*J)));
            I    = jacobianMatrix.transpose() * jacobianMatrix;
            temp = I.diagonal();
            I    = temp.asDiagonal();

            // Levenberg Marquardt
            // deltaND = inv(J'*J + alpha * I) * J' * objectiveFunction;
            temp = (jacobianMatrix.transpose() * jacobianMatrix + alpha * I);
            deltaND = temp.inverse() * jacobianMatrix.transpose() * objectiveFunction;

            // xCurrent  = xCurrent - deltaND
            for (int row = 0; row < xCurrent.rows(); row++) {
                xCurrent(row) = round(xCurrent(row) - deltaND(row));

                // Impose a bound constraint for maximum value
                if (xCurrent(row) > 4095) {
                    xCurrent(row) = 4095;
                }

                // Impose a bound constraint for minimum value
                if (xCurrent(row) < minimumDigitalLevelByChannel(row)){
                    xCurrent(row) = minimumDigitalLevelByChannel(row);
                }
            }

            msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
            if (stopThread == true) {
                getObjectiveFunction(xBest); // Return best solution
                break;
            }

            // fxCurrent = sqrt( sum(objectiveFunction^2) );
            getObjectiveFunction(xCurrent); // Get fxCurrent with new xCurrent value
            temp       = objectiveFunction.array().pow(2);
            fxCurrent  = sqrt( temp.sum() );

            // Stop Criteria
            if (fxCurrent > fxBest) {
                stopCriteria++;

                if (stopCriteria == 20) {
                    getObjectiveFunction(xBest); // Update solution available in getSolution() method
                    emit info(tr("\n\nLeast Square Finished by Stop Criteria: f(x) = %1").arg(fxBest));
                    break;
                }
            } else {
                stopCriteria = 0;
                xBest        = xCurrent;
                fxBest       = fxCurrent;
            }

            emit info(tr("Iteration: %1\tf(x): %2\tf(x)_best: %3\tDamping factor: %4").arg(i).arg(fxCurrent).arg(fxBest).arg(alpha));
        }
    } else if (chosenAlgorithm == gradientDescent) {
        MatrixXi xWithDelta(72,1);
        double fxPrevious;
        int delta = 50;
        alpha     = 0.05;  // Empirical value

        for (int i = 0; i < 1000; i++) {
            for (int channel = 25; channel <= 96; channel++) {
                // getObjectiveFunction(xCurrent);

                // fxPrevious = sqrt( sum(objectiveFunction^2) );
                temp       = objectiveFunction.array().pow(2);
                fxPrevious = temp.sum();

                // xCurrent with delta
                xWithDelta = xCurrent;

                // Prevents conversion problems
                if ((xWithDelta(channel - 25) + delta) > 4095) {
                    delta = -delta;
                } else {
                    delta = abs(delta);
                }

                xWithDelta(channel - 25) = xWithDelta(channel - 25) + delta;

                // Impose a bounds constraints
                for (int row = 0; row < xWithDelta.rows(); row++) {
                    // Impose a bound constraint for maximum value
                    if (xWithDelta(row) > 4095) {
                        xWithDelta(row) = 4095;
                    }

                    // Impose a bound constraint for minimum value
                    if (xWithDelta(row) < 0){
                        xWithDelta(row) = 0;
                    }
                }

                getObjectiveFunction(xWithDelta);

                // fxCurrent = sqrt( sum(objectiveFunction^2) );
                temp       = objectiveFunction.array().pow(2);
                fxCurrent  = temp.sum();

                // Calculates new value for xCurrent
                xCurrent(channel - 25) = round(xCurrent(channel - 25) - alpha * (fxCurrent - fxPrevious) / delta);

                // Impose a bounds constraints
                for (int row = 0; row < xCurrent.rows(); row++) {
                    // Impose a bound constraint for maximum value
                    if (xCurrent(row) > 4095) {
                        xCurrent(row) = 4095;
                    }

                    // Impose a bound constraint for minimum value
                    if (xCurrent(row) < 0){
                        xCurrent(row) = 0;
                    }
                }

                msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
                if (stopThread == true) {
                    break;
                }

                getObjectiveFunction(xCurrent); // Get objectiveFunction with new xCurrent value
            }

            // fxCurrent = sqrt( sum(objectiveFunction^2) );
            temp      = objectiveFunction.array().pow(2);
            fxCurrent = sqrt(temp.sum());

            // Stop Criteria
            if (fxCurrent > fxBest) {
                stopCriteria++;

                if (stopCriteria == 10) {
                    getObjectiveFunction(xBest); // Update solution available in getSolution() method
                    emit info(tr("\n\nLeast Square Finished by Stop Criteria: f(x) = %1").arg(fxBest));
                    break;
                }
            } else {
                stopCriteria = 0;
                xBest        = xCurrent;
                fxBest       = fxCurrent;
            }

            emit info(tr("Iteration: %1\tf(x): %2\tf(x)_best: %3\tDamping factor: %4").arg(i).arg(fxCurrent).arg(fxBest).arg(alpha));

            if (stopThread == true) {
                getObjectiveFunction(xBest); // Return best solution
                break;
            }
        }
    }

    emit finished();
}

void LSqNonLin::jacobian(const MatrixXi &x)
{
    int column;
    for (int channel = 25; channel <= 96; channel++) {
        column = 4095 - x(channel - 25) + 1; // First column contains the wavelength
        jacobianMatrix.col(channel - 25) = derivatives3DMatrix(channel - 25).col(column);
    }
}

void LSqNonLin::getObjectiveFunction(const MatrixXi &x)
{
    // Get Objective Function
    solution          = x;
    enabledToContinue = false;
    emit performScan();

    // Wait SMS500 performs the scan
    while (enabledToContinue == false) {
        msleep(1);
    }
}

void LSqNonLin::setObjectiveFunction(const MatrixXd &value)
{
    objectiveFunction = value;
    enabledToContinue = true;
}

MatrixXi LSqNonLin::getSolution()
{
    return solution;
}


