#ifndef SWFTCOMMANDBUTTON_H
#define SWFTCOMMANDBUTTON_H

#include <QWidget>
#include <QCommandLinkButton>

namespace Ui {
class swftCommandButton;
}

class swftCommandButton : public QCommandLinkButton
{
    Q_OBJECT
    
public:
    explicit swftCommandButton(QWidget *parent = 0);
    ~swftCommandButton();
    
private:
    Ui::swftCommandButton *ui;

signals:

};

#endif // SWFTCOMMANDBUTTON_H
