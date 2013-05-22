#ifndef SCINFOPROPWIDGET_H
#define SCINFOPROPWIDGET_H

#include "pqPropertyWidget.h"
#include "filterNetworkAccessModule.h"
#include "selectionBoxWidget.h"

#include <QWidget>

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

    //network managers
    filterNetworkAccessModule *SCListManager;
    filterNetworkAccessModule *SCInstrumentManager;
    filterNetworkAccessModule *SCDataManager;
    filterNetworkAccessModule *SCObservatoryManager;

    //listings
    QStringList GroupList;
    QStringList SubGroupList;
    QStringList InstrumentNameList;
    QStringList InstrumentDescList;
    QStringList DataSetList;

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


    //handlers
    bool getSCList();
    bool getSCInstrument();

    bool getGroupsList();
    bool getObservatoryList(QString Group);
    bool loadGroupListToGUI();

    bool getInstrumentList();
    bool getDataSet();

private:
    Ui::ScInfoPropWidget *ui;

private slots:
    void selectedGroup(QString selection);
    void selectedObservatory(QString selection);
    void selectedInstrument(QString selection);
    void selectedDataSet(QString selection);
    void selectedInstrumentElement(QModelIndex index);



};

#endif // SCINFOPROPWIDGET_H
