#include "ScInfoPropWidget.h"
#include "ui_ScInfoPropWidget.h"

#include "pqPropertiesPanel.h"

ScInfoPropWidget::ScInfoPropWidget(vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject)
    : Superclass(smproxy, parentObject),
    ui(new Ui::ScInfoPropWidget)
{
    ui->setupUi(this);

    ui->gridLayout->setMargin(pqPropertiesPanel::suggestedMargin());
    ui->gridLayout->setHorizontalSpacing(pqPropertiesPanel::suggestedHorizontalSpacing());
    ui->gridLayout->setVerticalSpacing(pqPropertiesPanel::suggestedVerticalSpacing());
    ui->gridLayout->setColumnStretch(0,0);
    ui->gridLayout->setColumnStretch(1,1);


}

ScInfoPropWidget::~ScInfoPropWidget()
{

    delete ui;
}
