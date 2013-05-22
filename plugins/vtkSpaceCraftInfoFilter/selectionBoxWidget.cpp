#include "selectionBoxWidget.h"
#include "ui_selectionBoxWidget.h"

selectionBoxWidget::selectionBoxWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::selectionBoxWidget)
{
    ui->setupUi(this);

    connect(ui->checkBox, SIGNAL(toggled(bool)), this, SLOT(itemSelected()));
}

selectionBoxWidget::~selectionBoxWidget()
{
    delete ui;
}

void selectionBoxWidget::configure(QString Name, QString Desc)
{
    this->Name = Name;
    this->Desc = Desc;

    ui->label->setText(Desc);
}

void selectionBoxWidget::itemSelected(bool state)
{

    emit signalSelected(this->Name, state);

}



