#include "swftControlsStackUnit.h"
#include "ui_swftControlsStackUnit.h"

swftControlsStackUnit::swftControlsStackUnit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::swftControlsStackUnit)
{
    ui->setupUi(this);
    ui->subButtonStack->hide();
}

swftControlsStackUnit::~swftControlsStackUnit()
{
    delete ui;
}
