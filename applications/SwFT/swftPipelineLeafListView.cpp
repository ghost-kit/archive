#include "swftPipelineLeafListView.h"
#include <QItemEditorFactory>
#include <QModelIndex>

#include "pqComponentsModule.h"
#include "pqApplicationCore.h"
#include "pqOutputPort.h"
#include "pqActiveObjects.h"
#include "pqDisplayPolicy.h"
#include "pqServerManagerModel.h"
#include "pqServer.h"
#include "pqView.h"
#include "pqPipelineSource.h"
#include "pqPipelineModel.h"
#include "pqPipelineModelSelectionAdaptor.h"


#include <assert.h>
swftPipelineLeafListView::swftPipelineLeafListView(QAbstractItemModel* model, QWidget *parent) :
    Superclass(parent)
{
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

    this->Model = 0;
    this->Root = new swftPipelineLeafListViewItem();
    this->Internal = new swftPipelineLeafListViewInternal();

    //set the model (if requested)
    if(model)
    {
        this->setModel(model);
    }
}

//==========================================================================

void swftPipelineLeafListView::setModel(QAbstractItemModel *model)
{
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

    //don't mess with a good thing if it is already a good thing!
    if(model == this->Model)
    {
        return;
    }

    //disconnect the old model if we are starting a new one
    if(this->Model)
    {
        this->disconnect(this->Model, 0, this, 0);
    }

    //TODO: See if we need a selection model to make this work correctly

    //clean up the current root
    this->resetRoot();

    //set the model
    this->Model = model;

    //set model connections
    if(this->Model)
    {
        this->connect(this->Model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateData(QModelIndex,QModelIndex)));
    }

    //add the children to the model
    this->addChildItems(this->Root, 1);

}


//==========================================================================

void swftPipelineLeafListView::setRootIndex(const QModelIndex &index)
{
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

    if(index.isValid() && index.model() != this->Model) return;
    if(this->Root->Index == index) return;

    //this->resetRoot();
    this->Root->Index = index;
    this->addChildItems(this->Root, 1);
}

//==========================================================================

void swftPipelineLeafListView::addChildItems(swftPipelineLeafListViewItem *root, int parentChildCount)
{
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;


    if(root)
    {
        //        std::cout << "Adding children" << std::endl;
        //TODO: strip this down to bare minimum

        if(this->Model->canFetchMore(root->Index))
        {
            if(parentChildCount > 1 && !root->Expanded)
            {
                root->Expandable = true;
                return;
            }
            else
            {
                this->Model->fetchMore(root->Index);
            }
        }

        int count = this->Model->rowCount(root->Index);
        if(count > 0)
        {
            root->HasChildren = true;
        }

        QModelIndex index;
        swftPipelineLeafListViewItem *child = 0;
        QString name;
        QVariant value;

        for(int i = 0; i < count; i++)
        {
            index = this->Model->index(i, 0, root->Index);
            if(index.isValid())
            {
                child = new swftPipelineLeafListViewItem();
                if(child)
                {


                    // itemIsDisplayed returns the oposite from what we want, so we will cang this here
                    //TODO: Fix itemIsDisplayed to fix the return value.
                    if(itemIsDisplayed(index))
                    {
                        child->itemSelected = false;
                    }
                    else
                    {
                        child->itemSelected = true;
                    }

                    child->Parent = root;
                    child->Index = index;
                    root->Children.append(child);
                    value = this->Model->data(index);
                    name = value.toString();
                    child->name = name;

                    this->nodeList.append(child);

                    std::cout << "Child Select state: " << child->itemSelected << " Name: " << name.toAscii().data() << std::endl;

                    this->addChildItems(child,count);



                }
            }
        }

    }

}

//==========================================================================
void swftPipelineLeafListView::updateData(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QModelIndex parentIndex = topLeft.parent();
    if(parentIndex != bottomRight.parent())
    {
        std::cout << "exiting updateData with not changes" << std::endl;
        return;
    }

    //if the data changes, we need to update everything!
    std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

    //need to re-create the tree
    this->resetRoot();
    this->addChildItems(this->Root,1);

}

