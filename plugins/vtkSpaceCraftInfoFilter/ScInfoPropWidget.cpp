#include "ScInfoPropWidget.h"
#include "ui_ScInfoPropWidget.h"

#include "pqPropertiesPanel.h"
#include "filterNetworkAccessModule.h"
#include <QListWidgetItem>
#include <vtkSMProxy.h>
#include <vtkSMProperty.h>
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
    this->getInventory = QString("variables");

    //Setup the UI
    ui->setupUi(this);
    ui->gridLayout->setMargin(pqPropertiesPanel::suggestedMargin());
    ui->gridLayout->setHorizontalSpacing(pqPropertiesPanel::suggestedHorizontalSpacing());
    ui->gridLayout->setVerticalSpacing(pqPropertiesPanel::suggestedVerticalSpacing());

    ui->Observatory->setDisabled(true);
    ui->Instruments->setDisabled(true);

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

    /** Property Links */
    this->addPropertyLink(ui->Instruments, smproxy->GetPropertyName(smproperty), SIGNAL(itemSelectionChanged()), this->svp);




}

ScInfoPropWidget::~ScInfoPropWidget()
{

    delete ui;
}

void ScInfoPropWidget::apply()
{

    //build a list of elements
    QString csvList;
    QStringList::Iterator iter;
    for(iter = this->DataSetList.begin(); iter != this->DataSetList.end(); ++iter)
    {
        QString item = (*iter);
        if(iter == this->DataSetList.begin())
        {
            csvList = item;
        }
        else
        {
            csvList.append(QString("," + item));
        }
    }

    this->svp->SetElement(0, this->currentGroup.toAscii().data());
    this->svp->SetElement(1, this->currentObservatory.toAscii().data());
    this->svp->SetElement(2, csvList.toAscii().data());

    //apply the upstream parameters
    Superclass::apply();

}

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

}

//Instrument Selection has Changed
void ScInfoPropWidget::instrumentSelectionChanged()
{
    std::cout << "Instrument Selection Changed" << std::endl;

    //if we have something to process, lets do it...
    this->selectedInstruments = ui->Instruments->selectedItems();

    //clear the old list
    this->DataSetList.clear();

    if(!selectedInstruments.isEmpty())
    {
        std::cout << "Selected Items: " << std::endl;

        QString csvList;

        //create a list of items
        QList<QListWidgetItem*>::iterator iter;
        for(iter = this->selectedInstruments.begin(); iter != this->selectedInstruments.end(); ++iter)
        {
            QString item = (*iter)->text();
            item = item.split("\t")[0];

            this->DataSetList.push_back(item);
            std::cout << "Item: " << item.toAscii().data() << std::endl;

        }

        std::cout << "Time: " <<  svp->GetMTime() << " Class Name: " << svp->GetClassName() << std::endl;

    }
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
            this->InstrumentList.insert(name, name);
        }
    }

    if(!this->currentInstrumentObjects->isEmpty())
    {
        GroupList.sort();
    }

    return true;
}
