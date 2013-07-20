#include "aboutsmsdialog.h"
#include "ui_aboutsmsdialog.h"

AboutSMSDialog::AboutSMSDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutSMSDialog)
{
    ui->setupUi(this);
}

AboutSMSDialog::~AboutSMSDialog()
{
    delete ui;
}

void AboutSMSDialog::setDLL(const QString &version) const
{
    ui->lineEditDLLVersion->setText(version);
}

void AboutSMSDialog::setSerialNumber(const QString &serialNumber) const
{
    ui->lineEditSerialNumber->setText(serialNumber);
}

void AboutSMSDialog::setFirstCoefficient(const QString &firstCoefficient) const
{
    ui->lineEditFirstCoefficient->setText(firstCoefficient);
}

void AboutSMSDialog::setSecondCoefficient(const QString &secondCoefficient) const
{
    ui->lineEditSecondCoefficient->setText(secondCoefficient);
}

void AboutSMSDialog::setThirdCoefficient(const QString &thirdCoefficient) const
{
    ui->lineEditThirdCoefficient->setText(thirdCoefficient);
}

void AboutSMSDialog::setIntercept(const QString &intercept) const
{
    ui->lineEditIntercept->setText(intercept);
}


