#ifndef LOADLEDDATATHREAD_H
#define LOADLEDDATATHREAD_H

#include <QThread>

#include <iostream>
#include <Eigen/Dense>
using namespace Eigen;

class LoadLedDataThread : public QThread
{
    Q_OBJECT
public:
    explicit LoadLedDataThread(QObject *parent = 0);

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    const Matrix<double, Dynamic, Dynamic> &channelData(int channel);
    
signals:
    void info(QString message);

public slots:

private:
    Matrix< Matrix<double, Dynamic, Dynamic>, 1, 72> vectorXM;

    void run();
};

#endif // LOADLEDDATATHREAD_H
