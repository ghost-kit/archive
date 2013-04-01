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

#include "vtkAbstractArray.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
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

#include "QtXml"
#include "QHttp"
#include "Q3Url"

#include <vector>
#include <vtkSmartPointer.h>
#include <vtksys/ios/sstream>

vtkStandardNewMacro(vtkSpaceCraftInfo)

vtkSpaceCraftInfo::vtkSpaceCraftInfo()
{
    this->NumberOfTimeSteps = 0;
}

vtkSpaceCraftInfo::~vtkSpaceCraftInfo()
{
}

//----- required overides -----//

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

//Request Inoformation
int vtkSpaceCraftInfo::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    if(inInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS()))
    {
        std::cout << "Getting Number of Time steps" << std::flush << std::endl;
        this->NumberOfTimeSteps = inInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
        double *test = inInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());

        //push time steps into list
        for (int y = 0; y < this->NumberOfTimeSteps; y++)  this->timeSteps.push_back(test[y]);

        //debug
//        for (int y = 0; y < this->NumberOfTimeSteps; y++) std::cout << this->timeSteps[y] << std::endl;
        //end debug
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

//    //provide extents
//    int wholeExtent[6] = {0,0,0,0,0,0};
//    wholeExtent[1] = this->NumberOfTimeSteps-1;

//    //provide extents to PV
//    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wholeExtent, 6);

  return 1;
}

//Request Data
int vtkSpaceCraftInfo::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    //Get the output Data object
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkTable *output = vtkTable::GetData(outInfo);

    //Get the Input Data Object
    vtkInformation* inInfo  = inputVector[0]->GetInformationObject(0);
    vtkDataSet* input = vtkDataSet::GetData(inInfo);

    //call to get info from CDAWeb
    bool result = this->processCDAWeb(inInfo, input, output);

  return 1;
}


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


//----- support copying of data arrays over -----//
void vtkSpaceCraftInfo::CopyDataToOutput(vtkInformation *inInfo, vtkDataSet *input, vtkTable *output)
{
}


//----- helper function -----//
double *vtkSpaceCraftInfo::getTimeSteps()
{
    double *ret = new double[this->NumberOfTimeSteps];

    for(int i = 0; i < this->NumberOfTimeSteps; i++)
    {
        ret[i] = this->timeSteps[i];
    }

    return ret;
}

bool vtkSpaceCraftInfo::getSCList()
{

    return true;
}

bool vtkSpaceCraftInfo::processCDAWeb(vtkInformation *inInfo, vtkDataSet *input, vtkTable *output)
{
    //get the data from CDAWeb, if it hasn't already been gotten.
    //  I will need to think about how to handle the retrieval
    //  WE need a list of avaiilable space craft, and then a way to get
    //  only the needed information.

    return true;
}


//----- other stuff needed ------//
void vtkSpaceCraftInfo::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
    os << indent << "NumberOfTimeSteps: " << this->NumberOfTimeSteps << std::endl;
}


