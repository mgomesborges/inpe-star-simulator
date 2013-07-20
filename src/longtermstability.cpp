#include "longtermstability.h"
#include "ui_longtermstabilitydialog.h"

LongTermStability::LongTermStability(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LongTermStability)
{
    ui->setupUi(this);
}

LongTermStability::~LongTermStability()
{
    delete ui;
}
