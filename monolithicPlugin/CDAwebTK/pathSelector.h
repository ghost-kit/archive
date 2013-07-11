#ifndef PATHSELECTOR_H
#define PATHSELECTOR_H


#include "pqPropertyWidget.h"
#include "pqFileChooserWidget.h"

#include <QWidget>
#include <QString>
#include <QMap>
#include <QStringList>

#include <vtkSMProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMStringVectorProperty.h>


namespace Ui {
class pathSelector;
}

class pathSelector : public pqPropertyWidget
{
    Q_OBJECT
    typedef pqPropertyWidget Superclass;
    
public:
    explicit pathSelector(vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject = 0);
    ~pathSelector();

    void apply();
    
private:
    Ui::pathSelector *ui;
    vtkSMStringVectorProperty *svp;

};

#endif // PATHSELECTOR_H
