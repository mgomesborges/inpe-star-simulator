#ifndef STAR_H
#define STAR_H

#include <QDialog>
#include <cmath>

namespace Ui {
class Star;
}

class Star : public QDialog
{
    Q_OBJECT
    
public:
    explicit Star(QWidget *parent = 0);
    ~Star();
    
    void setMagnitude(const QString& magnitude) const;
    void setTemperature(const QString& temperature) const;
    int magnitude();
    int temperature();

    std::vector< std::vector<double> > getData(int magnitude = 0, int temperature = 7500);

private:
    Ui::Star *ui;
};

#endif // STAR_H
