//TODO:
//      2) Fix DataSet repeated querries
//      3) Fix Variable Selection
//      4) Optimize the call pattern for information



#include "ScInfoPropWidget.h"
#include "ui_ScInfoPropWidget.h"

#include "DateTime.h"
#include "pqPropertiesPanel.h"
#include "filterNetworkAccessModule.h"
#include <QListWidgetItem>
#include <vtkSMProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMGlobalPropertiesManager.h>
#include <vtkSMStringVectorProperty.h>
#include <vtkSMDoubleVectorProperty.h>
#include <iomanip>

ScInfoPropWidget::ScInfoPropWidget(vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject)
    : Superclass(smproxy, parentObject),
      ui(new Ui::ScInfoPropWidget)
{

    this->smProperty = smproperty;
    this->smProxy = smproxy;

    this->svp = vtkSMStringVectorProperty::SafeDownCast(smproperty);

    if(!svp)
    {
        return;
    }

    //get properties
    vtkSMDoubleVectorProperty *timeStartProperties = vtkSMDoubleVectorProperty::SafeDownCast(smproxy->GetProperty("TimeRangeInfoStart"));
    this->startMJD = timeStartProperties->GetElement(0);

    vtkSMDoubleVectorProperty *timeEndProperties = vtkSMDoubleVectorProperty::SafeDownCast(smproxy->GetProperty("TimeRangeInfoEnd"));
    this->endMJD = timeEndProperties->GetElement(0);

    std::cout << "Start Time: " << std::setprecision(16)  << DateTime(this->startMJD).getDateTimeString() << std::endl;
    std::cout << "End Time:   " << DateTime(this->endMJD).getDateTimeString() << std::endl;


    //URLs for CDAWeb
    this->baseURL = QString("http://cdaweb.gsfc.nasa.gov/WS/cdasr/1");
    this->dataViewSpacePhys = QString("/dataviews/sp_phys/");
    this->getObservatorys = QString("observatories");
    this->getObservatoryGroups = QString("observatoryGroups");
    this->getInstrumentTypes = QString("instrumentTypes");
    this->getInstruments = QString("instruments");
    this->getDataSets = QString("datasets");
    this->getDataGroups=QString("datasets");
    this->getVariables=QString("variables");
    this->getInventory=QString("inventory");


    //Setup the UI
    this->dataColumn1 = new QTableWidgetItem("Instrument");
    this->dataColumn2 = new QTableWidgetItem("Description");

    ui->setupUi(this);
    ui->gridLayout->setMargin(pqPropertiesPanel::suggestedMargin());
    ui->gridLayout->setHorizontalSpacing(pqPropertiesPanel::suggestedHorizontalSpacing());
    ui->gridLayout->setVerticalSpacing(pqPropertiesPanel::suggestedVerticalSpacing());

    ui->Observatory->setDisabled(true);
    ui->Instruments->setDisabled(true);
    ui->DataSet->setDisabled(true);

    //Load first set of Values
    filterNetworkAccessModule SCListManager;
    this->getSCList(SCListManager);
    this->currentGroupObjects = SCListManager.getFinalOjects();

    //get the observatory group list
    this->getGroupsList();

    //Load the group list
    this->loadGroupListToGUI();

    //connect signals to slots

    /** Group Connections */
    connect(ui->Group, SIGNAL(activated(QString)), this, SLOT(selectedGroup(QString)));

    /** Observatory Connections */
    connect(ui->Observatory, SIGNAL(activated(QString)), this, SLOT(selectedObservatory(QString)));

    /** Instrument Connections */
    connect(ui->Instruments, SIGNAL(itemSelectionChanged()), this, SLOT(instrumentSelectionChanged()));
    connect(this, SIGNAL(recheckInstrumentSelections()), this, SLOT(instrumentSelectionChanged()));

    /** Data Selection Changed */
    connect(ui->DataSet, SIGNAL(itemSelectionChanged()), this, SLOT(dataGroupSelectionChanged()));
    connect(this, SIGNAL(recheckDataSetSelction()), this, SLOT(dataGroupSelectionChanged()));

    /** Property Links */
    this->addPropertyLink(ui->Variables, smproxy->GetPropertyName(smproperty), SIGNAL(itemSelectionChanged()), this->svp);

}


//==================================================================
ScInfoPropWidget::~ScInfoPropWidget()
{

    delete ui;
}

