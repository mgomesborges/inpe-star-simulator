#include "utils.h"

Utils::Utils(QObject *parent) :
    QObject(parent)
{
}

QVector<double> Utils::eigen2QVector(MatrixXi matrix)
{
    QVector<double> qvector;
    for (int row = 0; row < matrix.rows(); row++) {
        qvector.append(matrix(row));
    }
    return qvector;
}

QVector<QVector<double> > Utils::eigen2QVector(MatrixXd matrix)
{
    QVector< QVector<double> > qvector;
    qvector.resize(matrix.rows());
    for (int row = 0; row < matrix.rows(); row++) {
        qvector[row].resize(matrix.cols());
        for (int col = 0; col < matrix.cols(); col++) {
            qvector[row][col] = matrix(row, col);
        }
    }
    return qvector;
}

MatrixXi Utils::qvector2eigen(const QVector<int> &matrix)
{
    MatrixXi eigenMatrix(matrix.size(), 1);

    for (int i = 0; i < eigenMatrix.rows(); i++) {
        eigenMatrix(i) = matrix[i];
    }

    return eigenMatrix;
}

MatrixXd Utils::qvector2eigen(const QVector<QVector<double> > &matrix)
{
    int rows = matrix.size();
    int cols = matrix[0].size();

    MatrixXd eigenMatrix(rows, cols);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            eigenMatrix(i, j) = matrix[i][j];
        }
    }

    return eigenMatrix;
}

QVector<int> Utils::matrix2vector(const QVector< QVector<int> > &matrix, int column)
{
    QVector<int> qvector;
    for (int row = 0; row < matrix.size(); row++) {
        qvector.append(matrix[row][column]);
    }
    return qvector;
}

QVector<double> Utils::matrix2vector(const QVector<QVector<double> > &matrix, int column)
{
    QVector<double> qvector;
    for (int row = 0; row < matrix.size(); row++) {
        qvector.append(matrix[row][column]);
    }
    return qvector;
}

