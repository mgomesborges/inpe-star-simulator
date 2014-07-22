#include "filehandle.h"

FileHandle::FileHandle(QWidget *parent) :
    QWidget(parent)
{
}

FileHandle::FileHandle(QWidget *parent, const QString &caption, const QString &dir) :
    QWidget(parent)
{
    open(parent, caption, dir);
}

FileHandle::FileHandle(const QString &caption, const QString &filePath)
{
    open(caption, filePath);
}

FileHandle::FileHandle(QWidget *parent, const QString &data, const QString &caption, const QString &dir) :
    QWidget(parent)
{
    save(parent, data, caption, dir);
}

FileHandle::FileHandle(const QString &data, const QString &caption, const QString &filePath)
{
    save(data, caption, filePath);
}

FileHandle::~FileHandle()
{
    matrix.clear();
    inFile.close();
}

void FileHandle::close()
{
    inFile.close();
}

bool FileHandle::open(QWidget *parent, const QString &caption, const QString &dir)
{
    QString filePath = QFileDialog::getOpenFileName(parent, caption, dir, tr("Text document (*.txt);;All files (*.*)"));

    if (filePath.isEmpty())
        return false;

    updateLastDir(QFileInfo(filePath).path());

    return open(caption, filePath);
}

bool FileHandle::open(const QString &caption, const QString &filePath)
{
    if (inFile.isOpen())
        inFile.close();

    this->caption = caption;
    inFile.setFileName(filePath);

    if (inFile.open(QIODevice::ReadOnly) == false) {
        warningMessage(caption, tr("File %1 could not be opened\nor not found.\t").arg(inFile.fileName()));
        return false;
    }

    return true;
}

bool FileHandle::save(QWidget *parent, const QString &data, const QString &caption, const QString &dir)
{
    QString filePath = QFileDialog::getSaveFileName(parent, caption, dir, tr("Text document *.txt"));
    if (filePath.isEmpty())
        return false;

    if (!filePath.contains(".txt"))
        filePath.append(".txt");

    updateLastDir(QFileInfo(filePath).path());

    return save(data, caption, filePath);
}

bool FileHandle::save(const QString &data, const QString &caption, const QString &filePath)
{
    QFile outFile(filePath);

    if (outFile.open(QIODevice::WriteOnly | QIODevice::Text) == false) {
        warningMessage(caption, tr("File %1 could not be create.\t").arg(filePath));
        return false;
    }

    outFile.write(data.toUtf8());
    outFile.close();
    return true;
}

bool FileHandle::save(const QVector<QVector<double> > &data, const QString &caption, const QString &filePath)
{
    if (!QDir(QFileInfo(filePath).path()).exists())
        QDir().mkdir(QFileInfo(filePath).path());

    QFile outFile(filePath);

    if (outFile.open(QIODevice::WriteOnly) == false) {
        warningMessage(caption, tr("File %1 could not be create.\t").arg((filePath)));
        return false;
    }

    QDataStream out(&outFile);
    out.setVersion(QDataStream::Qt_5_0);
    out << data.size();    // rows
    out << data[0].size(); // cols

    for (int i = 0; i < data.size(); i++)
        for (int j = 0; j < data[i].size(); j++)
            out << data[i][j];

    outFile.close();
    return true;
}

QVector< QVector<double> > FileHandle::data(const QString &section, bool *ok)
{
    matrix.clear();

    if (inFile.isOpen()) {
        QTextStream in(&inFile);
        QStringList fields;
        QString line;
        int lineNumber = 0;

        // Prevents errors in case of "ok" not to be passed as argument
        if (ok == 0)
            ok = new bool;

        // Find section
        *ok = false;
        in.seek(0);
        do {
            line = in.readLine();
            lineNumber++;

            if (line.contains(section))
                *ok = true;
        } while ((*ok == false) && !in.atEnd());

        if (*ok == false) {
            warningMessage(caption, tr("File %1\n\nSection %2 not found.\t").arg(inFile.fileName()).arg(section));
            return matrix; // size zero
        }

        // Read data
        while (!in.atEnd()) {
            line   = in.readLine();
            lineNumber++;

            // Is a comment?
            while (line.startsWith(';', Qt::CaseInsensitive) && !in.atEnd()) {
                line = in.readLine();
                lineNumber++;
            }

            // Is other section or line without data?
            if (line.count() == 0) {
                if (matrix.isEmpty())
                    warningMessage(caption, tr("File %1\n\nData not found.\t").arg(inFile.fileName()));

                return matrix;
            }

            fields = line.split("\t");
            matrix.resize(matrix.size() + 1);
            for (int i = 0; i < fields.length(); i++) {
                fields.at(i).toDouble(ok);

                if (*ok == false) {
                    warningMessage(caption, tr("File %1\n\nInvalid data at line %2, column %3.\t").arg(inFile.fileName()).arg(lineNumber).arg(i + 1));
                    matrix.clear(); // size zero
                    return matrix;
                }

                matrix[matrix.size() - 1].append(fields.at(i).toDouble());
            }
        }
    }

    return matrix;
}

QString FileHandle::readSection(const QString &section, bool *ok)
{
    QString data;

    if (inFile.isOpen()) {
        QTextStream in(&inFile);
        QString line;

        // Prevents errors in case of "ok" not to be passed as argument
        if (ok == 0)
            ok = new bool;

        // Find section
        *ok = false;
        in.seek(0);
        do {
            line = in.readLine();

            if (line.contains(section))
                *ok = true;
        } while ((*ok == false) && !in.atEnd());

        if (*ok == false) {
            warningMessage(caption, tr("File %1\n\nSection %2 not found.\t").arg(inFile.fileName()).arg(section));
            return data; // size zero
        }

        // Read data
        data.append(line + "\n"); // First line contains section's name

        while (!in.atEnd()) {
            line = in.readLine();

            // Is other section?
            if (line.contains(QRegExp("^\\[[A-Za-z0-9-_.]+\\]$"))) {
                if (data.isEmpty())
                    warningMessage(caption, tr("File %1\n\nData not found.\t").arg(inFile.fileName()));

                break;
            }

            data.append(line + "\n");
        }
    }

    return data;
}

bool FileHandle::isValidData(int rows, int columns)
{
    if (matrix.isEmpty())
        return false;

    if ((matrix.size() == rows) && (columns == 0))
        return true;

    if ((rows == 0) && (matrix[0].size() == columns))
        return true;

    if ((matrix.size() == rows) && (matrix[0].size() == columns))
        return true;

    warningMessage(caption, tr("File %1\n\nData doesn't match expected length.\t").arg(inFile.fileName()));

    return false;
}