//==================================================================
void ScInfoPropWidget::apply()
{

    //TODO: Need to capture the Selected Variables to pass back to filter.


    std::cout << "APPLY CLICKED" << std::endl;

    //build a list of elements
    QList<QTreeWidgetItem *> selectedElements = ui->DataSet->selectedItems();
    QList<QTreeWidgetItem *>::Iterator iter;

    QMap<QString, QStringList> DataMap;

    //Get Instruments and Keys
    for(iter = selectedElements.begin(); iter != selectedElements.end(); ++iter)
    {
        QString Instrument;

        //Make sure we only get the Elements with Parents (i.e. non-insturment slecetions)
        if((*iter)->parent())
        {
            Instrument = (*iter)->parent()->text(0);
            DataMap[Instrument].push_back((*iter)->text(1));
        }
    }



    //get the list of variables
    QList<QTreeWidgetItem *> selectedVariables = ui->Variables->selectedItems();

    QMap<QString,QStringList> VariableMap;

    for(iter = selectedVariables.begin(); iter != selectedVariables.end(); ++iter)
    {

        QMap<QString, QString> DataSet;
        QString Data;

        if((*iter)->parent())
        {
            Data = (*iter)->parent()->text(0);
            Data = this->DataList[(*iter)->parent()->text(1)].key(Data);
            VariableMap[Data].push_back(this->VariableList[(*iter)->parent()->text(0)].key((*iter)->text(0)));

//            std::cout << "DataSet: " << Data.toAscii().data() << std::endl;
//            std::cout << "Variable: " << VariableMap[Data].back().toAscii().data() << std::endl;
//            std::cout << "==========" << std::endl;
        }
    }


    //Create the needed string
    //  Insturments separated by ;
    //  Data sets separated by ,
    QString DataString;

    QStringList keys = DataMap.keys();
    QList<QStringList> values = DataMap.values();

    for(int x = 0; x < keys.size(); x++)
    {
        if(x != 0)
        {
            DataString = DataString + ";";
        }

        DataString = DataString + keys[x] + ":";

        for(int y = 0; y < values[x].size(); y++)
        {
            if(y != 0)
            {
                DataString = DataString + "," ;
            }

            DataString = DataString + values[x][y] + "~" ;

            for(int g = 0; g < VariableMap[values[x][y]].size(); g++)
            {
                if(g != 0)
                {
                    DataString = DataString + "|";
                }

                DataString = DataString + VariableMap[values[x][y]][g];
            }
        }

    }

    std::cerr << "CodeString: " << DataString.toAscii().data() << std::endl;

    this->svp->SetElement(0, this->currentGroup.toAscii().data());
    this->svp->SetElement(1, this->currentObservatory.toAscii().data());
    this->svp->SetElement(2, DataString.toAscii().data());

    //apply the upstream parameters
    Superclass::apply();

}

//==================================================================
bool ScInfoPropWidget::getSCList(filterNetworkAccessModule &manager)
{

    //get data from network
    manager.Get(this->baseURL + this->dataViewSpacePhys + this->getObservatoryGroups,
                QString("ObservatoryGroups"),
                QString("ObservatoryGroupDescription"));

    return true;

}

//==================================================================
bool ScInfoPropWidget::getSCInstrument(filterNetworkAccessModule &manager)
{

    std::cout << QString(this->baseURL + this->dataViewSpacePhys + this->getInstrumentTypes
                         + "?observatory=" + this->currentObservatory).toAscii().data() << std::endl;

    manager.Get(this->baseURL + this->dataViewSpacePhys + this->getInstruments
                + "?observatory=" + this->currentObservatory,
                QString("Instruments"),
                QString("InstrumentDescription"));
    return true;

}

//==================================================================
bool ScInfoPropWidget::getSciDataGroup(filterNetworkAccessModule &manager, QString dataset)
{
    std::cout << QString(this->baseURL + this->dataViewSpacePhys + this->getDataGroups).toAscii().data() << std::endl;

    manager.Get(this->baseURL + this->dataViewSpacePhys + this->getDataSets + "?observatoryGroup="
                + this->currentGroup + "&observatory=" + this->currentObservatory + "&instrument=" + dataset ,
                QString("Datasets"),
                QString("DatasetDescription"));

    return true;

}

//==================================================================
bool ScInfoPropWidget::getSciVariables(filterNetworkAccessModule &manager, QString dataset)
{
    std::cout << QString(this->baseURL + this->dataViewSpacePhys + this->getDataGroups).toAscii().data() << std::endl;

    manager.Get(this->baseURL + this->dataViewSpacePhys + this->getDataSets + "/" + dataset + "/" + this->getVariables,
                QString("Variables"),
                QString("VariableDescription"));
    return true;
}

