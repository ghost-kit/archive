#include "swftCommandSubButton.h"
#include "ui_swftCommandSubButton.h"

swftCommandSubButton::swftCommandSubButton(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::swftCommandSubButton)
{
    ui->setupUi(this);
}

swftCommandSubButton::~swftCommandSubButton()
{
    delete ui;
}
