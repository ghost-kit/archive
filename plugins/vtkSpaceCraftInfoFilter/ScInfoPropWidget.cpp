#include "ScInfoPropWidget.h"
#include "ui_ScInfoPropWidget.h"

#include "pqPropertiesPanel.h"
#include "filterNetworkAccessModule.h"
#include <QListWidgetItem>


ScInfoPropWidget::ScInfoPropWidget(vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject)
    : Superclass(smproxy, parentObject),
    ui(new Ui::ScInfoPropWidget)
{
    //URLs for CDAWeb
    this->baseURL = QString("http://cdaweb.gsfc.nasa.gov/WS/cdasr/1");
    this->dataViewSpacePhys = QString("/dataviews/sp_phys/");
    this->getObservatorys = QString("observatories");
    this->getObservatoryGroups = QString("observatoryGroups");
    this->getInstrumentTypes = QString("instrumentTypes");
    this->getInstruments = QString("instruments");
    this->getDataSets = QString("datasets");
    this->getInventory = QString("variables");

    //Setup the UI
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
    connect(ui->Group, SIGNAL(highlighted(QString)), this, SLOT(selectedGroup(QString)));

    /** Observatory Connections */
    connect(ui->Observatory, SIGNAL(activated(QString)), this, SLOT(selectedObservatory(QString)));
    connect(ui->Observatory, SIGNAL(highlighted(QString)), this, SLOT(selectedObservatory(QString)));

    /** Instrument Connections */
    connect(ui->Instruments, SIGNAL(itemSelectionChanged()), this, SLOT(instrumentSelectionChanged()));

    /** DataSet Connections */
    connect(ui->DataSet, SIGNAL(itemSelectionChanged()), this, SLOT(dataSelectionChanged()));

    //mark changes
    this->setChangeAvailableAsChangeFinished(true);

}

ScInfoPropWidget::~ScInfoPropWidget()
{

    delete ui;
}

bool ScInfoPropWidget::getSCList(filterNetworkAccessModule &manager)
{

    //get data from network
    manager.Get(this->baseURL + this->dataViewSpacePhys + this->getObservatoryGroups, QString("ObservatoryGroups"), QString("ObservatoryGroupDescription"));

    if(manager.getNetworkAccessStatus() == 0)
        return true;
    else
        return false;
}

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


//load the group list to the GUI
bool ScInfoPropWidget::loadGroupListToGUI()
{
    ui->Group->clear();
    ui->Group->addItem("--- Select Group ---");
    ui->Group->addItems(this->GroupList);

    return true;
}

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
    ui->Instruments->setDisabled(true);

    this->currentDataSet = "";
    ui->DataSet->clear();
    ui->DataSet->setDisabled(true);

}

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
    ui->Instruments->addItems(this->InstrumentList.values());
    ui->Instruments->setEnabled(true);
}


//process Instruments
void ScInfoPropWidget::selectedInstrument(QString selection)
{
    std::cout << "Instrument Selected: " << selection.toAscii().data() << std::endl;
}

//process Data Set
void ScInfoPropWidget::selectedDataSet(QString selection)
{
    std::cout << "DataSet Selected: " << selection.toAscii().data() << std::endl;

}

void ScInfoPropWidget::selectedInstrumentElement(QListWidgetItem * item)
{
    std::cout << "Looking at Item " << item->text().toAscii().data() << std::endl;

    if(item->isSelected())
    {
        std::cout << "TRUE" << std::endl;
    }
    else
    {
        std::cout << "FALSE" << std::endl;
    }

    //selection made

}

void ScInfoPropWidget::instrumentSelectionChanged()
{
    std::cout << "Instrument Selection Changed" << std::endl;

    this->selectedInstruments.clear();
    this->selectedInstruments = ui->Instruments->selectedItems();

    std::cout << "Selected Items: " << std::endl;

    QList<QListWidgetItem*>::iterator iter;

    for(iter = this->selectedInstruments.begin(); iter != this->selectedInstruments.end(); ++iter)
    {
         QString item = (*iter)->text();

         item = item.split("\t")[0];

         std::cout << "Item: " << item.toAscii().data() << std::endl;

         filterNetworkAccessModule manager;
         this->getDataSetOptions(manager, item);
         this->currentDataObjects.push_back(manager.getFinalOjects());
    }

    ui->DataSet->setEnabled(true);

}

void ScInfoPropWidget::dataSelectionChanged()
{
    std::cout << "Data Selections have Changed" << std::endl;

}

void ScInfoPropWidget::getDataOptions()
{


}


//get active DataSet
bool ScInfoPropWidget::getDataSetOptions(filterNetworkAccessModule &manager, QString dataset)
{

    std::cout << "Get Data Set Options" << std::endl;


    std::cout << QString(this->baseURL + this->dataViewSpacePhys + this->getDataSets
                         + "/" + dataset + "/" + this->getInventory).toAscii().data() << std::endl;


    manager.Get(this->baseURL + this->dataViewSpacePhys + this->getDataSets
                + "/" + dataset + "/" + this->getInventory,
                                   QString("Variables"),
                                   QString("VariableDescription"));

    if(manager.getNetworkAccessStatus() == 0)
        return true;
    else
        return false;



    return true;
}

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

        if(name != desc)
        {
            this->InstrumentList.insert(name, name + "\t" + desc);
        }
        else
        {
            this->InstrumentList.insert(name, desc);
        }

    }

    GroupList.sort();

    return true;
}
