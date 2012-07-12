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
#include "vtk_netcdf.h"
#include "vtk_netcdfcpp.h"

vtkStandardNewMacro(vtkGenericReader)


//---------------------------------------------------------------
//    Constructors and Destructors
//---------------------------------------------------------------
vtkGenericReader::vtkGenericReader()
{

  this->FileName = NULL;

  //set the number of output ports you will need
  this->SetNumberOfOutputPorts(2);

  //set the number of input ports (Default 0)
  this->SetNumberOfInputPorts(0);

  //configure array status selectors
  this->PointDataArraySelection = vtkDataArraySelection::New();
  this->CellDataArraySelection  = vtkDataArraySelection::New();

  //Configure sytem array interfaces
  this->Points = vtkPoints::New();

  this->SelectionObserver = vtkCallbackCommand::New();
  this->SelectionObserver->SetCallback(&vtkGenericReader::SelectionCallback);
  this->SelectionObserver->SetClientData(this);
  this->PointDataArraySelection->AddObserver(vtkCommand::ModifiedEvent, this->SelectionObserver);
  this->CellDataArraySelection->AddObserver(vtkCommand::ModifiedEvent, this->SelectionObserver);
}

//--
vtkGenericReader::~vtkGenericReader()
{
  this->PointDataArraySelection->Delete();
  this->CellDataArraySelection->Delete();
  this->Points->Delete();

  this->SelectionObserver->Delete();
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

  //  this->Modified();
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

  //  this->Modified();
}

/*
 *Disables ALL Point arrays registered in system
 *   This is an internal function
 */
void vtkGenericReader::DisableAllPointArrays()
{
  this->PointDataArraySelection->DisableAllArrays();
  //  this->Modified();
}

/*
 *Disables ALL Cell arrays registered in system
 *   This is an internal function
 */
void vtkGenericReader::DisableAllCellArrays()
{
  this->CellDataArraySelection->DisableAllArrays();
  //  this->Modified();
}

/*
 *Enables ALL Point arrays registered in system
 *   This is an internal function
 */
void vtkGenericReader::EnableAllPointArrays()
{
  this->PointDataArraySelection->EnableAllArrays();
  //  this->Modified();
}

/*
 *Enables ALL Cell arrays registered in system
 *   This is an internal function
 */
void vtkGenericReader::EnableAllCellArrays()
{
  this->CellDataArraySelection->EnableAllArrays();
  //  this->Modified();
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


  std::cerr << "Number of Information Objects: "
            << outInfo->GetNumberOfInformationObjects()
            << std::endl;

  return this->Superclass::ProcessRequest(reqInfo, inInfo, outInfo);
}

