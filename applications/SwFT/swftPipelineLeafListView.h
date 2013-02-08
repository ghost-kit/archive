#ifndef SWFTPIPELINELEAFLISTVIEW_H
#define SWFTPIPELINELEAFLISTVIEW_H

#include "pqWidgetsModule.h"
#include "pqFlatTreeView.h"
#include "pqComponentsModule.h"

#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QAbstractItemModel>
#include <QPersistentModelIndex>
#include <QFilterTreeProxyModel.h>
#include <QSortFilterProxyModel>

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
        this->itemSelected = false;

        this->Parent = 0;
        this->ContentsY = 0;
        this->Height = 0;
        this->Indent = 0;
        this->Expandable = false;
        this->Expanded = false;
        this->itemSelected = false;
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
    bool itemSelected;

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

class swftPipelineLeafListViewItemRows: public QList<int> {};


//====================================================
class swftPipelineLeafListView : public QObject
{
    Q_OBJECT
    typedef QObject Superclass;

public:
    explicit swftPipelineLeafListView(QAbstractItemModel *model = 0, QWidget *parent = 0);
    
    void setModel(QAbstractItemModel *model);
    void setRootIndex(const QModelIndex &index);
    void addChildItems(swftPipelineLeafListViewItem *root, int paraentChildCount);
    const swftPipelineLeafListViewItem* getRoot() const {return this->Root;}

    QList<swftPipelineLeafListViewItem *> nodeList;

signals:
    
public slots:

protected slots:
    void updateData(const QModelIndex &topLeft, const QModelIndex &bottomRight);

protected:
    void resetRoot();

    swftPipelineLeafListViewItem *Root;
    swftPipelineLeafListViewInternal *Internal;

    swftPipelineLeafListViewItem* getItem(const QModelIndex &index) const;
    swftPipelineLeafListViewItem* getItem(const swftPipelineLeafListViewItemRows &rowList) const;
    bool getIndexRowList(const QModelIndex &index, swftPipelineLeafListViewItemRows &rowList) const;

    const QModelIndex pipelineModelIndex(const QModelIndex & index) const;

    bool itemIsDisplayed(const QModelIndex &index_);

    const pqPipelineModel* getPipelineModel(const QModelIndex &index) const;

private:
    QAbstractItemModel *Model;


};

#endif // SWFTPIPELINELEAFLISTVIEW_H
