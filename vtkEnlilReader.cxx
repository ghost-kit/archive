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
#include <iostream>

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
          //Load the current Cell array
          vtkstd::string array = vtkstd::string(this->CellDataArraySelection->GetArrayName(c));
          if(CellDataArraySelection->ArrayIsEnabled(array.c_str()))
            {
              this->LoadArrayValues(array, outputVector);
            }
        }

      //Load Point Data
      for(c=0; c < this->PointDataArraySelection->GetNumberOfArrays(); c++)
        {
          vtkstd::string array = vtkstd::string(this->PointDataArraySelection->GetArrayName(c));

          //Load the current Point array
          if(PointDataArraySelection->ArrayIsEnabled(array.c_str()))
            {
              this->LoadArrayValues(array, outputVector);
            }
        }
    }

  return 1;
}

//-- Return 0 for Failure, 1 for Success --//
int vtkEnlilReader::LoadArrayValues(vtkstd::string array, vtkInformationVector* outputVector)
{

  bool vector
      = (this->VectorVariableMap.find(vtkstd::string(array)) != this->VectorVariableMap.end());

  double xyz[3] = {0.0, 0.0, 0.0};

  //get data from system
  vtkStructuredGrid *Data = vtkStructuredGrid::GetData(outputVector,0);

  //set up array to be added
  vtkDoubleArray *DataArray = vtkDoubleArray::New();
  DataArray->SetName(array.c_str());

  if(vector)      //load as a vector
    {
      //need three arrays for vector reads
      double* newArrayR;
      double* newArrayT;
      double* newArrayP;

      //configure DataArray
      DataArray->SetNumberOfComponents(3);  //3-Dim Vector

      //read in the arrays
      newArrayR
          = this->read3dPartialToArray((char*)this->VectorVariableMap[array][0].c_str(), this->SubExtent);

      newArrayT
          = this->read3dPartialToArray((char*)this->VectorVariableMap[array][1].c_str(), this->SubExtent);

      newArrayP
          = this->read3dPartialToArray((char*)this->VectorVariableMap[array][2].c_str(), this->SubExtent);


      // convert from spherical to cartesian
      int loc=0;
      int i,j,k;
      for(k=0; k<this->SubDimension[2]; k++)
        {
          for(j=0; j<this->SubDimension[1]; j++)
            {
              for(i=0; i<this->SubDimension[0]; i++)

                {

                  xyz[0] =newArrayR[loc]*sin(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][k]);
                  xyz[1] =newArrayR[loc]*sin(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][k]);
                  xyz[2] =newArrayR[loc]*cos(this->sphericalGridCoords[1][j]);

                  xyz[0] += newArrayT[loc]*cos(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][k]);
                  xyz[1] += newArrayT[loc]*cos(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][k]);
                  xyz[2] += -1.0*newArrayT[loc]*sin(this->sphericalGridCoords[1][j]);

                  xyz[0] += -1.0*newArrayP[loc]*sin(this->sphericalGridCoords[2][k]);
                  xyz[1] += newArrayP[loc]*cos(this->sphericalGridCoords[2][k]);

                  DataArray->InsertNextTuple(xyz);

                  loc++;

                }
            }
        }
      //Add array to grid
      Data->GetPointData()->AddArray(DataArray);
      DataArray->Delete();
      //free temporary memory
      delete [] newArrayR; newArrayR = NULL;
      delete [] newArrayP; newArrayP = NULL;
      delete [] newArrayT; newArrayT = NULL;

    }
  else
    {
      //load as a scalar
      //configure DataArray
      DataArray->SetNumberOfComponents(1);  //Scalar

      //array pointers
      double* newArray;

      //get data array
      newArray
          = this->read3dPartialToArray((char*)this->ScalarVariableMap[array].c_str(), this->SubExtent);

      //insert points
      int k;
      for(k=0; k<this->SubDimension[2]*this->SubDimension[1]*this->SubDimension[0]; k++)
        {
          DataArray->InsertNextValue(newArray[k]);
        }

      //Add array to grid
      Data->GetPointData()->AddArray(DataArray);
      DataArray->Delete();
      //free temporary memory
      delete [] newArray; newArray = NULL;

    }


  return 1;
}