//==================================================================
//parse for groups
bool ScInfoPropWidget::getGroupsList()
{

    std::cout << "Size of List: " << this->currentGroupObjects->size() << std::endl;

    for(int x = 0; x < this->currentGroupObjects->size(); x++)
    {
        filterNetworkObject *currentMap = this->currentGroupObjects->operator [](x);

        QList<QString> keys = currentMap->keys();

        QString name = currentMap->operator []("Name");

        this->ObsGroupList.push_back(name);

    }

    ObsGroupList.sort();

    return true;
}

//==================================================================
bool ScInfoPropWidget::getDataSetsList()
{

    return true;
}

//==================================================================
//parse Observatory List
bool ScInfoPropWidget::getObservatoryList(QString Group)
{
    //clear what is there already
    this->ObservatoryList.clear();

    //find the ObservatoryId
    for(int x = 0; x < this->currentGroupObjects->size(); x++)
    {
        filterNetworkObject *currentMap = this->currentGroupObjects->operator [](x);

        if(currentMap->operator []("Name") == Group)
        {
            this->ObservatoryList = currentMap->values("ObservatoryId");
        }
    }

    this->ObservatoryList.sort();

    return true;
}

//==================================================================
//load the group list to the GUI
bool ScInfoPropWidget::loadGroupListToGUI()
{
    ui->Group->clear();
    ui->Group->addItem("--- Select Group ---");
    ui->Group->addItems(this->ObsGroupList);

    ui->Observatory->setDisabled(true);
    ui->Instruments->setDisabled(true);
    ui->DataSet->setDisabled(true);
    ui->Variables->setDisabled(true);

    return true;
}

//==================================================================
//process the change in selection on the selection box
void ScInfoPropWidget::selectedGroup(QString selection)
{
    std::cout << "Group Selected: " << selection.toAscii().data() << std::endl;

    this->currentGroup = selection;
    this->currentObservatory = "";

    if(selection != "--- Select Group ---")
    {
        this->getObservatoryList(selection);
        ui->Observatory->clear();
        ui->Observatory->addItem("--- Select Observatory ---");
        ui->Observatory->addItems(this->ObservatoryList);
        ui->Observatory->setEnabled(true);
    }
    else
    {
        ui->Observatory->clear();
        ui->Observatory->setDisabled(true);
    }

    //clear downstream elements
    this->currentInstrument = "";

    ui->Instruments->clear();
    ui->Instruments->setDisabled(true);
    ui->DataSet->clear();
    ui->DataSet->setDisabled(true);

    this->currentDataSet = "";
}

//==================================================================
//process Observatories
void ScInfoPropWidget::selectedObservatory(QString selection)
{
    std::cout << "Observatory Selected: " << selection.toAscii().data() << std::endl;

    this->currentObservatory = selection;
    //this->currentInstrumentObjects->clear();

    filterNetworkAccessModule SCInstrumentManager;

    //get data for next item
    this->getSCInstrument(SCInstrumentManager);
    this->currentInstrumentObjects = SCInstrumentManager.getFinalOjects();

    //get the required list
    this->getInstrumentList();

    ui->Variables->clear();
    ui->Variables->setDisabled(true);

    ui->Instruments->clear();
    ui->Instruments->setRowCount(this->InstrumentList.size());
    ui->Instruments->setColumnCount(2);

    ui->Instruments->setWordWrap(false);
    QTableWidgetItem *column1 = new QTableWidgetItem();
    QTableWidgetItem *column2 = new QTableWidgetItem();

    column1->setText( "Instrument");
    column2->setText( "Description");

    ui->Instruments->setHorizontalHeaderItem(0, column1);
    ui->Instruments->setHorizontalHeaderItem(1, column2);

    for(int x = 0; x < this->InstrumentList.size(); x++)
    {
        std::cout << "Adding Instruments..." << std::endl;

        QTableWidgetItem *newItem = new QTableWidgetItem();
        QTableWidgetItem *newItem2 = new QTableWidgetItem();

        newItem->setText( this->InstrumentList.keys()[x]);
        newItem2->setText( this->InstrumentList.values()[x]);

        newItem->setTextColor(QColor("Dark Blue"));

        ui->Instruments->setItem(x,0,newItem);
        ui->Instruments->setItem(x,1,newItem2);
    }


    ui->Instruments->setEnabled(true);

    ui->DataSet->clear();
    ui->DataSet->setDisabled(true);
}


