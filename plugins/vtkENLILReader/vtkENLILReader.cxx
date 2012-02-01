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

#include <vtk_netcdfcpp.h>
#include <vtk_netcdf.h>



using namespace std;

vtkStandardNewMacro(vtkENLILReader);

vtkENLILReader::vtkENLILReader()
{
  this->EnlilFileName = NULL;
  // print vtkDebugMacro messages by turning debug mode on:
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
  map<string, double> metaDoubles;
  map<string, float> metaFloats;
  map<string, int> metaInts;
  map < string, string > metaStrings;

  //Open Enlil NetCDF file
  NcFile dataFile = NcFile(filename);

  NcVar* DataVars = NULL;

  int numVars = dataFile.num_vars();
  int numDims = dataFile.num_dims();
  int numAtts = dataFile.num_atts();

  // TODO: Make sure all the relevant meta data exists
  //    We need a valid test to confirm the files
  if (numVars != 16)
    {
      return 0;
    }



  return 1;
}


//Gets information on file.
//TODO: See Documentation to see best way of doing this.
int vtkENLILReader::RequestInformation(vtkInformation* request,
    vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkDebugMacro(<< __FILE__ << " " << __FUNCTION__ << " (L" <<  __LINE__ << "): "
		<<  "Hello world!" 
		<< endl);

  NcFile dataFile(this->EnlilFileName);

  if (!dataFile.is_valid())
    {
      return 0;
    }

  //get sample data
  NcVar *B1 = dataFile.get_var("B1");

  int dimR = B1->get_dim(3)->size();
  int dimTheta = B1->get_dim(2)->size();
  int dimPhi = B1->get_dim(1)->size();

  this->dimR = dimR;
  this->dimTheta = dimTheta;
  this->dimPhi = dimPhi;


  int extent[6] =
      { 0, (this->dimR - 1), 0, (this->dimTheta - 1), 0, (this->dimPhi) };

  if (!dimR || !dimTheta || !dimPhi)
    {
      vtkDebugMacro(<< "Extents Failure");
      return 0;
    }

  vtkDebugMacro(<< "Whole extents: "
      << extent[0] << ", " << extent[1] << ", "
      << extent[2] << ", " << extent[3] << ", "
      << extent[4] << ", " << extent[5]);

  vtkDebugMacro(<< "DIMS: " << dimR << ", " << dimTheta << ", " << dimPhi)

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6);




  return 1;
}

/**
 * Callback that gets the actual data
 * TODO: look to see if we are doing it correctly by reading everything.
 */
