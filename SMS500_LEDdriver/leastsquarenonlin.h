#ifndef LEASTSQUARENONLIN_H
#define LEASTSQUARENONLIN_H

#include <QThread>
#include <QVector>
#include <QDebug>
#include <QFileDialog>

#include <cmath>
#include <fstream>

#include <iostream>
#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

class LeastSquareNonLin : public QThread
{
    Q_OBJECT
public:
    explicit LeastSquareNonLin(QObject *parent = 0);
    ~LeastSquareNonLin();

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
signals:
    void info(QString message);
    void performScan();
    
public slots:
    void stop();
    void jacobian(Matrix<int, 71, 1> x0);
    void setObjectiveFunction(Matrix<double, Dynamic, Dynamic> value);
    Matrix<int, 71, 1> getSolution();
    
private:
    bool initialized;
    bool enabledToContinue;
    bool stopThread;

    Matrix<double, 1, 71> minimumDigitalLevel;
    Matrix< Matrix<double, Dynamic, Dynamic>, 1, 71> derivatives;

    Matrix<int, 71, 1> x;
    Matrix<double, 641, 71> J;
    Matrix<double, 641, 1> objectiveFunction;

    void run();
};

#endif // LEASTSQUARENONLIN_H
