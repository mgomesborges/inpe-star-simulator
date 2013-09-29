#include "lsqnonlin.h"

LSqNonLin::LSqNonLin(QObject *parent) :
    QThread(parent)
{
    solution.resize(72,1);
    jacobianMatrix.resize(641,72);
    objectiveFunction.resize(641,1);
    minimumDigitalLevelByChannel.resize(1,72);

    initialized       = false;
    stopThread        = false;
    enabledToContinue = false;

    // Create seed for the random
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    resetToDefault();
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

void LSqNonLin::setx0Type(int x0SearchType, MatrixXi x)
{
    x0Type = x0SearchType;
    x0     = x;
}

void LSqNonLin::setDampingFactor(int type, double value)
{
    dampingFactorType = type;
    dampingFactor     = value;
}

void LSqNonLin::run()
{
    stopThread = false;

    if (loadDerivates() == true) {
        initialized = true;
    } else {
        resetToDefault();
        emit finished();
        return;
    }

    int stopCriteria;
    double alpha;
    double fxCurrent;
    double fxPrevious;
    MatrixXd I;
    MatrixXd temp;
    MatrixXd deltaND;
    MatrixXi xCurrent(72,1);
    MatrixXi xPrevious(72,1);

    // Initial Solution
    if (x0Type == x0Random) {
        for (int row = 0; row < xCurrent.rows(); row++) {
            xCurrent(row) = randomInt(0, 4095);
        }
    } else if (x0Type == x0UserDefined) {
        xCurrent = x0;
    } else { // x0GeneticAlgorithmSearch

        // Genetic Algorithm here!

    }

    // Impose a bound constraint for minimum value
    for (int row = 0; row < xCurrent.rows(); row++) {
        if (xCurrent(row) < minimumDigitalLevelByChannel(row)){
            xCurrent(row) = minimumDigitalLevelByChannel(row);
        }
    }

    stopCriteria = 0;

    if (dampingFactorType == dampingFactorFixed) {
        alpha = dampingFactor;
    } else if (dampingFactorTechnique1) {
        alpha = getAlpha( xCurrent );
    } else {
        // damping factor technique 2
    }

    for (int i = 0; i < 5000; i++) {
        getObjectiveFunction(xCurrent);
        jacobian(xCurrent);

        // Diagonal Matrix :: I = diag(diag((J'*J)));
        I    = jacobianMatrix.transpose() * jacobianMatrix;
        temp = I.diagonal();
        I    = temp.asDiagonal();

        // Levenberg Marquardt
        // deltaND = inv(J'*J + alpha * I) * J' * objectiveFunction;
        temp = (jacobianMatrix.transpose() * jacobianMatrix + alpha * I);
        deltaND = temp.inverse() * jacobianMatrix.transpose() * objectiveFunction;

        // Stores the value of xCurrent
        xPrevious  = xCurrent;

        // xCurrent  = xCurrent + deltaND
        for (int row = 0; row < xCurrent.rows(); row++) {
            xCurrent(row) = xCurrent(row) + round(deltaND(row));

            // Impose a bound constraint for maximum value
            if (xCurrent(row) > 4095) {
                xCurrent(row) = 4095;
            }

            // Impose a bound constraint for minimum value
            if (xCurrent(row) < minimumDigitalLevelByChannel(row)){
                xCurrent(row) = minimumDigitalLevelByChannel(row);
            }
        }

        // fxPrevious = sqrt( sum(objectiveFunction^2) );
        temp       = objectiveFunction.array().pow(2);
        fxPrevious = sqrt( temp.sum() );

        msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
        if (stopThread == true) {
            break;
        }

        // fxCurrent = sqrt( sum(objectiveFunction^2) );
        getObjectiveFunction(xCurrent); // Get fxCurrent with new xCurrent value
        temp       = objectiveFunction.array().pow(2);
        fxCurrent  = sqrt( temp.sum() );

        if (fxCurrent > fxPrevious) {
            xCurrent  = xPrevious;
            fxCurrent = fxPrevious;

            if (dampingFactorType == dampingFactorFixed) {
                alpha = dampingFactor;
            } else if (dampingFactorTechnique1) {
                alpha = getAlpha( xCurrent );
            } else {
                // damping factor technique 2
            }

            stopCriteria++;

            if (stopCriteria == 5) {
                emit info(tr("\n\nLeast Square Finished by Stop Criteria: f(x) = %1").arg(fxCurrent));
                break;
            }
        } else {
            stopCriteria = 0;
            emit info(tr("Iteration: %1\tf(x): %2\tAlpha: %3").arg(i).arg(fxCurrent).arg(alpha));
        }
    }

    emit finished();
}

void LSqNonLin::resetToDefault()
{
    x0Type            = x0Random;
    dampingFactorType = dampingFactorTechnique1;
}

void LSqNonLin::jacobian(const MatrixXi &x)
{
    int column;
    for (int channel = 25; channel <= 96; channel++) {
        column = 4095 - x(channel - 25) + 1;
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

double LSqNonLin::getAlpha(const MatrixXi &xCurrent)
{
    double fx0;
    double fx1;
    double alpha;
    double alphaPrevious;
    MatrixXd I;
    MatrixXd temp;
    MatrixXd deltaND;
    MatrixXi x0(72, 1);
    MatrixXi x1(72, 1);

    alpha  = 10;

    getObjectiveFunction(xCurrent);
    jacobian(xCurrent);

    // Diagonal Matrix :: I = diag(diag((J'*J)));
    I    = jacobianMatrix.transpose() * jacobianMatrix;
    temp = I.diagonal();
    I    = temp.asDiagonal();

    while (true) {
        msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
        if (stopThread == true) {
            return alpha;
        }

        // x0 --------------------------------
        // deltaND = inv(J'*J + alpha * I) * J' * objectiveFunction;
        temp = (jacobianMatrix.transpose() * jacobianMatrix + alpha * I);
        deltaND = temp.inverse() * jacobianMatrix.transpose() * objectiveFunction;

        // x0 = xCurrent + deltaND
        for (int row = 0; row < xCurrent.rows(); row++) {
            x0(row) = xCurrent(row) + round(deltaND(row));

            // Impose a bound constraint for maximum value
            if (x0(row) > 4095) {
                x0(row) = 4095;
            }

            // Impose a bound constraint for minimum value
            if (x0(row) < minimumDigitalLevelByChannel(row)){
                x0(row) = minimumDigitalLevelByChannel(row);
            }
        }

        // New value for alpha
        alphaPrevious = alpha;
        alpha         = alpha - (alpha * 0.30);

        // x1 --------------------------------
        // deltaND = inv(J'*J + alpha * I) * J' * objectiveFunction;
        temp = (jacobianMatrix.transpose() * jacobianMatrix + alpha * I);
        deltaND = temp.inverse() * jacobianMatrix.transpose() * objectiveFunction;

        // x1 = xCurrent + deltaND
        for (int row = 0; row < xCurrent.rows(); row++) {
            x1(row) = xCurrent(row) + round(deltaND(row));

            // Impose a bound constraint for maximum value
            if (x1(row) > 4095) {
                x1(row) = 4095;
            }

            // Impose a bound constraint for minimum value
            if (x1(row) < minimumDigitalLevelByChannel(row)){
                x1(row) = minimumDigitalLevelByChannel(row);
            }
        }

        // fx0 = sqrt( sum(objectiveFunction^2) );
        getObjectiveFunction(x0);
        temp = objectiveFunction.array().pow(2);
        fx0  = sqrt( temp.sum() );

        // fx1 = sqrt( sum(objectiveFunction^2) );
        getObjectiveFunction(x1);
        temp = objectiveFunction.array().pow(2);
        fx1  = sqrt( temp.sum() );

        emit info(tr("  Getting Alpha:\tfx0: %1\tfx1: %2\talpha: %3").arg(fx0).arg(fx1).arg(alpha));

        // Stopping condition
        if (fx1 > fx0) {

            return alphaPrevious;
        }
    }
}

