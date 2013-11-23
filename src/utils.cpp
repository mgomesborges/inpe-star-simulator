#include "utils.h"

Utils::Utils(QObject *parent) :
    QObject(parent)
{
}

QVector<int> Utils::eigen2QVector(MatrixXi matrix)
{
    QVector<int> qvector;
    for (int row = 0; row < matrix.rows(); row++) {
        qvector.append(matrix(row));
    }
    return qvector;
}

QVector<int> Utils::matrix2vector(const QVector< QVector<int> > &matrix, int column)
{
    QVector<int> qvector;
    for (int row = 0; row < matrix.size(); row++) {
        qvector.append(matrix[row][column]);
    }
    return qvector;
}
