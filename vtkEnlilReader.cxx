#include "vtkEnlilReader.h"

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

#include "vtk_netcdfcpp.h"


vtkStandardNewMacro(vtkEnlilReader)


//---------------------------------------------------------------
//    Constructors and Destructors
//---------------------------------------------------------------
vtkEnlilReader::vtkEnlilReader()
{
  int nulExtent[6] = {0,0,0,0,0,0};
  this->FileName = NULL;

  //set the number of output ports you will need
  this->SetNumberOfOutputPorts(2);

  //set the number of input ports (Default 0)
  this->SetNumberOfInputPorts(0);

  //configure array status selectors
  this->PointDataArraySelection = vtkDataArraySelection::New();
  this->CellDataArraySelection  = vtkDataArraySelection::New();

  //Configure sytem array interfaces
  this->Points = NULL;
  this->Radius = NULL;
  this->gridClean = false;
  this->infoClean = false;

  this->setExtents(this->WholeExtent, nulExtent);
  this->setExtents(this->SubExtent, nulExtent);

  this->SelectionObserver = vtkCallbackCommand::New();
  this->SelectionObserver->SetCallback(&vtkEnlilReader::SelectionCallback);
  this->SelectionObserver->SetClientData(this);
  this->PointDataArraySelection->AddObserver(vtkCommand::ModifiedEvent, this->SelectionObserver);
  this->CellDataArraySelection->AddObserver(vtkCommand::ModifiedEvent, this->SelectionObserver);
}

