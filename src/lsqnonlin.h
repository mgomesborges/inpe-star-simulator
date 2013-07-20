#ifndef LSQNONLIN_H
#define LSQNONLIN_H

#include <QThread>
#include <QDir>

#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

class LSqNonLin : public QThread
{
    Q_OBJECT
public:
    explicit LSqNonLin(QObject *parent = 0);
    ~LSqNonLin();
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

public:
    bool loadDerivates();

signals:
    void ledDataNotFound();
    void info(QString message);
    void performScan();

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
    MatrixXi solution;
    MatrixXd jacobianMatrix;
    MatrixXd objectiveFunction;
    MatrixXi minimumDigitalLevelByChannel;
    Matrix< MatrixXd, 1, 72> derivatives3DMatrix;

    void run();
};

#endif // LSQNONLIN_H
