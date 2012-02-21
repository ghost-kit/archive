//======================================================================================================
// File: vtkENLILReader.cxx
// Author: Joshua Murphy
// Date Last Updated: 13 DEC 2011
// NOTES: This is a ParaView Reader for the Space Weather Prediction Center's Version of the
//      Enlil Solar Wind model as of the data above.  IF the data format of your model output is
//      the same as the SWPC model version, then you should have no trouble using this reader.
//
//      This reader will be updated to accept other versions of the Enlil files as the need arises.
//=====================================================================================================

//TODO: Set Variable Names
//TODO: ensure names are being returned from functions properly


#include "vtkENLILReader.h"

#include "vtkPointData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"

#include <vtkstd/map>
#include <vtkstd/string>

#include <vtk_netcdf.h>

using namespace std;

vtkStandardNewMacro(vtkENLILReader);

vtkENLILReader::vtkENLILReader()
{
  this->EnlilFileName = NULL;
    

  //this->DebugOn();
}

vtkENLILReader::~vtkENLILReader()
{
  if (this->EnlilFileName)
    {
      delete[] this->EnlilFileName;
      this->EnlilFileName = NULL;
    }
    

}

int vtkENLILReader::CanReadFile(const char *filename)
{
  int ncStatus = 0;
  int ncFileID = 0;
  int status = 0;
    
    int ncStatus = 0;
    int ncFileID = 0;
    int ncSDSID = 0;
    
    int ndims=0, nvars=0, ngatts=0, unlimdimid=0;
    
    ncStatus = nc_open(filename, NC_NOWRITE, &ncFileID);
    
    if (ncStatus != NC_NOERR) 
    {
        return 0;
    }
    
  int ndims=0, nvars=0, ngatts=0, unlimdimid=0;

  ncStatus = nc_open(filename, NC_NOWRITE, &ncFileID);
  status = nc_inq(ncFileID, &ndims, &nvars, &ngatts, &unlimdimid);
  nc_close(ncFileID);


  // TODO: Make sure all the relevant meta data exists
  //    We need a valid test to confirm the files
  if (nvars != 16)
    {
      return 0;
    }
    
  return 1;
}

//Gets information on file.
//TODO: See Documentation to see best way of doing this.
int vtkENLILReader::RequestInformation(vtkInformation* request,
                                       vtkInformationVector** inputVector,
                                       vtkInformationVector* outputVector)

{

  int ncStatus = 0;
  int ncFileID = 0;
  int ncSDSID = 0;

  double TIME = 0;
  double timeRange[2];
  double timeStep=0;

  size_t dim_r = 0;
  size_t dim_theta = 0;
  size_t dim_phi = 0;

  vtkDebugMacro(<< __FILE__ << " " << __FUNCTION__ << " (L" << __LINE__ << "): "
                << "Hello world!"
                << endl);

  ncStatus = nc_open(this->EnlilFileName, NC_NOWRITE, &ncFileID);

  if (ncStatus != NC_NOERR)
    {
      vtkDebugMacro(<<"ERROR Opening File " << this->EnlilFileName);
      return 0;
    }


  //get dimension data
  ncStatus = nc_inq_dimid(ncFileID, "n1", &ncSDSID);
  ncStatus = nc_inq_dimlen(ncFileID, ncSDSID, &dim_r);

  ncStatus = nc_inq_dimid(ncFileID, "n2", &ncSDSID);
  ncStatus = nc_inq_dimlen(ncFileID, ncSDSID, &dim_theta);

  ncStatus = nc_inq_dimid(ncFileID, "n3", &ncSDSID);
  ncStatus = nc_inq_dimlen(ncFileID, ncSDSID, &dim_phi);

  ncStatus = nc_inq_varid(ncFileID, "TIME", &ncSDSID);
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, &TIME);

  nc_close(ncFileID);

  cout << __FUNCTION__ << " nc_close" << endl;

  this->dimR = dim_r;
  this->dimTheta = dim_theta;
  this->dimPhi = dim_phi;

  int extent[6] = { 0, (this->dimR - 1), 0, (this->dimTheta - 1), 0,
                    (this->dimPhi) };

  if (!dimR || !dimTheta || !dimPhi)
    {
      vtkDebugMacro(<< "Extents Failure");
      return 0;
    }
    
  vtkDebugMacro(<< "Whole extents: "
                << extent[0] << ", " << extent[1] << ", "
                << extent[2] << ", " << extent[3] << ", "
                << extent[4] << ", " << extent[5]);
    
  vtkDebugMacro(<< "DIMS: " << dimR << ", " << dimTheta << ", " << dimPhi);
    
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6);

  //Set Time step Information
  this->NumberOfTimeSteps = 1;
  this->TimeStepValues.assign(this->NumberOfTimeSteps, 0.0);
  this->TimeStepValues[0] = TIME;
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
               &this->TimeStepValues[0],
               static_cast<int>(this->TimeStepValues.size()));



  //Set Time Range for file
  timeRange[0] = this->TimeStepValues.front();
  timeRange[1] = this->TimeStepValues.back();

  //Update Pipeline
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);
    
    
    
    
  return 1;
}

