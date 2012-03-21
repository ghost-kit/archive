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
#include "vtkENLILReaderMetaDataKeys.h"

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
  this->NumberOfCellArrays = 0;
  this->NumberOfPointArrays = 0;


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


  int ncFileID = 0;
  int status = 0;

  int ndims=0, nvars=0, ngatts=0, unlimdimid=0;

  CALL_NETCDF_NO_FEEDBACK(nc_open(filename, NC_NOWRITE, &ncFileID));
  CALL_NETCDF_NO_FEEDBACK(nc_inq(ncFileID, &ndims, &nvars, &ngatts, &unlimdimid));
  CALL_NETCDF_NO_FEEDBACK(nc_close(ncFileID));


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


  int ncFileID = 0;
  int ncSDSID = 0;
  int idp = 0;
  nc_type xtype;

  double TIME = 0;
  double timeRange[2];

  size_t dim_r = 0;
  size_t dim_theta = 0;
  size_t dim_phi = 0;

  char long_name[256];
  char temp[256];

   vtkInformation* outInfo = outputVector->GetInformationObject(0);


  CALL_NETCDF(nc_open(this->EnlilFileName, NC_NOWRITE, &ncFileID));


  //TEMP
  int ncid, ndims, nvars, ngatts, unlimdimid;
  CALL_NETCDF(nc_inq(ncFileID, &ndims, &nvars, &ngatts, &unlimdimid));

  CALL_NETCDF(nc_get_att_text(ncFileID, NC_GLOBAL, "refdate_cal", temp));

  vtkENLILMetaDataKeys::add_global_attributes(outInfo);
  vtkENLILMetaDataKeys::populate_meta_data(outInfo, vtkENLILMetaDataKeys::REFDATE_CAL(), temp);

  cout << "Reference Data: " << outInfo->Get(vtkENLILMetaDataKeys::REFDATE_CAL()) << endl;

  cout << "ndims: " << ndims << " nvars: " << nvars << " ngatts: " << ngatts
       << " unlimdimid: " << unlimdimid << endl;

  //END TEMP
  std::cout << "File Opened: " << this->EnlilFileName << std::endl;

  //get dimension data
  CALL_NETCDF(nc_inq_dimid(ncFileID, "n1", &ncSDSID));
  CALL_NETCDF(nc_inq_dimlen(ncFileID, ncSDSID, &dim_r));

  CALL_NETCDF(nc_inq_dimid(ncFileID, "n2", &ncSDSID));
  CALL_NETCDF(nc_inq_dimlen(ncFileID, ncSDSID, &dim_theta));

  CALL_NETCDF(nc_inq_dimid(ncFileID, "n3", &ncSDSID));
  CALL_NETCDF(nc_inq_dimlen(ncFileID, ncSDSID, &dim_phi));

  CALL_NETCDF(nc_inq_varid(ncFileID, "TIME", &ncSDSID));
  CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, &TIME));

  //Need to be a one-up lookup

  if(NumberOfCellArrays == 0)
    {
      this->clearString(long_name,256);
      CALL_NETCDF(nc_inq_varid(ncFileID, "D", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;

      this->SetArrayName("D", string(long_name));

      this->clearString(long_name,256);
      CALL_NETCDF(nc_inq_varid(ncFileID, "DP", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;

      this->SetArrayName("DP", string(long_name));

      this->clearString(long_name,256);
      CALL_NETCDF(nc_inq_varid(ncFileID, "T", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;

      this->SetArrayName("T", string(long_name));


      this->clearString(long_name,256);
      CALL_NETCDF(nc_inq_varid(ncFileID, "BP", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;

      this->SetArrayName("BP", string(long_name));


      this->clearString(long_name,256);
      CALL_NETCDF(nc_inq_varid(ncFileID, "B1", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;

      this->SetArrayName("B1", "B2", "B3", string(long_name));


      this->clearString(long_name,256);
      CALL_NETCDF(nc_inq_varid(ncFileID, "V1", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;

      this->SetArrayName("V1", "V2", "V3", string(long_name));
      this->SetArrayName("VR", "Radial Velocity");
    }

  //Need to get Long Names to add to arrays



  CALL_NETCDF(nc_close(ncFileID));

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
  CALL_NETCDF(nc_open(this->EnlilFileName, NC_NOWRITE, &ncFileID));

  int64_t arraySize = this->dimPhi*this->dimR*this->dimTheta;
  char long_name[256];

  X1 = new double[this->dimR];
  X2 = new double[this->dimTheta];
  X3 = new double[this->dimPhi];
  D = new double[arraySize];
  DP = new double[arraySize];
  BP = new double[arraySize];
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

  int readD = GetCellArrayStatus(GetDesc("D"));
  int readDP = GetCellArrayStatus(GetDesc("DP"));
  int readBP = GetCellArrayStatus(GetDesc("BP"));
  int readT = GetCellArrayStatus(GetDesc("T"));
  int readB = GetCellArrayStatus(GetDesc("B1"));
  int readV = GetCellArrayStatus(GetDesc("V1"));
  int readVR = GetCellArrayStatus(GetDesc("VR"));

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkStructuredGrid *output = vtkStructuredGrid::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkStructuredGrid *structOutput = vtkStructuredGrid::SafeDownCast(output);

  int *updateExtents = NULL;
  int updateNumberPieces = 0;
  int *extents = NULL;
  int parts = 0;

  updateExtents = structOutput->GetUpdateExtent();
  updateNumberPieces = structOutput->GetUpdateNumberOfPieces();
  parts = structOutput->GetUpdatePiece();


  std::cout << "Piece: " << parts << std::endl;



  //get extents
  std::cout << "Number of Parts: "
            << updateNumberPieces
            << "\nExtents Requested: "
            << updateExtents[0]
            << ":"
            << updateExtents[1]
            << ":"
            << updateExtents[2]
            << ":"
            << updateExtents[3]
            << ":"
            << updateExtents[4]
            << ":"
            << updateExtents[5]
            << std::endl;


  //Get Coordinate Array and Sizes
  //TODO: Separate these items into own functions

  CALL_NETCDF(nc_inq_varid(ncFileID, "X1", &ncSDSID));
  CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, X1));

  CALL_NETCDF(nc_inq_varid(ncFileID, "X2", &ncSDSID));
  CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, X2));

  CALL_NETCDF(nc_inq_varid(ncFileID, "X3", &ncSDSID));
  CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, X3));

  if(this->sphericalGridCoords.size() == 0)
    {
      //save the spherical coordinates (only once so we dont run out of memory quickley!)

      vtkstd::vector<double> R(X1, X1 + this->dimR);
      vtkstd::vector<double> T(X2, X2 + this->dimTheta);
      vtkstd::vector<double> P(X3, X3 + this->dimPhi);

      this->sphericalGridCoords.push_back(R);
      this->sphericalGridCoords.push_back(T);
      this->sphericalGridCoords.push_back(P);

    }

  if(readD)
    {
      clearString(long_name, 256);

      CALL_NETCDF(nc_inq_varid(ncFileID, "D", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;
      CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, D));
    }

  if(readDP)
    {
      clearString(long_name, 256);

      CALL_NETCDF(nc_inq_varid(ncFileID, "DP", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;
      CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, DP));
    }

  if(readBP)
    {
      clearString(long_name, 256);

      CALL_NETCDF(nc_inq_varid(ncFileID, "BP", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;
      CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, BP));
    }

  if(readT)
    {
      clearString(long_name, 256);

      CALL_NETCDF(nc_inq_varid(ncFileID, "T", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;
      CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, T));
    }

  if(readB)
    {
      clearString(long_name, 256);

      CALL_NETCDF(nc_inq_varid(ncFileID, "B1", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;
      CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, B1));
    }
  if(readB)
    {
      clearString(long_name, 256);

      CALL_NETCDF(nc_inq_varid(ncFileID, "B2", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;
      CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, B2));
    }


  if(readB)
    {
      clearString(long_name, 256);

      CALL_NETCDF(nc_inq_varid(ncFileID, "B3", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;
      CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, B3));
    }

  //either read the V vector or the V-R array
  if(readV || readVR)
    {
      clearString(long_name, 256);

      CALL_NETCDF(nc_inq_varid(ncFileID, "V1", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;
      CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, V1));
    }

  if(readV)
    {
      clearString(long_name, 256);

      CALL_NETCDF(nc_inq_varid(ncFileID, "V2", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;
      CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, V2));
    }

  if(readV)
    {
      clearString(long_name, 256);

      CALL_NETCDF(nc_inq_varid(ncFileID, "V3", &ncSDSID));
      CALL_NETCDF(nc_get_att_text(ncFileID, ncSDSID, "long_name", long_name));
      cout << "long name: " << long_name << endl;
      CALL_NETCDF(nc_get_var_double(ncFileID, ncSDSID, V3));
    }

  CALL_NETCDF(nc_close(ncFileID));


  int subext[6] = { 0, (this->dimR - 1), 0, (this->dimTheta - 1), 0,
                    (this->dimPhi) };

  vtkDebugMacro(<< "sub extents: "
                << subext[0] << ", " << subext[1] << ", "
                << subext[2] << ", " << subext[3] << ", "
                << subext[4] << ", " << subext[5]);

  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), subext);


  ///////////////////////////////////////////////////////////////////////////
  //read that part of the data in from the file and put it in the output data

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
              radiusValue = X1[i];

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
              / GRID_SCALE::ScaleFactor[GridScale];
          xyz[1] = X1[i] * sin(X2[j]) * sin(X3[0])
              / GRID_SCALE::ScaleFactor[GridScale];
          xyz[2] = X1[i] * cos(X2[j])
              / GRID_SCALE::ScaleFactor[GridScale];


          radiusValue = X1[i];
          //Fix the Gap... insert the contiguous points

          gridPoints->InsertNextPoint(xyz);
          Radius->InsertNextValue(radiusValue);

        }
    }

  output->GetPointData()->AddArray(Radius);
  output->SetPoints(gridPoints);
  gridPoints->Delete();
  Radius->Delete();

  vtkDoubleArray *cellScalar_Density = NULL;
  vtkDoubleArray *cellScalar_PlasmaCloudDensity = NULL;
  vtkDoubleArray *cellScalar_Temperature = NULL;
  vtkDoubleArray *cellScalar_MagneticPolarity = NULL;
  vtkDoubleArray *cellVector_MagneticField = NULL;
  vtkDoubleArray *cellVector_Velocity = NULL;

  //Experimenting with Radial Velocity
  vtkDoubleArray *cellVector_RadialVelocity = NULL;


  /*
     * Data Read Section
     */
  if(readD)
    {
      cellScalar_Density = vtkDoubleArray::New();
      cellScalar_Density->SetName("Density");
      cellScalar_Density->SetNumberOfComponents(1);
    }

  if(readDP)
    {
      cellScalar_PlasmaCloudDensity = vtkDoubleArray::New();
      cellScalar_PlasmaCloudDensity->SetName("Plasma Cloud Density");
      cellScalar_PlasmaCloudDensity->SetNumberOfComponents(1);
    }

  if(readBP)
    {
      cellScalar_MagneticPolarity = vtkDoubleArray::New();
      cellScalar_MagneticPolarity->SetName("Magnetic Polarity");
      cellScalar_MagneticPolarity->SetNumberOfComponents(1);
    }

  if(readT)
    {
      cellScalar_Temperature = vtkDoubleArray::New();
      cellScalar_Temperature->SetName("Temperature");
      cellScalar_Temperature->SetNumberOfComponents(1);
    }

  if(readB)
    {
      cellVector_MagneticField = vtkDoubleArray::New();
      cellVector_MagneticField->SetName("Magnetic Field");
      cellVector_MagneticField->SetNumberOfComponents(3);
    }

  if(readV)
    {
      cellVector_Velocity = vtkDoubleArray::New();
      cellVector_Velocity->SetName("Velocity");
      cellVector_Velocity->SetNumberOfComponents(3);
    }

  //Experimenting with Radial Velocity
  if(readVR)
    {
      cellVector_RadialVelocity = vtkDoubleArray::New();
      cellVector_RadialVelocity->SetName("Radial Velocity");
      cellVector_RadialVelocity->SetNumberOfComponents(3);
    }

  //load Scalar Grids
  for (int x = 0; x < (this->dimPhi*this->dimTheta*this->dimR); x++ )
    {

      if(readD)
        {
          cellScalar_Density->InsertNextValue(D[x]);
        }

      if(readDP)
        {
          cellScalar_PlasmaCloudDensity->InsertNextValue(DP[x]);
        }

      if(readBP)
        {
          cellScalar_MagneticPolarity->InsertNextValue(BP[x]);
        }

      if(readT)
        {
          cellScalar_Temperature->InsertNextValue(T[x]);
        }

    }


  //Load Vector Grids
  int loc=0;
  for(k=0; k<this->dimPhi; k++)
    {
      for(j=0; j<this->dimTheta; j++)
        {
          for(i=0; i<this->dimR; i++)

            {
              if(readVR)
                {

                  xyz[0] =V1[loc]*sin(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][k]);
                  xyz[1] =V1[loc]*sin(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][k]);
                  xyz[2] =V1[loc]*cos(this->sphericalGridCoords[1][j]);

                  cellVector_RadialVelocity->InsertNextTuple(xyz);

                }

              if(readB)
                {
                  //Magnetic Field

                  xyz[0] =B1[loc]*sin(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][k]);
                  xyz[1] =B1[loc]*sin(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][k]);
                  xyz[2] =B1[loc]*cos(this->sphericalGridCoords[1][j]);

                  xyz[0] += B2[loc]*cos(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][k]);
                  xyz[1] += B2[loc]*cos(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][k]);
                  xyz[2] += -1.0*B2[loc]*sin(this->sphericalGridCoords[1][j]);

                  xyz[0] += -1.0*B3[loc]*sin(this->sphericalGridCoords[2][k]);
                  xyz[1] += B3[loc]*cos(this->sphericalGridCoords[2][k]);

                  cellVector_MagneticField->InsertNextTuple(xyz);
                }

              if(readV)
                {
                  //Velocity
                  xyz[0] =V1[loc]*sin(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][k]);
                  xyz[1] =V1[loc]*sin(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][k]);
                  xyz[2] =V1[loc]*cos(this->sphericalGridCoords[1][j]);

                  xyz[0] += V2[loc]*cos(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][k]);
                  xyz[1] += V2[loc]*cos(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][k]);
                  xyz[2] += -1.0*V2[loc]*sin(this->sphericalGridCoords[1][j]);

                  xyz[0] += -1.0*V3[loc]*sin(this->sphericalGridCoords[2][k]);
                  xyz[1] += V3[loc]*cos(this->sphericalGridCoords[2][k]);

                  cellVector_Velocity->InsertNextTuple(xyz);

                }


              loc++;

            }
        }
    }




  //Close off Scalar grids
  for(int x = 0; x < this->dimTheta*this->dimR; x++)
    {
      if(readD)
        {
          cellScalar_Density->InsertNextValue(D[x]);
        }

      if(readDP)
        {
          cellScalar_PlasmaCloudDensity->InsertNextValue(DP[x]);
        }

      if(readBP)
        {
          cellScalar_MagneticPolarity->InsertNextValue(BP[x]);
        }

      if(readT)
        {
          cellScalar_Temperature->InsertNextValue(T[x]);
        }

    }


  //close off Vector Grids
  //TODO #341: copy out of already computed instead of re-calculating
  loc=0;
  for(j=0; j<this->dimTheta; j++)
    {
      for(i=0; i<this->dimR; i++)

        {
          if(readVR)
            {

              xyz[0] = V1[loc]*sin(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][0]);
              xyz[1] = V1[loc]*sin(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][0]);
              xyz[2] = V1[loc]*cos(this->sphericalGridCoords[1][j]);

              cellVector_RadialVelocity->InsertNextTuple(xyz);

            }

          if(readB)
            {
              //Magnetic Field

              xyz[0] =B1[loc]*sin(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][0]);
              xyz[1] =B1[loc]*sin(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][0]);
              xyz[2] =B1[loc]*cos(this->sphericalGridCoords[1][j]);

              xyz[0] += B2[loc]*cos(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][0]);
              xyz[1] += B2[loc]*cos(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][0]);
              xyz[2] += -1.0*B2[loc]*sin(this->sphericalGridCoords[1][j]);

              xyz[0] += -1.0*B3[loc]*sin(this->sphericalGridCoords[2][0]);
              xyz[1] += B3[loc]*cos(this->sphericalGridCoords[2][0]);

              cellVector_MagneticField->InsertNextTuple(xyz);
            }

          if(readV)
            {
              //Velocity
              xyz[0] =V1[loc]*sin(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][0]);
              xyz[1] =V1[loc]*sin(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][0]);
              xyz[2] =V1[loc]*cos(this->sphericalGridCoords[1][j]);

              xyz[0] += V2[loc]*cos(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][0]);
              xyz[1] += V2[loc]*cos(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][0]);
              xyz[2] += -1.0*V2[loc]*sin(this->sphericalGridCoords[1][j]);

              xyz[0] += -1.0*V3[loc]*sin(this->sphericalGridCoords[2][0]);
              xyz[1] += V3[loc]*cos(this->sphericalGridCoords[2][0]);

              cellVector_Velocity->InsertNextTuple(xyz);

            }

          loc++;

        }
    }



  // Commit grids to ParaView
  if(readD)
    {
      output->GetPointData()->AddArray(cellScalar_Density);
      cellScalar_Density->Delete();

    }

  if(readDP)
    {
      output->GetPointData()->AddArray(cellScalar_PlasmaCloudDensity);
      cellScalar_PlasmaCloudDensity->Delete();

    }

  if(readBP)
    {
      output->GetPointData()->AddArray(cellScalar_MagneticPolarity);
      cellScalar_MagneticPolarity->Delete();

    }

  if(readT)
    {
      output->GetPointData()->AddArray(cellScalar_Temperature);
      cellScalar_Temperature->Delete();

    }
  if(readB)
    {
      output->GetPointData()->AddArray(cellVector_MagneticField);
      cellVector_MagneticField->Delete();

    }
  if(readV)
    {
      output->GetPointData()->AddArray(cellVector_Velocity);
      cellVector_Velocity->Delete();

    }

  if(readVR)
    {
      output->GetPointData()->AddArray(cellVector_RadialVelocity);
      cellVector_RadialVelocity->Delete();
    }


  // Clean up memory
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
  //  cout << __FUNCTION__ << endl;

  const char* name;

  name = this->CellArrayName[index].c_str();

  return name;
}

