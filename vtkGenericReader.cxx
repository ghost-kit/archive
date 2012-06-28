#include "vtkGenericReader.h"

#include "vtkCallbackCommand.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCellType.h"
#include "vtkDataArraySelection.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkTable.h"
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

    //configure array status selectors
    this->PointDataArraySelection = vtkDataArraySelection::New();
    this->CellDataArraySelection  = vtkDataArraySelection::New();

    //Configure metadata arrays
  this->MetaData = vtkTable::New();


    //Add test data array
    this->CellDataArraySelection->AddArray("Test 1");
    this->PointDataArraySelection->AddArray("Test 2");
    this->PointDataArraySelection->AddArray("Test 3");


}

//--
vtkGenericReader::~vtkGenericReader()
{
  this->PointDataArraySelection->Delete();
  this->CellDataArraySelection->Delete();
  this->MetaData->Delete();
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

  return this->PointDataArraySelection->GetNumberOfArrays();
}

//--
int vtkGenericReader::GetNumberOfCellArrays()
{

  return this->CellDataArraySelection->GetNumberOfArrays();
}


//--
const char* vtkGenericReader::GetPointArrayName(int index)
{

  return this->PointDataArraySelection->GetArrayName(index);
}

//--
const char* vtkGenericReader::GetCellArrayName(int index)
{

  return this->CellDataArraySelection->GetArrayName(index);
}

//--
int vtkGenericReader::GetPointArrayStatus(const char *name)
{

  return this->PointDataArraySelection->GetArraySetting(name);
}

//--
int vtkGenericReader::GetCellArrayStatus(const char *name)
{

  return this->CellDataArraySelection->GetArraySetting(name);
}


//--
void vtkGenericReader::SetPointArrayStatus(const char *name, int status)
{
  if(status == 1)
    this->PointDataArraySelection->EnableArray(name);
  else
    this->PointDataArraySelection->DisableArray(name);

}

//--
void vtkGenericReader::SetCellArrayStatus(const char *name, int status)
{
  if(status == 1)
    this->CellDataArraySelection->EnableArray(name);
  else
    this->CellDataArraySelection->DisableArray(name);

}

//--
void vtkGenericReader::DisableAllPointArrays()
{
  this->PointDataArraySelection->DisableAllArrays();

}

//--
void vtkGenericReader::DisableAllCellArrays()
{
  this->CellDataArraySelection->DisableAllArrays();

}

//--
void vtkGenericReader::EnableAllPointArrays()
{
  this->PointDataArraySelection->EnableAllArrays();

}

//--
void vtkGenericReader::EnableAllCellArrays()
{
  this->CellDataArraySelection->EnableAllArrays();

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
        info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkStructuredGrid");
        break;

      case 1:
        info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTable");
        break;
      }

    return 1;
}
