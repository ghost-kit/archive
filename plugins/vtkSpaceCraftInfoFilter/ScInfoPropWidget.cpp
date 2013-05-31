//TODO: 1) Adjust the system to accept Instrument Type as the top level selection
//      2) Fix DataSet repeated querries
//      3) Fix Variable Selection
//      4) Optimize the call pattern for information



#include "ScInfoPropWidget.h"
#include "ui_ScInfoPropWidget.h"

#include "pqPropertiesPanel.h"
#include "filterNetworkAccessModule.h"
#include <QListWidgetItem>
#include <vtkSMProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMGlobalPropertiesManager.h>
#include <vtkSMStringVectorProperty.h>

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
    connect(this, SIGNAL(recheckSelections()), this, SLOT(instrumentSelectionChanged()));

    /** Data Selection Changed */
    connect(ui->DataSet, SIGNAL(itemSelectionChanged()), this, SLOT(dataGroupSelectionChanged()));
    connect(this, SIGNAL(recheckSelections()), this, SLOT(instrumentSelectionChanged()));

    /** Property Links */
    this->addPropertyLink(ui->DataSet, smproxy->GetPropertyName(smproperty), SIGNAL(itemSelectionChanged()), this->svp);

}


//==================================================================
ScInfoPropWidget::~ScInfoPropWidget()
{

    delete ui;
}

