#ifndef FTDIDEVICECHOOSERDIALOG_H
#define FTDIDEVICECHOOSERDIALOG_H

#include <QDialog>
#include <QStringList>
#include <Windows.h>
#include "ftd2xx.h"

#include "filehandle.h"

namespace Ui {
class FTDIDeviceChooserDialog;
}

class FTDIDeviceChooserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FTDIDeviceChooserDialog(QWidget *parent = 0);
    ~FTDIDeviceChooserDialog();

    QString deviceName(ulong type);
    int numberOfDevices();
    int defaultConnection();

public slots:
    void deviceInfoList();
    void updateInfo(int index);
    void saveDefaultConnection();

private:
    Ui::FTDIDeviceChooserDialog *ui;
    FT_DEVICE_LIST_INFO_NODE *devInfo;
    DWORD numDevs;
    DWORD iDevice;
    FT_STATUS ftStatus;
};

#endif // FTDIDEVICECHOOSERDIALOG_H
