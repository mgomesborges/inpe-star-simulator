#include "starsimulator.h"

StarSimulator::StarSimulator(QObject *parent) :
    QThread(parent)
{
    activeChannels.resize(96, 1);
    solution.resize(96, 1);
    objectiveFunction.resize(641, 1);
    modelingStep.resize(96, 1);
    derivatives3DMatrix.resize(1, 96);

    status            = STOPPED;
    iteration         = 0;
    fxCurrent         = 0;
    isGDInitialized   = false;
    isLMInitialized   = false;
    stopThread        = false;
    enabledToContinue = false;
    enablePlot        = true;
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

void StarSimulator::setSettings(StarSimulatorParameters parameters)
{
    settings = parameters;
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
        if (matrix(row) < 0)
            matrix(row) = 0;
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

uint StarSimulator::iterationNumber()
{
    return iteration;
}

double StarSimulator::fx()
{
    return fxCurrent;
}

bool StarSimulator::enableUpdatePlot()
{
    return enablePlot;
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

    MatrixXi xCurrent(numberOfValidChannels, 1); // Current solution
    MatrixXi xBest(numberOfValidChannels, 1);    // Best solution
    MatrixXd deltaX;
    MatrixXd I;

    double fxBest     = 1e15; // Big number
    double fxPrevious = 0;
    int stopCriteria  = 0;

    /*************************************************************************
     * Initial Solution: x0
     ************************************************************************/
    if (x0Type == x0Random) {
        for (int row = 0; row < xCurrent.rows(); row++)
            xCurrent(row) = randomInt(0, 4095);
    } else if ((x0Type == x0UserDefined) || (x0Type == x0Current)) {
        for (int i = 0; i < xCurrent.rows(); i++)
            xCurrent(i) = x0(activeChannels(i));
    }

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

            emit info("=========================================================="
                      "<pre>\n              Levenberg Marquardt Algorithm\n</pre>"
                      "==========================================================");

            for (uint i = 0; i < 1000000 && stopThread == false; i++) {
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

                emit info(tr("<pre>It %1    f(x):<font color='#ff0000'> %2</font></pre>").arg(i, 3, 'f', 0, '0').arg(fxCurrent, 0, 'e', 4));

                // Check stopping criterion
                if (fxCurrent < fxBest) {
                    fxBest       = fxCurrent;
                    xBest        = xCurrent;
                    stopCriteria = 0;
                } else {
                    stopCriteria++;

                    if (stopCriteria == settings.lmMaxIteration) {
                        fxCurrent = fxBest;
                        getObjectiveFunction(xBest);
                        emit info(tr("<pre>\n\nLeast Square Finished by Stop Criteria: f(x) = <font color='#ff0000'>%1</font>\n\n</pre>").arg(fxCurrent, 0, 'e', 4));
                        break;
                    }
                }

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
                deltaX = (jacobianMatrix.transpose() * jacobianMatrix + settings.lmDampingFactor * I).inverse() *
                          jacobianMatrix.transpose() * objectiveFunction;

                /****************************************************
                 * Update xCurrent (Current solution)
                 ****************************************************/
                xCurrent = xWithConstraint(xCurrent - (deltaX).cast<int>());
            }

       /****************************************************
        * Gradient Descent
        ****************************************************/
        } else if (chosenAlgorithm == gradientDescent) {

            MatrixXi xUpdated         = xCurrent;
            double fxInternalPrevious = 0;
            double fxInternalCurrent  = 0;
            int delta                 = 50;

            emit info("=========================================================="
                      "<pre>\n                 Gradient Descent Algorithm\n</pre>"
                      "==========================================================");

            for (uint i = 0; i < 1000000 && stopThread == false; i++) {
                iteration++;

                xCurrent   = xWithConstraint(xUpdated);
                getObjectiveFunction(xCurrent);

                /****************************************************
                 * Update fxCurrent, fxPrevious and fxInternalPrevious
                 *
                 * fxInternalPrevious = sum(objectiveFunction^2);
                 * fxCurrent          = sqrt( sum(objectiveFunction^2) );
                 ****************************************************/
                fxPrevious         = fxCurrent;
                fxInternalPrevious = (objectiveFunction.array().pow(2)).sum();
                fxCurrent          = sqrt(fxInternalPrevious);

                // Check stopping criterion
                if (fxCurrent < fxBest) {
                    fxBest       = fxCurrent;
                    xBest        = xCurrent;
                    stopCriteria = 0;
                } else {
                    stopCriteria++;

                    if (stopCriteria == settings.gdMaxIteration) {
                        fxCurrent = fxBest;
                        getObjectiveFunction(xBest);
                        emit info(tr("<pre>\n\nGradient Descent Finished by Stop Criteria: f(x) = <font color='#ff0000'>%1</font>\n\n</pre>").arg(fxCurrent, 0, 'e', 4));
                        break;
                    }
                }

                /****************************************************
                 * Computing derivatives for each channel
                 ****************************************************/
                enablePlot = false;

                for (int channel = 0; channel < numberOfValidChannels; channel++) {
                    emit info(tr("<pre>It %1    f(x):<font color='#ff0000'> %2</font>    f(x)_previous:<font color='#ff0000'> %3</font>    [&part;ch<font color='#ff0000'>%4</font>]</pre>").arg(i, 3, 'f', 0, '0').arg(fxCurrent, 0, 'e', 4).arg(fxPrevious, 0, 'e', 4).arg(activeChannels(channel) + 1, 2, 'f', 0, '0'));

                    /****************************************************
                     * Get Objective Function with xCurrent + delta
                     ****************************************************/
                    xCurrent(channel) = xCurrent(channel) + delta;
                    getObjectiveFunction(xWithConstraint(xCurrent));
                    xCurrent(channel) = xCurrent(channel) - delta;

                    msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
                    if (stopThread == true) {
                        enablePlot = true;
                        getObjectiveFunction(xWithConstraint(xCurrent));
                        break;
                    }

                   /****************************************************
                    * Updated fxInternalCurrent
                    * fxInternalCurrent = sum(objectiveFunction^2);
                    ****************************************************/
                    fxInternalCurrent = (objectiveFunction.array().pow(2)).sum();

                   /****************************************************
                    * Update xUpdated (new solution)
                    ****************************************************/
                    xUpdated(channel) = round(xCurrent(channel) - settings.gdDampingFactor * (fxInternalCurrent - fxInternalPrevious) / delta);
                }

                emit info("==========================================================");

                enablePlot = true;
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

        emit info("=========================================================="
                  "<pre>\n                 Stabilization routine\n</pre>"
                  "==========================================================");


        while (stopThread == false) {
            secondMedia = media(10);

            emit info(tr("<pre>Min:<font color='#ff0000'>%1</font>    &ge;    %2    &le;    Max: <font color='#ff0000'>%3</font></pre>").arg(min, 0, 'e', 4).arg(secondMedia, 0, 'e', 4).arg(max, 0, 'e', 4));

            if (secondMedia < min || secondMedia > max)
                break;
        }
    }

    status = STOPPED;
    emit finished();
}

bool StarSimulator::loadDerivates()
{
    if (isLMInitialized == false) {

        status = LOAD_DERIVATIVES;
        numberOfValidChannels = 0;

        int derivativeStep;

        for (int channel = 0; channel < 96; channel++) {
            msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
            if (stopThread == true)
                return false;

            FileHandle *file = new FileHandle();

            if (!file->open(tr("Load LED Data"), QDir::currentPath() + "/led_database/ch" + QString::number(channel + 1) + ".txt")) {
                emit ledDataNotFound();
                return false;
            }

            QVector< QVector<double> > matrix = file->data("[SMS500Data]");
            QVector< QVector<double> > config = file->data("[LEDModelingConfiguration]");

            if (matrix.isEmpty() || config.isEmpty()) {
                emit ledDataNotFound();
                return false;
            }

            if (matrix[0].size() > 2) {
                modelingStep(numberOfValidChannels) = config[0][1];

                // LED Modeling Configuration
                if (config[0][0] == 0)
                    // If the digital level is from 0 to 4095 (see modeling type)
                    derivativeStep = config[0][1];
                else
                    // If the digital level is from 4095 to 0 (see modeling type)
                    derivativeStep = - config[0][1];

                MatrixXd channelData = Utils::qvector2eigen(matrix);
                MatrixXd derivatives;

                derivatives.resize(channelData.rows(), channelData.cols() - 1);
                derivatives.col(0) = channelData.col(0); // Column 0 contains wavelengths values

                // Computing Derivatives
                for (int column = 1; column < channelData.cols() - 1; column++)
                    derivatives.col(column) = (channelData.col(column + 1) - channelData.col(column)) / derivativeStep;

                activeChannels(numberOfValidChannels)      = channel;
                derivatives3DMatrix(numberOfValidChannels) = derivatives;
                numberOfValidChannels++;
            }

            emit info(tr("<pre>Loading LED Data from channel  <font color='#ff0000'><b>%1</b></font>  of  <font color='#ff0000'><b>96</b></font></pre>").arg(channel + 1, 2, 'f', 0, '0'));
        }

        isLMInitialized = true;
        activeChannels.conservativeResize(numberOfValidChannels, 1);
        jacobianMatrix.conservativeResize(641, numberOfValidChannels);
        derivatives3DMatrix.conservativeResize(1, numberOfValidChannels);
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

        // value += sqrt( sum(objectiveFunction^2) );
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
     * wavelength.
     *************************************************************************/

    for (int i = 0; i < numberOfValidChannels; i++)
    {
        int column = (int)((4095 - x(i)) / modelingStep(i) + 1);

        if (column >= derivatives3DMatrix(i).cols())
            column = derivatives3DMatrix(i).cols() - 1;

        jacobianMatrix.col(i) = derivatives3DMatrix(i).col(column);
    }
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