int vtkENLILReader::RequestData(vtkInformation* request,
    vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkDebugMacro(<<"Reading ENLIL NETCDF file as a vtkStructuredGrid...");
  vtkDebugMacro(<< "GridScaleType is \"" << this->GetGridScaleType() << "\".");
  vtkDebugMacro(<< "GridScaleFactor is \"" << GRID_SCALE::ScaleFactor[this->GetGridScaleType()] << "\"");

  ///////////////////
  // Set sub extents
  int ncStatus=0;
  int ncFileID=0;
  int ncSDSID=0;

  int ncDimID_r=0;
  int ncDimID_theta=0;
  int ncDimID_phi=0;

  int i=0;
  int j=0;
  int k=0;

  int nk=0;
  int nj=0;
  int ni=0;

  size_t dimR=0;
  size_t dimTheta=0;
  size_t dimPhi=0;



  double *B1=NULL;
  double *B2=NULL;
  double *B3=NULL;
  double *BP=NULL;
  double *D=NULL;
  double *DP=NULL;
  double *DT=NULL;
  double *T=NULL;
  double *NSTEP=NULL;
  double *TIME=NULL;
  double *V1=NULL;
  double *V2=NULL;
  double *V3=NULL;
  double *X1=NULL;
  double *X2=NULL;
  double *X3=NULL;

  //TODO: Convert to C++ libraries
  ncStatus = nc_open(this->EnlilFileName, NC_NOWRITE, &ncFileID);
  if(ncStatus != NC_NOERR)
    {
      vtkDebugMacro(<<"ERROR Opening File " << this->EnlilFileName);
      return 0;
    }

  X1 = (double*)calloc(this->dimR, sizeof(double));
  X2 = (double*)calloc(this->dimTheta, sizeof(double));
  X3 = (double*)calloc(this->dimPhi, sizeof(double));

  //Get Coordinate Array and Sizes
  //TODO: Separate these items into own functions

  ncStatus = nc_inq_varid(ncFileID, "X1", &ncSDSID);
  ncStatus = nc_get_var_double(ncFileID,ncSDSID, X1);

  ncStatus = nc_inq_varid(ncFileID, "X2", &ncSDSID);
  ncStatus = nc_get_var_double(ncFileID,ncSDSID, X2);

  ncStatus = nc_inq_varid(ncFileID, "X3", &ncSDSID);
  ncStatus = nc_get_var_double(ncFileID,ncSDSID, X3);

  nc_close(ncFileID);

  vtkDebugMacro(<< "NcFile Opened");

  vtkDebugMacro(<< "NcVar's Declared");

  vtkDebugMacro(<< "line 188");

  int subext[6] =
      { 0, (this->dimR - 1), 0, (this->dimTheta - 1), 0, (this->dimPhi) };

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

  output->SetDimensions(this->dimR, this->dimTheta, (this->dimPhi+1));

  vtkDebugMacro(<< "Line 203");

  //////////////////////
  // Point-centered data
  vtkPoints *points = vtkPoints::New();
  this->numberOfPoints = (this->dimR * this->dimTheta * (this->dimPhi+1));

  float xyz[3] =
      { 0, 0, 0 };
  int gridIndex = 0;
  int oldOffset = -1;
  int count = 0;

/*
 * GRID read section
 */


/*
 * Read in the grid, and construct it in the output file
 */
for (k = 0; k < this->dimPhi; k++)
    {
      for (j=0; j < this->dimTheta; j++)
        {
          for(i=0; i < this->dimR; i++)
            {
              xyz[0] = X1[i]*sin(X2[j])*cos(X3[k]) / GRID_SCALE::ScaleFactor[this->GetGridScaleType()];
              xyz[1] = X1[i]*sin(X2[j])*sin(X3[k]) / GRID_SCALE::ScaleFactor[this->GetGridScaleType()];
              xyz[2] = X1[i]*cos(X2[j]) / GRID_SCALE::ScaleFactor[this->GetGridScaleType()];

	      //xyz[0] = i;
	      //xyz[1] = j;
	      //xyz[2] = k;

              //insert point information into the grid
              points->InsertNextPoint(xyz);
            }
        }

    }
  //Close off the gap in the grid (make sphere continuous
  for (j=0; j < this->dimTheta; j++)
    {
      for(i=0; i < this->dimR; i++)
        {
          xyz[0] = X1[i]*sin(X2[j])*cos(X3[0]) / GRID_SCALE::ScaleFactor[this->GetGridScaleType()];
          xyz[1] = X1[i]*sin(X2[j])*sin(X3[0]) / GRID_SCALE::ScaleFactor[this->GetGridScaleType()];
          xyz[2] = X1[i]*cos(X2[j]) / GRID_SCALE::ScaleFactor[this->GetGridScaleType()];
	  //xyz[0] = i;
	  //xyz[1] = j;
	  //xyz[2] = 0;

          //Fix the Gap... insert the contiguous points
          points->InsertNextPoint(xyz);
        }
    }
  vtkDebugMacro(<< "NumberOfPoints after mesh read: " << points->GetNumberOfPoints());

  output->SetPoints(points);
  points->Delete();

  /*
   * Data Read Section
   */

  vtkDoubleArray *cellScalar_Density = vtkDoubleArray::New();
  cellScalar_Density->SetName("Density");
  cellScalar_Density->SetNumberOfComponents(1);
  //cellScalar_Density->SetNumberOfTuples(this->dimR*this->dimTheta*this->dimPhi);

  double scalar;
  for (int k = 0; k < this->dimR; k++)
    {
      for (int j = 0; j < this->dimTheta; j++)
        {
          for (int i = 0; i < this->dimPhi; i++)
            {
	      scalar = i + 100 * j + 10000*k;
	      cellScalar_Density->InsertNextValue(scalar);
	    }
	}
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



  /*
   * Read in the actual values
   */

  double tuple[3];
  for (int k = 0; k < nk; k++)
    {
      for (int j = 0; j < nj; j++)
        {
          for (int i = 0; i < ni; i++)
            {

//              DValue=D->values()->as_double(offsetData);
//              DPValue=DP->values()->as_double(offsetData);
//              TValue=T->values()->as_double(offsetData);
//              BPValue=BP->values()->as_double(offsetData);
//
//              cellScalar_Density->SetTupleValue(offsetCell, &DValue);
//              cellScalar_PlasmaDensity->SetTupleValue(offsetCell, &DPValue);
//              cellScalar_Temperature->SetTupleValue(offsetCell, &TValue);
//              cellScalar_Polarity->SetTupleValue(offsetCell, &BPValue);


            }
        }
    }


  /*
   * Set the read values to the appropriate outputs
   */

  //	output->GetPointData()->AddArray(cellScalar_Density);
  //	cellScalar_Density->Delete();
  //
  //	output->GetPointData()->AddArray(cellScalar_PlasmaDensity);
  //	cellScalar_PlasmaDensity->Delete();
  //
  //	output->GetPointData()->AddArray(cellScalar_Temperature);
  //	cellScalar_Temperature->Delete();
  //
  //	output->GetPointData()->AddArray(cellScalar_Polarity);
  //	cellScalar_Polarity->Delete();

  /*
   * Clean up memory a bit.
   */

  if(X1) free(X1);
  if(X2) free(X2);
  if(X3) free(X3);
  if(B1) free(B1);
  if(B2) free(B2);
  if(B3) free(B3);
  if(BP) free(BP);

#endif
  return 1;
}

void vtkENLILReader::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkENLILReader says \"Hello, World!\" " << "\n";
}
