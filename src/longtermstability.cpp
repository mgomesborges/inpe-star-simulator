#include "longtermstability.h"

LongTermStability::LongTermStability(QObject *parent) :
    QThread(parent)
{
    //set database driver to QSQLITE
    db = QSqlDatabase::addDatabase("QSQLITE");

    scanInfoModel               = new QSqlTableModel(this, db);
    parametersModel             = new QSqlTableModel(this, db);
    ledDriverConfigurationModel = new QSqlTableModel(this, db);
    scanDataModel               = new QSqlTableModel(this, db);
}

LongTermStability::~LongTermStability()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool LongTermStability::createDB(const QString &filePath)
{
    if (db.isOpen()) {
        db.close();
    }

    db.setDatabaseName(filePath);

    //can be removed
    db.setHostName("localhost");
    db.setUserName("");
    db.setPassword("");

    if(db.open() == false) {
        lastErrorMessage = tr("Error: could not open database connection.");
        return false;
    }

    // Creates tables
    if (db.isOpen()) {
        QSqlQuery query;
        bool queryReturn;

        queryReturn = query.exec("CREATE TABLE parameters("
                                 "id INTEGER, "
                                 "start_wavelength INTEGER, "
                                 "stop_wavelength INTEGER, "
                                 "integration_time INTEGER, "
                                 "samples_to_average INTEGER, "
                                 "boxcar_smoothing INTEGER, "
                                 "noise_reduction INTEGER, "
                                 "dynamic_dark INTEGER);");

        queryReturn &= query.exec("CREATE TABLE led_driver_configuration("
                                  "id INTEGER, "
                                  "channel INTEGER, "
                                  "digital_level INTEGER);");

        queryReturn &= query.exec("CREATE TABLE scan_info("
                                  "id INTEGER, "
                                  "scan_number INTEGER, "
                                  "time TEXT, "
                                  "dominate_wavelength INTEGER, "
                                  "peak_wavelength INTEGER, "
                                  "fwhm INTEGER,"
                                  "power REAL, "
                                  "purity REAL);");

        queryReturn &= query.exec("CREATE TABLE scan("
                                  "scan_number INTEGER, "
                                  "wavelength INTEGER, "
                                  "intensity REAL);");

        if (queryReturn == false) {
            lastErrorMessage = tr("Error: table cannot be created.");
            return false;
        }
    }

    lastErrorMessage = "";
    return true;
}

bool LongTermStability::openDB(const QString &filePath)
{
    // Prevents errors
    if (db.isOpen()) {
        db.close();
    }

    db.setDatabaseName(filePath);

    //can be removed
    db.setHostName("localhost");
    db.setUserName("");
    db.setPassword("");

    if(db.open() == false) {
        lastErrorMessage = tr("Error: could not open database connection.");
        return false;
    }

    lastErrorMessage = "";
    return true;
}

QSqlTableModel *LongTermStability::scanInfoTableModel()
{
    QSqlTableModel *scanInfoModel = new QSqlTableModel(this, db);
    scanInfoModel->setTable("scan_info");
    scanInfoModel->setHeaderData(1, Qt::Horizontal, tr("Scan"));
    scanInfoModel->setHeaderData(2, Qt::Horizontal, tr("Time"));
    scanInfoModel->setHeaderData(3, Qt::Horizontal, tr("Dominate\nWavelength"));
    scanInfoModel->setHeaderData(4, Qt::Horizontal, tr("Peak\nWavelength"));
    scanInfoModel->setHeaderData(5, Qt::Horizontal, tr("FWHM"));
    scanInfoModel->setHeaderData(6, Qt::Horizontal, tr("Power"));
    scanInfoModel->setHeaderData(7, Qt::Horizontal, tr("Purity"));

    scanInfoModel->select();

    return scanInfoModel;
}

/**
 *  Esta funcao necessita de melhorias
 */
