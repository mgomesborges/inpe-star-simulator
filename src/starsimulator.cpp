#include "starsimulator.h"

StarSimulator::StarSimulator(QObject *parent) :
    QThread(parent)
{
    activeChannels.resize(96, 1);
    solution.resize(96, 1);
    objectiveFunction.resize(641, 1);
    derivatives3DMatrix.resize(1, 96);
    minimumDigitalLevelByChannel.resize(1, 96);

    status            = STOPPED;
    iteration         = 0;
    fxCurrent         = 0;
    isGDInitialized   = false;
    isLsqInitialized  = false;
    stopThread        = false;
    enabledToContinue = false;
    x0Type            = x0Random;

    // Create seed for the random
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
}

StarSimulator::~StarSimulator()
{
    stop();
}

void StarSimulator::stop()
{
    stopThread = true;
    status     = STOPPED;
}

void StarSimulator::setObjectiveFunction(const MatrixXd &value)
{
    objectiveFunction = value;
    enabledToContinue = true;
}

void StarSimulator::setActiveChannels(const MatrixXi &activeChannels)
{
    this->activeChannels  = activeChannels;
    numberOfValidChannels = activeChannels.rows();
    isGDInitialized       = true;
}

MatrixXi StarSimulator::getSolution()
{
    return solution;
}

MatrixXi StarSimulator::xWithConstraint(const MatrixXi &x)
{
    MatrixXi matrix = x;

    for (int row = 0; row < matrix.rows(); row++) {
        // Impose a bound constraint for maximum value
        if (matrix(row) > 4095)
            matrix(row) = 4095;

        // Impose a bound constraint for minimum value
        if (chosenAlgorithm == leastSquareNonLinear) {
            if (matrix(row) < minimumDigitalLevelByChannel(row))
                matrix(row) = minimumDigitalLevelByChannel(row);
        } else {
            if (matrix(row) < 0)
                matrix(row) = 0;
        }
    }

    return matrix;
}

void StarSimulator::setAlgorithm(int algorithm)
{
    chosenAlgorithm = algorithm;
}

void StarSimulator::setx0Type(int x0SearchType, MatrixXi x)
{
    x0Type = x0SearchType;
    x0     = x;
}

int StarSimulator::algorithmStatus()
{
    return status;
}

int StarSimulator::iterationNumber()
{
    return iteration;
}

double StarSimulator::fx()
{
    return fxCurrent;
}

