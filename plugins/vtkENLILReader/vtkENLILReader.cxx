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
  cout << __FUNCTION__ << endl;


  int ncStatus = 0;
  int ncFileID = 0;
  int status = 0;

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

  cout << __FUNCTION__ << endl;


  int ncStatus = 0;
  int ncFileID = 0;
  int ncSDSID = 0;

  double TIME = 0;
  double timeRange[2];

  size_t dim_r = 0;
  size_t dim_theta = 0;
  size_t dim_phi = 0;

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

  //Need to get Long Names to add to arrays



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


  cout << "END " << __FUNCTION__ << endl;

  return 1;
}

/**
 * Callback that gets the actual data
 * TODO: look to see if we are doing it correctly by reading everything.
 */
int vtkENLILReader::RequestData(vtkInformation* request,
                                vtkInformationVector** inputVector,
                                vtkInformationVector* outputVector)
{

  cout << __FUNCTION__ << endl;

  ///////////////////
  // Set sub extents
  int ncStatus = 0;
  int ncFileID = 0;
  int ncSDSID = 0;

  int i = 0;
  int j = 0;
  int k = 0;

  double *B1 = NULL;
  double *B2 = NULL;
  double *B3 = NULL;
  double *BP = NULL;
  double *D = NULL;
  double *DP = NULL;
  double *T = NULL;
  double *V1 = NULL;
  double *V2 = NULL;
  double *V3 = NULL;
  double *X1 = NULL;
  double *X2 = NULL;
  double *X3 = NULL;

  //TODO: Convert to C++ libraries
  ncStatus = nc_open(this->EnlilFileName, NC_NOWRITE, &ncFileID);

  if (ncStatus != NC_NOERR) {
      vtkDebugMacro(<<"ERROR Opening File " << this->EnlilFileName);
      return 0;
    }

  int64_t arraySize = this->dimPhi*this->dimR*this->dimTheta;
  char long_name[256];

  X1 = new double[this->dimR];
  X2 = new double[this->dimTheta];
  X3 = new double[this->dimPhi];
  D = new double[arraySize];
  DP = new double[arraySize];
  T = new double[arraySize];
  B1 = new double[arraySize];
  B2 = new double[arraySize];
  B3 = new double[arraySize];
  V1 = new double[arraySize];
  V2 = new double[arraySize];
  V3 = new double[arraySize];

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

  clearString(long_name, 256);

  ncStatus = nc_inq_varid(ncFileID, "D", &ncSDSID);
  ncStatus = nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name);
  cout << "long name: " << long_name << endl;
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, D);

  clearString(long_name, 256);

  ncStatus = nc_inq_varid(ncFileID, "DP", &ncSDSID);
  ncStatus = nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name);
  cout << "long name: " << long_name << endl;
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, DP);

  clearString(long_name, 256);

  ncStatus = nc_inq_varid(ncFileID, "T", &ncSDSID);
  ncStatus = nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name);
  cout << "long name: " << long_name << endl;
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, T);

  clearString(long_name, 256);

  ncStatus = nc_inq_varid(ncFileID, "B1", &ncSDSID);
  ncStatus = nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name);
  cout << "long name: " << long_name << endl;
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, B1);

  clearString(long_name, 256);

  ncStatus = nc_inq_varid(ncFileID, "B2", &ncSDSID);
  ncStatus = nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name);
  cout << "long name: " << long_name << endl;
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, B2);

  clearString(long_name, 256);

  ncStatus = nc_inq_varid(ncFileID, "B3", &ncSDSID);
  ncStatus = nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name);
  cout << "long name: " << long_name << endl;
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, B3);

  clearString(long_name, 256);

  ncStatus = nc_inq_varid(ncFileID, "V1", &ncSDSID);
  ncStatus = nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name);
  cout << "long name: " << long_name << endl;
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, V1);

  clearString(long_name, 256);

  ncStatus = nc_inq_varid(ncFileID, "V2", &ncSDSID);
  ncStatus = nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name);
  cout << "long name: " << long_name << endl;
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, V2);

  clearString(long_name, 256);

  ncStatus = nc_inq_varid(ncFileID, "V3", &ncSDSID);
  ncStatus = nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name);
  cout << "long name: " << long_name << endl;
  ncStatus = nc_get_var_double(ncFileID, ncSDSID, V3);

  nc_close(ncFileID);

  int subext[6] = { 0, (this->dimR - 1), 0, (this->dimTheta - 1), 0,
                    (this->dimPhi) };

  vtkDebugMacro(<< "sub extents: "
                << subext[0] << ", " << subext[1] << ", "
                << subext[2] << ", " << subext[3] << ", "
                << subext[4] << ", " << subext[5]);

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), subext);

  ///////////////////////////////////////////////////////////////////////////
  //read that part of the data in from the file and put it in the output data

  vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

  output->SetDimensions(this->dimR, this->dimTheta, (this->dimPhi + 1));

  //////////////////////
  // Point-centered data
  double xyz[3] = { 0, 0, 0 };

  double radiusValue;
  const int GridScale = this->GetGridScaleType();

  /*
     * GRID read section
     */

  //Grid Configuration
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
  output->SetPoints(gridPoints);
  gridPoints->Delete();
  Radius->Delete();


  /*
     * Data Read Section
     */

  vtkDoubleArray *cellScalar_Density = vtkDoubleArray::New();
  cellScalar_Density->SetName("Density");
  cellScalar_Density->SetNumberOfComponents(1);

  vtkDoubleArray *cellScalar_PlasmaCloudDensity = vtkDoubleArray::New();
  cellScalar_PlasmaCloudDensity->SetName("Plasma Cloud Density");
  cellScalar_PlasmaCloudDensity->SetNumberOfComponents(1);

  vtkDoubleArray *cellScalar_Temperature = vtkDoubleArray::New();
  cellScalar_Temperature->SetName("Temperature");
  cellScalar_Temperature->SetNumberOfComponents(1);

  vtkDoubleArray *cellVector_MagneticField = vtkDoubleArray::New();
  cellVector_MagneticField->SetName("Magnetic Field");
  cellVector_MagneticField->SetNumberOfComponents(3);

  vtkDoubleArray *cellVector_Velocity = vtkDoubleArray::New();
  cellVector_Velocity->SetName("Velocity");
  cellVector_Velocity->SetNumberOfComponents(3);

  for (int x = 0; x < (this->dimPhi*this->dimTheta*this->dimR); x++ )
    {
      cellScalar_Density->InsertNextValue(D[x]);
      cellScalar_PlasmaCloudDensity->InsertNextValue(DP[x]);
      cellScalar_Temperature->InsertNextValue(T[x]);

      //Magnetic Field
      xyz[0] = B1[x];
      xyz[1] = B2[x];
      xyz[2] = B3[x];
      cellVector_MagneticField->InsertNextTuple(xyz);

      //Velocity
      xyz[0] = V1[x];
      xyz[0] = V2[x];
      xyz[0] = V3[x];
      cellVector_Velocity->InsertNextTuple(xyz);

    }
  for(int x = 0; x < this->dimTheta*this->dimR; x++)
    {
      cellScalar_Density->InsertNextValue(D[x]);
      cellScalar_PlasmaCloudDensity->InsertNextValue(DP[x]);
      cellScalar_Temperature->InsertNextValue(T[x]);

      //Magnetic Field
      xyz[0] = B1[x];
      xyz[1] = B2[x];
      xyz[2] = B3[x];
      cellVector_MagneticField->InsertNextTuple(xyz);

      //Velocity
      xyz[0] = V1[x];
      xyz[0] = V2[x];
      xyz[0] = V3[x];
      cellVector_Velocity->InsertNextTuple(xyz);
    }

  output->GetPointData()->AddArray(cellScalar_Density);
  output->GetPointData()->AddArray(cellScalar_PlasmaCloudDensity);
  output->GetPointData()->AddArray(cellScalar_Temperature);

  output->GetPointData()->AddArray(cellVector_MagneticField);
  output->GetPointData()->AddArray(cellVector_Velocity);

  cellScalar_Density->Delete();
  cellScalar_PlasmaCloudDensity->Delete();
  cellScalar_Temperature->Delete();
  cellVector_MagneticField->Delete();
  cellVector_Velocity->Delete();

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


  if(X1){ delete[] X1; X1 = NULL;}
  if(X2){ delete[] X2; X2 = NULL;}
  if(X3){ delete[] X3; X3 = NULL;}
  if(B1){ delete[] B1; B1 = NULL;}
  if(B2){ delete[] B2; B2 = NULL;}
  if(B3){ delete[] B3; B3 = NULL;}
  if(V1){ delete[] V1; V1 = NULL;}
  if(V2){ delete[] V2; V2 = NULL;}
  if(V3){ delete[] V3; V3 = NULL;}
  if(BP){ delete[] BP; BP = NULL;}
  if(DP){ delete[] DP; DP = NULL;}
  if(D) { delete[] D;  D  = NULL;}
  if(T) { delete[] T;  T  = NULL;}


  cout << "END " << __FUNCTION__ << endl;


  return 1;
}

