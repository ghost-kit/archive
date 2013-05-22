#include "ScInfoPropWidget.h"
#include "ui_ScInfoPropWidget.h"

#include "pqPropertiesPanel.h"


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

    //configure the network manager
    this->SCListManager = new filterNetworkAccessModule();
    this->SCInstrumentManager = new filterNetworkAccessModule();
    this->SCDataManager = new filterNetworkAccessModule();


    //Setup the UI
    ui->setupUi(this);
    ui->gridLayout->setMargin(pqPropertiesPanel::suggestedMargin());
    ui->gridLayout->setHorizontalSpacing(pqPropertiesPanel::suggestedHorizontalSpacing());
    ui->gridLayout->setVerticalSpacing(pqPropertiesPanel::suggestedVerticalSpacing());

    //Load first set of Values
    this->getSCList();
    this->currentGroupObjects = this->SCListManager->getFinalOjects();

    //get the observatory group list
    this->getGroupsList();

    //Load the group list
    this->loadGroupListToGUI();

    //connect signals to slots
    connect(ui->Group, SIGNAL(activated(QString)), this, SLOT(selectedGroup(QString)));
    connect(ui->Observatory, SIGNAL(activated(QString)), this, SLOT(selectedObservatory(QString)));
    connect(ui->Instruments, SIGNAL(clicked(QModelIndex)), this, SLOT(selectedInstrumentElement(QModelIndex)));
    connect(ui->DataSet, SIGNAL(activated(QString)), this, SLOT(selectedDataSet(QString)));

    //mark changes
    this->setChangeAvailableAsChangeFinished(true);

}

ScInfoPropWidget::~ScInfoPropWidget()
{

    delete ui;
}

bool ScInfoPropWidget::getSCList()
{

    //get data from network
    this->SCListManager->Get(this->baseURL + this->dataViewSpacePhys + this->getObservatoryGroups, QString("ObservatoryGroups"), QString("ObservatoryGroupDescription"));

    if(this->SCListManager->getNetworkAccessStatus() == 0)
        return true;
    else
        return false;
}

bool ScInfoPropWidget::getSCInstrument()
{

    std::cout << QString(this->baseURL + this->dataViewSpacePhys + this->getInstrumentTypes
                 + "?observatory=" + this->currentObservatory).toAscii().data() << std::endl;

    this->SCInstrumentManager->Get(this->baseURL + this->dataViewSpacePhys + this->getInstruments
                                   + "?observatory=" + this->currentObservatory,
                                   QString("Instruments"),
                                   QString("InstrumentDescription"));

    if(this->SCInstrumentManager->getNetworkAccessStatus() == 0)
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

    this->getObservatoryList(selection);
    ui->Observatory->clear();
    ui->Observatory->addItem("--- Select Observatory ---");
    ui->Observatory->addItems(this->SubGroupList);
    ui->Observatory->setEnabled(true);

    //clear downstream elements
    this->currentInstrument = "";
    ui->Instruments->setDisabled(true);

    this->currentDataSet = "";
    ui->DataSet->setDisabled(true);





}

//process Observatories
void ScInfoPropWidget::selectedObservatory(QString selection)
{
    std::cout << "Observatory Selected: " << selection.toAscii().data() << std::endl;

    this->currentObservatory = selection;

    //get data for next item
    this->getSCInstrument();
    this->currentInstrumentObjects = this->SCInstrumentManager->getFinalOjects();

    //get the required list
    this->getInstrumentList();

    ui->Instruments->clear();
    ui->Instruments->addItems(this->InstrumentDescList);
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

void ScInfoPropWidget::selectedInstrumentElement(QModelIndex index)
{

    //selection made

}


//get active DataSet
bool ScInfoPropWidget::getDataSet()
{

    return true;
}

//get instrument list for current selections
bool ScInfoPropWidget::getInstrumentList()
{

    std::cout << "Getting the Data List" << std::endl;
    this->InstrumentDescList.clear();
    this->InstrumentNameList.clear();

    for(int x = 0; x < this->currentInstrumentObjects->size(); x++)
    {
        filterNetworkObject *currentMap = this->currentInstrumentObjects->operator [](x);

        QString name = currentMap->operator []("Name");
        QString desc = currentMap->operator []("ShortDescription");

        std::cout << "Name: " << name.toAscii().data() << "\tDescription: " << desc.toAscii().data() << std::endl;

        this->InstrumentNameList.push_back(name);
        this->InstrumentDescList.push_back(desc);
    }

    GroupList.sort();

    return true;
}