//-- returns array read via partial IO limited by extents --//
/* This method will automatically adjust for the periodic boundary
 *  condition that does not exist sequentially in file */
double* vtkEnlilReader::read3dPartialToArray(char* arrayName, int extents[])
{
  int extDims[3] = {0,0,0};
  size_t readDims[4]   = {1,1,1,1};
  long readStart[4]  = {0,extents[4],extents[2],extents[0]};

  // get dimensions from extents
  this->extractDimensions(extDims, extents);

  // Enlil encodes in reverse, so reverse the order, add fourth dimension 1st.
  readDims[1] = extDims[2];
  readDims[2] = extDims[1];
  readDims[3] = extDims[0];

  // DEBUG VERIFICATION
  std::cout << "Read Dimensions: "
            << readDims[0] << ":"
            << readDims[1] << ":"
            << readDims[2] << ":"
            << readDims[3] << std::endl;

  std::cout << "Read Start: "
            << readStart[0] << ":"
            << readStart[1] << ":"
            << readStart[2] << ":"
            << readStart[3] << std::endl;

  //find all conditions that need to be accounted for
  bool periodic = false;
  bool periodicRead = false;
  bool periodicOnly = false;

  if(extents[5] == this->WholeExtent[5])
    {
      periodic = true;
      if(extents[4] > 0)
        {
          periodicRead = true;
          if(extents[4] == this->WholeExtent[5])
            {
              periodicOnly = true;
            }
        }
    }

  // allocate memory for complete array
  double *array = new double[extDims[0]*extDims[1]*extDims[2]];

  //open file
  NcFile file(this->FileName);
  NcVar* variable = file.get_var(arrayName);

  // start to read in data
  if(periodic && !periodicOnly)
    {
      std::cout << "periodic && !periodicOnly" << std::endl;

      //adjust dims
      readDims[1] = readDims[1]-1;

      //adjust the start point
      variable->set_cur(readStart);

      //read the file
      variable->get(array, readDims);

    }
  else if(periodicOnly)
    {
      std::cout << "periodicOnly" << std::endl;

      //set periodic only
      readDims[1] = 1;
      readStart[1] = 0;
      readStart[2] = 0;
      readStart[3] = 0;

      //set read location
      variable->set_cur(readStart);

      //read the file
      variable->get(array, readDims);

    }
  else
    {
      std::cout << "!periodic" << std::endl;

      //set read location as stated
      variable->set_cur(readStart);

      //read as stated
      variable->get(array, readDims);

    }

  //fix periodic boundary if necesary
  if(periodic && !periodicRead && !periodicOnly)
    {
      std::cout << "periodic && !periodicRead && !periodicOnly" << std::endl;

      //copy periodic data from begining to end
      size_t wedgeSize = (extDims[0]*extDims[1]);
      size_t wedgeLoc  = (extDims[0]*extDims[1])*(extDims[2]-1);

      for(int x = 0; x < wedgeSize; x++)
        {
          //copy the wedge
          array[wedgeLoc] = array[x];

          //advance index
          wedgeLoc++;
        }

    }
  else if (periodic && periodicRead && !periodicOnly)
    {
      std::cout << "periodic && periodicRead && !periodicOnly" << std::endl;

      //read in periodic data and place at end of array
      size_t wedgeSize = extDims[0]*extDims[1];
      size_t wedgeLoc  = (extDims[0]*extDims[1])*(extDims[2]-1);

      double * wedge = new double[wedgeSize];

      //start at 0,0,0
      readStart[1] = 0;
      readStart[2] = 0;
      readStart[3] = 0;

      //restrict to phi = 1 dimension
      readDims[1] = 1;

      //set start
      variable->set_cur(readStart);

      //read data
      variable->get(wedge, readDims);

      //populate wedge to array
      for(int x = 0; x < wedgeSize; x++)
        {
          //copy the wedge
          array[wedgeLoc] = wedge[x];

          //advance index
          wedgeLoc++;
        }

      //free temp memory
      delete [] wedge; wedge = NULL;
    }


  //close file
  file.close();

  std::cout << "returning array" << std::endl;

  return array;

}