//--
vtkEnlilReader::~vtkEnlilReader()
{
  this->PointDataArraySelection->Delete();
  this->CellDataArraySelection->Delete();

  if(this->gridClean)
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
int vtkEnlilReader::GetNumberOfPointArrays()
{
  return this->PointDataArraySelection->GetNumberOfArrays();
}

/*
 * The Number of Cell Arrays in current selection
 *  This is an internal function
 */
int vtkEnlilReader::GetNumberOfCellArrays()
{
  return this->CellDataArraySelection->GetNumberOfArrays();
}

/*
 *Return the NAME (characters) of the Point array at index
 *   This is an internal function
 */
const char* vtkEnlilReader::GetPointArrayName(int index)
{
  return this->PointDataArraySelection->GetArrayName(index);
}

/*
 *Return the NAME (characters) of the Cell array at index
 *   This is an internal function
 */
const char* vtkEnlilReader::GetCellArrayName(int index)
{
  return this->CellDataArraySelection->GetArrayName(index);
}

/*
 *Get the status of the Point Array of "name"
 *   This is an internal function
 */
int vtkEnlilReader::GetPointArrayStatus(const char *name)
{
  return this->PointDataArraySelection->GetArraySetting(name);
}

/*
 *Get the status of the Cell Array of "name"
 *   This is an internal function
 */
int vtkEnlilReader::GetCellArrayStatus(const char *name)
{
  return this->CellDataArraySelection->GetArraySetting(name);
}

/*
 *Set the status of the Point Array of "name"
 *   This is an internal function
 */
void vtkEnlilReader::SetPointArrayStatus(const char *name, int status)
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
void vtkEnlilReader::SetCellArrayStatus(const char *name, int status)
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
void vtkEnlilReader::DisableAllPointArrays()
{
  this->PointDataArraySelection->DisableAllArrays();
}

/*
 *Disables ALL Cell arrays registered in system
 *   This is an internal function
 */
void vtkEnlilReader::DisableAllCellArrays()
{
  this->CellDataArraySelection->DisableAllArrays();
}

/*
 *Enables ALL Point arrays registered in system
 *   This is an internal function
 */
void vtkEnlilReader::EnableAllPointArrays()
{
  this->PointDataArraySelection->EnableAllArrays();
}

/*
 *Enables ALL Cell arrays registered in system
 *   This is an internal function
 */
void vtkEnlilReader::EnableAllCellArrays()
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

int vtkEnlilReader::CanReadFile(const char *filename)
{
  //This doesn't really do anything right now...
  return 1;
}

//--
int vtkEnlilReader::ProcessRequest(
    vtkInformation *reqInfo,
    vtkInformationVector **inInfo,
    vtkInformationVector *outInfo)
{
  return this->Superclass::ProcessRequest(reqInfo, inInfo, outInfo);
}

//--
int vtkEnlilReader::RequestInformation(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{
  int status = 0;

  //get Data output port information
  vtkInformation* MetaDataOutInfo = outputVector->GetInformationObject(1);
  status = this->checkStatus(MetaDataOutInfo, (char*)"Meta Data Output Information");

  //If status has been verified, load MetaData Information
  if(status)
    {
      MetaDataOutInfo->Set(vtkTable::FIELD_ASSOCIATION(), vtkTable::FIELD_ASSOCIATION_ROWS);

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
      this->printExtents(this->WholeExtent, (char*)"Whole Extent:");

      /*Set Information*/
      //Set Time
      DataOutputInfo->Set(
            vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
            &this->TimeSteps[0],
            1);

      //Set Extents
      DataOutputInfo->Set(
            vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
            this->WholeExtent,
            6);
    }
  return 1;
}

//--
int vtkEnlilReader::RequestData(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{
  //Import the MetaData - Port 1
  this->LoadMetaData(outputVector);

  //Import the actual Data - Port 0
  this->LoadVariableData(outputVector);

  return 1;

}

//=================== END CORE METHODS =======================

//-- Callback
void vtkEnlilReader::SelectionCallback(
    vtkObject*,
    unsigned long vtkNotUsed(eventid),
    void* clientdata,
    void* vtkNotUsed(calldata))
{
  static_cast<vtkEnlilReader*>(clientdata)->Modified();
}


//------------------------------------------------------------
//    These methods to load the requested variables.
//  These are provided so that we can abstract out the reading
//  of the data from the rest of the reader.
//
//  Override these methods for your reader
//------------------------------------------------------------


//-- Return 0 for failure, 1 for success --//
int vtkEnlilReader::LoadVariableData(vtkInformationVector* outputVector)
{
  int newExtent[6];

  vtkStructuredGrid* Data = vtkStructuredGrid::GetData(outputVector, 0);
  vtkInformation* fieldInfo = outputVector->GetInformationObject(0);

  int status = this->checkStatus(Data, (char*)"Data Array Structured Grid");

  if(status)
    {

      //print extents for debug purposes
      this->printExtents(this->WholeExtent, (char*)"DEBUG: Whole Extent: ");
      this->printExtents(this->SubExtent, (char*)"DEBUG: SUBEXTENTS: ");

      //get new extent request
      fieldInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), newExtent);


      //check to see if exents have changed
      if(!this->eq(this->SubExtent, newExtent))
        {
          // Set the SubExtents to the NewExtents
          this->setExtents(this->SubExtent, newExtent);

          // The extents have changes, so mark grid dirty.
          this->gridClean = false;
        }

      //set the extents provided to Paraview
      Data->SetExtent(this->SubExtent);

      //Calculate Sub Dimensions
      this->extractDimensions(this->SubDimension, this->SubExtent);
      this->printExtents(this->WholeExtent, (char*)"Whole Extent:");
      this->printExtents(this->SubExtent, (char*)"Sub Extent:");

      //Generate the Grid
      this->GenerateGrid();

      //set points and radius
      Data->SetPoints(this->Points);
      Data->GetPointData()->AddArray(this->Radius);

      //Load Variables
      int c = 0;

      //Load Cell Data
      for(c = 0; c < this->CellDataArraySelection->GetNumberOfArrays(); c++)
        {
          //          std::cerr << "Loading Cell Variable " << c << std::endl;
          //Load the current Cell array
          this->LoadGridValues(this->CellDataArraySelection->GetArrayName(c));
        }

      //Load Point Data
      for(c=0; c < this->PointDataArraySelection->GetNumberOfArrays(); c++)
        {
          //          std::cerr << "Loading Point Variable " << c << std::endl;
          //Load the current Point array
          this->LoadGridValues(this->CellDataArraySelection->GetArrayName(c));
        }
    }

  return 1;
}

//-- Return 0 for Failure, 1 for Success --//
int vtkEnlilReader::LoadGridValues(const char* array)
{

  return 1;
}

//-- Return 0 for failure, 1 for success --//
/* You will want to over-ride this method to
 * Populate the system with your own arrays */
int vtkEnlilReader::PopulateArrays()
{

  /*Open File and Find array names*/
  NcFile file(this->FileName);
  int numVars = file.num_vars();

  this->addPointArray((char*)"D");
  this->addPointArray((char*)"DP");
  this->addPointArray((char*)"T");
  this->addPointArray((char*)"BP");
  this->addPointArray((char*)"B1");
  this->addPointArray((char*)"V1");


  file.close();
  return 1;
}

//-- Meta Data Population
int vtkEnlilReader::LoadMetaData(vtkInformationVector *outputVector)
{
  int ncFileID = 0;
  int ncSDSID = 0;
  int natts = 0;

  nc_type type;

  char attname[256];

  char    attvalc[256];
  int     attvali;
  double  attvald;

  vtkTable* MetaData = vtkTable::GetData(outputVector,1);
  int status = this->checkStatus(MetaData, (char*)"(PMD) Meta Data Table Object");


  if(status)
    {

      CALL_NETCDF(nc_open(this->FileName, NC_NOWRITE, &ncFileID));
      CALL_NETCDF(nc_inq_natts(ncFileID, &natts));

      for(int q=0; q < natts; q++)
        {
          vtkStringArray *MetaString = vtkStringArray::New();
          vtkIntArray *MetaInt = vtkIntArray::New();
          vtkDoubleArray *MetaDouble = vtkDoubleArray::New();

          CALL_NETCDF(nc_inq_attname(ncFileID, NC_GLOBAL, q, attname));
          CALL_NETCDF(nc_inq_atttype(ncFileID, NC_GLOBAL, attname, &type));

          switch(type)
            {
            case 1:
              break;

            case 2: //text
              this->clearString(attvalc,256);
              CALL_NETCDF(nc_get_att_text(ncFileID, NC_GLOBAL, attname, attvalc));

              MetaString->SetName(attname);
              MetaString->SetNumberOfComponents(1);
              MetaString->InsertNextValue(attvalc);

              MetaData->AddColumn(MetaString);

              break;

            case 3:
              break;

            case 4: //int
              CALL_NETCDF(nc_get_att_int(ncFileID, NC_GLOBAL, attname, &attvali));

              MetaInt->SetName(attname);
              MetaInt->SetNumberOfComponents(1);
              MetaInt->InsertNextValue(attvali);

              MetaData->AddColumn(MetaInt);
              break;

            case 5:
              break;

            case 6: //double
              CALL_NETCDF(nc_get_att_double(ncFileID, NC_GLOBAL, attname, &attvald));

              MetaDouble->SetName(attname);
              MetaDouble->SetNumberOfComponents(1);
              MetaDouble->InsertNextValue(attvald);

              MetaData->AddColumn(MetaDouble);
              break;
            }



        }

      CALL_NETCDF(nc_close(ncFileID));

    }

  return 1;
}

int vtkEnlilReader::checkStatus(void *Object, char *name)
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
      //      std::cerr << "SUCCESS: " << name
      //                << " has successfully initialized"
      //                << std::endl;
    }

  return 1;
}

