#ifndef SWFTPIPELINELEAFLISTVIEW_H
#define SWFTPIPELINELEAFLISTVIEW_H

#include "pqWidgetsModule.h"
#include <QModelIndex>
#include <QStyleOptionViewItem>

class QAbstractItemModel;
class QColor;
class QFontMetrics;
class QHeaderView;
class QItemSelection;
class QItemSlectionModel;
class QPoint;
class QRect;


class swftPipelineLeafListView : public QObject
{
    Q_OBJECT
public:
    explicit swftPipelineLeafListView(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // SWFTPIPELINELEAFLISTVIEW_H
