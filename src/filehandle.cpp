#include "filehandle.h"

FileHandle::FileHandle(QWidget *parent) :
    QWidget(parent)
{
}

FileHandle::FileHandle(QWidget *parent, const QString &caption) :
    QWidget(parent)
{
    open(parent, caption);
}

FileHandle::FileHandle(const QString &caption, const QString &filePath)
{
    open(caption, filePath);
}

FileHandle::FileHandle(QWidget *parent, const QString &data, const QString &caption) :
    QWidget(parent)
{
    save(parent, data, caption);
}

FileHandle::FileHandle(const QString &data, const QString &caption, const QString &filePath)
{
    save(data, caption, filePath);
}

FileHandle::~FileHandle()
{
    matrix.clear();
    inFile.close();
    outFile.close();
}

void FileHandle::close()
{
    inStatus  = false;
    outStatus = false;
    inFile.close();
}

bool FileHandle::open(QWidget *parent, const QString &caption)
{
    QString filePath = QFileDialog::getOpenFileName(parent,
                                                    caption,
                                                    QDir::homePath(),
                                                    tr("Text document (*.txt);;All files (*.*)"));

    if (filePath.isEmpty()) {
        inStatus = false;
        return false;
    }

    return open(caption, filePath);
}

bool FileHandle::open(const QString &caption, const QString &filePath)
{
    inFile.setFileName(filePath);
    inStatus = inFile.open(QIODevice::ReadOnly);
    FileHandle::caption = caption;

    return inStatus;
}

bool FileHandle::save(QWidget *parent, const QString &data, const QString &caption)
{
    QString filePath = QFileDialog::getSaveFileName(parent,
                                                    caption,
                                                    QDir::homePath(),
                                                    tr("Text document *.txt"));
    if (filePath.isEmpty()) {
        outStatus = false;
        return false;
    }

    // Checks if exist extension '.txt'
    if (!filePath.contains(".txt")) {
        filePath.append(".txt");
    }

    return save(data, caption, filePath);
}

bool FileHandle::save(const QString &data, const QString &caption, const QString &filePath)
{
    outFile.setFileName(filePath);
    outStatus = outFile.open(QIODevice::WriteOnly | QIODevice::Text);
    if (outStatus == false) {
        QMessageBox::warning(0, caption, tr("File could not be create.\t"));
        return false;
    }

    outFile.write(data.toUtf8());
    return true;
}

QVector< QVector<int> > FileHandle::data(const QString &section, bool *ok)
{
    if (inStatus == true) {
        QTextStream in(&inFile);
        QStringList fields;
        QString line;
        int lineNumber = 0;

        // Prevents errors in case of "ok" not to be passed as argument
        if (ok == 0) {
            ok = new bool;
        }

        // Find section
        *ok = false;
        do {
            line = in.readLine();
            lineNumber++;

            if (line.contains(section)) {
                *ok = true;
            }
        } while ((*ok == false) && !in.atEnd());

        if (*ok == false) {
            QMessageBox::warning(0, caption, tr("Section %1 not found.\t").arg(section));
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
            if (line.count() <= 2) {
                if (matrix.isEmpty()) {
                    QMessageBox::warning(0, caption, tr("Data not foud.\t"));
                }
                break;
            }

            fields = line.split("\t");
            matrix.resize(matrix.size() + 1);
            for (int i = 0; i < fields.length(); i++) {
                fields.at(i).toInt(ok);

                if (*ok == false) {
                    QMessageBox::warning(0, caption, tr("Invalid data at line %1, column %2.\t")
                                         .arg(lineNumber).arg(i + 1));
                    matrix.clear(); // size zero
                    return matrix;
                }

                matrix[matrix.size() - 1].append(fields.at(i).toInt());
            }
        }
    }

    return matrix;
}

bool FileHandle::isValidData(int rows, int columns)
{
    if (matrix.isEmpty()) {
        return false;
    }

    if ((matrix.size() == rows) && (columns== 0)) {
        return true;
    }

    if ((rows == 0) && (matrix[0].size() == columns)) {
        return true;
    }

    if ((matrix.size() == rows) && (matrix[0].size() == columns)) {
        return true;
    }

    QMessageBox::warning(0, caption, tr("Data doesn't match expected length.\t"));

    return false;
}