//----------------------------------------------------------------

const char * vtkENLILReader::GetPointArrayName(int index)
{
  //  cout << __FUNCTION__ << endl;

  const char* name;

  name = this->PointArrayName[index].c_str();

  return name;
}


//----------------------------------------------------------------

//Cell Array Status Retrieval
int vtkENLILReader::GetCellArrayStatus(const char *CellArray)
{
  //  cout << __FUNCTION__ << endl;

  return this->CellArrayStatus[string(CellArray)];
}

//Cell Array Status Retrieval
int vtkENLILReader::GetCellArrayStatus(vtkstd::string CellArray)
{
  //  cout << __FUNCTION__ << endl;

  return this->CellArrayStatus[CellArray];
}


//----------------------------------------------------------------

int vtkENLILReader::GetPointArrayStatus(const char *PointArray)
{
  //  cout << __FUNCTION__ << endl;

  return this->PointArrayStatus[string(PointArray)];

}

int vtkENLILReader::GetPointArrayStatus(vtkstd::string PointArray)
{
  //  cout << __FUNCTION__ << endl;

  return this->PointArrayStatus[PointArray];
}


//----------------------------------------------------------------

//Cell Array Status Set
void vtkENLILReader::SetCellArrayStatus(const char* CellArray, int status)
{
  //  cout << __FUNCTION__ << endl;

  this->CellArrayStatus[CellArray] = status;
  this->Modified();

}

