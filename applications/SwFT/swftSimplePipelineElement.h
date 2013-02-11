#ifndef SWFTSIMPLEPIPELINEELEMENT_H
#define SWFTSIMPLEPIPELINEELEMENT_H

#include <QWidget>
#include <QString>
#include <QModelIndex>

#include "swftPipelineLeafListView.h"

class swftPipelineLeafListViewItem;

namespace Ui {
class swftSimplePipelineElement;
}

class swftSimplePipelineElement : public QWidget
{
    Q_OBJECT

public:
    explicit swftSimplePipelineElement(QWidget *parent = 0);
    ~swftSimplePipelineElement();

    void setToolName(const QString &title);
    void setToolState(bool state);

    void setPipelineLink(swftPipelineLeafListView *item);

    swftPipelineLeafListViewItem *getControllerItem()
    {
        if(this->pipeline && this->pipelineIndex >= 0)
            return this->pipeline->nodeList[pipelineIndex];
        else
            return NULL;
    }

    void setPipelineIndex(int index)
    {
        this->pipelineIndex = index;
    }

    int getPipelineIndex()
    {
        return this->pipelineIndex;
    }


public slots:
    void setVisibilityState(bool visible);

signals:

private:
    Ui::swftSimplePipelineElement *ui;

//    swftPipelineLeafListViewItem *controllerItem;

    swftPipelineLeafListView * pipeline;
    int pipelineIndex;

    const QModelIndex pipelineModelIndex(const QModelIndex &index) const;
    const pqPipelineModel *getPipelineModel(const QModelIndex &index) const;

};

#endif // SWFTSIMPLEPIPELINEELEMENT_H
