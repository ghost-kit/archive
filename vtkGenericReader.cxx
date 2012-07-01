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

  //Configure sytem array interfaces
  this->MetaData = vtkTable::New();
  this->Data = vtkStructuredGrid::New();
  this->Points = vtkPoints::New();


}

//--
vtkGenericReader::~vtkGenericReader()
{
  this->PointDataArraySelection->Delete();
  this->CellDataArraySelection->Delete();
//  this->MetaData->Delete();
//  this->Data->Delete();
//  this->Points->Delete();
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
  //This doesn't really do anything right now...
  return 1;
}

//--
int vtkGenericReader::ProcessRequest(
    vtkInformation *reqInfo,
    vtkInformationVector **inInfo,
    vtkInformationVector *outInfo)
{
  return this->Superclass::ProcessRequest(reqInfo, inInfo, outInfo);
}

//--
int vtkGenericReader::RequestInformation(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{
  int ActivePort = outputVector->GetNumberOfInformationObjects()-1;
  std::cerr << "Active Port: " << ActivePort << std::endl;

  //get Data output port information
  if(ActivePort == 0)
    {
      this->DataOutInfo = outputVector->GetInformationObject(0);
      this->checkStatus(this->DataOutInfo, (char*)"Data Output Information");

      // Array names and extents
      if(this->CellDataArraySelection->GetNumberOfArrays() == 0 &&
         this->PointDataArraySelection->GetNumberOfArrays() == 0)
        {
          this->PopulateArrays();
          this->PopulateWholeExtents();
        }

      // Time Step Data
      if(this->NumberOfTimeSteps == 0)
        {
          this->PopulateTimeStepInfo();
        }

      //Set Whole Extents for data
      this->printWholeExtents();

      this->DataOutInfo->Set(
            vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
            this->WholeExtent,
            6);

    }
  //get MetaData output port information
  if(ActivePort == 1)
    {
      this->MetaDataOutInfo = outputVector->GetInformationObject(1);
      this->checkStatus(this->MetaDataOutInfo, (char*)"Meta Data Output Info");
    }
  else
    {
      std::cerr << "Only one port on this loop" << std::endl;
    }


  return 1;
}

//--
int vtkGenericReader::RequestData(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{
  int port = request->Get(vtkDemandDrivenPipeline::FROM_OUTPUT_PORT());
  int numberObjects = outputVector->GetNumberOfInformationObjects();

  if(numberObjects != 2)
    {
      outputVector->GetInformationObject(1);
      numberObjects = outputVector->GetNumberOfInformationObjects();
    }

  std::cerr << "Port: " << port << std::endl;
  std::cerr << "Objs: " << numberObjects << std::endl;

  this->MetaData = dynamic_cast<vtkTable*>
      (this->MetaDataOutInfo->Get(vtkDataObject::DATA_OBJECT()));

  outputVector->Print(std::cerr);
  request->Print(std::cerr);

  vtkStringArray *MetaString = vtkStringArray::New();
  MetaString->SetName("Meta Data");
  MetaString->SetNumberOfComponents(1);
  MetaString->InsertNextValue("This is a Test");
  cout << "Configured Table Column" << endl;
  this->MetaData->AddColumn(MetaString);
  cout << "Added column to Table" << endl;
  MetaString->Delete();




  return 1;
}
//=================== END CORE METHODS =======================


//------------------------------------------------------------
//    These are callback functions for ParaView relating to
//  Selections and Events.
//------------------------------------------------------------
void vtkGenericReader::SelectionCallback(
    vtkObject*,
    unsigned long vtkNotUsed(eventid),
    void* clientdata,
    void* vtkNotUsed(calldata))
{
  static_cast<vtkGenericReader*>(clientdata)->Modified();
}

//--
void vtkGenericReader::EventCallback(
    vtkObject* caller,
    unsigned long eid,
    void* clientdata, void* calldata)
{
  caller->Modified();
  std::cout << "Event Callback Activated" << std::endl;
}
//====================== END CALLBACKS =======================

//------------------------------------------------------------
//    These methods to load the requested variables.
//  These are provided so that we can abstract out the reading
//  of the data from the rest of the reader.
//
//  Override these methods for your reader
//------------------------------------------------------------


//-- Return 0 for failure, 1 for success --//
int vtkGenericReader::LoadVariableData(char* name)
{

  return 1;
}

//-- Return 0 for failure, 1 for success --//
/* You will want to over-ride this method to
 * Populate the system with your own arrays */
int vtkGenericReader::PopulateArrays()
{

  /* Add Test Arrays */
  this->CellDataArraySelection->AddArray("Test Array 1");
  this->PointDataArraySelection->AddArray("Test Array 2");

  return 1;
}

//-- Meta Data Population
int vtkGenericReader::PopulateMetaData(vtkInformationVector *outputVector)
{
  vtkInformation* info = outputVector->GetInformationObject(1);

  vtkTable* MetaData =
      dynamic_cast<vtkTable*>(info->Get(vtkDataObject::DATA_OBJECT()));

  vtkStringArray *MetaString = vtkStringArray::New();
  MetaString->SetName("Meta Data");
  MetaString->SetNumberOfComponents(1);
  MetaString->InsertNextValue("This is a Test");

  MetaData->AddColumn(MetaString);

  MetaString->Delete();

  return 1;
}

int vtkGenericReader::checkStatus(vtkObject *Object, char *name)
{
  if(Object == NULL)
    {
      std::cerr << "ERROR: " << name
                << " has failed to initialize"
                << std::endl;

      return 0;
    }
  else
    {
      std::cerr << "SUCCESS: " << name
                << " has successfully initialized"
                << std::endl;
    }

  return 1;
}

//-- Return 0 for failure, 1 for success --//
/* You will want to over-ride this method
 * to provide time-step information for your own
 * data */
int vtkGenericReader::PopulateTimeStepInfo()
{
  return 1;
}

//-- Return 0 for failure, 1 for success --//
/* Over-ride this method to provide the
 * extents of your data */
int vtkGenericReader::PopulateWholeExtents()
{

  this->WholeExtent[0] = 0;
  this->WholeExtent[1] = 2;
  this->WholeExtent[2] = 0;
  this->WholeExtent[3] = 2;
  this->WholeExtent[4] = 0;
  this->WholeExtent[5] = 2;

  return 1;
}

//-- print whole extents --//
void vtkGenericReader::printWholeExtents()
{
  std::cout << this->WholeExtent[0] << " " <<
               this->WholeExtent[1] << " " <<
               this->WholeExtent[2] << " " <<
               this->WholeExtent[3] << " " <<
               this->WholeExtent[4] << " " <<
               this->WholeExtent[5] << std::endl;
}

//-- Return 0 for failure, 1 for success --//
/* You will need to over-ride this method to provide
 * your own grid-information */
int vtkGenericReader::GenerateGrid()
{

  return 1;
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

//================= END PORT CONFIGURATION ===================

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