/**
 * Callback that gets the actual data
 * TODO: look to see if we are doing it correctly by reading everything.
 */
int vtkENLILReader::RequestData(vtkInformation* request,
                                vtkInformationVector** inputVector,
                                vtkInformationVector* outputVector) {
  vtkDebugMacro(<<"Reading ENLIL NETCDF file as a vtkStructuredGrid...");
  vtkDebugMacro(<< "GridScaleType is \"" << this->GetGridScaleType() << "\".");
  vtkDebugMacro(<< "GridScaleFactor is \"" << GRID_SCALE::ScaleFactor[this->GetGridScaleType()] << "\"");
    
  ///////////////////
  // Set sub extents
  int ncStatus = 0;
  int ncFileID = 0;
  int ncSDSID = 0;
    
  int ncDimID_r = 0;
  int ncDimID_theta = 0;
  int ncDimID_phi = 0;
    
  int i = 0;
  int j = 0;
  int k = 0;
    
  double *B1 = NULL;
  double *B2 = NULL;
  double *B3 = NULL;
  double *BP = NULL;
  //	double *D = NULL;
  //	double *DP = NULL;
  //	double *DT = NULL;
  //	double *T = NULL;
  //	double *NSTEP = NULL;
  //	double *TIME = NULL;
  //	double *V1 = NULL;
  //	double *V2 = NULL;
  //	double *V3 = NULL;
  double *X1 = NULL;
  double *X2 = NULL;
  double *X3 = NULL;
    
  //TODO: Convert to C++ libraries
  ncStatus = nc_open(this->EnlilFileName, NC_NOWRITE, &ncFileID);

  if (ncStatus != NC_NOERR) {
      vtkDebugMacro(<<"ERROR Opening File " << this->EnlilFileName);
      return 0;
    }

  X1 = new double[this->dimR];
  X2 = new double[this->dimTheta];
  X3 = new double[this->dimPhi];
  BP = new double[this->dimR * this->dimTheta * this->dimPhi];
    
  int64_t     ni = this->dimR-1,
      nj = this->dimTheta-1,
      nk = this->dimPhi-1;
    

  //Get Coordinate Array and Sizes
  //TODO: Separate these items into own functions

  ncStatus = nc_inq_varid(ncFileID, "X1", &ncSDSID);
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, X1);
    
  ncStatus = nc_inq_varid(ncFileID, "X2", &ncSDSID);
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, X2);
    
  ncStatus = nc_inq_varid(ncFileID, "X3", &ncSDSID);
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, X3);
    
  ncStatus = nc_inq_varid(ncFileID, "BP", &ncSDSID);
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, BP);
  nc_close(ncFileID);
    

  vtkDebugMacro(<< "NcFile Opened");

  vtkDebugMacro(<< "NcVar's Declared");
    
  vtkDebugMacro(<< "line 188");
    
  int subext[6] = { 0, (this->dimR - 1), 0, (this->dimTheta - 1), 0,
                    (this->dimPhi) };
    
  vtkDebugMacro(<< "sub extents: "
                << subext[0] << ", " << subext[1] << ", "
                << subext[2] << ", " << subext[3] << ", "
                << subext[4] << ", " << subext[5]);

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), subext);
    
  vtkDebugMacro(<< "Line 193");
    
  ///////////////////////////////////////////////////////////////////////////
  //read that part of the data in from the file and put it in the output data
    
  vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));
    
  output->SetDimensions(this->dimR, this->dimTheta, (this->dimPhi + 1));
  this->numberOfPoints = (this->dimR * this->dimTheta * (this->dimPhi + 1));

  vtkDebugMacro(<< "Line 203");
    
  //////////////////////
  // Point-centered data
    

  double xyz[3] = { 0, 0, 0 };
  int64_t gridIndex = 0;
  int64_t oldOffset = -1;
  int64_t count = 0;

  double radiusValue;
  const int GridScale = this->GetGridScaleType();
    

  /*
     * GRID read section
     */
    
  /*
  * Read in the grid, and construct it in the output file
  */

  vtkPoints *gridPoints = vtkPoints::New();

  //Radius Configuration
  vtkDoubleArray *Radius = vtkDoubleArray::New();
  Radius->SetName("Radius");
  Radius->SetNumberOfComponents(1);

  for (k = 0; k < this->dimPhi; k++)
    {
      for (j = 0; j < this->dimTheta; j++)
        {
          for (i = 0; i < this->dimR; i++)
            {
              xyz[0] = X1[i] * sin(X2[j]) * cos(X3[k])
                  / GRID_SCALE::ScaleFactor[GridScale];
              xyz[1] = X1[i] * sin(X2[j]) * sin(X3[k])
                  / GRID_SCALE::ScaleFactor[GridScale];
              xyz[2] = X1[i] * cos(X2[j])
                  / GRID_SCALE::ScaleFactor[GridScale];

              //calculate the radius at this point
              radiusValue = sqrt(xyz[0]*xyz[0]+xyz[1]*xyz[1]+xyz[2]*xyz[2]);


              //insert point information into the grid

              gridPoints->InsertNextPoint(xyz);
              Radius->InsertNextValue(radiusValue);
            }
        }
        
    }
  //Close off the gap in the grid (make sphere continuous
  for (j = 0; j < this->dimTheta; j++)
    {
      for (i = 0; i < this->dimR; i++)
        {
          xyz[0] = X1[i] * sin(X2[j]) * cos(X3[0])
              / GRID_SCALE::ScaleFactor[this->GetGridScaleType()];
          xyz[1] = X1[i] * sin(X2[j]) * sin(X3[0])
              / GRID_SCALE::ScaleFactor[this->GetGridScaleType()];
          xyz[2] = X1[i] * cos(X2[j])
              / GRID_SCALE::ScaleFactor[this->GetGridScaleType()];


          radiusValue = sqrt(xyz[0]*xyz[0]+xyz[1]*xyz[1]+xyz[2]*xyz[2]);
          //Fix the Gap... insert the contiguous points

          gridPoints->InsertNextPoint(xyz);
          Radius->InsertNextValue(radiusValue);

        }
    }
    