//-- Return 0 for failure, 1 for success --//
/* Over-ride this method to provide the
 * extents of your data */
int vtkEnlilReader::PopulateDataInformation()
{

  NcFile data(this->FileName);
  NcDim* dims_x = data.get_dim(0);
  NcDim* dims_y = data.get_dim(1);
  NcDim* dims_z = data.get_dim(2);

  NcVar* time = data.get_var("TIME");

  this->Dimension[0] = (int)dims_x->size();
  this->Dimension[1] = (int)dims_y->size();
  this->Dimension[2] = (int)dims_z->size()+1;

  int Time = time->as_double(0);

  data.close();

  //Populate Extents
  this->setExtents(this->WholeExtent,
                   0, this->Dimension[0]-1,
                   0, this->Dimension[1]-1,
                   0, this->Dimension[2]-1);

  //Whole Extent
  this->printExtents(this->WholeExtent, (char*)"Whole Extent:");

  //Set Time step Information
  this->NumberOfTimeSteps = 1;
  this->TimeSteps = new double[this->NumberOfTimeSteps];
  this->TimeSteps[0] = Time;

  //We just populated info, so we are clean
  this->infoClean = true;

  return 1;
}

//-- print extents --//
void vtkEnlilReader::printExtents(int extent[], char* description)
{
  std::cout << description << " "
            << extent[0] << " " <<
               extent[1] << " " <<
               extent[2] << " " <<
               extent[3] << " " <<
               extent[4] << " " <<
               extent[5] << std::endl;
}

