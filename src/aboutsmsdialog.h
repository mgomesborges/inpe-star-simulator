#ifndef ABOUTSMSDIALOG_H
#define ABOUTSMSDIALOG_H

#include <QDialog>

namespace Ui {
class AboutSMSDialog;
}

class AboutSMSDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AboutSMSDialog(QWidget *parent = 0);
    ~AboutSMSDialog();

    void setDLL(const QString&) const;
    void setSerialNumber(const QString&) const;
    void setFirstCoefficient(const QString&) const;
    void setSecondCoefficient(const QString&) const;
    void setThirdCoefficient(const QString&) const;
    void setIntercept(const QString&) const;

private:
    Ui::AboutSMSDialog *ui;
};

#endif // ABOUTSMSDIALOG_H
