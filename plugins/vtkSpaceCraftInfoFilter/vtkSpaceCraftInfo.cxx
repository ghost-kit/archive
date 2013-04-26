//==============================================================================//
// vtkSpaceCraftInfo filter.  This filter requests data from CDA-Web and        //
//  processes it for use with ParaView time-dependent data sets.  The filter    //
//  will also interpolate the requested information over time to get the proper //
//  time positions.                                                             //
//                                                                              //
//  Author: Joshua Murphy                                                       //
//  Date:   01 Apr 2013                                                         //
//==============================================================================//

#include "vtkSpaceCraftInfo.h"

#include "vtkCommand.h"
#include "vtkAbstractArray.h"
#include "vtkCallbackCommand.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDataSetAttributes.h"
#include "vtkDataArraySelection.h"
#include "vtkCompositeDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkDataArray.h"
#include "vtkDataObject.h"
#include "vtkFieldData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGridAlgorithm.h"
#include "vtkStructuredGrid.h"
#include "vtkStructuredData.h"
#include "vtkTable.h"
#include "vtksys/SystemTools.hxx"


#include "QObject"
#include "QtXml"
#include "QNetworkAccessManager"
#include "QNetworkReply"
#include "QURL"
#include "QStringList"

#include <vector>
#include <vtkSmartPointer.h>
#include <vtksys/ios/sstream>

vtkStandardNewMacro(vtkSpaceCraftInfo)

vtkSpaceCraftInfo::vtkSpaceCraftInfo()
{
    this->NumberOfTimeSteps = 0;
    this->NumberOfSCInfoArrays = 0;
    this->SpaceCraftArraySelections = vtkDataArraySelection::New();
    this->SpaceCraftSubArraySelections = vtkDataArraySelection::New();
    this->currentGroupObjects = NULL;

    //URLs for CDAWeb
    this->baseURL = QString("http://cdaweb.gsfc.nasa.gov/WS/cdasr/1");
    this->dataViewSpacePhys = QString("/dataviews/sp_phys/");
    this->getObservatorys = QString("observatories");
    this->getObservatoryGroups = QString("observatoryGroups");
    this->getInstrumentTypes = QString("instrumentTypes");

    //    http://cdaweb.gsfc.nasa.gov/WS/cdasr/1/dataviews/sp_phys/observatoryGroups

    //configure the network manager
    this->SCListManager = new filterNetworkAccessModule();
    this->networkAccessStatus = -1;
}

vtkSpaceCraftInfo::~vtkSpaceCraftInfo()
{
    this->SpaceCraftArraySelections->Delete();

    //TODO: do i need to free the network access manager?
}

//----- required overides -----//
//=========================================================================================//
//Process Request
int vtkSpaceCraftInfo::ProcessRequest(vtkInformation *request, vtkInformationVector ** inputVector, vtkInformationVector *outputVector)
{
    if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
    {
        return this->RequestInformation(request, inputVector, outputVector);
    }
    else if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_DATA()))
    {
        return this->RequestData(request, inputVector, outputVector);
    }

    return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//=========================================================================================//
//Request Inoformation
int vtkSpaceCraftInfo::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    if(inInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS()))
    {
        std::cout << "Getting Number of Time steps" << std::flush << std::endl;
        this->NumberOfTimeSteps = inInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
        double *timeValues = inInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());

        //push time steps into list
        for (int y = 0; y < this->NumberOfTimeSteps; y++)
        {
            this->timeSteps.push_back(timeValues[y]);
        }
    }
    else
    {
        this->NumberOfTimeSteps = 0;
    }

    //Provide information to PV on how many time steps for which we will be providing information
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
                 this->getTimeSteps(),
                 this->NumberOfTimeSteps);

    double timeRange[2] = {this->timeSteps.first(), this->timeSteps.last()};

    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
                 timeRange,
                 2);


    if(this->GetNumberOfSCinfoArrays() == 0)
    {
        //get the space craft list
        this->getSCList();

        this->currentGroupObjects = this->SCListManager->getFinalOjects();
        QStringList SortedList;

        std::cout << "Size of List: " << this->currentGroupObjects->size() << std::endl;

        for(int x = 0; x < this->currentGroupObjects->size(); x++)
        {
            QMap<QString,QString> *currentMap = this->currentGroupObjects->operator [](x);

            QList<QString> keys = currentMap->keys();

            QString name = currentMap->operator []("Name");

            std::cout << "Name: " << name.toAscii().data() << std::endl;
            std::cout << "Number of Keys available: " << keys.size() << std::endl;
            std::cout << "==============================" << std::endl;

            SortedList.push_back(name);

        }

        SortedList.sort();

        for(int x = 0; x < SortedList.size(); x++)
        {
            this->SpaceCraftArraySelections->AddArray(SortedList[x].toAscii().data());
        }

        this->SetNumberOfSCinfoArrays(SortedList.size());

    }

    this->DisableAllSCArrays();
    return 1;
}


//=========================================================================================//
//Request Data
int vtkSpaceCraftInfo::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    //Get the output Data object
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkTable *output = vtkTable::GetData(outInfo);

    //get time request data
    if(outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
    {
        this->requestedTimeValue = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
    }

    //call to get info from CDAWeb
    bool result = this->processCDAWeb(output);

    return 1;
}


