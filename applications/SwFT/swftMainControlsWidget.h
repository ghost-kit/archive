#ifndef SWFTMAINCONTROLSWIDGET_H
#define SWFTMAINCONTROLSWIDGET_H

#include <QWidget>

namespace Ui {
class swftMainControlsWidget;
}

class swftMainControlsWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit swftMainControlsWidget(QWidget *parent = 0);
    ~swftMainControlsWidget();
    
private:
    Ui::swftMainControlsWidget *ui;
};

#endif // SWFTMAINCONTROLSWIDGET_H
