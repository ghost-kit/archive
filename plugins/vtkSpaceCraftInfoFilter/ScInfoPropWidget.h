#ifndef SCINFOPROPWIDGET_H
#define SCINFOPROPWIDGET_H

#include "pqPropertyWidget.h"
#include "filterNetworkAccessModule.h"
#include "selectionBoxWidget.h"

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
class ScInfoPropWidget;
}

class ScInfoPropWidget : public pqPropertyWidget
{
    Q_OBJECT
    typedef pqPropertyWidget Superclass;

public:
    ScInfoPropWidget(vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject = 0);
    ~ScInfoPropWidget();


protected:
    //Cached Objects
    filterNetworkList *currentGroupObjects;
    filterNetworkList *currentInstrumentObjects;
    filterNetworkList *currentObservatoryObjects;
    QList<filterNetworkList *> currentDataObjects;

    //listings
    QStringList GroupList;
    QStringList SubGroupList;

    //maps
    QMultiMap<QString , QString> InstrumentList;
    QMultiMap<QString , QString> DataSetList;


    //current target
    QString currentGroup;
    QString currentObservatory;
    QString currentInstrument;
    QString currentDataSet;

    // URL Information
    QString baseURL;
    QString dataViewSpacePhys;
    QString getObservatorys;
    QString getObservatoryGroups;
    QString getInstrumentTypes;
    QString getInstruments;
    QString getInventory;
    QString getDataSets;

    //instrument Selection
    QList<QListWidgetItem*> selectedInstruments;

    //data selection
    QList<QListWidgetItem*> selectedData;

    //handlers
    bool getSCList(filterNetworkAccessModule &manager);
    bool getSCInstrument(filterNetworkAccessModule &manager);

    bool getGroupsList();
    bool getObservatoryList(QString Group);
    bool loadGroupListToGUI();

    bool getInstrumentList();
    bool getDataSetOptions(filterNetworkAccessModule &manager, QString dataset);

private:
    Ui::ScInfoPropWidget *ui;

private slots:
    void selectedGroup(QString selection);
    void selectedObservatory(QString selection);
    void selectedInstrument(QString selection);
    void selectedDataSet(QString selection);
    void selectedInstrumentElement(QListWidgetItem *item);
    void instrumentSelectionChanged();
    void dataSelectionChanged();
    void getDataOptions();



};

#endif // SCINFOPROPWIDGET_H
