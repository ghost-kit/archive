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
    this->SetNumberOfInputPorts(1);
    this->SetNumberOfOutputPorts(1);

}

//===============================================//
vtkSpaceCraftInfoFilter::~vtkSpaceCraftInfoFilter()
{
}

//===============================================//
int vtkSpaceCraftInfoFilter::RequestInformation(vtkInformation * request, vtkInformationVector ** inputVector, vtkInformationVector * outputVector)
{
    return this->infoHandler.RequestInfoFilter(request, inputVector, outputVector);
}

//===============================================//
int vtkSpaceCraftInfoFilter::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    return this->infoHandler.RequestData(request, inputVector, outputVector);
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
}


//===============================================//
double vtkSpaceCraftInfoFilter::getStartTime()
{
    return this->infoHandler.getStartTime();
}

//===============================================//
double vtkSpaceCraftInfoFilter::getEndTime()
{
    return this->infoHandler.getEndTime();
}

//===============================================//
void vtkSpaceCraftInfoFilter::SetSCIData(const char *group, const char *observatory, const char *list)
{
    return this->infoHandler.SetSCIData(group, observatory, list);
    this->infoHandler.setProcessed(false);
    this->Modified();

}

//===============================================//
void vtkSpaceCraftInfoFilter::SetTimeFitHandler(int handler)
{
    return this->infoHandler.SetTimeFitHandler(handler);
    this->infoHandler.setProcessed(false);
    this->Modified();

}

//===============================================//
void vtkSpaceCraftInfoFilter::SetBadDataHandler(int handler)
{
    return this->infoHandler.SetBadDataHandler(handler);
    this->infoHandler.setProcessed(false);
    this->Modified();

}