//--
int vtkGenericReader::RequestInformation(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{

  std::cerr << getSerialNumber() << ": " << __FUNCTION__ << std::endl;


  int port = request->Get(vtkDemandDrivenPipeline::FROM_OUTPUT_PORT());
  int status = 0;

  std::cerr << "Object Count: " << outputVector->GetNumberOfInformationObjects()
            << std::endl;

  std::cout << "Ports: " << this->GetNumberOfOutputPorts() << std::endl;

  //get Data output port information
  vtkInformation* MetaDataOutInfo = outputVector->GetInformationObject(1);
  status = this->checkStatus(MetaDataOutInfo, (char*)"Meta Data Output Information");

  //If status has been verified, load MetaData Information
  if(status)
    {
      MetaDataOutInfo->Set(vtkTable::FIELD_ASSOCIATION(), vtkTable::FIELD_ASSOCIATION_ROWS);
      std::cerr << "Field Array Association: "
                << MetaDataOutInfo->Get(vtkTable::FIELD_ASSOCIATION())
                << std::endl;

      MetaDataOutInfo->Print(std::cerr);
    }

  // Array names and extents
  vtkInformation* DataOutputInfo = outputVector->GetInformationObject(0);
  status = this->checkStatus(
        DataOutputInfo,
        (char*)" Array Name: Data Info Output Information");

  if(status)
    {
      //If Information has not yet been loaded, load it.
      if(this->CellDataArraySelection->GetNumberOfArrays() == 0 &&
         this->PointDataArraySelection->GetNumberOfArrays() == 0)
        {
          //Set the Names of the Arrays
          this->PopulateArrays();

          //Set the Whole Extents and Time
          this->PopulateDataInformation();
        }

      //  Set Whole Extents for data
      this->printWholeExtents();

      /*Set Information*/
      //Set Extents
      DataOutputInfo->Set(
            vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
            this->WholeExtent,
            6);

      //Set Time
      DataOutputInfo->Set(
            vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
            &this->TimeSteps[0],
            1);

    }
  return 1;
}

//--
int vtkGenericReader::RequestData(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{
  std::cerr << getSerialNumber() << ": " << __FUNCTION__ << std::endl;

  int port = request->Get(vtkDemandDrivenPipeline::FROM_OUTPUT_PORT());

  //check number of Information Objects being offered
  int numberObjects = outputVector->GetNumberOfInformationObjects();
  std::cerr << "Objs: " << numberObjects << std::endl;

  std::cerr << "Request Data: Loading Ports:" << std::endl;


  //Import the MetaData - Port 1
  this->PopulateMetaData(outputVector);

  //Import the actual Data
  this->LoadVariableData(outputVector);
  this->CellDataArraySelection->Print(std::cerr);
  this->PointDataArraySelection->Print(std::cerr);

  return 1;

}

//=================== END CORE METHODS =======================

//-- Callback
void vtkGenericReader::SelectionCallback(
    vtkObject*,
    unsigned long vtkNotUsed(eventid),
    void* clientdata,
    void* vtkNotUsed(calldata))
{
  static_cast<vtkGenericReader*>(clientdata)->Modified();
}


//------------------------------------------------------------
//    These methods to load the requested variables.
//  These are provided so that we can abstract out the reading
//  of the data from the rest of the reader.
//
//  Override these methods for your reader
//------------------------------------------------------------


//-- Return 0 for failure, 1 for success --//
int vtkGenericReader::LoadVariableData(vtkInformationVector* outputVector)
{
  vtkStructuredGrid* Data = vtkStructuredGrid::GetData(outputVector, 0);
  int status = this->checkStatus(Data, (char*)"Data Array Structured Grid");

  if(status)
    {
      //Generate the Grid
      this->GenerateGrid();

      //Commit the grid
      Data->SetDimensions(100,
                          100,
                          100);

      Data->SetPoints(this->Points);

      //Load Variables
      int c = 0;

      //Load Cell Data
      for(c = 0; c < this->CellDataArraySelection->GetNumberOfArrays(); c++)
        {
          //Load the current Cell array
          this->LoadGridValues(this->CellDataArraySelection->GetArrayName(c));
        }

      //Load Point Data
      for(c=0; c < this->PointDataArraySelection->GetNumberOfArrays(); c++)
        {
          //Load the current Point array
          this->LoadGridValues(this->CellDataArraySelection->GetArrayName(c));
        }
    }

  return 1;
}

//-- Return 0 for Failure, 1 for Success --//
int vtkGenericReader::LoadGridValues(const char* array)
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
  vtkTable* MetaData = vtkTable::GetData(outputVector,1);
  int status = this->checkStatus(MetaData, (char*)"(PMD) Meta Data Table Object");

  if(status)
    {
      std::cerr << ":::Loading Meta Data:::" << std::endl;

      vtkStringArray *MetaString = vtkStringArray::New();
      MetaString->SetName("Meta Data");
      MetaString->SetNumberOfComponents(1);
      MetaString->InsertNextValue("This is a Test");
      MetaString->InsertNextValue("Test 2");
      MetaString->InsertNextValue("Test 3");

      MetaData->AddColumn(MetaString);

      vtkStringArray *MetaString2 = vtkStringArray::New();
      MetaString2->SetName("Other Data");
      MetaString2->SetNumberOfComponents(1);
      MetaString2->InsertNextValue("Test 2,1");
      MetaString2->InsertNextValue("Test 2,2");
      MetaString2->InsertNextValue("Test 2,3");

      MetaData->AddColumn(MetaString2);

      vtkDoubleArray* MetaString3 = vtkDoubleArray::New();
      MetaString3->SetName("Numeric Data (Double)");
      MetaString3->SetNumberOfComponents(1);
      MetaString3->InsertNextValue(1.234);
      MetaString3->InsertNextValue(2.3456);
      MetaString3->InsertNextValue(3.456);

      MetaData->AddColumn(MetaString3);

      MetaString->Delete();

    }

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
/* Over-ride this method to provide the
 * extents of your data */
int vtkGenericReader::PopulateDataInformation()
{

  //Populate Dimensions
  this->Dimension[0] = 100;
  this->Dimension[1] = 100;
  this->Dimension[2] = 100;

  //Populate Extents
  this->WholeExtent[0] = 0;
  this->WholeExtent[1] = (this->Dimension[0] - 1);
  this->WholeExtent[2] = 0;
  this->WholeExtent[3] = (this->Dimension[1] - 1);
  this->WholeExtent[4] = 0;
  this->WholeExtent[5] = (this->Dimension[2]-1);

  //Set Time step Information
  this->NumberOfTimeSteps = 1;
  this->TimeSteps = new double[this->NumberOfTimeSteps];
  this->TimeSteps[0] = 10.00;

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

  int i = 0;
  int j = 0;
  int k = 0;

  // Point grid data
  double xyz[3] = { 0, 0, 0 };

  for (k = 0; k < this->Dimension[2]; k++)
    {
      for (j = 0; j < this->Dimension[1]; j++)
        {
          for (i = 0; i < this->Dimension[0]; i++)
            {
              xyz[0] = i;
              xyz[1] = j;
              xyz[2] = k;

              //insert point information into the grid
              this->Points->InsertNextPoint(xyz);
            }
        }
    }

  // Close off the gap in the grid (make sphere continuous


  return 1;
}

//=================== END USER METHODS =========================


//--------------------------------------------------------------
//    Output Port Configuration
//--------------------------------------------------------------
int vtkGenericReader::FillOutputPortInformation(int port, vtkInformation* info)
{

  if (port==0)
    {
      info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkStructuredGrid");

      return this->Superclass::FillInputPortInformation(port, info);

    }
  else if (port==1)
    {
      info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTable");
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

}
