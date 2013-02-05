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
    ui->setupUi(this);

    this->PipelineModel = new pqPipelineModel(this);
    this->FilteredPipelineModel = new pqPipelineAnnotationFilterModel(this);
    this->FilteredPipelineModel->setSourceModel(this->PipelineModel);

    //Connect the model to the ServerManager model
    pqServerManagerModel *smModel = pqApplicationCore::instance()->getServerManagerModel();

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


    //new pqPipelineModelSelectionAdaptor(this->getSelectionModel)));

    //add test element
    swftSimplePipelineElement *test = new swftSimplePipelineElement();
    swftSimplePipelineElement *test2 = new swftSimplePipelineElement();
    QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    QFrame *line0 = this->lineWidget(QString("line0"));
    QFrame *line1 = this->lineWidget(QString("line1"));
    QFrame *line2 = this->lineWidget(QString("line2"));

    test->setToolName(QString("Test Item"));
    test2->setToolName(QString("Test Item Two!"));

    ui->scrollAreaWidgetContents->layout()->addWidget(line0);
    ui->scrollAreaWidgetContents->layout()->addWidget(test);
    ui->scrollAreaWidgetContents->layout()->addWidget(line1);
    ui->scrollAreaWidgetContents->layout()->addWidget(test2);
    ui->scrollAreaWidgetContents->layout()->addWidget(line2);
    ui->scrollAreaWidgetContents->layout()->addItem(verticalSpacer);


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
    this->PipelineModel->setView(view);
}


//----------------------------------------------------------------------//
void swftSimplePipelineViewer::handleIndexClicked(const QModelIndex &index_)
{


}

//----------------------------------------------------------------------//
void swftSimplePipelineViewer::setVisibility(bool visible, const QModelIndexList &indexes)
{
    pqDisplayPolicy* display_policy = pqApplicationCore::instance()->getDisplayPolicy();
    bool begun_undo_set = false;

    foreach(QModelIndex index_, indexes)
    {
        //get object relative to pqPipelineModel
        const pqPipelineModel* model = this->getPipelineModel(index_);
        QModelIndex index = this->pipelineModelIndex(index_);

        pqServerManagerModelItem* smModelItem = model->getItemFor(index);
        pqPipelineSource *source = qobject_cast<pqPipelineSource*>(smModelItem);
        pqOutputPort *port = source? source->getOutputPort(0) : qobject_cast<pqOutputPort*>(smModelItem);

        if(port)
        {
            if(!begun_undo_set)
            {
                begun_undo_set = true;
                if(indexes.size() == 1)
                {
                    source = port->getSource();
                    BEGIN_UNDO_SET(QString("%1  %2").arg(visible? "show" : "hide").arg(source->getSMName()));

                }
                else
                {
                    BEGIN_UNDO_SET(QString("%1 Selected").arg(visible? "show" : "hide"));

                }
            }

            if(visible)
            {
                //make sure the given port is selected
                pqActiveObjects::instance().setActivePort(port);
            }
            display_policy->setRepresentationVisibility(port, pqActiveObjects::instance().activeView(), visible);
        }

    }

    if(begun_undo_set)
    {
        END_UNDO_SET();
    }

    if(pqActiveObjects::instance().activeView())
    {
        pqActiveObjects::instance().activeView()->render();
    }

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
const QModelIndex swftSimplePipelineViewer::pipelineModelIndex(const QModelIndex &index) const
{
    if(qobject_cast<const pqPipelineModel*>(index.model()))
    {
        return index;
    }

    const QSortFilterProxyModel *filterModel = qobject_cast<const QSortFilterProxyModel*>(index.model());

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

void swftSimplePipelineViewer::expandWithModelIndexTranslation(const QModelIndex &index)
{
    // remove if we dont need this function
}
