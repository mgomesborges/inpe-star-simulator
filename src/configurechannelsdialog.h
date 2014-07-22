#ifndef CONFIGURECHANNELSDIALOG_H
#define CONFIGURECHANNELSDIALOG_H

#include <QDialog>

namespace Ui {
    class ConfigureChannelsDialog;
}

class ConfigureChannelsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureChannelsDialog(QWidget *parent = 0);
    ~ConfigureChannelsDialog();

signals:
    void loadValues();
    void setValues(QVector<double> values);

public slots:
    void on_btnLoadValues_clicked();
    void accept();

private:
    Ui::ConfigureChannelsDialog *ui;
};

#endif // CONFIGURECHANNELSDIALOG_H
