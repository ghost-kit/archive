#ifndef SCINFOPROPWIDGET_H
#define SCINFOPROPWIDGET_H

#include "pqPropertyWidget.h"
#include "filterNetworkAccessModule.h"

#include <QWidget>
#include <QListWidgetItem>

#include <vtkSMProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMStringVectorProperty.h>

#include <QReadWriteLock>
#include <QTableWidgetItem>

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

    void apply();

protected:
    //Filter Components
    vtkSMProxy *smProxy;
    vtkSMProperty *smProperty;
    vtkSMStringVectorProperty *svp;

    //Cached Objects
    filterNetworkList *currentGroupObjects;
    filterNetworkList *currentInstrumentObjects;
    filterNetworkList *currentObservatoryObjects;
    QSet<filterNetworkList *> currentDataGroupObjects;
    QMap<QString, QList<filterNetworkList *> > currentVariablesObjects;

    //listings
    QStringList ObsGroupList;
    QStringList ObservatoryList;

    //maps
    QMultiMap<QString , QString> InstrumentList;
    QMap<QString, QMap<QString , QString> > DataList;
    QMap<QString, QMap<QString, QString> > VariableList;
    QStringList DataSetRetrunList;

    //current target
    QString currentGroup;
    QString currentObservatory;
    QString currentInstrument;
    QString currentDataSet;
    QMap <QString, QStringList> currentDataGroups;

    // URL Information
    QString baseURL;
    QString dataViewSpacePhys;
    QString getObservatorys;
    QString getObservatoryGroups;
    QString getInstrumentTypes;
    QString getInstruments;
    QString getInventory;
    QString getDataSets;
    QString getDataGroups;
    QString getVariables;

    //instrument Selection
    QList<QListWidgetItem*> selectedInstruments;

    //data selection
    QList<QListWidgetItem*> selectedData;
    QAtomicInt InstrumentLock;
    QAtomicInt DataLock;

    QAtomicInt ReprocessLock;

    QAtomicInt InstruemntSelectionsDenied;
    QAtomicInt DataSelectionDenied;

    QTableWidgetItem *dataColumn1;
    QTableWidgetItem *dataColumn2;


    //handlers
    bool getSCList(filterNetworkAccessModule &manager);
    bool getSCInstrument(filterNetworkAccessModule &manager);
    bool getSciDataGroup(filterNetworkAccessModule &manager, QString dataset);
    bool getSciVariables(filterNetworkAccessModule &manager, QString dataset);

    bool getGroupsList();
    bool getObservatoryList(QString Group);
    bool loadGroupListToGUI();

    bool getInstrumentList();
    bool getDataSetsList();

    void getAllDataSetInfo(QStringList dataSets);
    void getAllVariableSetInfo(QMap<QString, QStringList> DataSetList);

    void setupDataSets();
    void setupVariableSets();




private:
    Ui::ScInfoPropWidget *ui;

private slots:
    void selectedGroup(QString selection);
    void selectedObservatory(QString selection);

    void instrumentSelectionChanged();
    void dataGroupSelectionChanged();

    void processDeniedInstrumentRequests();
    void processDeniedDataRequests();

signals:
    void recheckInstrumentSelections();
    void recheckDataSetSelction();
    void completedInstrumentProcessing();
    void completedDataProcessing();

};

#endif // SCINFOPROPWIDGET_H
