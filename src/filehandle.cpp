#include "filehandle.h"

FileHandle::FileHandle(QWidget *parent) :
    QWidget(parent)
{
}

FileHandle::FileHandle(QWidget *parent, const QString &caption, QString *dir) :
    QWidget(parent)
{
    open(parent, caption, dir);
}

FileHandle::FileHandle(const QString &caption, const QString &filePath)
{
    open(caption, filePath);
}

FileHandle::FileHandle(QWidget *parent, const QString &data, const QString &caption, QString *dir) :
    QWidget(parent)
{
    save(parent, data, caption, dir);
}

FileHandle::FileHandle(const QString &data, const QString &caption, const QString &filePath)
{
    save(data, caption, filePath);
}

FileHandle::FileHandle(const QString &data, const QString &caption, const QString &section, const QString &filePath)
{
    save(data, caption, section, filePath);
}

FileHandle::~FileHandle()
{
    inFile.close();
}

bool FileHandle::open(QWidget *parent, const QString &caption, QString *dir)
{
    QString filePath = QFileDialog::getOpenFileName(parent, caption, *dir, tr("Text document (*.txt);;All files (*.*)"));

    if (filePath.isEmpty())
        return false;

    *dir = QFileInfo(filePath).path();
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

bool FileHandle::save(QWidget *parent, const QString &data, const QString &caption, QString *dir)
{
    QString filePath = QFileDialog::getSaveFileName(parent, caption, *dir, tr("Text document *.txt"));
    if (filePath.isEmpty())
        return false;

    if (!filePath.contains(".txt"))
        filePath.append(".txt");

    *dir = QFileInfo(filePath).path();
    return save(data, caption, filePath);
}

bool FileHandle::save(QWidget *parent, const QString &data, const QString &caption, const QString &section, QString *dir)
{
    QString filePath = QFileDialog::getSaveFileName(parent, caption, *dir, tr("Text document *.txt"));
    if (filePath.isEmpty())
        return false;

    if (!filePath.contains(".txt"))
        filePath.append(".txt");

    *dir = QFileInfo(filePath).path();
    return save(data, section, caption, filePath);
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

bool FileHandle::save(const QString &data, const QString &caption, const QString &section, const QString &filePath)
{
    // If file exists, update Section
    if (open(caption, filePath)) {
        SectionInfo info = sectionInfo(section);

        QTextStream in(&inFile);
        in.seek(0);

        QString temp = in.readAll();

        if (info.length != 0) // If section found
            temp.replace(info.startPosition, info.length, data + "\n");
        else // If section NOT found
            temp.append("\n" + data + "\n");

        temp.replace("\r\n", "\n"); // Prevents errors in Windows file
        return save(temp, caption, filePath);
    }

    return save(data, caption, filePath);
}

/**
 * @brief Read a matrix in txt file previously opened.
 */
QVector< QVector<double> > FileHandle::data(const QString &section)
{
    matrix.clear();

    if (inFile.isOpen()) {
        QTextStream in(&inFile);
        QStringList fields;
        QString line;
        int lineNumber = 0;
        bool ok = false;

        // Find section
        in.seek(0);
        do {
            line = in.readLine();
            lineNumber++;

            if (line.contains(section))
                ok = true;
        } while ((ok == false) && !in.atEnd());

        if (ok == false) {
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
                fields.at(i).toDouble(&ok);

                if (ok == false) {
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

FileHandle::SectionInfo FileHandle::sectionInfo(const QString &section)
{
    SectionInfo info;

    if (inFile.isOpen()) {
        QTextStream in(&inFile);
        QString line;
        int lineNumber = 0;
        bool ok = false;

        // Find section
        in.seek(0);
        do {
            info.startPosition = in.pos();
            line = in.readLine();
            lineNumber++;

            if (line.contains(section)) {
                info.startLineNumber = lineNumber;
                ok = true;
            }
        } while ((ok == false) && !in.atEnd());

        // Section not found
        if (ok == false) {
            warningMessage(caption, tr("File %1\n\nSection %2 not found.\t").arg(inFile.fileName()).arg(section));
            info.startPosition = -1;
            return info;
        }

        // Read section
        while (!in.atEnd()) {
            line = in.readLine();

            // Is other section?
            if (line.contains(QRegExp("^\\[[A-Za-z0-9-_.]+\\]$"))) {
                break;
            } else
                info.endPosition = in.pos();

            lineNumber++;
        }

        info.length       = info.endPosition - info.startPosition;
        info.endLinNumber = lineNumber;

        // Prevents errors in Windows file
        if (line.contains("\r\n")) {
            info.startPosition -= lineNumber;
            info.endPosition   -= lineNumber;
        }
    }

    return info;
}

QString FileHandle::readSection(const QString &section)
{
    QString data;

    if (inFile.isOpen()) {
        SectionInfo info = sectionInfo(section);
        QTextStream in(&inFile);
        in.seek(info.startPosition);
        data = in.read(info.length);
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