void StarSimulator::run()
{
    stopThread = false;
    iteration  = 0;
    fxCurrent  = 0;

    /*************************************************************************
     * The number of valid channels to Least Square are defined after the
     * loadDerivates() function.
     *
     * To the Gradient Descent, the number of valid channels need to be defined
     * through the setActiveChannels() function, before starting the algorithm.
     ************************************************************************/
    if (chosenAlgorithm == leastSquareNonLinear) {
        if (loadDerivates() == false) {
            emit finished();
            return;
        }
    } else if (chosenAlgorithm == gradientDescent) {
        if (isGDInitialized == false) {
            emit info(tr("Gradient Descent Algorithm was not initialized correctly."));
            emit finished();
            return;
        }
    }

    MatrixXi xCurrent(numberOfValidChannels, 1);  // Current solution
    MatrixXd deltaX;
    MatrixXd I;

    double fxPrevious = 0;
    double alpha      = 6.5;  // Empirical value
    int stopCriteria  = 0;

    /*************************************************************************
     * Initial Solution: x0
     ************************************************************************/
    if (x0Type == x0Random)
        for (int row = 0; row < xCurrent.rows(); row++)
            xCurrent(row) = randomInt(0, 4095);

    else if ((x0Type == x0UserDefined) || (x0Type == x0Current))
        for (int i = 0; i < xCurrent.rows(); i++)
            xCurrent(i) = x0(activeChannels(i));

    // Impose a bound constraint
    xCurrent = xWithConstraint(xCurrent);

    /*************************************************************************
     * Star Simulator Algorithm
     ************************************************************************/
    while (stopThread == false) {
        status = PERFORMING_FITING;

        /****************************************************
         * Least Square Non Linear :: Levenberg Marquardt (LM)
         ****************************************************/
        if (chosenAlgorithm == leastSquareNonLinear) {
            for (int i = 0; i < 1000 && stopThread == false; i++) {
                iteration++;

                getObjectiveFunction(xCurrent);

                msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
                if (stopThread == true)
                    break;

                /****************************************************
                 * Update fxCurrent and fxPrevious
                 * fxCurrent = sqrt( sum(objectiveFunction^2) );
                 ****************************************************/
                fxPrevious = fxCurrent;
                fxCurrent  = sqrt((objectiveFunction.array().pow(2)).sum());

                emit info(tr("Iteration: %1\tf(x): %2\tf(x)_previous: %3").arg(i).arg(fxCurrent).arg(fxPrevious));

                // Check stopping criterion
                if (fxCurrent > fxPrevious) {
                    stopCriteria++;

                    if (stopCriteria == 20) {
                        emit info(tr("\n\nLeast Square Finished by Stop Criteria: f(x) = %1").arg(fxCurrent));
                        break;
                    }
                } else
                    stopCriteria = 0;

                /****************************************************
                 * Create Jacobian Matrix (J)
                 ****************************************************/
                createJacobianMatrix(xCurrent);

                /****************************************************
                 * Diagonal Matrix :: I = diag(diag((J' * J)));
                 ****************************************************/
                I = ((jacobianMatrix.transpose() * jacobianMatrix).diagonal()).asDiagonal();

                /****************************************************
                 * deltaX = inv(J' * J + alpha * I) * J' * objectiveFunction;
                 ****************************************************/
                deltaX = (jacobianMatrix.transpose() * jacobianMatrix + alpha * I).inverse() *
                          jacobianMatrix.transpose() * objectiveFunction;

                /****************************************************
                 * Update xCurrent (Current solution)
                 ****************************************************/
                xCurrent  = xWithConstraint(xCurrent - (deltaX).cast<int>());
            }

       /****************************************************
        * Gradient Descent
        ****************************************************/
        } else if (chosenAlgorithm == gradientDescent) {

            int delta = 50;
            alpha     = 0.05;  // Empirical value

            for (int i = 0; i < 1000 && stopThread == false; i++) {
                iteration++;

                /****************************************************
                 * Computing derivatives for each channel
                 ****************************************************/
                for (int channel = 0; channel < numberOfValidChannels; channel++) {
                    /****************************************************
                     * Update DELTA value to prevents constraint problem
                     ****************************************************/
                    if ((xCurrent(channel) + delta) > 4095)
                        delta = - delta;
                    else
                        delta = abs(delta);

                    /****************************************************
                     * Get Objective Function with xCurrent + delta
                     ****************************************************/
                    xCurrent(channel) = xCurrent(channel) + delta;
                    getObjectiveFunction(xWithConstraint(xCurrent));
                    xCurrent(channel) = xCurrent(channel) - delta;

                    msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
                    if (stopThread == true)
                        break;

                   /****************************************************
                    * Update fxCurrent and fxPrevious
                    * fxCurrent = sum(objectiveFunction^2);
                    ****************************************************/
                   fxPrevious = fxCurrent;
                   fxCurrent  = (objectiveFunction.array().pow(2)).sum();

                   /****************************************************
                    * Update xCurrent (Current solution)
                    ****************************************************/
                    xCurrent(channel) = round(xCurrent(channel) - alpha * (fxCurrent - fxPrevious) / delta);
                    xCurrent = xWithConstraint(xCurrent);
                }

                /****************************************************
                 * Update fxCurrent and fxPrevious
                 * fxCurrent = sqrt( sum(objectiveFunction^2) );
                 ****************************************************/
                fxPrevious = sqrt(fxPrevious);
                fxCurrent  = sqrt(fxCurrent);

                // Stop Criteria
                if (fxCurrent > fxPrevious) {
                    stopCriteria++;

                    if (stopCriteria == 10) {
                        emit info(tr("\n\nLeast Square Finished by Stop Criteria: f(x) = %1").arg(fxCurrent));
                        break;
                    }
                } else
                    stopCriteria = 0;

                emit info(tr("Iteration: %1\tf(x): %2\tf(x)_previous: %3").arg(i).arg(fxCurrent).arg(fxPrevious));
            }
        }

        status = FITING_OK;

        /*************************************************************************
         * Stabilization routine
         ************************************************************************/
        double firstMedia = media(10);
        double secondMedia;

        double max = firstMedia * 1.01;
        double min = firstMedia * 0.99;

        while (stopThread == false) {
            secondMedia = media(10);

            if (secondMedia < min || secondMedia > max) {
                emit info(tr("Min: %1\t>= %2 <=\t\tMax: %3\t[>1%]").arg(min).arg(secondMedia).arg(max));
                break;
            } else
                emit info(tr("Min: %1\t>= %2 <=\t\tMax: %3").arg(min).arg(secondMedia).arg(max));
        }
    }

    status = STOPPED;
    emit finished();
}

