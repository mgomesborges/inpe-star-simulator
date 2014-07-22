#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QVector>

#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

class Utils : public QObject
{
    Q_OBJECT
public:
    explicit Utils(QObject *parent = 0);

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    static QVector<double> eigen2QVector(MatrixXi matrix);
    static QVector< QVector<double> > eigen2QVector(MatrixXd matrix);
    static MatrixXi qvector2eigen(const QVector<int> &matrix);
    static MatrixXd qvector2eigen(const QVector< QVector<double> > &matrix);
    static QVector<int> matrix2vector(const QVector< QVector<int> > &matrix, int column);
    static QVector<double> matrix2vector(const QVector< QVector<double> > &matrix, int column);
};

#endif // UTILS_H