//-- returns array read via partial IO limited by extents --//
/* This method will automatically adjust for the periodic boundary
 *  condition that does not exist sequentially in file */
double* vtkEnlilReader::readGridPartialToArray(char *arrayName, int subExtents[], bool isPeriodic = false)
{
  int     extDim = subExtents[1]-subExtents[0]+1;;
  size_t  readDims[2]  = {1,extDim};
  long    readStart[2] = {0,subExtents[0]};

  // DEBUG VERIFICATION
  std::cout << "Read Dimensions: "
            << readDims[0] << ":"
            << readDims[1] << std::endl;

  std::cout << "Read Start: "
            << readStart[0] << ":"
            << readStart[1] << std::endl;

  //Find conditions that need to be handled
  bool periodic = false;
  bool periodicRead = false;
  bool periodicOnly = false;

  //if isPeriodic is set, then we are looking at phi
  if(isPeriodic)
    {
      if(subExtents[1] == this->WholeExtent[5])
        {
          periodic = true;
          if(subExtents[0] > 0)
            {
              periodicRead = true;
              if(subExtents[0] == this->WholeExtent[5])
                {
                  periodicOnly = true;
                }
            }
        }
    }

  //allocate Memory for complete array
  double *array = new double[extDim];

  //Open file
  NcFile file(this->FileName);
  NcVar* variable = file.get_var(arrayName);

  //start to read in data
  if(periodic && !periodicOnly)
    {
      std::cout << "periodic && !periodicOnly" << std::endl;

      //adjust dims
      readDims[1] = readDims[1]-1;

      //adjust the start point
      variable->set_cur(readStart);

      //read the file
      variable->get(array, readDims);

    }
  else if(periodicOnly)
    {
      std::cout << "periodicOnly" << std::endl;

      //set periodic only
      readDims[1] = 1;
      readStart[1] = 0;

      //set read location
      variable->set_cur(readStart);

      //read the file
      variable->get(array, readDims);

    }
  else
    {
      std::cout << "!periodic" << std::endl;

      //set read location as stated
      variable->set_cur(readStart);

      //read as stated
      variable->get(array, readDims);
    }

  //fix periodic boundary if necesary
  if(periodic && !periodicRead && !periodicOnly)
    {
      std::cout << "periodic && !periodicRead && !periodicOnly" << std::endl;

      //copy periodic data from begining to end
      array[extDim-1] = array[0];

    }
  else if (periodic && periodicRead && !periodicOnly)
    {
      std::cout << "periodic && periodicRead && !periodicOnly" << std::endl;

      //read in periodic data and place at end of array
      size_t wedgeSize = 1;
      size_t wedgeLoc  = (extDim-1);

      double * wedge = new double[wedgeSize];

      //start at 0,0,0
      readStart[1] = 0;

      //restrict to phi = 1 dimension
      readDims[1] = 1;

      //set start
      variable->set_cur(readStart);

      //read data
      variable->get(wedge, readDims);

      //populate wedge to array

      array[wedgeLoc] = wedge[0];

      //free temp memory
      delete [] wedge; wedge = NULL;
    }

  //close the file
  file.close();

  //return completed array
  return array;
}

//-- Return 0 for failure, 1 for success --//
/* You will want to over-ride this method to
 * Populate the system with your own arrays */
