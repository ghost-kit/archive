#ifndef SCINFOPROPWIDGET_H
#define SCINFOPROPWIDGET_H

#include "pqPropertyWidget.h"
#include <QWidget>

namespace Ui {
class ScInfoPropWidget;
}

class ScInfoPropWidget : public pqPropertyWidget
{
    Q_OBJECT
    typedef pqPropertyWidget Superclass;

public:
    ScInfoPropWidget(vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject = 0);
    ~ScInfoPropWidget();
    
private:
    Ui::ScInfoPropWidget *ui;
};

#endif // SCINFOPROPWIDGET_H
