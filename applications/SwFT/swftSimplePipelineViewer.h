#ifndef SWFTSIMPLEPIPELINEVIEWER_H
#define SWFTSIMPLEPIPELINEVIEWER_H

#include <QWidget>
#include "pqComponentsModule.h"

#include <QModelIndex>
#include <QFrame>



class pqPipelineModel;
class pqPipelineAnnotationFilterModel;
class pqPipelineSource;
class pqView;
class vtkSession;

namespace Ui {
class swftSimplePipelineViewer;
}

class swftSimplePipelineViewer : public QWidget
{
    Q_OBJECT

public:
    explicit swftSimplePipelineViewer(QWidget *parent = 0);
    ~swftSimplePipelineViewer();

    //Set the visibility of selected item
    void setSelectionVisibility(bool visible);

    //set Annotation filter to use
    void enableAnnotationFilter(const QString &annotationKey);

    // Disable any Annotation filter
    void disableAnnotationFilter();

    // set session filter to use
    void enableSessionFilter(vtkSession * session);

    // Disable any Session filter
    void disableSessionFilter();

    void updateData(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    QFrame *lineWidget(const QString name);
signals:

public slots:
    // Set the Active view.  By Default connected to
    // pqActiveObjects::viewChanged() so it keeps track of the active view.
    void setActiveView(pqView*);

protected slots:
    void handleIndexClicked(const QModelIndex &index_);
    void expandWithModelIndexTranslation(const QModelIndex &);

protected:
    //sets te visibility for items in the indices list.
    void setVisibility(bool visible, const QModelIndexList &indexes);
    pqPipelineModel *PipelineModel;
    pqPipelineAnnotationFilterModel *FilteredPipelineModel;
    const QModelIndex pipelineModelIndex(const QModelIndex &index) const;
    const pqPipelineModel *getPipelineModel(const QModelIndex &index) const;

private:
    Ui::swftSimplePipelineViewer *ui;
};


#endif // SWFTSIMPLEPIPELINEVIEWER_H