//----------------------------------------------------------------

const char * vtkENLILReader::GetCellArrayName(int index)
{
  const char* name;

  //name = this->CellArrayName[index].c_str();
  name = "Density";

  return name;
}

//----------------------------------------------------------------

const char * vtkENLILReader::GetPointArrayName(int index)
{
  const char* name;
  int nameSize;

  //name = this->PointArrayName[index].c_str();
  name = "Density";

  return name;
}


//----------------------------------------------------------------

//Cell Array Status Retrieval
int vtkENLILReader::GetCellArrayStatus(const char *CellArray)
{
  //return this->CellArrayStatus[string(CellArray)];
  return 1;
}

//----------------------------------------------------------------

int vtkENLILReader::GetPointArrayStatus(const char *PointArray)
{
  //return this->PointArrayStatus[string(PointArray)];
  return 1;
}

//----------------------------------------------------------------

//Cell Array Status Set
void vtkENLILReader::SetCellArrayStatus(const char* CellArray, int status)
{

//  this->CellArrayStatus[CellArray] = status;
  this->Modified();

}

//----------------------------------------------------------------

void vtkENLILReader::SetPointArrayStatus(const char* PointArray, int status)
{
//  this->PointArrayStatus[PointArray] = status;
  this->Modified();
}

//----------------------------------------------------------------


void vtkENLILReader::PrintSelf(ostream &os, vtkIndent indent) {
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkENLILReader says \"Hello, World!\" " << "\n";
}
