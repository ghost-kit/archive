#ifndef SWFTMAINPROPERTIESWIDGET_H
#define SWFTMAINPROPERTIESWIDGET_H

#include <QWidget>

namespace Ui {
class swftMainPropertiesWidget;
}

class swftMainPropertiesWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit swftMainPropertiesWidget(QWidget *parent = 0);
    ~swftMainPropertiesWidget();
    
private:
    Ui::swftMainPropertiesWidget *ui;
};

#endif // SWFTMAINPROPERTIESWIDGET_H
