#include "swftPipelineLeafListView.h"
#include <QItemEditorFactory>

swftPipelineLeafListView::swftPipelineLeafListView(QWidget *parent, QAbstractItemModel* model) :
    Superclass(parent)
{
    std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;

    this->setModel(model);
    this->Root = new swftPipelineLeafListViewItem();
    this->Internal = new swftPipelineLeafListViewInternal();
    this->addChildItems(this->Root, 1);
}

//==========================================================================

void swftPipelineLeafListView::setModel(QAbstractItemModel *model)
{
    this->Model = model;
}


//==========================================================================

void swftPipelineLeafListView::setRootIndex(const QModelIndex &index)
{
    if(index.isValid() && index.model() != this->Model) return;
    if(this->Root->Index == index) return;

    //this->resetRoot();
    this->Root->Index = index;
    this->addChildItems(this->Root, 1);
}

//==========================================================================

void swftPipelineLeafListView::addChildItems(swftPipelineLeafListViewItem *root, int parentChildCount)
{
//    std::cout << __FUNCTION__ << " " << __LINE__ << std::endl;
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
void swftPipelineLeafListView::flattenTree()
{


}

//==========================================================================