QVector< QVector<double> > LongTermStability::selectedData(QModelIndexList indexList)
{
    QVector< QVector<double> > matrix;

    // SQL Statments
    parametersModel->setTable("parameters");
    parametersModel->select();

    QSqlRecord record   = parametersModel->record(0);
    int startWavelength = record.value(1).toInt();
    int stopWavelength  = record.value(2).toInt();

    matrix.resize(stopWavelength - startWavelength + 1);

    // SQL Statments
    scanDataModel->setTable("scan");

    qSort(indexList.begin(), indexList.end());

    int column = 0;
    foreach (QModelIndex index, indexList) {
        scanDataModel->setFilter(tr("scan_number = %1").arg(index.row() + 1));
        scanDataModel->select();

        for (int row = 0; row < scanDataModel->rowCount(); row++) {
            if (row == scanDataModel->rowCount() - 1) {
                scanDataModel->fetchMore();
            }
            matrix[row].resize(row+1);
            QSqlRecord record = scanDataModel->record(row);
            matrix[row][column] = record.value(2).toDouble();
        }
        column++;
    }

    return matrix;
}

QString LongTermStability::lastError()
{
    return lastErrorMessage;
}

void LongTermStability::exportData(const QString &filePath)
{
    exportFilePath = filePath;
    this->start();
}

void LongTermStability::stop()
{
    stopThread = true;
}

bool LongTermStability::status()
{
    return currentStatus;
}

bool LongTermStability::saveSMS500andLedDriverParameters(
        int id,
        int startWavelength,
        int stopWavelength,
        int integrationTime,
        int samplesToAverage,
        int boxcarSmoothing,
        int noiseReduction,
        bool dynamicDark,
        QStringList channelValue)
{
    if (db.isOpen() == false) {
        lastErrorMessage = tr("Database not connected.");
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO parameters (id, start_wavelength, stop_wavelength, integration_time, samples_to_average, boxcar_smoothing, noise_reduction, dynamic_dark) "
                  "VALUES (:id, :start_wavelength, :stop_wavelength, :integration_time, :samples_to_average, :boxcar_smoothing, :noise_reduction, :dynamic_dark)");
    query.bindValue(":id", id);
    query.bindValue(":start_wavelength", startWavelength);
    query.bindValue(":stop_wavelength", stopWavelength);
    query.bindValue(":integration_time", integrationTime);
    query.bindValue(":samples_to_average", samplesToAverage);
    query.bindValue(":boxcar_smoothing", boxcarSmoothing);
    query.bindValue(":noise_reduction", noiseReduction);
    query.bindValue(":dynamic_dark", dynamicDark);
    if (query.exec() == false) {
        lastErrorMessage = tr("Error: cannot store SMS500 and LED Driver data in table.");
        return false;
    }

    for (int i = 0; i < channelValue.size(); i++) {
        query.prepare("INSERT INTO led_driver_configuration (id, channel, digital_level) "
                      "VALUES (:id, :channel, :digital_level)");
        query.bindValue(":id", id);
        query.bindValue(":channel", i + 25);
        query.bindValue(":digital_level", channelValue.at(i).toInt());

        if (!query.exec()) {
            lastErrorMessage = tr("Error: cannot store SMS500 and LED Driver data in table.");
            return false;
        }
    }

    return true;
}

