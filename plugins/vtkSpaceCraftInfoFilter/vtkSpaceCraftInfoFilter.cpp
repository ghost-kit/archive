#include "vtkSpaceCraftInfoFilter.h"

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

//===============================================//
vtkStandardNewMacro(vtkSpaceCraftInfoFilter)

//===============================================//
vtkSpaceCraftInfoFilter::vtkSpaceCraftInfoFilter()
    : Superclass()
{
}

//===============================================//
vtkSpaceCraftInfoFilter::~vtkSpaceCraftInfoFilter()
{
}

//===============================================//
int vtkSpaceCraftInfoFilter::RequestInformation(vtkInformation * request, vtkInformationVector ** inputVector, vtkInformationVector * outputVector)
{
        std::cout << __FUNCTION__ << " on line " << __LINE__ << std::endl;

        this->inInfo = inputVector[0]->GetInformationObject(0);
        if(inInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS()))
        {
            std::cout << "Getting Number of Time steps" << std::flush << std::endl;
            this->NumberOfTimeSteps = this->inInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
            double *timeValues = this->inInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());

            //push time steps into list
            this->timeSteps.clear();
            for (int y = 0; y < this->NumberOfTimeSteps; y++)
            {
                this->timeSteps.push_back(timeValues[y]);
            }
            this->TimeRange[0] = this->timeSteps.first();
            this->TimeRange[1] = this->timeSteps.last();
        }
        else
        {
            this->NumberOfTimeSteps = 0;
        }

        //Provide information to PV on how many time steps for which we will be providing information
        this->outInfo = outputVector->GetInformationObject(0);

        this->outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
                           this->getTimeSteps(),
                           this->NumberOfTimeSteps);

        double timeRange[2] = {this->timeSteps.first(), this->timeSteps.last()};

        this->outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
                           timeRange,
                           2);

        this->inInfo = NULL;
        this->outInfo = NULL;
        return 1;
}

//===============================================//
int vtkSpaceCraftInfoFilter::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
        //Get the output Data object
        this->outInfo = outputVector->GetInformationObject(0);
        this->output = vtkTable::GetData(outInfo);

        //get time request data
        if(this->outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
        {
            this->requestedTimeValue = this->outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
        }

        //if the data still needs to be loaded, load it...
        if(!this->processed)
        {
            this->DataCache.clear();
            this->LoadCDFData();
        }

        //process to return the needed information
        this->processCDAWeb(this->output);
        return 1;
}

//===============================================//
int vtkSpaceCraftInfoFilter::FillInputPortInformation(int port, vtkInformation *info)
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    return 1;
}

//===============================================//
int vtkSpaceCraftInfoFilter::FillOutputPortInformation(int port, vtkInformation *info)
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTable");
    return 1;
}

//===============================================//
void vtkSpaceCraftInfoFilter::PrintSelf(ostream &os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
    os << indent << "NumberOfTimeSteps: " << this->NumberOfTimeSteps << std::endl;
}
