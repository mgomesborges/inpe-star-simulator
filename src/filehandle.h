#ifndef FILEHANDLE_H
#define FILEHANDLE_H

#include <QObject>
#include <QFile>
#include <QFileDialog>
#include <QString>
#include <QTextStream>
#include <QVector>

#include "datatype.h"

class FileHandle : public QWidget
{
    Q_OBJECT

public:
    struct SectionInfo {
        int startLineNumber;
        int endLinNumber;
        int startPosition;
        int endPosition;
        int length;

        // Default values
        SectionInfo()
            : startLineNumber(-1)
            , endLinNumber(-1)
            , startPosition(-1)
            , endPosition(-1)
            , length(0)
        {}
    };

    explicit FileHandle(QWidget *parent = 0);

    // Open file
    explicit FileHandle(QWidget *parent, const QString &caption, QString *dir);
    explicit FileHandle(const QString &caption, const QString &filePath);

    // Save file
    explicit FileHandle(QWidget *parent, const QString &data, const QString &caption, QString *dir);
    explicit FileHandle(const QString &data, const QString &caption, const QString &filePath);
    explicit FileHandle(const QString &data, const QString &caption, const QString &section, const QString &filePath);

    ~FileHandle();
    bool open(QWidget *parent, const QString &caption, QString *dir);
    bool open(const QString &caption, const QString &filePath);

    bool save(QWidget *parent, const QString &data, const QString &caption, QString *dir);
    bool save(QWidget *parent, const QString &data, const QString &section, const QString &caption, QString *dir);
    bool save(const QString &data, const QString &caption, const QString &filePath);
    bool save(const QString &data, const QString &caption, const QString &section, const QString &filePath);

    QVector< QVector<double> > data(const QString &section = QString());
    SectionInfo sectionInfo(const QString &section);
    QString readSection(const QString &section = QString());
    bool isValidData(int rows = 0, int columns = 0);

signals:
    void warningMessage(QString caption, QString message);

private:
    QString caption;
    QFile inFile;
    QVector< QVector<double> > matrix;
};

#endif // FILEHANDLE_H

