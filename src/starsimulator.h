#ifndef STARSIMULATOR_H
#define STARSIMULATOR_H

#include <QThread>
#include <QTime>
#include <QDir>

#include "datatype.h"

#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

class StarSimulator : public QThread
{
    Q_OBJECT
public:
    enum parameters
    {
        x0Random = 0,
        x0UserDefined,
        x0Current
    };

    enum algorithm
    {
        leastSquareNonLinear = 0,
        gradientDescent
    };

    enum algorithmConditions
    {
        FITING_OK = 0,
        PERFORMING_FITING,
        LOAD_DERIVATIVES,
        STOPPED
    };

    explicit StarSimulator(QObject *parent = 0);
    ~StarSimulator();
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

signals:
    void ledDataNotFound();
    void info(QString message);
    void performScan();
    void performScanWithUpdate();
    void finished();

public slots:
    void stop();
    void setSettings(StarSimulatorParameters parameters);
    void setObjectiveFunction(const MatrixXd &value);
    void setActiveChannels(const MatrixXi &activeChannels);
    MatrixXi getSolution();
    MatrixXi xWithConstraint(const MatrixXi &x);
    void setAlgorithm(int algorithm);
    void setx0Type(int x0SearchType, MatrixXi x = MatrixXi());
    int  algorithmStatus();
    uint  iterationNumber();
    double fx();
    bool enableUpdatePlot();

private:
    bool isGDInitialized;
    bool isLMInitialized;
    bool enabledToContinue;
    bool enablePlot;
    bool stopThread;
    int chosenAlgorithm;
    int x0Type;
    int status;
    int numberOfValidChannels;
    uint iteration;
    double fxCurrent;
    MatrixXi x0;
    MatrixXi solution;
    MatrixXi activeChannels;
    MatrixXd jacobianMatrix;
    MatrixXd objectiveFunction;
    MatrixXi minimumDigitalLevelByChannel;
    Matrix< MatrixXd, Dynamic, Dynamic> derivatives3DMatrix;
    StarSimulatorParameters settings;

    void run();
    bool loadDerivates();
    double media(int qty);
    int randomInt(int low, int high);
    void createJacobianMatrix(const MatrixXi &x);
    void getObjectiveFunction(const MatrixXi &x);
};

#endif // STARSIMULATOR_H