bool StarSimulator::loadDerivates()
{
    if (isLsqInitialized == false) {
        int rows;
        int cols;

        status = LOAD_DERIVATIVES;
        numberOfValidChannels = 0;

        for (int channel = 0; channel < 96; channel++) {
            msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
            if (stopThread == true)
                return false;

            QFile inFile(QDir::currentPath() + "/led_database/ch" + QString::number(channel + 1) + ".led");
            if (inFile.open(QIODevice::ReadOnly) == false) {
                emit ledDataNotFound();
                return false;
            }

            emit info(tr("Loading LED Data from channel %1 of 96").arg(channel + 1));

            QDataStream in(&inFile);
            in.setVersion(QDataStream::Qt_5_0);
            in >> rows;
            in >> cols;

            // If channel ok, with correct data
            if (cols > 2) {
                MatrixXd matrixWithChannelData;
                matrixWithChannelData.resize(rows, cols);

                for (int i = 0; i < rows; i++)
                    for (int j = 0; j < cols; j++)
                        in >> matrixWithChannelData(i, j);

                inFile.close();

                activeChannels(numberOfValidChannels)      = channel;
                derivatives3DMatrix(numberOfValidChannels) = matrixWithChannelData;

                // Record the Minimum Digital Level
                minimumDigitalLevelByChannel(numberOfValidChannels) = 4095 - (cols - 2);
                numberOfValidChannels++;
            }
        }

        isLsqInitialized = true;
        activeChannels.conservativeResize(numberOfValidChannels, 1);
        jacobianMatrix.conservativeResize(641, numberOfValidChannels);
        derivatives3DMatrix.conservativeResize(1, numberOfValidChannels);
        minimumDigitalLevelByChannel.conservativeResize(1, numberOfValidChannels);
    }

    return true;
}

double StarSimulator::media(int qty)
{
    double value = 0;

    for (int i = 0; i < qty; i++) {
        // Get Objective Function
        enabledToContinue = false;
        emit performScan();

        while (enabledToContinue == false && stopThread == false) // Wait SMS500 performs the scan
            msleep(1);

        // value += sqrt(sum(objectiveFunction^2));
        value += sqrt((objectiveFunction.array().pow(2)).sum());
    }

    return (value / qty);
}

int StarSimulator::randomInt(int low, int high)
{
    return qrand() % ((high + 1) - low) + low;
}

void StarSimulator::createJacobianMatrix(const MatrixXi &x)
{
    /*************************************************************************
     * Each line of derivatives3DMatrix contains a specific LED data.
     *
     * Each column contains the derivatives of LED depending on
     * the digital level applied in the channel. First column contains the
     * wavelength, thus the specific column is calculated by:
     * Column = 4095 - x(i) + 1
     *************************************************************************/
    for (int i = 0; i < numberOfValidChannels; i++)
        jacobianMatrix.col(i) = derivatives3DMatrix(i).col(4095 - x(i) + 1);
}

void StarSimulator::getObjectiveFunction(const MatrixXi &x)
{
    /*************************************************************************
     * The Objective Function is calculated as the difference between the
     * obtained spectrum and desired spectrum.
     *
     * For this, the LED Driver is updated and the SMS500 is used to obtain
     * the current spectrum. The difference is calculated by main class
     * (MainWindow), and the objectiveFunction and enabledToContinue variables
     * are updated by StarSimulator::setObjectiveFunction() function.
     *************************************************************************/
    solution = MatrixXi::Zero(96, 1);

    for (int i = 0; i < numberOfValidChannels; i++)
        solution(activeChannels(i)) = x(i);

    enabledToContinue = false;
    emit performScanWithUpdate();

    while (enabledToContinue == false && stopThread == false) // Wait SMS500 performs the scan
        msleep(1);
}
