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

#include <vector>
#include <vtkSmartPointer.h>
#include <vtksys/ios/sstream>

vtkStandardNewMacro(vtkSpaceCraftInfo)

vtkSpaceCraftInfo::vtkSpaceCraftInfo()
{
    this->NumberOfTimeSteps = 0;
    this->NumberOfSCInfoArrays = 0;
    this->SpaceCraftArraySelections = vtkDataArraySelection::New();

    //URLs for CDAWeb
    this->baseURL = QString("http://cdaweb.gsfc.nasa.gov/WS/cdasr/1");
    this->getObservatoryURLext = QString("/dataviews/sp_phys/observatories");

    //configure the network manager
    this->netManager = new filterNetworkAccessModule();
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


    //get the space craft list
    if(this->getSCList())
    {
        //process the space craft list
        std::cout << "processing Space Craft information" << std::endl;
    }



    std::cout << "adding test arrays to array list" << std::endl;
    //temporary activation of arrays
    if(this->GetNumberOfSCinfoArrays() == 0)
    {
        this->SpaceCraftArraySelections->AddArray("Stereo A");
        this->SpaceCraftArraySelections->AddArray("Stereo B");
        this->SpaceCraftArraySelections->AddArray("Earth");
        this->SpaceCraftArraySelections->AddArray("Mercury");
        this->SpaceCraftArraySelections->AddArray("Mars");
        this->SpaceCraftArraySelections->AddArray("Wind");
        this->SpaceCraftArraySelections->AddArray("ACE");

        this->SetNumberOfSCinfoArrays(7);
    }


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
    this->netManager->Get(this->baseURL+getObservatoryURLext);


    return true;
}


//=========================================================================================//
//------ get actuall space craft data -----//
bool vtkSpaceCraftInfo::getSCData()
{

    return true;
}

//=====================================//
void vtkSpaceCraftInfo::networkReply()
{
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
    }
    else
    {
        this->SpaceCraftArraySelections->DisableArray(name);
    }

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
    this->Modified();
}


//=========================================================================================//
//enable all arrays
void vtkSpaceCraftInfo::EnableAllSCArrays()
{
    this->SpaceCraftArraySelections->EnableAllArrays();
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




