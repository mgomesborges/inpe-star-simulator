#ifndef LONGTERMSTABILITY_H
#define LONGTERMSTABILITY_H

#include <QDialog>

namespace Ui {
class LongTermStability;
}

class LongTermStability : public QDialog
{
    Q_OBJECT
    
public:
    explicit LongTermStability(QWidget *parent = 0);
    ~LongTermStability();
    
private:
    Ui::LongTermStability *ui;
};

#endif // LONGTERMSTABILITY_H
