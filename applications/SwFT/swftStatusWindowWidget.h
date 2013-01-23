#ifndef SWFTSTATUSWINDOWWIDGET_H
#define SWFTSTATUSWINDOWWIDGET_H

#include <QWidget>
#include <QPointer>
#include "pqComponentsModule.h"

class pqOutputPort;
class QTreeWidgetItem;
class vtkEventQtSlotConnect;
class vtkPVDataInformation;

namespace Ui {
class swftStatusWindowWidget;
}

class swftStatusWindowWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit swftStatusWindowWidget(QWidget *parent = 0);
    ~swftStatusWindowWidget();
    
    pqOutputPort *getOutputPort();

public slots:
    void updateInformation();

    void setOutputPort(pqOutputPort *source);

private slots:


private:
    void fillDataInformation(vtkPVDataInformation* info);

private:
    Ui::swftStatusWindowWidget *ui;

    QPointer<pqOutputPort> OutputPort;
    vtkEventQtSlotConnect *VTKConnect;

};

#endif // SWFTSTATUSWINDOWWIDGET_H
