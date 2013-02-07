#ifndef SWFTPIPELINELEAFLISTVIEW_H
#define SWFTPIPELINELEAFLISTVIEW_H

#include "pqWidgetsModule.h"
#include "pqFlatTreeView.h"
#include "pqComponentsModule.h"

#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QAbstractItemModel>


class pqPipelineModel;
class swftPipelineLeafListViewInternal;
class pqPipelineAnnotationFilterModel;
class pqPipelineSource;
class pqView;
class vtkSession;

//================================================
class swftPipelineLeafListViewItem
{
public:
    swftPipelineLeafListViewItem()
        : Children(), Index()
    {
        this->Parent = NULL;
        this->RowSelected = false;

        this->Parent = 0;
        this->ContentsY = 0;
        this->Height = 0;
        this->Indent = 0;
        this->Expandable = false;
        this->Expanded = false;
        this->RowSelected = false;
        this->HasChildren = false;

    }

    ~swftPipelineLeafListViewItem()
    {
        //cleanup children
        QList<swftPipelineLeafListViewItem *>::Iterator iter = this->Children.begin();
        for( ; iter != this->Children.end(); ++ iter)
        {
            delete *iter;
        }
        this->Children.clear();
    }

public:
    swftPipelineLeafListViewItem *Parent;
    QList<swftPipelineLeafListViewItem *> Children;
    QPersistentModelIndex Index;

    int ContentsY;
    int Height;
    int Indent;

    bool Expandable;
    bool Expanded;
    bool RowSelected;

    QString name;
    bool HasChildren;

};
//====================================================
class swftPipelineLeafListViewInternal
{
public:
    swftPipelineLeafListViewInternal()
        : ShfitStart(), Index(), KeySearch()
    {
        this->Editor = 0;
    }

    ~swftPipelineLeafListViewInternal() {}

    QPersistentModelIndex ShfitStart;
    QPersistentModelIndex Index;
    QString KeySearch;
    QWidget *Editor;
};

//====================================================
class swftPipelineLeafListView : public QObject
{
    Q_OBJECT
    typedef QObject Superclass;

public:
    explicit swftPipelineLeafListView(QWidget *parent = 0, QAbstractItemModel *model = 0);
    
    void setModel(QAbstractItemModel *model);

    void setRootIndex(const QModelIndex &index);

    void addChildItems(swftPipelineLeafListViewItem *root, int paraentChildCount);


    const swftPipelineLeafListViewItem* getRoot() const
    {
        return this->Root;
    }

signals:
    
public slots:

protected:

    swftPipelineLeafListViewItem *Root;
    swftPipelineLeafListViewInternal *Internal;
private:
    QAbstractItemModel *Model;


};

#endif // SWFTPIPELINELEAFLISTVIEW_H