//==================================================================
//Instrument Selection has Changed
void ScInfoPropWidget::getAllDataSetInfo(QStringList dataSets)
{

    this->currentDataGroupObjects.clear();

    QSet<filterNetworkList *> newDataObjectGroup;

    QStringList::Iterator iter2;
    for(iter2 = dataSets.begin(); iter2 != dataSets.end(); ++iter2)
    {
        QString item = (*iter2);

        filterNetworkAccessModule SCDataSetListManager;
        this->getSciDataGroup(SCDataSetListManager, item);
        newDataObjectGroup.insert(SCDataSetListManager.getFinalOjects());
    }

    //this is the poor mans way of defeating the race condition...
    //still need to fix out of order access when copying...
    this->currentDataGroupObjects = newDataObjectGroup;

}

//==================================================================
void ScInfoPropWidget::getAllVariableSetInfo(QMap<QString, QStringList> DataSetList)
{
    int count = 0;

    QMap<QString, QList<filterNetworkList *> > newVariableObjectGroup;

    QMap<QString, QStringList>::Iterator iter;
    for(iter = DataSetList.begin(); iter != DataSetList.end(); ++iter)
    {
        QStringList Instrument = DataSetList.keys();
        QList<QStringList> DataSets = DataSetList.values();

        for(int i = 0; i < DataSets[count].size(); i++)
        {
            //Configure the Needs for Variables Selection List
            filterNetworkAccessModule SCVariableListManager;
            this->getSciVariables(SCVariableListManager, DataSets[count][i]);

            QString DataName = this->DataList[Instrument[count]][DataSets[count][i]];

            newVariableObjectGroup[Instrument[count] + "\t" + DataName].append(SCVariableListManager.getFinalOjects());
        }
        count ++;
    }
    //this is the poor mans way of defeating the race condition...
    //still need to fix out of order access when copying...
    this->currentVariablesObjects = newVariableObjectGroup;
}

//==================================================================
void ScInfoPropWidget::setupDataSets()
{

    QSet<filterNetworkList *>::Iterator iter;

    ui->DataSet->clear();
    this->DataList.clear();

    QMap<QString, QMap<QString , QString> > List;

    for(iter=this->currentDataGroupObjects.begin(); iter != this->currentDataGroupObjects.end(); ++iter)
    {
        filterNetworkList *item = (*iter);

        QList<QTreeWidgetItem*> treelist;
        QMap<QString, QString> temp;

        QString obsGroup;

        for(int x = 0; x < item->size(); x++)
        {
            filterNetworkObject *currentMap = item->operator [](x);

            QString label = currentMap->operator []("Label");
            QString id = currentMap->operator []("Id");
            obsGroup = currentMap->operator[]("Instrument");

            temp.insert(id,label);

            QTreeWidgetItem * child = new QTreeWidgetItem();
            child->setText(0,label);
            child->setText(1,id);

            treelist.push_back(child);

        }

        List.insert(obsGroup, temp);

        QTreeWidgetItem *newItem = new QTreeWidgetItem();
        newItem->setText(0,obsGroup);
        newItem->setText(1,obsGroup);
        newItem->setTextColor(0, QColor("dark blue"));
        newItem->addChildren(treelist);

        ui->DataSet->setColumnCount(2);
        ui->DataSet->hideColumn(1);
        ui->DataSet->addTopLevelItem(newItem);
        ui->DataSet->setEnabled(true);
        ui->DataSet->expandAll();

    }

    this->DataList = List;
}

//==================================================================
void ScInfoPropWidget::setupVariableSets()
{
    QMap<QString, QList<filterNetworkList *> >::Iterator iter;
    QList<filterNetworkList *>::Iterator iter2;

    int count = 0;

    ui->Variables->clear();
    this->VariableList.clear();

    QMap<QString, QMap<QString, QString> > List;

    for(iter=this->currentVariablesObjects.begin(); iter != this->currentVariablesObjects.end(); ++iter)
    {
        QStringList keys = this->currentVariablesObjects.keys();

        QList<filterNetworkList *> item = (*iter);

        QList<QTreeWidgetItem*> treelist;
        QMap<QString, QString> temp;

        QStringList DataSet = keys[count].split("\t");


        for(iter2= item.begin(); iter2 != item.end(); ++iter2)
        {
            filterNetworkList *item2 = (*iter2);

            for(int x = 0; x < item2->size(); x++)
            {
                filterNetworkObject *currentMap = item2->operator [](x);

                QString Name = currentMap->operator []("Name");
                QString Desc = currentMap->operator []("LongDescription");

                temp.insert(Name, Desc);

                QTreeWidgetItem * child = new QTreeWidgetItem();
                child->setText(0, Desc);
                child->setText(1, Name);

                treelist.push_back(child);

            }
            List.insert(DataSet[1], temp);
        }

        count ++;

        QTreeWidgetItem *newItem = new QTreeWidgetItem();
        newItem->setText(0,DataSet[1]);     //this is the DataSet
        newItem->setText(1, DataSet[0]);    //this is the Instrument
        newItem->setTextColor(0, QColor("dark blue"));
        newItem->addChildren(treelist);

        ui->Variables->setColumnCount(2);
        ui->Variables->hideColumn(1);
        ui->Variables->addTopLevelItem(newItem);
        ui->Variables->setEnabled(true);
        ui->Variables->expandAll();

    }

    this->VariableList = List;
}

