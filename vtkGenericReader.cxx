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


//---------------------------------------------------------------
//    Constructors and Destructors
//---------------------------------------------------------------
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

    /* Add Test Arrays */
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

//-------------------------------------------------------------
//    The following Methods provide basic functionality for
//  Selective read arrays.  These methods provide the list of
//  arrays in Paraview.  You MUST populate the arrays in you
//  RequestInformation routine.  This can be done with the
//  *****DataArraySlection->AddArray(char* name) routines
//  where ***** represents the type of arrays you are
//  populating. (i.e. Point or Cell)
//-------------------------------------------------------------

/*
 * The Number of Point Arrays in current selection
 *  This is an internal function
 */
int vtkGenericReader::GetNumberOfPointArrays()
{
  return this->PointDataArraySelection->GetNumberOfArrays();
}

/*
 * The Number of Cell Arrays in current selection
 *  This is an internal function
 */
int vtkGenericReader::GetNumberOfCellArrays()
{
  return this->CellDataArraySelection->GetNumberOfArrays();
}

/*
 *Return the NAME (characters) of the Point array at index
 *   This is an internal function
 */
const char* vtkGenericReader::GetPointArrayName(int index)
{
  return this->PointDataArraySelection->GetArrayName(index);
}

/*
 *Return the NAME (characters) of the Cell array at index
 *   This is an internal function
 */
const char* vtkGenericReader::GetCellArrayName(int index)
{
  return this->CellDataArraySelection->GetArrayName(index);
}

/*
 *Get the status of the Point Array of "name"
 *   This is an internal function
 */
int vtkGenericReader::GetPointArrayStatus(const char *name)
{
  return this->PointDataArraySelection->GetArraySetting(name);
}

/*
 *Get the status of the Cell Array of "name"
 *   This is an internal function
 */
int vtkGenericReader::GetCellArrayStatus(const char *name)
{
  return this->CellDataArraySelection->GetArraySetting(name);
}

/*
 *Set the status of the Point Array of "name"
 *   This is an internal function
 */
void vtkGenericReader::SetPointArrayStatus(const char *name, int status)
{
  if(status == 1)
    this->PointDataArraySelection->EnableArray(name);
  else
    this->PointDataArraySelection->DisableArray(name);
}

/*
 *Set the status of the Cell Array of "name"
 *   This is an internal function
 */
void vtkGenericReader::SetCellArrayStatus(const char *name, int status)
{
  if(status == 1)
    this->CellDataArraySelection->EnableArray(name);
  else
    this->CellDataArraySelection->DisableArray(name);
}

/*
 *Disables ALL Point arrays registered in system
 *   This is an internal function
 */
void vtkGenericReader::DisableAllPointArrays()
{
  this->PointDataArraySelection->DisableAllArrays();
}

/*
 *Disables ALL Cell arrays registered in system
 *   This is an internal function
 */
void vtkGenericReader::DisableAllCellArrays()
{
  this->CellDataArraySelection->DisableAllArrays();
}

/*
 *Enables ALL Point arrays registered in system
 *   This is an internal function
 */
void vtkGenericReader::EnableAllPointArrays()
{
  this->PointDataArraySelection->EnableAllArrays();
}

/*
 *Enables ALL Cell arrays registered in system
 *   This is an internal function
 */
void vtkGenericReader::EnableAllCellArrays()
{
  this->CellDataArraySelection->EnableAllArrays();
}
//=============== END SELECTIVE READER METHODS================

//------------------------------------------------------------
//    These functions are the meat of the readers... i.e. they
//  are the calls that ParaView uses to get information from
//  your data source.   This is where the logic of the reader
//  is implemented.
//------------------------------------------------------------

int vtkGenericReader::CanReadFile(const char *filename)
{

  return 0;
}

//--
int vtkGenericReader::ProcessRequest(
    vtkInformation *request,
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
//=================== END CORE METHODS =======================


//------------------------------------------------------------
//    These are callback functions for ParaView relating to
//  Selections and Events.
//------------------------------------------------------------
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
//====================== END CALLBACKS =======================

//------------------------------------------------------------
//    These methods to load the requested variables.
//  These are provided so that we can abstract out the reading
//  of the data from the rest of the reader.
//
//  Override these methods for your reader
//------------------------------------------------------------
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
void vtkGenericReader::LoadVariableData(char* name)
{


}

//--
void vtkGenericReader::LoadVariableData(int index)
{


}
//=================== END USER METHODS =========================


//--------------------------------------------------------------
//    Output Port Configuration
//--------------------------------------------------------------
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

//------------------------------------------------------------
//    Internal functions -- required for system to work
//------------------------------------------------------------
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
