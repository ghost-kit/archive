#include "swftControlsStackUnit.h"
#include "ui_swftControlsStackUnit.h"

swftControlsStackUnit::swftControlsStackUnit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::swftControlsStackUnit)
{
    ui->setupUi(this);
}

swftControlsStackUnit::~swftControlsStackUnit()
{
    delete ui;
}