void vtkEnlilReader::setExtents(int extentToSet[], int sourceExtent[])
{
  extentToSet[0] = sourceExtent[0];
  extentToSet[1] = sourceExtent[1];
  extentToSet[2] = sourceExtent[2];
  extentToSet[3] = sourceExtent[3];
  extentToSet[4] = sourceExtent[4];
  extentToSet[5] = sourceExtent[5];

}

void vtkEnlilReader::setExtents(int extentToSet[], int dim1, int dim2, int dim3, int dim4, int dim5, int dim6)
{
  extentToSet[0] = dim1;
  extentToSet[1] = dim2;
  extentToSet[2] = dim3;
  extentToSet[3] = dim4;
  extentToSet[4] = dim5;
  extentToSet[5] = dim6;
}

bool vtkEnlilReader::eq(int extent1[], int extent2[])
{
  return (extent1[0] == extent2[0] && extent1[1] == extent2[1]
          && extent1[2] == extent2[2] && extent1[3] == extent2[3]
          && extent1[4] == extent2[4] && extent1[5] == extent2[5]);
}

bool vtkEnlilReader::ExtentOutOfBounds(int extToCheck[], int extStandard[])
{
  return extToCheck[0] < 0 || (extToCheck[0] > extStandard[0])
      || extToCheck[1] < 0 || (extToCheck[1] > extStandard[1])
      || extToCheck[2] < 0 || (extToCheck[2] > extStandard[2])
      || extToCheck[3] < 0 || (extToCheck[3] > extStandard[3])
      || extToCheck[4] < 0 || (extToCheck[4] > extStandard[4])
      || extToCheck[5] < 0 || (extToCheck[5] > extStandard[5]);

}

void vtkEnlilReader::extractDimensions(int dims[], int extent[])
{
  dims[0] = extent[1] - extent[0]+1;
  dims[1] = extent[3] - extent[2]+1;
  dims[2] = extent[5] - extent[4]+1;
}

void vtkEnlilReader::addPointArray(char* name)
{
  NcFile file(this->FileName);
  try
  {
    // look up the "Long Name" of the variable
    vtkstd::string varname = file.get_var(name)->get_att("long_name")->as_string(0);
    this->variableMap[varname] = vtkstd::string(name);

    // Add it to the point grid
    this->PointDataArraySelection->AddArray(varname.c_str());
  }
  catch (...)
  {
    std::cerr << "Failed to retrieve variable " << name
              << ". Verify variable name." << std::endl;

    file.close();
    return;
  }

  file.close();
}

void vtkEnlilReader::addPointArray(char* name1, char* name2, char* name3)
{

}

//-- Return 0 for failure, 1 for success --//
/* You will need to over-ride this method to provide
 * your own grid-information */
