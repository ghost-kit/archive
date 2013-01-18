#include "swftStatusWindowWidget.h"
#include "ui_swftStatusWindowWidget.h"

swftStatusWindowWidget::swftStatusWindowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::swftStatusWindowWidget)
{
    ui->setupUi(this);
}

swftStatusWindowWidget::~swftStatusWindowWidget()
{
    delete ui;
}
