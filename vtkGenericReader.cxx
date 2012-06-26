#include "vtkGenericReader.h"

#include "vtkCallbackCommand.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCellType.h"
#include "vtkDataArraySelection.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "vtkStringArray.h"
#include "vtkFloatArray.h"
#include "vtkIntArray.h"
#include "vtkPoints.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtksys/SystemTools.hxx"

#include <string>
#include <sstream>
#include <iostream>

#include "vtkMultiProcessController.h"
#include "vtkToolkits.h"



vtkStandardNewMacro(vtkGenericReader)


//--
vtkGenericReader::vtkGenericReader()
{
    //set the number of output ports you will need
    this->SetNumberOfOutputPorts(2);

    //set the number of input ports (Default 0)
    this->SetNumberOfInputPorts(0);
}

//--
vtkGenericReader::~vtkGenericReader()
{

}

//--
void vtkGenericReader::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Filename: "
     << (this->Filename ? this->Filename : "(NULL)") << endl;

  os << indent << "WholeExent: {" << this->WholeExtent[0] << ", "
     << this->WholeExtent[1] << ", " << this->WholeExtent[2] << ", "
     << this->WholeExtent[3] << ", " << this->WholeExtent[4] << ", "
     << this->WholeExtent[5] << "}" << endl;
  os << indent << "SubExtent: {" << this->SubExtent[0] << ", "
     << this->SubExtent[1] << ", " << this->SubExtent[2] << ", "
     << this->SubExtent[3] << ", " << this->SubExtent[4] << ", "
     << this->SubExtent[5] << "}" << endl;
  os << indent << "VariableArraySelection:" << endl;

  this->PointDataArraySelection->PrintSelf(os, indent.GetNextIndent());
}


//--
vtkStructuredGrid* vtkGenericReader::GetFieldOutput()
{

  return NULL;
}

//--
vtkTable* vtkGenericReader::GetMetaDataOutput()
{

  return NULL;
}

//--
int vtkGenericReader::GetNumberOfPointArrays()
{

  return 1;
}

//--
int vtkGenericReader::GetNumberOfCellArrays()
{

  return 1;
}


//--
const char* vtkGenericReader::GetPointArrayName(int index)
{

  return NULL;
}

//--
const char* vtkGenericReader::GetCellArrayName(int index)
{

  return NULL;
}

//--
int vtkGenericReader::GetPointArrayStatus(const char *name)
{

  return 1;
}

//--
int vtkGenericReader::GetCellArrayStatus(const char *name)
{

  return 1;
}


//--
void vtkGenericReader::SetPointArrayStatus(const char *name, int status)
{


}

//--
void vtkGenericReader::SetCellArrayStatus(const char *name, int status)
{


}

//--
void vtkGenericReader::DisableAllPointArrays()
{


}

//--
void vtkGenericReader::DisableAllCellArrays()
{


}

//--
void vtkGenericReader::EnableAllPointArrays()
{


}

//--
void vtkGenericReader::EnableAllCellArrays()
{


}

//--
void vtkGenericReader::LoadVariableData(int var)
{


}

//-- The MEAT of the Reader -- Required Methods
int vtkGenericReader::ProcessRequest(vtkInformation *request,
                           vtkInformationVector **inInfo,
                           vtkInformationVector *outInfo)
{

  return 1;
}


//--
int vtkGenericReader::RequestData(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{

  return 1;
}

//--
int vtkGenericReader::RequestInformation(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{

  return 1;
}

//--
void vtkGenericReader::SelectionCallback(
    vtkObject *caller,
    unsigned long eid,
    void *clientdata,
    void *calldata)
{


}

//--
void vtkGenericReader::EventCallback(
    vtkObject* caller,
    unsigned long eid,
    void* clientdata, void* calldata)
{


}

//--
int vtkGenericReader::FillOutputPortInformation(int port, vtkInformation* info)
{
    switch(port)
      {
      case 0:
      return this->Superclass::FillOutputPortInformation(port, info);
        break;

      case 1:
        info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTable");
        break;
      }

    return 1;
}
