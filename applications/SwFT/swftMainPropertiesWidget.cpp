#include "swftMainPropertiesWidget.h"
#include "ui_swftMainPropertiesWidget.h"

swftMainPropertiesWidget::swftMainPropertiesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::swftMainPropertiesWidget)
{
    ui->setupUi(this);
}

swftMainPropertiesWidget::~swftMainPropertiesWidget()
{
    delete ui;
}