bool LongTermStability::saveScanData(
        int id,
        int scanNumber,
        int dominateWavelength,
        int peakWavelength,
        int fwhm,
        double power,
        double purity,
        int startWavelength,
        int stopWavelength,
        const double *masterData)
{
    QSqlQuery query;

    // Starts transaction
    db.transaction();

    query.prepare("INSERT INTO scan_info (id, scan_number, time, dominate_wavelength, peak_wavelength, power, purity, fwhm) "
                  "VALUES (:id, :scan_number, :time, :dominate_wavelength, :peak_wavelength, :power, :purity, :fwhm)");
    query.bindValue(":id", id);
    query.bindValue(":scan_number", scanNumber);
    query.bindValue(":time", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":dominate_wavelength", dominateWavelength);
    query.bindValue(":peak_wavelength", peakWavelength);
    query.bindValue(":fwhm", fwhm);
    query.bindValue(":power", power);
    query.bindValue(":purity", purity);
    if (query.exec() == false) {
        lastErrorMessage = tr("Error: cannot store SMS500 and LED Driver data in table.");
        return false;
    }

    for (int i = startWavelength; i <= stopWavelength; i++) {
        query.prepare("INSERT INTO scan (scan_number, wavelength, intensity) "
                      "VALUES (:scan_number, :wavelength, :intensity)");
        query.bindValue(":scan_number", scanNumber);
        query.bindValue(":wavelength", i);
        query.bindValue(":intensity", masterData[i - startWavelength]);
        if (query.exec() == false) {
            lastErrorMessage = tr("Error: cannot store SMS500 and LED Driver data in table.");
            return false;
        }
    }

    // Commit transaction
    db.commit();

    return true;
}

/**
 * @brief LongTermStability::run(). Performs exporting data into text file.
 */
