#include "swftAnalysisToolsWidget.h"
#include "ui_swftAnalysisToolsWidget.h"

swftAnalysisToolsWidget::swftAnalysisToolsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::swftAnalysisToolsWidget)
{
    ui->setupUi(this);
}

swftAnalysisToolsWidget::~swftAnalysisToolsWidget()
{
    delete ui;
}
