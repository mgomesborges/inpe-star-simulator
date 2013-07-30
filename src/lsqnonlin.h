#ifndef LSQNONLIN_H
#define LSQNONLIN_H

#include <QThread>
#include <QTime>
#include <QDir>

#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

class LSqNonLin : public QThread
{
    Q_OBJECT
public:
    enum parameters
    {
        x0Random = 0,
        x0UserDefined,
        x0GeneticAlgorithmSearch,
        dampingFactorTechnique1,
        dampingFactorTechnique2,
        dampingFactorFixed
    };

    explicit LSqNonLin(QObject *parent = 0);
    ~LSqNonLin();
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    int randomInt(int low, int high);
    bool loadDerivates();
    void setx0Type(int x0SearchType, MatrixXi x = MatrixXi());
    void setDampingFactor(int type, double value = 0);

signals:
    void ledDataNotFound();
    void info(QString message);
    void performScan();
    void finished();

public slots:
    void stop();
    void jacobian(const MatrixXi &x);
    double getAlpha(const MatrixXi &xCurrent);
    void getObjectiveFunction(const MatrixXi &x);
    void setObjectiveFunction(const MatrixXd &value);
    MatrixXi getSolution();

private:
    bool initialized;
    bool enabledToContinue;
    bool stopThread;
    int x0Type;
    int dampingFactorType;
    double dampingFactor;
    MatrixXi x0;
    MatrixXi solution;
    MatrixXd jacobianMatrix;
    MatrixXd objectiveFunction;
    MatrixXi minimumDigitalLevelByChannel;
    Matrix< MatrixXd, 1, 72> derivatives3DMatrix;

    void run();
    void resetToDefault();
};

#endif // LSQNONLIN_H
