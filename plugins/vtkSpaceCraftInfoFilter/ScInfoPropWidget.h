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

    //listings
    QStringList GroupList;
    QStringList SubGroupList;

    //maps
    QMultiMap<QString , QString> InstrumentList;
    QMultiMap<QString , QString> DataList;
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

    //instrument Selection
    QList<QListWidgetItem*> selectedInstruments;

    //data selection
    QList<QListWidgetItem*> selectedData;
    QAtomicInt InstrumentLock;
    QAtomicInt DataLock;

    QAtomicInt ReprocessLock;

    QAtomicInt InstruemntSelectionsDenied;



    //handlers
    bool getSCList(filterNetworkAccessModule &manager);
    bool getSCInstrument(filterNetworkAccessModule &manager);
    bool getSciDataGroup(filterNetworkAccessModule &manager, QString dataset);

    bool getGroupsList();
    bool getObservatoryList(QString Group);
    bool loadGroupListToGUI();

    bool getInstrumentList();
    bool getDataSetsList();

    void getAllDataSetInfo(QStringList dataSets);
    void setupDataSets();




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
    void recheckSelections();
    void completedInstrumentProcessing();

};

#endif // SCINFOPROPWIDGET_H
