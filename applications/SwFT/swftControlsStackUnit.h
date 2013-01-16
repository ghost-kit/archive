#ifndef SWFTCONTROLSSTACKUNIT_H
#define SWFTCONTROLSSTACKUNIT_H

#include <QWidget>

namespace Ui {
class swftControlsStackUnit;
}

class swftControlsStackUnit : public QWidget
{
    Q_OBJECT
    
public:
    explicit swftControlsStackUnit(QWidget *parent = 0);
    ~swftControlsStackUnit();
    
private:
    Ui::swftControlsStackUnit *ui;
};

#endif // SWFTCONTROLSSTACKUNIT_H