//==================================================================
void ScInfoPropWidget::apply()
{
    std::cout << "APPLY CLICKED" << std::endl;

    //build a list of elements
    QList<QTreeWidgetItem *> selectedElements = ui->DataSet->selectedItems();
    QList<QTreeWidgetItem *>::Iterator iter = selectedElements.begin();

    QMap<QString, QStringList> DataMap;

    //Get Instruments and Keys
    for(iter = selectedElements.begin(); iter != selectedElements.end(); ++iter)
    {
        QString Instrument;

        //Make sure we only get the Elements with Parents (i.e. non-insturment slecetions)
        if((*iter)->parent())
        {
            Instrument = (*iter)->parent()->text(0);
            DataMap[Instrument].push_back(this->DataList[Instrument].key((*iter)->text(0)));
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
        if(DataString.isEmpty())
        {
            DataString = keys[x] + ":";

            std::cout << "Size of Value: " << values[x].size() << std::endl;
            for(int y = 0; y < values[x].size(); y++)
            {
                if(y == 0)
                {
                    DataString = DataString + values[x][y];
                }
                else
                {
                    DataString = DataString + "," + values[x][y];
                }

            }
        }
        else
        {
            DataString = DataString + ";" + keys[x] + ":";

            std::cout << "Size of Value: " << values[x].size() << std::endl;
            for(int y = 0; y < values[x].size(); y++)
            {
                if(y == 0)
                {
                    DataString = DataString + values[x][y];
                }
                else
                {
                    DataString = DataString + "," + values[x][y];
                }
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

    if(manager.getNetworkAccessStatus() == 0)
        return true;
    else
        return false;
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

    if(manager.getNetworkAccessStatus() == 0)
        return true;
    else
        return false;
}

//==================================================================
bool ScInfoPropWidget::getSciDataGroup(filterNetworkAccessModule &manager, QString dataset)
{
    std::cout << QString(this->baseURL + this->dataViewSpacePhys + this->getDataGroups).toAscii().data() << std::endl;

    manager.Get(this->baseURL + this->dataViewSpacePhys + this->getDataSets + "?observatoryGroup="
                + this->currentGroup + "&observatory=" + this->currentObservatory + "&instrument=" + dataset ,
                QString("Datasets"),
                QString("DatasetDescription"));

    if(manager.getNetworkAccessStatus() == 0)
        return true;
    else
        return false;
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

        this->GroupList.push_back(name);

    }

    GroupList.sort();

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
    this->SubGroupList.clear();

    //find the ObservatoryId
    for(int x = 0; x < this->currentGroupObjects->size(); x++)
    {
        filterNetworkObject *currentMap = this->currentGroupObjects->operator [](x);

        if(currentMap->operator []("Name") == Group)
        {
            this->SubGroupList = currentMap->values("ObservatoryId");
        }
    }

    this->SubGroupList.sort();

    return true;
}

//==================================================================
//load the group list to the GUI
bool ScInfoPropWidget::loadGroupListToGUI()
{
    ui->Group->clear();
    ui->Group->addItem("--- Select Group ---");
    ui->Group->addItems(this->GroupList);

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
        ui->Observatory->addItems(this->SubGroupList);
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
    ui->Instruments->setHorizontalHeaderItem(0,  this->dataColumn1);
    ui->Instruments->setHorizontalHeaderItem(1, this->dataColumn2);

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
    std::cerr << "Size of Current Data objects Group(Pre Processing): " << this->currentDataGroupObjects.size() << std::endl;

    this->currentDataGroupObjects.clear();

    std::cerr << "Size of Current Data objects Group (Post Clearing): " << this->currentDataGroupObjects.size() << std::endl;

    std::cout << "Processing List to get next dataset" << std::flush << std::endl;

    QSet<filterNetworkList *> newDataObjectGroup;


    QStringList::Iterator iter2;
    for(iter2 = dataSets.begin(); iter2 != dataSets.end(); ++iter2)
    {
        QString item = (*iter2);

        std::cout << "Item Contents: " << item.toAscii().data() << std::endl;

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
    std::cout << "Getting All Variable Set Information" << std::endl;
    int count = 0;

    QSet<filterNetworkList *> newVariableObjectGroup;

   QMap<QString, QStringList>::Iterator iter;
   for(iter = DataSetList.begin(); iter != DataSetList.end(); ++iter)
   {
       QStringList Instrument = DataSetList.keys();
       QList<QStringList> DataSets = DataSetList.values();

       std::cout << "Instrument: " << Instrument[count].toAscii().data() << std::endl;
       for(int i = 0; i < DataSets[count].size(); i++)
       {
           std::cout << "\t|" << DataSets[count][i].toAscii().data()
                     << ":\t" << (this->DataList[Instrument[count]][DataSets[count][i]]).toAscii().data() << std::endl;

           //Configure the Needs for Variables Selection List
           filterNetworkAccessModule SCVariableListManager;
           this->getSciVariables(SCVariableListManager, DataSets[count][i]);
           newVariableObjectGroup.insert(SCVariableListManager.getFinalOjects());

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

    std::cerr << "Size of Selection List: " << this->currentDataGroupObjects.size() << std::endl;

    for(iter=this->currentDataGroupObjects.begin(); iter != this->currentDataGroupObjects.end(); ++iter)
    {
        std::cout << "Processing..." << std::endl;
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

            std::cout << "Name: " << id.toAscii().data() << " Description: " << label.toAscii().data() << std::endl;

            temp.insert(id,label);

            QTreeWidgetItem * child = new QTreeWidgetItem();
            child->setText(0,QString(label));

            treelist.push_back(child);

        }

        List.insert(obsGroup, temp);

        QTreeWidgetItem *newItem = new QTreeWidgetItem();
        newItem->setText(0,obsGroup);
        newItem->addChildren(treelist);
        newItem->setExpanded(true);

        ui->DataSet->addTopLevelItem(newItem);
        ui->DataSet->setEnabled(true);

    }

    this->DataList = List;
}

//==================================================================
void ScInfoPropWidget::setupVariableSets()
{
    std::cout << "Setting up Variables list" << std::endl;
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
    std::cout << "Data Selection has changed." << std::endl;

    if(this->DataLock.testAndSetAcquire(0,1))
    {
        std::cout << "Lock Aquired" << std::endl;

        QList<QTreeWidgetItem*> dataSets = ui->DataSet->selectedItems();

        QMap<QString, QStringList> DataMap;
        std::cerr << "Size of Selection List (Pre Processing): " << dataSets.size() << std::endl;

        if(!dataSets.isEmpty())
        {
            std::cout << "Selected Items: " << std::endl;

            //create a list of items
            QList<QTreeWidgetItem*>::iterator iter;
            for(iter = dataSets.begin(); iter != dataSets.end(); ++iter)
            {

                QString Instrument;

                //Make sure we only get the Elements with Parents (i.e. non-insturment slecetions)
                if((*iter)->parent())
                {
                    Instrument = (*iter)->parent()->text(0);
                    DataMap[Instrument].push_back(this->DataList[Instrument].key((*iter)->text(0)));
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
        std::cerr << "Lock Not Aquired... marking for later use..." << std::endl;

        this->DataSelectionDenied.testAndSetAcquire(0,1);

    }

}

//==================================================================
void ScInfoPropWidget::processDeniedInstrumentRequests()
{

    if(this->InstruemntSelectionsDenied.testAndSetAcquire(1,0))
    {
        emit this->recheckSelections();
    }

}

//==================================================================
void ScInfoPropWidget::processDeniedDataRequests()
{


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
        GroupList.sort();
    }

    return true;
}


