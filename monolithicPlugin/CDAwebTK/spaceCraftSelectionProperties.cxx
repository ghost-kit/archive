#include "spaceCraftSelectionProperties.h"
#include "ui_spaceCraftSelectionProperties.h"

spaceCraftSelectionProperties::spaceCraftSelectionProperties(vtkSMProxy *smproxy, vtkSMProperty *smgroup, QWidget *parentObject) :
    Superclass(smproxy, parentObject), ui(new Ui::spaceCraftSelectionProperties)
{
    ui->setupUi(this);
}

spaceCraftSelectionProperties::~spaceCraftSelectionProperties()
{
    delete ui;
}
