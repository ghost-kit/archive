#include "swftMainControlsWidget.h"
#include "ui_swftMainControlsWidget.h"

swftMainControlsWidget::swftMainControlsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::swftMainControlsWidget)
{
    ui->setupUi(this);
}

swftMainControlsWidget::~swftMainControlsWidget()
{
    delete ui;
}
