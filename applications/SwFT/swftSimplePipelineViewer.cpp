#include "swftSimplePipelineViewer.h"
#include "ui_swftSimplePipelineViewer.h"

#include "swftSimplePipelineElement.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDisplayPolicy.h"
#include "pqLiveInsituVisualizationManager.h"
#include "pqOutputPort.h"
#include "pqPipelineAnnotationFilterModel.h"
#include "pqPipelineModel.h"
#include "pqPipelineModelSelectionAdaptor.h"
#include "pqPipelineSource.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqUndoStack.h"
#include "pqView.h"

#include <QHeaderView>
#include <QKeyEvent>
#include <QSpacerItem>

#include <assert.h>

//=========================================================================

swftSimplePipelineViewer::swftSimplePipelineViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::swftSimplePipelineViewer)
{
//    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

    ui->setupUi(this);

    //get the pipeline model
    this->PipelineModel = new pqPipelineModel(this);
    this->FilteredPipelineModel = new pqPipelineAnnotationFilterModel(this);
    this->FilteredPipelineModel->setSourceModel(this->PipelineModel);

    //build the node list (will be empty at first)
    this->leafList = new swftPipelineLeafListView(this->PipelineModel);


    //Connect the model to the ServerManager model
    pqServerManagerModel *smModel = pqApplicationCore::instance()->getServerManagerModel();


    //connect the relevent signals/slots
    QObject::connect(smModel, SIGNAL(preServerAdded(pqServer*)),
                     this->PipelineModel, SLOT(addServer(pqServer*)));

    QObject::connect(smModel, SIGNAL(serverRemoved(pqServer*)),
                     this->PipelineModel, SLOT(removeServer(pqServer*)));

    QObject::connect(smModel, SIGNAL(sourceAdded(pqPipelineSource*)),
                     this->PipelineModel, SLOT(addSource(pqPipelineSource*)));

    QObject::connect(smModel, SIGNAL(sourceRemoved(pqPipelineSource*)),
                     this->PipelineModel, SLOT(removeSource(pqPipelineSource*)));

    QObject::connect(smModel, SIGNAL(connectionAdded(pqPipelineSource*,pqPipelineSource*,int)),
                     this->PipelineModel, SLOT(addConnection(pqPipelineSource*,pqPipelineSource*,int)));

    QObject::connect(smModel, SIGNAL(connectionRemoved(pqPipelineSource*,pqPipelineSource*,int)),
                     this->PipelineModel, SLOT(removeConnection(pqPipelineSource*,pqPipelineSource*,int)));

    QObject::connect(&pqActiveObjects::instance(), SIGNAL(viewChanged(pqView*)),
                     this, SLOT(setActiveView(pqView*)));


}

QFrame* swftSimplePipelineViewer::lineWidget(const QString name)
{
    QFrame *line;
    line = new QFrame();
    line->setObjectName(name);
    line->setFrameShape(QFrame::HLine);

    return line;
}

swftSimplePipelineViewer::~swftSimplePipelineViewer()
{
    delete ui;
}

//-----------------------------------------------------------------------//
void swftSimplePipelineViewer::setActiveView(pqView *view)
{
//    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;


    this->PipelineModel->setView(view);

    if(view)
    {
        std::cout << "Active View: " << this->PipelineModel->view()->getSMName().toAscii().data() << std::endl;

        if(this->PipelineModel->view()->getSMName().compare(QString("RV2D1")) || this->PipelineModel->view()->getSMName().compare(QString("RV2D2")))
        {
//            this->PipelineModel->
        }

    }
//    this->leafList->resetRoot();
    this->populateControls();


}


//----------------------------------------------------------------------//
void swftSimplePipelineViewer::handleIndexClicked(const QModelIndex &index_)
{
//    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;


}



//-------------------------------------------------------------------//
void swftSimplePipelineViewer::enableAnnotationFilter(const QString &annotationKey)
{
    this->FilteredPipelineModel->enableAnnotationFilter(annotationKey);
}

//-------------------------------------------------------------------//
void swftSimplePipelineViewer::disableAnnotationFilter()
{
    this->FilteredPipelineModel->disableAnnotationFilter();
}

//------------------------------------------------------------------//
void swftSimplePipelineViewer::enableSessionFilter(vtkSession *session)
{
    this->FilteredPipelineModel->enableSessionFilter(session);
}

//------------------------------------------------------------------//
void swftSimplePipelineViewer::disableSessionFilter()
{
    this->FilteredPipelineModel->disableSessionFilter();
}

//------------------------------------------------------------------//

void swftSimplePipelineViewer::updateData(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
//    std::cout << __FILE__ << " " << __FUNCTION__ << " Line: " << __LINE__ << std::endl;
}

//------------------------------------------------------------------//
const QModelIndex swftSimplePipelineViewer::pipelineModelIndex(const QModelIndex &index) const
{
    if(qobject_cast<const pqPipelineModel*>(index.model()))
    {
        return index;
    }

    const QSortFilterProxyModel *filterModel = qobject_cast<const QSortFilterProxyModel *>(index.model());

    //make a recursive call to support unknown filter depth
    return this->pipelineModelIndex(filterModel->mapToSource(index));
}

//-----------------------------------------------------------------//
const pqPipelineModel* swftSimplePipelineViewer::getPipelineModel(const QModelIndex &index) const
{
    if(const pqPipelineModel* model = qobject_cast<const pqPipelineModel*>(index.model()))
    {
        return model;
    }

    const QSortFilterProxyModel *filterModel = qobject_cast<const QSortFilterProxyModel*>(index.model());
    assert("Invalid model used inside index" && filterModel);

    return this->getPipelineModel(filterModel->mapToSource(index));
}

//-----------------------------------------------------------------//
void swftSimplePipelineViewer::populateControls()
{
    //1) Remove existing controlls

//    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;


    if(ui->scrollAreaWidgetContents->layout() != NULL)
    {
        QLayoutItem* item;
        while ( ( item = ui->scrollAreaWidgetContents->layout()->takeAt( 0 ) ) != NULL )
        {
            if(item)
            {
                delete item->widget();
                delete item;
            }
        }
    }

    //2) populate with new controlls
    QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

//    std::cout << "Number of nodes: " << this->leafList->nodeList.count() << std::endl;

    for(int y = 0; y < this->leafList->nodeList.count(); y++)
    {
        if(!this->leafList->nodeList[y]->HasChildren)
        {
            swftSimplePipelineElement *newControl = new swftSimplePipelineElement();

//            std::cout << "Controler: " << this->leafList->nodeList[y]->name.toAscii().data() << std::endl;

            const QString toolName = this->leafList->nodeList[y]->name;
            //store reference in controller object (so we know what is being clicked!)
            newControl->setPipelineLink(this->leafList);
            newControl->setPipelineIndex(y);
            //set the name in the tool to represent pipeline name
            newControl->setToolName(toolName);
            //set the tool status
            newControl->setToolState(this->leafList->nodeList[y]->itemSelected);

            ui->scrollAreaWidgetContents->layout()->addWidget(newControl);
        }

    }

    ui->scrollAreaWidgetContents->layout()->addItem(verticalSpacer);

}

//-----------------------------------------------------------------//
void swftSimplePipelineViewer::expandWithModelIndexTranslation(const QModelIndex &index)
{
    // remove if we dont need this function
}
