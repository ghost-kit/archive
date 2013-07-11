#ifndef SPACECRAFTSELECTIONPROPERTIES_H
#define SPACECRAFTSELECTIONPROPERTIES_H

#include <QWidget>
#include "pqPropertyWidget.h"

namespace Ui {
class spaceCraftSelectionProperties;
}

class spaceCraftSelectionProperties : public QWidget, public pqPropertyWidget
{
    Q_OBJECT
    typedef pqPropertyWidget Superclass;
    
public:
    explicit spaceCraftSelectionProperties(vtkSMProxy *smproxy, vtkSMProperty *smgroup, QWidget *parentObject = 0);
    ~spaceCraftSelectionProperties();
    
private:
    Ui::spaceCraftSelectionProperties *ui;
};

#endif // SPACECRAFTSELECTIONPROPERTIES_H