//----------------------------------------------------------------

void vtkENLILReader::SetPointArrayStatus(const char* PointArray, int status)
{
  //  cout << __FUNCTION__ << endl;

  this->PointArrayStatus[PointArray] = status;
  this->Modified();
}

//----------------------------------------------------------------
//This version of SetIfExists is for scalars
void vtkENLILReader::SetArrayName(vtkstd::string VarName, vtkstd::string VarDescription)
{
  //  cout << __FUNCTION__ << endl;


  //Set Variable->description map
  this->ArrayNameLookup[VarName] = VarDescription;

  //Set other Array Variables
  this->NumberOfCellArrays++;
  this->CellArrayName.push_back(VarDescription);
  this->CellArrayStatus[VarDescription] = 1;

  cout << VarName << ": " << VarDescription << endl;
}

//----------------------------------------------------------------
//This Version of SetIfExists is for Vectors (3D)
void vtkENLILReader::SetArrayName(vtkstd::string xVar, vtkstd::string yVar, vtkstd::string zVar, vtkstd::string VarDescription)
{

  //  cout << __FUNCTION__ << endl;

  //Set variable->desciption map
  this->ArrayNameLookup[xVar] = VarDescription;
  this->ArrayNameLookup[yVar] = VarDescription;
  this->ArrayNameLookup[zVar] = VarDescription;

  //Set other Array Variables
  this->NumberOfCellArrays++;
  this->CellArrayName.push_back(VarDescription);
  this->CellArrayStatus[VarDescription] = 1;


  cout << xVar << "," << yVar << "," << zVar << ": " << VarDescription << endl;
}

//--------------------------------------------------------------
//Convert coordinates from an 3-dimension grid (single array)
//  from spherical coordinates to Cartesian coordinates

void vtkENLILReader::convertSphericalCartesian(double *grid, const int dims[])
{
  //Convert spherical to cartesion


}

void vtkENLILReader::constructSphericalGrid(double *grid, const int dims[])
{


}



void vtkENLILReader::PrintSelf(ostream &os, vtkIndent indent) {
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkENLILReader says \"Hello, World!\" " << "\n";
}