int vtkEnlilReader::GenerateGrid()
{

  int i = 0;
  int j = 0;
  int k = 0;

  size_t startLoc[2]={1,0};
  size_t startDim[2]={1,1};

  //set read start:
  startLoc[0] = this->SubExtent[0];
  startLoc[1] = this->SubExtent[2];
  startLoc[2] = this->SubExtent[4];

  double *X1 = new double[this->SubDimension[0]];
  double *X2 = new double[this->SubDimension[1]];
  double *X3 = new double[this->SubDimension[2]];

  int ncFileID = 0;
  int ncSDSID = 0;
  int varDim = 0;

  int X3_dims = 0;

  char tempName[256];
  this->clearString(tempName, 256);

  const int GridScale = this->GetGridScaleType();

  //build the grid if it is dirty (modified in application)
  if(!this->gridClean)
    {
      if(this->Points != NULL)
        {
          this->Points->Delete();
          this->Radius->Delete();
        }

      std::cerr << "Grid Dirty; Rebuilding " << std::endl;

      //build the Grid
      this->Points = vtkPoints::New();

      //build the Radius Array
      this->Radius = vtkDoubleArray::New();
      this->Radius->SetName("Radius");
      this->Radius->SetNumberOfComponents(1);

      //Start grid partial read.
      CALL_NETCDF(nc_open(this->FileName, NC_NOWRITE, &ncFileID));

      //Read the Radius Component//
      //-------------------------//
      CALL_NETCDF(nc_inq_varid(ncFileID, "X1", &ncSDSID));
      CALL_NETCDF(nc_inq_varndims(ncFileID, ncSDSID, &varDim));

      //start location at {1,0}
      startLoc[0] = 0;
      startLoc[1] = this->SubExtent[0];

      //dimensions from start
      startDim[1] = this->SubDimension[0];
      startDim[0] = 1;

      CALL_NETCDF(nc_get_vara_double(ncFileID,
                                     ncSDSID,
                                     startLoc,
                                     startDim,
                                     X1));

      //Read Theta Component//
      //--------------------//
      CALL_NETCDF(nc_inq_varid(ncFileID, "X2", &ncSDSID));
      CALL_NETCDF(nc_inq_varndims(ncFileID, ncSDSID, &varDim));

      //start location at {1,0}
      startLoc[0] = 0;
      startLoc[1] = this->SubExtent[2];

      //dimensions from start
      startDim[1] = this->SubDimension[1];
      startDim[0] = 1;

      CALL_NETCDF(nc_get_vara_double(ncFileID,
                                     ncSDSID,
                                     startLoc,
                                     startDim,
                                     X2));

      //Read Phi Component//
      //------------------//
      CALL_NETCDF(nc_inq_varid(ncFileID, "X3", &ncSDSID));
      CALL_NETCDF(nc_inq_varndims(ncFileID, ncSDSID, &varDim));

      //start location at {1,0}
      startLoc[0] = 0;

      //Check for reading of Periodic Boundry ONLY
      // Fix so we read the 0 position instead
      if(this->SubExtent[4] == this->Dimension[2]-1)
        {
          //periodic correction
          startLoc[1] = 0;
        }
      else
        {
          //not reading the periodic boundry
          startLoc[1] = this->SubExtent[4];
        }

      //if reading through loop of grid, must make adjustments to X dimension
      if(this->SubDimension[2] == this->Dimension[2])
        {
          //if full grid, we need to recognize that
          //the file does not contain the grid closure.
          X3_dims = this->SubDimension[2] - 1;
        }
      else
        {
          // if not reading the entire grid, we don't need to
          // worry about the grid closure.
          X3_dims = this->SubDimension[2];
        }

      std::cerr << "X3_dims:  " << X3_dims << std::endl;
      std::cerr << "startLoc: " << startLoc[0] << ":" << startLoc[1] << std::endl;

      //TODO: Reading ONLY 180 phi will cause read error...
      //need to read 0 in this case

      //dimensions from start
      startDim[1] = X3_dims;
      //this->SubDimension[2];
      startDim[0] = 1;

      CALL_NETCDF(nc_get_vara_double(ncFileID,
                                     ncSDSID,
                                     startLoc,
                                     startDim,
                                     X3));

      //end partial read on grid
      CALL_NETCDF(nc_close(ncFileID));

      //if whole extent on X3, must add grid closure.
      if(this->SubDimension[2] == this->Dimension[2])
        {
          //close off X3 if reading the entire grid in the Phi direction
          X3[this->Dimension[2]-1] = X3[0];
        }

      // Generate the grid based on the R-P-T coordinate system.
      double xyz[3] = { 0, 0, 0 };
      for (k = 0; k < this->SubDimension[2]; k++)
        {
          for (j = 0; j < this->SubDimension[1]; j++)
            {
              for (i = 0; i < this->SubDimension[0]; i++)
                {
                  xyz[0] = (X1[i] * sin(X2[j]) * cos(X3[k]))
                      / GRID_SCALE::ScaleFactor[GridScale];
                  xyz[1] = (X1[i] * sin(X2[j]) * sin(X3[k]))
                      / GRID_SCALE::ScaleFactor[GridScale];
                  xyz[2] = (X1[i] * cos(X2[j]))
                      / GRID_SCALE::ScaleFactor[GridScale];

                  //insert point information into the grid
                  this->Points->InsertNextPoint(xyz);

                  // insert radius value into radius array.
                  // Scaled by grid scale factor
                  this->Radius->InsertNextValue(
                        X1[i]
                        / GRID_SCALE::ScaleFactor[GridScale]);
                }
            }
        }

      //grid just created, so clean by definition.
      this->gridClean=true;
    }
  return 1;
}

//=================== END USER METHODS =========================


//--------------------------------------------------------------
//    Output Port Configuration
//--------------------------------------------------------------
int vtkEnlilReader::FillOutputPortInformation(int port, vtkInformation* info)
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
void vtkEnlilReader::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

}
