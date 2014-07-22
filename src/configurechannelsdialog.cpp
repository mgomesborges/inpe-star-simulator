#include "configurechannelsdialog.h"
#include "ui_configurechannelsdialog.h"

ConfigureChannelsDialog::ConfigureChannelsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureChannelsDialog)
{
    ui->setupUi(this);

    QValidator *channelValidator =
            new QRegExpValidator(QRegExp("^0$|^[1-9][0-9]{0,2}$|^[1-3][0-9]{0,3}$|^40([0-8][0-9]|[9][0-5])$"), this);
    ui->value->setValidator(channelValidator);
}

ConfigureChannelsDialog::~ConfigureChannelsDialog()
{
    delete ui;
}

void ConfigureChannelsDialog::on_btnLoadValues_clicked()
{
    close();
    emit loadValues();
}

void ConfigureChannelsDialog::accept()
{
    QVector<double> values;
    emit setValues(values.fill(ui->value->text().toInt(), 96));
    close();
}
