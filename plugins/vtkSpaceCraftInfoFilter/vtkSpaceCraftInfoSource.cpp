#include "vtkSpaceCraftInfoSource.h"


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
vtkStandardNewMacro(vtkSpaceCraftInfoSource)

//===============================================//
vtkSpaceCraftInfoSource::vtkSpaceCraftInfoSource()
    : Superclass()
{
    this->SetNumberOfInputPorts(0);
    this->SetNumberOfOutputPorts(1);
}

//===============================================//
vtkSpaceCraftInfoSource::~vtkSpaceCraftInfoSource()
{
}

//===============================================//
int vtkSpaceCraftInfoSource::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{

    this->inInfo = NULL;
    this->outInfo = NULL;
    return 1;
}

//===============================================//
int vtkSpaceCraftInfoSource::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
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
int vtkSpaceCraftInfoSource::FillOutputPortInformation(int port, vtkInformation *info)
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTable");
    return 1;
}


//===============================================//
int vtkSpaceCraftInfoSource::FillInputPortInformation(int port, vtkInformation *info)
{
    return Superclass::FillInputPortInformation(port,info);
}

//===============================================//
void vtkSpaceCraftInfoSource::PrintSelf(ostream &os, vtkIndent indent)
{
}


