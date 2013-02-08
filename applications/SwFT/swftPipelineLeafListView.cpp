#include "swftPipelineLeafListView.h"
#include <QItemEditorFactory>
#include <QModelIndex>

swftPipelineLeafListView::swftPipelineLeafListView(QAbstractItemModel* model, QWidget *parent) :
    Superclass(parent)
{
    std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;

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
    std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;

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
    std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;

    if(index.isValid() && index.model() != this->Model) return;
    if(this->Root->Index == index) return;

    //this->resetRoot();
    this->Root->Index = index;
    this->addChildItems(this->Root, 1);
}

//==========================================================================

void swftPipelineLeafListView::addChildItems(swftPipelineLeafListViewItem *root, int parentChildCount)
{
    std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;
    if(root)
    {
//        std::cout << "Adding children" << std::endl;

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
                    child->Parent = root;
                    child->Index = index;
                    root->Children.append(child);
                    value = this->Model->data(index);
                    name = value.toString();
                    child->name = name;

                    this->nodeList.append(child);
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
    std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;



}

//==========================================================================
void swftPipelineLeafListView::resetRoot()
{
    std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;

    //clean out and reset all root items
    QList<swftPipelineLeafListViewItem *>::Iterator iter = this->Root->Children.begin();
    for( ; iter != this->Root->Children.end(); ++iter)
    {
        delete *iter;
    }

    this->Root->Children.clear();

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



