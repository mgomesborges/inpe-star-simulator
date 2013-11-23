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

    static QVector<int> eigen2QVector(MatrixXi matrix);
    static QVector<int> matrix2vector(const QVector< QVector<int> > &matrix, int column);
};

#endif // UTILS_H
