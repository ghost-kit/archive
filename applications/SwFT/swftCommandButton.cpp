#include "swftCommandButton.h"
#include "ui_swftCommandButton.h"

swftCommandButton::swftCommandButton(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::swftCommandButton)
{
    ui->setupUi(this);
}

swftCommandButton::~swftCommandButton()
{
    delete ui;
}
