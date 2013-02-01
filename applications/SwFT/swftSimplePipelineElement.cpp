#include "swftSimplePipelineElement.h"
#include "ui_swftSimplePipelineElement.h"

swftSimplePipelineElement::swftSimplePipelineElement(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::swftSimplePipelineElement)
{
    ui->setupUi(this);
}

swftSimplePipelineElement::~swftSimplePipelineElement()
{
    delete ui;
}

void swftSimplePipelineElement::setToolName(const QString &title)
{
    ui->ViewElementName->setText(title);
}

