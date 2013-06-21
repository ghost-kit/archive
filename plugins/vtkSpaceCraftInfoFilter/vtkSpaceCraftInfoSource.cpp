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

    this->infoHandler.setNumInputPorts(0);
    this->infoHandler.setNumOutputPorts(1);
}

//===============================================//
vtkSpaceCraftInfoSource::~vtkSpaceCraftInfoSource()
{
}

//===============================================//
int vtkSpaceCraftInfoSource::ProcessRequest(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
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

//===============================================//
int vtkSpaceCraftInfoSource::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{

    return 1;
}

//===============================================//
int vtkSpaceCraftInfoSource::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    return this->infoHandler.RequestDataSource(request, inputVector, outputVector);
}

//===============================================//
int vtkSpaceCraftInfoSource::FillOutputPortInformation(int port, vtkInformation *info)
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkMultiBlockDataSet");
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
    this->Superclass::PrintSelf(os, indent);
}

//===============================================//
double vtkSpaceCraftInfoSource::getStartTime()
{
    return -1;
}

//===============================================//
double vtkSpaceCraftInfoSource::getEndTime()
{
    return -1;
}

//===============================================//
void vtkSpaceCraftInfoSource::SetSCIData(const char *group, const char *observatory, const char *list)
{
    this->infoHandler.setProcessed(false);
    this->Modified();
    return this->infoHandler.SetSCIData(group, observatory, list);

}

//===============================================//
void vtkSpaceCraftInfoSource::SetTimeFitHandler(int handler)
{
    this->infoHandler.setProcessed(false);
    this->Modified();
    return this->infoHandler.SetTimeFitHandler(handler);

}

//===============================================//
void vtkSpaceCraftInfoSource::SetBadDataHandler(int handler)
{
    this->infoHandler.setProcessed(false);
    this->Modified();
    return this->infoHandler.SetBadDataHandler(handler);

}

//===============================================//
void vtkSpaceCraftInfoSource::setTimeRange(const double start, const double end)
{
    std::cout << "Start MJD: " << start << std::endl;
    std::cout << "End MJD:   " << end << std::endl;

    this->infoHandler.setStartTime(start);
    this->infoHandler.setEndTime(end);

    this->infoHandler.setProcessed( false);

    this->Modified();
}