//=========================================================================================//
//------ Port Information ------//
int vtkSpaceCraftInfo::FillInputPortInformation(int port, vtkInformation *info)
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    return 1;
}

int vtkSpaceCraftInfo::FillOutputPortInformation(int port, vtkInformation *info)
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTable");
    return 1;
}


//=========================================================================================//
//----- helper function -----//
//getTimeSteps() returns a NEW double array that *must*
//  be managed by the caller. TODO: Fix this to manage memory slef.
double *vtkSpaceCraftInfo::getTimeSteps()
{
    double *ret = new double[this->NumberOfTimeSteps];

    for(int i = 0; i < this->NumberOfTimeSteps; i++)
    {
        ret[i] = this->timeSteps[i];
    }

    return ret;
}


//=========================================================================================//
//------ get list of space craft -----//
bool vtkSpaceCraftInfo::getSCList()
{
    //get data from network
    this->SCListManager->Get(this->baseURL+dataViewSpacePhys+getObservatoryGroups, QString("ObservatoryGroups"), QString("ObservatoryGroupDescription"));

    if(this->SCListManager->getNetworkAccessStatus() == 0)
        return true;
    else
        return false;
}


//=========================================================================================//
//------ get actuall space craft data -----//
bool vtkSpaceCraftInfo::getSCData()
{

    return true;
}




//=========================================================================================//
bool vtkSpaceCraftInfo::processCDAWeb(vtkTable *output)
{
    //get the data from CDAWeb, if it hasn't already been gotten.
    //  I will need to think about how to handle the retrieval
    //  WE need a list of avaiilable space craft, and then a way to get
    //  only the needed information.


    vtkDoubleArray *timeArray = vtkDoubleArray::New();
    timeArray->SetNumberOfComponents(1);
    timeArray->SetName("Time");

    timeArray->InsertNextValue(this->requestedTimeValue);
    output->GetRowData()->AddArray(timeArray);

    timeArray->Delete();

    return true;
}

//----- GUI SC info array manipulators -----//

//=========================================================================================//
//set individual arrays
void vtkSpaceCraftInfo::SetSCArrayStatus(const char *name, int status)
{
    if(status)
    {
        this->SpaceCraftArraySelections->EnableArray(name);
        QMultiMap<QString, QString> *currentMap = NULL;

        //find the correct cache object
        for(int x = 0; x < this->currentGroupObjects->size(); x++)
        {
            currentMap = this->currentGroupObjects->operator [](x);
            if(currentMap->value("Name") == QString(name))
            {
                std::cout << "Found Object" << std::endl;
                break;
            }
        }

        QList<QString> values = currentMap->values();
        for(int c = 0; c < values.size(); c++)
        {
            std::cout << "Adding Objects: " << values[c].toAscii().data() << std::endl;
            this->SpaceCraftSubArraySelections->AddArray(values[c].toAscii().data());
        }
    }
    else
    {
        this->SpaceCraftArraySelections->DisableArray(name);

        //TODO: Clear sub-array (for this item)
    }
    SpaceCraftSubArraySelections->Modified();
    this->Modified();
}


//=========================================================================================//
//get individual arrays status
int vtkSpaceCraftInfo::GetSCinfoArrayStatus(const char *name)
{
    return this->SpaceCraftArraySelections->GetArraySetting(name);
}


//=========================================================================================//
//disable all arrays
void vtkSpaceCraftInfo::DisableAllSCArrays()
{
    this->SpaceCraftArraySelections->DisableAllArrays();

    //TODO: clear sub-menus

    this->Modified();
}


//=========================================================================================//
//enable all arrays
void vtkSpaceCraftInfo::EnableAllSCArrays()
{
    this->SpaceCraftArraySelections->EnableAllArrays();

    //TODO: LOAD all SubMenus

    this->Modified();
}


//=========================================================================================//
//----- other stuff needed ------//
void vtkSpaceCraftInfo::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
    os << indent << "NumberOfTimeSteps: " << this->NumberOfTimeSteps << std::endl;
}

int vtkSpaceCraftInfo::GetNumberOfSCinfoArrays()
{
    return this->SpaceCraftArraySelections->GetNumberOfArrays();
}

const char *vtkSpaceCraftInfo::GetSCinfoArrayName(int index)
{
    return this->SpaceCraftArraySelections->GetArrayName(index);
}


void vtkSpaceCraftInfo::SetSCSubArrayStatus(const char *name, int status)
{
    if(status)
    {
        this->SpaceCraftSubArraySelections->EnableArray(name);

        //TODO: setup sub-array (for this item)

    }
    else
    {
        this->SpaceCraftSubArraySelections->DisableArray(name);

        //TODO: Clear sub-array (for this item)
    }

    this->Modified();
}

int vtkSpaceCraftInfo::GetSCsubinfoArrayStatus(const char *name)
{
    return this->SpaceCraftSubArraySelections->GetArraySetting(name);
}

int vtkSpaceCraftInfo::GetNumberOfSCsubinfoArrays()
{
    return this->SpaceCraftSubArraySelections->GetNumberOfArrays();
}

const char *vtkSpaceCraftInfo::GetSCsubinfoArrayName(int index)
{
    return this->SpaceCraftSubArraySelections->GetArrayName(index);
}




