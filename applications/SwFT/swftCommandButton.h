#ifndef SWFTCOMMANDBUTTON_H
#define SWFTCOMMANDBUTTON_H

#include <QWidget>

namespace Ui {
class swftCommandButton;
}

class swftCommandButton : public QWidget
{
    Q_OBJECT
    
public:
    explicit swftCommandButton(QWidget *parent = 0);
    ~swftCommandButton();
    
private:
    Ui::swftCommandButton *ui;
};

#endif // SWFTCOMMANDBUTTON_H
