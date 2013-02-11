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

    void setControllerItem(swftPipelineLeafListViewItem *item);

    swftPipelineLeafListViewItem *getControllerItem()
    {
        return this->controllerItem;
    }


public slots:
    void visibilitySet(bool visible);

signals:

private:
    Ui::swftSimplePipelineElement *ui;

    swftPipelineLeafListViewItem *controllerItem;

};

#endif // SWFTSIMPLEPIPELINEELEMENT_H
