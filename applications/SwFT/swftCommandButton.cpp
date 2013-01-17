#include "swftCommandButton.h"
#include "ui_swftCommandButton.h"

swftCommandButton::swftCommandButton( QWidget *parent) :
    QCommandLinkButton(parent),
    ui(new Ui::swftCommandButton)
{
    ui->setupUi(this);

}

swftCommandButton::~swftCommandButton()
{
    delete ui;
}
