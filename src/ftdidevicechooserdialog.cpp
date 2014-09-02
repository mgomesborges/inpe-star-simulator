#include "ftdidevicechooserdialog.h"
#include "ui_ftdidevicechooserdialog.h"

FTDIDeviceChooserDialog::FTDIDeviceChooserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FTDIDeviceChooserDialog)
{
    ui->setupUi(this);
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateInfo(int)));
    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(saveDefaultConnection()));
    connect(ui->btnRescan, SIGNAL(clicked()), this, SLOT(deviceInfoList()));
    deviceInfoList();
    iDevice = -1;

    filePath = QDir::currentPath() + "/config.txt";
    file.open(tr("Load Config file"), filePath);
}

FTDIDeviceChooserDialog::~FTDIDeviceChooserDialog()
{
    delete ui;
}

void FTDIDeviceChooserDialog::deviceInfoList()
{
    // Create the device information list
    ftStatus = FT_CreateDeviceInfoList(&numDevs);

    if (ftStatus == FT_OK && numDevs > 0) {
        ui->btnOk->setEnabled(true);
        ui->comboBox->setEnabled(true);

        // allocate storage for list based on numDevs
        devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);

        // get the device information list
        ftStatus = FT_GetDeviceInfoList(devInfo,&numDevs);

        if (ftStatus == FT_OK) {
            QStringList devices;
            for (DWORD i = 0; i < numDevs; i++)
                devices << tr("USB-%1: %2").arg(i).arg(deviceName(devInfo[i].Type));

            ui->comboBox->clear();
            ui->comboBox->addItems(devices);
            iDevice = 0;
            updateInfo(0);
        }
    } else {
        ui->btnOk->setEnabled(false);
        ui->comboBox->setEnabled(false);
    }
}

QString FTDIDeviceChooserDialog::deviceName(ulong type)
{
    QString name;

    switch (type) {
        case 0:
            name = "FT_DEVICE_BM";
            break;
        case 1:
            name = "FT_DEVICE_AM";
            break;
        case 2:
            name = "FT_DEVICE_100AX";
            break;
        case 3:
            name = "FT_DEVICE_UNKNOWN";
            break;
        case 4:
            name = "FT_DEVICE_2232C";
            break;
        case 5:
            name = "FT_DEVICE_232R";
            break;
        case 6:
            name = "FT_DEVICE_2232H";
            break;
        case 7:
            name = "FT_DEVICE_4232H";
            break;
        case 8:
            name = "FT_DEVICE_232H";
            break;
        case 9:
            name = "FT_DEVICE_X_SERIES";
            break;
        default:
            name =  "UNKNOWN DEVICE";
    }

    return name;
}

int FTDIDeviceChooserDialog::numberOfDevices()
{
    return numDevs;
}

int FTDIDeviceChooserDialog::defaultConnection(bool enableChooseDevice)
{
    deviceInfoList();

    if (numDevs == 0)
        return -1; // Error: without device

    iDevice = -1; // Initial condition

    // Get default connection
    QString defaultConnection = file.readSection(tr("[FTDIDefaultConnection]"));

    for (DWORD i = 0; i < numDevs; i++)
        // Magic <=> do not touch
        if (defaultConnection.contains(tr("Device=%1").arg(deviceName(devInfo[i].Type))))
            if (defaultConnection.contains(tr("ID=%1").arg(devInfo[i].ID)))
                if (defaultConnection.contains(tr("SerialNumber=%1").arg(devInfo[i].SerialNumber)))
                    if (defaultConnection.contains(tr("LocID=%1").arg(devInfo[i].LocId)))
                        if (defaultConnection.contains(tr("Description=%1").arg(devInfo[i].Description)))
                            iDevice = i;

    if (enableChooseDevice == true && iDevice == -1)
        this->exec();

    return iDevice;
}

void FTDIDeviceChooserDialog::saveDefaultConnection()
{
    QString data;

    iDevice = ui->comboBox->currentIndex();

    data.append(tr("[FTDIDefaultConnection]\n"));
    data.append(tr("Device=%1\n").arg(deviceName(devInfo[iDevice].Type)));
    data.append(tr("ID=%1\n").arg(devInfo[iDevice].ID));
    data.append(tr("SerialNumber=%1\n").arg(devInfo[iDevice].SerialNumber));
    data.append(tr("LocID=%1\n").arg(devInfo[iDevice].LocId));
    data.append(tr("Description=%1\n").arg(devInfo[iDevice].Description));

    file.save(data, tr("Save Config File"), tr("[FTDIDefaultConnection]"), filePath);

    this->close();
}

void FTDIDeviceChooserDialog::updateInfo(int index)
{
    if (ftStatus == FT_OK) {
        iDevice = index;
        ui->id->setText(tr("0x%1").arg(devInfo[index].ID));
        ui->serialNumber->setText(tr("%1").arg(devInfo[index].SerialNumber));
        ui->locationID->setText(tr("%1").arg(devInfo[index].LocId));
        ui->description->setText(tr("%1").arg(devInfo[index].Description));
    }
}
