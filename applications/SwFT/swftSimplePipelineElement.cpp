#include "swftSimplePipelineElement.h"
#include "ui_swftSimplePipelineElement.h"

#include "pqPipelineSource.h"
#include "pqServerManagerModel.h"
#include "pqServer.h"
#include "pqDisplayPolicy.h"
#include "pqApplicationCore.h"
#include "pqActiveObjects.h"
#include "pqLiveInsituVisualizationManager.h"
#include "pqOutputPort.h"
#include "pqPipelineAnnotationFilterModel.h"
#include "pqPipelineModel.h"
#include "pqPipelineModelSelectionAdaptor.h"
#include "pqView.h"

#include <QKeyEvent>
#include <assert.h>

swftSimplePipelineElement::swftSimplePipelineElement(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::swftSimplePipelineElement)
{
    ui->setupUi(this);
    this->pipeline = NULL;
    this->pipelineIndex = -1;

    //connect view button to setVisibilityState
    QObject::connect(ui->visibleButton, SIGNAL(toggled(bool)), this, SLOT(setVisibilityState(bool)));
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
    ui->visibleButton->setText((state?"ON":"OFF"));
}

void swftSimplePipelineElement::setPipelineLink(swftPipelineLeafListView *item)
{
    //this is just a reference... do not delete
    this->pipeline = item;
}

void swftSimplePipelineElement::setVisibilityState(bool visible)
{
//    std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;
    std::cout << "Setting Visibility to " << ((visible) ? "ON" : "OFF") << std::endl;

    //get display policy
    pqDisplayPolicy * display_policy = pqApplicationCore::instance()->getDisplayPolicy();


    //Get object relative to pqPipelineModel
    if(this->pipeline)
    {
        if(visible)
        {
            ui->visibleButton->setText(QString("ON"));
        }
        else
        {
            ui->visibleButton->setText(QString("OFF"));
        }

        QModelIndex index_ = this->getControllerItem()->Index;
        QModelIndex index;
        const pqPipelineModel *model = this->getPipelineModel(index_);
        index = this->pipelineModelIndex(index_);

        //get server manager item
        pqServerManagerModelItem * smModelItem = model->getItemFor(index);
        pqPipelineSource *source = qobject_cast<pqPipelineSource*>(smModelItem);
        pqOutputPort *port = (source ? source->getOutputPort(0) : qobject_cast<pqOutputPort*>(smModelItem));

        if(port)
        {
            source = port->getSource();

            if(port->getServer()->getResource().scheme() == "catalyst")
            {
                pqLiveInsituVisualizationManager *mgr =
                        qobject_cast<pqLiveInsituVisualizationManager*>(port->getServer()->property("LiveInsituVisualizationManager").value<QObject*>());

                if(mgr && mgr->addExtract(port))
                {
                    // refresh the pipeline browser icon.
                }

            }
            else
            {
                if(visible)
                {
                    // Make sure the given port is selected specially if we are in multi-server / catylist configuration type
                    pqActiveObjects::instance().setActivePort(port);
                }
                display_policy->setRepresentationVisibility(port, pqActiveObjects::instance().activeView(), visible);

                if(pqActiveObjects::instance().activeView())
                {
                    pqActiveObjects::instance().activeView()->render();
                }

            }
        }
    }
    else
    {
        std::cerr << "button object not instantiated" << std::endl;
    }

}

const QModelIndex swftSimplePipelineElement::pipelineModelIndex(const QModelIndex &index) const
{
    if(qobject_cast<const pqPipelineModel*>(index.model()))
    {
        return index;
    }

    const QSortFilterProxyModel *filterModel = qobject_cast<const QSortFilterProxyModel*>(index.model());

    //make a recursive call to support unknown filter depth
    return this->pipelineModelIndex(filterModel->mapToSource(index));
}

const pqPipelineModel *swftSimplePipelineElement::getPipelineModel(const QModelIndex &index) const
{
    if(const pqPipelineModel* model = qobject_cast<const pqPipelineModel*>(index.model()))
    {
        return model;
    }

    const QSortFilterProxyModel *filterModel = qobject_cast<const QSortFilterProxyModel *>(index.model());
    assert("Invalid model used inside index" && filterModel);

    return this->getPipelineModel(filterModel->mapToSource(index));
}


