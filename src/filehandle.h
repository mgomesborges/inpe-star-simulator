#ifndef FILEHANDLE_H
#define FILEHANDLE_H

#include <QObject>
#include <QFile>
#include <QFileDialog>
#include <QString>
#include <QTextStream>
#include <QVector>

class FileHandle : public QWidget
{
    Q_OBJECT
public:
    explicit FileHandle(QWidget *parent = 0);

    // Open file
    explicit FileHandle(QWidget *parent,
                        const QString &caption,
                        const QString &dir);
    explicit FileHandle(const QString &caption,
                        const QString &filePath);

    // Save file
    explicit FileHandle(QWidget *parent,
                        const QString &data,
                        const QString &caption,
                        const QString &dir);
    explicit FileHandle(const QString &data,
                        const QString &caption,
                        const QString &filePath);

    ~FileHandle();
    void close();
    bool open(QWidget *parent, const QString &caption, const QString &dir = QDir::homePath());
    bool open(const QString &caption, const QString &filePath);
    bool save(QWidget *parent, const QString &data, const QString &caption, const QString &dir = QDir::homePath());
    bool save(const QString &data, const QString &caption, const QString &filePath);
    bool save(const QVector< QVector<double> > &data, const QString &caption, const QString &filePath);
    QVector< QVector<double> > data(const QString &section = QString(), bool *ok = 0);
    QString readSection(const QString &section = QString(), bool *ok = 0);
    bool isValidData(int rows = 0, int columns = 0);

signals:
    void updateLastDir(QString lastDir);
    void warningMessage(QString caption, QString message);

private:
    QString caption;
    QFile inFile;
    QVector< QVector<double> > matrix;
};

#endif // FILEHANDLE_H