output->GetPointData()->AddArray(Radius);
Radius->Delete();
output->SetPoints(gridPoints);
gridPoints->Delete();

/*
     * Data Read Section
     */
    
vtkDoubleArray *cellScalar_Density = vtkDoubleArray::New();
cellScalar_Density->SetName("Density");
cellScalar_Density->SetNumberOfComponents(1);
    
for (int x = 0; x < ((this->dimPhi+1)*this->dimTheta*this->dimR); x++ )
{
  cellScalar_Density->InsertNextValue(BP[x]);
}
    
output->GetPointData()->AddArray(cellScalar_Density);
cellScalar_Density->Delete();
            
#if 0
/*****************************************************************************
     * Cell-centered scalar data
     * ==========================
     * This Block will extract requested Cell-Centered Scalar Data from the Data
     * File and provide that information to ParaView.
     *
     * Values Extracted:
     *      Density
     *      Plasma Density
     *      Temperature
     *      Magnetic Polarity
     *
     *
     ****************************************************************************/
vtkDoubleArray *cellScalar_Density = vtkDoubleArray::New();
cellScalar_Density->SetName("Density");
cellScalar_Density->SetNumberOfComponents(1);
cellScalar_Density->SetNumberOfTuples(this->dimR*this->dimTheta*this->dimPhi);
    
