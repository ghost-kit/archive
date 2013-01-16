#ifndef SWFTCOMMANDSUBBUTTON_H
#define SWFTCOMMANDSUBBUTTON_H

#include <QWidget>

namespace Ui {
class swftCommandSubButton;
}

class swftCommandSubButton : public QWidget
{
    Q_OBJECT
    
public:
    explicit swftCommandSubButton(QWidget *parent = 0);
    ~swftCommandSubButton();
    
private:
    Ui::swftCommandSubButton *ui;
};

#endif // SWFTCOMMANDSUBBUTTON_H