int vtkEnlilReader::PopulateArrays()
{

  /*Open File and Find array names*/
  NcFile file(this->FileName);

  this->addPointArray((char*)"D");
  this->addPointArray((char*)"DP");
  this->addPointArray((char*)"T");
  this->addPointArray((char*)"BP");
  this->addPointArray((char*)"B1", (char*)"B2", (char*)"B3");
  this->addPointArray((char*)"V1", (char*)"V2", (char*)"V3");

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
    this->ScalarVariableMap[varname] = vtkstd::string(name);

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
  NcFile file(this->FileName);
  try
  {
    //get the long name of the first variable in the vector
    vtkstd::string varname1 = file.get_var(name1)->get_att("long_name")->as_string(0);

    //remove the vector component of the name
    size_t pos = varname1.find("-");
    vtkstd::string varname2 = varname1.substr(pos+1);

    //ensure that first work is capitalized
    varname2[0] = toupper((unsigned char) varname2[0]);

    //add components of vector to vector map
    vtkstd::vector<vtkstd::string> nameArray;
    nameArray.push_back(name1);
    nameArray.push_back(name2);
    nameArray.push_back(name3);
    this->VectorVariableMap[varname2] = nameArray;

    //add array to point array name list
    this->PointDataArraySelection->AddArray(varname2.c_str());




  }
  catch(...)
  {
    std::cerr << "Failed to retrieve variable "
              << name1 << " or " << name2 << " or " << name3
              << ". Verify variable names." << std::endl;

    file.close();
    return;
  }
  file.close();
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
          this->sphericalGridCoords.clear();
        }

      std::cout << "Grid Dirty; Rebuilding " << std::endl;

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

      //if we need to read in Phi = 0 for periodic boundary.
      bool readZero = false;
      bool periodicOnly = false;

      //start location at {0,0}
      startLoc[0] = 0;

      //Default to NOT reading the periodic boundry
      startLoc[1] = this->SubExtent[4];

      if(startLoc[1] == this->WholeExtent[5])
        {
          periodicOnly = true;
        }

      //if reading through loop of grid, must make adjustments to phi dimension
      if(this->SubExtent[5] == this->WholeExtent[5])
        {
          //if full grid, we need to recognize that
          //the file does not contain the grid closure.
          X3_dims = this->SubDimension[2] - 1;

          if(this->SubExtent[4] > 0)
            {
              //mark periodic boundary for read
              readZero = true;
            }
        }
      else
        {
          // if not reading the end of the grid, we don't need to
          // worry about the grid closure.
          X3_dims = this->SubDimension[2];
        }

      startDim[0] = 1;
      startDim[1] = X3_dims;

      //we only need to make this call if we have more than the
      // periodic boundary to read.
      if(!periodicOnly)
        {
          CALL_NETCDF(nc_get_vara_double(ncFileID,
                                         ncSDSID,
                                         startLoc,
                                         startDim,
                                         X3));

        }

      //if whole extent on X3, must add grid closure.
      if(this->SubExtent[5] == this->WholeExtent[5])
        {
          //close off X3 if reading the entire grid in the Phi direction
          if(this->SubExtent[4] == this->WholeExtent[4])
            {
              X3[this->SubDimension[2]-1] = X3[0];
            }
          else  // we need to read in the phi = 0 and add it to X3
            {
              double X3_0_value;

              //reset the starting point to beginig
              startLoc[0] = 0;
              startLoc[1] = 0;

              //set read dimensions to
              startDim[1] = 1;

              //read the periodic boundary information
              CALL_NETCDF(nc_get_vara_double(ncFileID,
                                             ncSDSID,
                                             startLoc,
                                             startDim,
                                             &X3_0_value));

              X3[this->SubDimension[2]-1] = X3_0_value;
            }
        }

      //end partial read on grid
      CALL_NETCDF(nc_close(ncFileID));

      // Populate the Spherical Grid Coordinates (to be used in calcs later)
      vtkstd::vector<double> R(X1, X1 + this->SubDimension[0]);
      vtkstd::vector<double> T(X2, X2 + this->SubDimension[1]);
      vtkstd::vector<double> P(X3, X3 + this->SubDimension[2]);

      this->sphericalGridCoords.push_back(R);
      this->sphericalGridCoords.push_back(T);
      this->sphericalGridCoords.push_back(P);

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
                        X1[i] / GRID_SCALE::ScaleFactor[GridScale]);
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
