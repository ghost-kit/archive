#include "scInfoProp.h"
#include "vtkSMProperty.h"
#include "vtkSMSourceProxy.h"
#include "pqPropertiesPanel.h"

#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>

scInfoProp::scInfoProp(vtkSMProxy* smproxy, vtkSMProperty* smproperty, QWidget* parentObject)
    : Superclass(smproxy, parentObject)
{

    this->setShowLabel(false);

    QGridLayout * gridLayout = new QGridLayout(this);
    gridLayout->setMargin(pqPropertiesPanel::suggestedMargin());
    gridLayout->setHorizontalSpacing(pqPropertiesPanel::suggestedHorizontalSpacing());
    gridLayout->setVerticalSpacing(pqPropertiesPanel::suggestedVerticalSpacing());
    gridLayout->setColumnStretch(0,0);
    gridLayout->setColumnStretch(1,1);

    QLabel* customLabel = new QLabel("Custom Widget", this);
    gridLayout->addWidget(customLabel);

    QCheckBox* checkbox = new QCheckBox("<--- pqMyPropertyWidgetForProperty",this);
    checkbox->setObjectName("CheckBox");
    this->addPropertyLink(checkbox, "checked", SIGNAL(toggled(bool)), smproperty);
    gridLayout->addWidget(checkbox);

    this->setChangeAvailableAsChangeFinished(true);

}


scInfoProp::~scInfoProp()
{

}