vtkDoubleArray *cellScalar_PlasmaDensity = vtkDoubleArray::New();
cellScalar_PlasmaDensity->SetName("Plasma Density");
cellScalar_PlasmaDensity->SetNumberOfComponents(1);
cellScalar_PlasmaDensity->SetNumberOfTuples(this->dimR*this->dimTheta*this->dimPhi);
    
vtkDoubleArray *cellScalar_Temperature = vtkDoubleArray::New();
cellScalar_Temperature->SetName("Temperature");
cellScalar_Temperature->SetNumberOfComponents(1);
cellScalar_Temperature->SetNumberOfTuples(this->dimR*this->dimTheta*this->dimPhi);
    
vtkDoubleArray *cellScalar_Polarity = vtkDoubleArray::New();
cellScalar_Polarity->SetName("Magnetic Polarity");
cellScalar_Polarity->SetNumberOfComponents(1);
cellScalar_Polarity->SetNumberOfTuples(this->dimR*this->dimTheta*this->dimPhi);
    
/***************************************************************************
     * Cell-centered Vector data
     * =========================
     * This Block will extract requested Cell-Centered Vector Data from the Data
     * File and provide that information to ParaView.
     * Values Extracted:
     *       Velocity
     *       Magnetic Field
     *
     ****************************************************************************/
#endif
    
    
if(X1) free(X1);
if(X2) free(X2);
if(X3) free(X3);
if(B1) free(B1);
if(B2) free(B2);
if(B3) free(B3);
if(BP) free(BP);
    
    
return 1;
}

//----------------------------------------------------------------

const char * vtkENLILReader::GetCellArrayName(int index)
{
  const char* name;
  int nameSize;
    
  //name = this->CellArrayName[index].c_str();
    
  return "name";
}

//----------------------------------------------------------------

const char * vtkENLILReader::GetPointArrayName(int index)
{
  const char* name;
  int nameSize;
    
  //name = this->PointArrayName[index].c_str();
    

  return "name";
}


//----------------------------------------------------------------

//Cell Array Status Retrieval
int vtkENLILReader::GetCellArrayStatus(const char *CellArray)
{
  return this->CellArrayStatus[string(CellArray)];
}

//----------------------------------------------------------------

int vtkENLILReader::GetPointArrayStatus(const char *PointArray)
{
  return this->PointArrayStatus[string(PointArray)];
}

//----------------------------------------------------------------

//Cell Array Status Set
void vtkENLILReader::SetCellArrayStatus(const char* CellArray, int status)
{
    
  this->CellArrayStatus[CellArray] = status;
  this->Modified();
    
}

//----------------------------------------------------------------

void vtkENLILReader::SetPointArrayStatus(const char* PointArray, int status)
{
  this->PointArrayStatus[PointArray] = status;
  this->Modified();
}

//----------------------------------------------------------------


void vtkENLILReader::PrintSelf(ostream &os, vtkIndent indent) {
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkENLILReader says \"Hello, World!\" " << "\n";
}