//==========================================================================
void swftPipelineLeafListView::resetRoot()
{
    std::cout << __FILE__ << __FUNCTION__ << " " << __LINE__ << std::endl;

    //clean out and reset all root items
    QList<swftPipelineLeafListViewItem *>::Iterator iter = this->Root->Children.begin();
    for( ; iter != this->Root->Children.end(); ++iter)
    {
        delete *iter;
    }

    //clear the lists
    //these lists contain the same objects
    this->Root->Children.clear();
    this->nodeList.clear();

    if(this->Root->Index.isValid())
    {
        this->Root->Index = QPersistentModelIndex();
    }
}

//==========================================================================
swftPipelineLeafListViewItem *swftPipelineLeafListView::getItem(const QModelIndex &index) const
{
    swftPipelineLeafListViewItem *item = 0;
    swftPipelineLeafListViewItemRows rowList;
    if(this->getIndexRowList(index, rowList))
    {
        item = this->getItem(rowList);
    }

    return item;
}

//==========================================================================
swftPipelineLeafListViewItem *swftPipelineLeafListView::getItem(const swftPipelineLeafListViewItemRows &rowList) const
{
    swftPipelineLeafListViewItem *item = this->Root;
    swftPipelineLeafListViewItemRows::ConstIterator iter = rowList.begin();
    for( ; iter != rowList.end(); ++iter)
    {
        if(*iter >= 0 && *iter < item->Children.size())
        {
            item = item->Children[*iter];
        }
        else
        {
            return 0;
        }
    }

    return item;
}

//==========================================================================
bool swftPipelineLeafListView::getIndexRowList(const QModelIndex &index, swftPipelineLeafListViewItemRows &rowList) const
{
    //if looking at the wrong model, return
    if(index.isValid() && index.model() != this->Model)
    {
        return false;
    }

    //if we haven't initialized root yet, return.
    if(!this->Root)
    {
        return false;
    }

    //Get Hierarchy
    QModelIndex tempIndex = index;
    if(index.isValid() && index.column() > 0)
    {
        tempIndex = index.sibling(0, 0);
    }

    while(tempIndex.isValid() && tempIndex != this->Root->Index)
    {
        rowList.prepend(tempIndex.row());
        tempIndex = tempIndex.parent();
    }

    //return false if the root was not found in ancestry
    return tempIndex == this->Root->Index;
}

//==========================================================================
const QModelIndex swftPipelineLeafListView::pipelineModelIndex(const QModelIndex &index) const
{
    if(qobject_cast<const pqPipelineModel*>(index.model()))
    {
        return index;
    }

    const QSortFilterProxyModel* filterModel = qobject_cast<const QSortFilterProxyModel*>(index.model());

    // assert ("Invalid model used inside index" && filterModel);

    //make recursive call for depth checking
    return this->pipelineModelIndex(filterModel->mapToSource(index));


}

//==========================================================================
bool swftPipelineLeafListView::itemIsDisplayed(const QModelIndex &index_)
{
    bool visible = false;
    pqDisplayPolicy* display_policy = pqApplicationCore::instance()->getDisplayPolicy();

    const pqPipelineModel * model = this->getPipelineModel(index_);
    QModelIndex index = this->pipelineModelIndex(index_);

    pqServerManagerModelItem *smModelItem = model->getItemFor(index);
    pqPipelineSource *source = qobject_cast<pqPipelineSource*>(smModelItem);
    pqOutputPort* port = source ? source->getOutputPort(0) : qobject_cast<pqOutputPort*>(smModelItem);

    if(port)
    {
        source = port->getSource();

        visible = display_policy->getVisibility(pqActiveObjects::instance().activeView(), port);

    }

    return visible;
}

//==========================================================================
const pqPipelineModel *swftPipelineLeafListView::getPipelineModel(const QModelIndex &index) const
{
    if(const pqPipelineModel * model = qobject_cast<const pqPipelineModel *>(index.model()))
    {
        return model;
    }

    const QSortFilterProxyModel* filterModel = qobject_cast<const QSortFilterProxyModel*>(index.model());
    assert("Invalid model used inside index" && filterModel);

    //depth recursive call
    return this->getPipelineModel(filterModel->mapToSource(index));
}


//==========================================================================