void LongTermStability::run()
{
    int startWavelength;
    int stopWavelength;

    currentStatus = false;
    stopThread    = false;

    // Prevents errors
    if (db.isOpen() == false) {
        emit error(tr("Error: database not connected."));
        return;
    }

    // Scan Info Data
    QFile outFileScanInfo(exportFilePath + "_scan_info.txt");
    if (outFileScanInfo.open(QIODevice::WriteOnly | QFile::Text) == false) {
        emit error(tr("Error: can not create %1_scan_info.txt").arg(exportFilePath));
        return;
    }
    QTextStream outScanInfo(&outFileScanInfo);

    // SQL Statments
    scanInfoModel->setTable("scan_info");
    scanInfoModel->select();

    QString currentTime(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate));
    outScanInfo << "Star Simulator file, Platform: Windows, Created on: " << currentTime << "\n\n";
    outScanInfo << "Scan Info\n";
    outScanInfo << "#1 Scan\n";
    outScanInfo << "#2 Date\n";
    outScanInfo << "#3 Time\n";
    outScanInfo << "#4 Dominate Wavelength\n";
    outScanInfo << "#5 Peak Wavelength\n";
    outScanInfo << "#6 FWHM\n";
    outScanInfo << "#7 Power\n";
    outScanInfo << "#8 Purity\n\n";
    outScanInfo << "#1\t#2        \t#3      \t#4\t#5\t#6\t#7      \t#8\n";

    // Update progress bar
    emit progressMinimumInfo(0);
    emit progressMaximumInfo(scanInfoModel->rowCount());

    for (int row = 0; row < scanInfoModel->rowCount(); ++row) {
        msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
        if (stopThread == true) {
            // Close Scan Info file
            outFileScanInfo.close();

            // Delete File
            QFile::remove(exportFilePath + "_scan_info.txt");
            return;
        }

        if (row == scanInfoModel->rowCount() - 1) {
            scanInfoModel->fetchMore();

            // Update progress bar
            emit progressMaximumInfo(scanInfoModel->rowCount());
        }

        // Update progress bar
        emit progressInfo(row);

        QSqlRecord record = scanInfoModel->record(row);
        QStringList fields = record.value(2).toString().split("T"); // Get Date and Time
        outScanInfo << record.value(1).toInt() << "\t";     // Scan
        outScanInfo << fields.at(0) << "\t";                // Date
        outScanInfo << fields.at(1) << "\t";                // Time
        outScanInfo << record.value(3).toInt() << "\t";     // Dominate Wavelength
        outScanInfo << record.value(4).toInt() << "\t";     // Peak Wavelength
        outScanInfo << record.value(5).toInt() << "\t";     // FWHM
        outScanInfo << record.value(6).toDouble() << "\t";  // Power
        outScanInfo << record.value(7).toDouble() << "\t";  // Purity
        outScanInfo << "\n";
    }

    // Close Scan Info file
    outFileScanInfo.close();

    // Scan Data
    QFile outFileScanData(exportFilePath + "_scan.txt");
    if (outFileScanData.open(QIODevice::WriteOnly | QFile::Text) == false) {
        emit error(tr("Error: can not create %1_scan.txt").arg(exportFilePath));
        return;
    }
    QTextStream outScanData(&outFileScanData);

    // SQL Statments
    parametersModel->setTable("parameters");
    parametersModel->select();

    QSqlRecord record = parametersModel->record(0);

    outScanData << "Star Simulator file, Platform: Windows, Created on: " << currentTime << "\n\n";
    outScanData << "Parameters\n";
    outScanData << "Start Wavelength..: " << record.value(1).toInt() << "\n";
    outScanData << "Stop Wavelength...: " << record.value(2).toInt() << "\n";
    outScanData << "Integration Time..: " << record.value(3).toInt() << "\n";
    outScanData << "Samples to Average: " << record.value(4).toInt() << "\n";
    outScanData << "Boxcar Smoothing..: " << record.value(5).toInt() << "\n";
    outScanData << "Noise Reduction...: " << record.value(6).toInt() << "\n";
    outScanData << "Dynamic Dark......: " << record.value(7).toInt() << "\n";

    // Star and Stop wavelength values
    startWavelength = record.value(1).toInt();
    stopWavelength  = record.value(2).toInt();

    // SQL Statments
    ledDriverConfigurationModel->setTable("led_driver_configuration");
    ledDriverConfigurationModel->select();

    outScanData << "\nLED Driver Configuration\n";

    for (int row = 0; row < ledDriverConfigurationModel->rowCount(); ++row) {
        msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
        if (stopThread == true) {
            // Close Scan Data file
            outFileScanData.close();

            // Delete File
            QFile::remove(exportFilePath + "_scan_info.txt");
            QFile::remove(exportFilePath + "_scan.txt");
            return;
        }

        if (row == ledDriverConfigurationModel->rowCount() - 1) {
            ledDriverConfigurationModel->fetchMore();
        }

        QSqlRecord record = ledDriverConfigurationModel->record(row);
        outScanData << "ch" << row + 25 << ": " << record.value(2).toInt() << "\n";
    }

    // SQL Statments
    scanDataModel->setTable("scan");

    // Scan Data Header
    scanDataModel->setFilter(tr("wavelength = %1").arg(startWavelength));
    scanDataModel->select();

    outScanData << "\n#The first line contains the number of scan (for more information see scan_info file)\n" << "\t";
    for (int row = 0; row < scanDataModel->rowCount(); row++) {
        if (row == scanDataModel->rowCount() - 1) {
            scanDataModel->fetchMore();
        }
        outScanData << row + 1 << "\t";
    }
    outScanData << "\n";

    // Update progress bar
    emit progressMinimumInfo(startWavelength);
    emit progressMaximumInfo(stopWavelength);

    // Scan Data values
    for (int wavelength = startWavelength; wavelength <= stopWavelength; wavelength++) {
        msleep(1); // wait 1ms for continue, see Qt Thread's Documentation
        if (stopThread == true) {
            // Close Scan Data file
            outFileScanData.close();

            // Delete File
            QFile::remove(exportFilePath + "_scan_info.txt");
            QFile::remove(exportFilePath + "_scan.txt");
            return;
        }
        scanDataModel->setFilter(tr("wavelength = %1").arg(wavelength));
        scanDataModel->select();

        outScanData << wavelength << "\t";

        for (int row = 0; row < scanDataModel->rowCount(); row++) {
            if (row == scanDataModel->rowCount() - 1) {
                scanDataModel->fetchMore();
            }

            QSqlRecord record = scanDataModel->record(row);
            outScanData << record.value(2).toDouble() << "\t";
        }

        outScanData << "\n";

        // Update progress bar
        emit progressInfo(wavelength);
    }

    // Close Scan Data File
    outFileScanData.close();

    currentStatus = true;
}