//==================================================================
void ScInfoPropWidget::instrumentSelectionChanged()
{
    //get the lock and process, or mark as future request
    if(this->InstrumentLock.testAndSetAcquire(0,1))
    {
        //get all selected items... we will strip out columns other than 0 later..
        QList<QTableWidgetItem*> instruments = ui->Instruments->selectedItems();
        QStringList dataSet;

        //verify that the instruments list is no empty before processing.
        if(!instruments.isEmpty())
        {
            //create a list of items
            QList<QTableWidgetItem*>::iterator iter;
            for(iter = instruments.begin(); iter != instruments.end(); ++iter)
            {
                QString item = (*iter)->text();

                //make sure we only get the info from column 0
                if(ui->Instruments->column((*iter)) == 0)
                {
                    //add to the data list
                    dataSet.push_back(item);
                }
            }

            //retrieve all of the data information
            getAllDataSetInfo(dataSet);
        }

        //setup the next selection box.
        this->setupDataSets();

        //release lock.
        this->InstrumentLock.deref();

        //see if requests came in while we were locked.
        connect(this, SIGNAL(completedInstrumentProcessing()), this, SLOT(processDeniedInstrumentRequests()));

        if(this->InstruemntSelectionsDenied.testAndSetAcquire(1,1))
        {
            //requests pending, so process them.
            emit this->completedInstrumentProcessing();
        }
    }
    else
    {
        //Lock was not aquired, thus request is being processed.  since we only really need to
        // process the last request, we will just set this counter to 1, regardless of how many
        // requests are made.
        this->InstruemntSelectionsDenied.testAndSetAcquire(0,1);
    }
}


//==================================================================
void ScInfoPropWidget::dataGroupSelectionChanged()
{

    if(this->DataLock.testAndSetAcquire(0,1))
    {

        QList<QTreeWidgetItem*> dataSets = ui->DataSet->selectedItems();

        QMap<QString, QStringList> DataMap;

        if(!dataSets.isEmpty())
        {
            //create a list of items
            QList<QTreeWidgetItem*>::iterator iter;
            for(iter = dataSets.begin(); iter != dataSets.end(); ++iter)
            {
                QString Instrument;

                //Make sure we only get the Elements with Parents (i.e. non-insturment slecetions)
                if((*iter)->parent())
                {
                    Instrument = (*iter)->parent()->text(0);
                    DataMap[Instrument].push_back((*iter)->text(1));
                }
            }
            getAllVariableSetInfo(DataMap);
        }

        this->setupVariableSets();
        this->DataLock.deref();

        connect(this, SIGNAL(completedDataProcessing()), this, SLOT(processDeniedDataRequests()));

        if(this->DataSelectionDenied.testAndSetAcquire(1,1))
        {
            emit this->completedDataProcessing();
        }
    }
    else
    {
        this->DataSelectionDenied.testAndSetAcquire(0,1);
    }

}

//==================================================================
void ScInfoPropWidget::processDeniedInstrumentRequests()
{

    if(this->InstruemntSelectionsDenied.testAndSetAcquire(1,0))
    {
        emit this->recheckInstrumentSelections();
    }

}

//==================================================================
void ScInfoPropWidget::processDeniedDataRequests()
{


    if(this->DataSelectionDenied.testAndSetAcquire(1,0))
    {
        std::cout << "Retrying requests" << std::endl;

        emit this->recheckDataSetSelction();
    }

}


//==================================================================
//get instrument list for current selections
bool ScInfoPropWidget::getInstrumentList()
{

    std::cout << "Getting the Data List" << std::endl;
    this->InstrumentList.clear();

    for(int x = 0; x < this->currentInstrumentObjects->size(); x++)
    {
        filterNetworkObject *currentMap = this->currentInstrumentObjects->operator [](x);

        QString name = currentMap->operator []("Name");
        QString desc = currentMap->operator []("LongDescription");

        std::cout << "Name: " << name.toAscii().data() << " Description: " << desc.toAscii().data() << std::endl;

        this->InstrumentList.insert(name, desc);

    }

    if(!this->currentInstrumentObjects->isEmpty())
    {
        ObsGroupList.sort();
    }

    return true;
}


