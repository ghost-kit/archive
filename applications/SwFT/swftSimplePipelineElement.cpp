#include "swftSimplePipelineElement.h"
#include "ui_swftSimplePipelineElement.h"

#include "pqPipelineSource.h"
#include "pqServerManagerModel.h"
#include "pqServer.h"
#include "pqServerManagerModelItem.h"
#include "pqDisplayPolicy.h"
#include "pqApplicationCore.h"


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

void swftSimplePipelineElement::setToolState(bool state)
{

    ui->visibleButton->setChecked(state);
}

void swftSimplePipelineElement::setControllerItem(swftPipelineLeafListViewItem *item)
{
    //this is just a reference... do not delete
    this->controllerItem = item;
}

void swftSimplePipelineElement::visibilitySet(bool visible)
{
}


