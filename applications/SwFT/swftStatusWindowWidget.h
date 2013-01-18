#ifndef SWFTSTATUSWINDOWWIDGET_H
#define SWFTSTATUSWINDOWWIDGET_H

#include <QWidget>

namespace Ui {
class swftStatusWindowWidget;
}

class swftStatusWindowWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit swftStatusWindowWidget(QWidget *parent = 0);
    ~swftStatusWindowWidget();
    
private:
    Ui::swftStatusWindowWidget *ui;
};

#endif // SWFTSTATUSWINDOWWIDGET_H
