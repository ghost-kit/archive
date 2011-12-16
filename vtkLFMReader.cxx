#include "vtkLFMReader.h"

#include "Hdf4.h"

#include "vtkPointData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"

#include <vtkstd/map>
#include <vtkstd/string>
#include <vtksys/SystemTools.hxx>
#include <vtksys/RegularExpression.hxx>
#include <vtkstd/vector>

using namespace std;

vtkStandardNewMacro(vtkLFMReader);

vtkLFMReader::vtkLFMReader()
{
  this->HdfFileName = NULL;
  this->NumberOfTimeSteps = 1;
  // print vtkDebugMacro messages by turning debug mode on:
  //this->DebugOn();
}

vtkLFMReader::~vtkLFMReader()
{
  if ( this->HdfFileName ){
    delete [] this->HdfFileName;
    this->HdfFileName = NULL;
  }
}

int vtkLFMReader::CanReadFile(const char *filename)
{
  Hdf4 f;
  f.open(string(filename), IO::READ);
  
  map<string, double> metaDoubles;
  map<string, float> metaFloats;
  map<string, int> metaInts;
  map<string, string> metaStrings;
  f.readMetaData(metaDoubles, metaFloats, metaInts, metaStrings);
  
  f.close();
  
  // Make sure all the relevant metadata exists
  if ( //(metaDoubles.count(string("mjd")) == 0) ||
      (metaInts.count(string("time_step")) == 0) || 
      (metaFloats.count(string("time")) == 0) ||
      (metaFloats.count(string("tilt_angle")) == 0) ||
      //(metaStrings.count(string("I/O Revision")) == 0) ||
      //(metaStrings.count(string("Repository Revision")) == 0) ||
      (metaStrings.count(string("file_contents")) == 0) ||
      (metaStrings.count(string("dipole_moment")) == 0) ||
      (metaStrings.count(string("written_by")) == 0) ){
    
    return 0;
  }
  fprintf(stderr,"Time Step: %d\n", (int)metaInts.count(string("time_step")));
  fprintf(stderr,"Time: %f\n", (double)metaFloats["time"]);
  
  // If we've made it this far, assume it's a valid file.
  return 1;
}

int vtkLFMReader::RequestInformation (vtkInformation* request,
                                      vtkInformationVector** inputVector,
                                      vtkInformationVector* outputVector)
{ 
  // Read entire extents from Hdf4 file.  This requires reading an
  // entire variable.  Let's arbitrarily choose X_grid:
  Hdf4 f;
  f.open(string(this->GetFileName()), IO::READ);
  
  float *X_grid;
  int rank;
  int *dims;
  f.readVariable("X_grid", X_grid, rank, dims);
  delete [] X_grid;
  
  map<string, double> metaDoubles;
  map<string, float> metaFloats;
  map<string, int> metaInts;
  map<string, string> metaStrings;
  
  f.readMetaData(metaDoubles, metaFloats, metaInts, metaStrings);
  
  
  f.close();
  
  double timeRange[2];
  
  
  const int nip1 = dims[2];
  const int ni = nip1-1;
  const int nim1 = ni-1;
  const int njp1 = dims[1];
  const int njp2 = njp1+1;
  const int nj = njp1-1;
  const int nkp1 = dims[0];
  const int nkp2 = nkp1+1;
  const int nk = nkp1-1;
  
  int extent[6] = {0, nim1,
		   0, njp1,
		   0, nk};
  
  vtkDebugMacro(<< "Whole extents: "
                << extent[0] << ", " << extent[1] << ", "
                << extent[2] << ", " << extent[3] << ", "
                << extent[4] << ", " << extent[5]); 
  
  vtkInformation* outInfo = outputVector->GetInformationObject(0); 
  
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),extent,6);
  
  
  //BEGIN TIME SERIES  
  // Added by Joshua Murphy 1 DEC 2011
  
  this->NumberOfTimeSteps = metaInts.count(string("time_step"));
  this->TimeStepValues.assign(this->NumberOfTimeSteps, 0.0);
  
  // insert read of Time array here
  this->TimeStepValues[0] = metaDoubles["mjd"];
  
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
               &this->TimeStepValues[0],
               static_cast<int>(this->TimeStepValues.size()));
  
  timeRange[0] = this->TimeStepValues.front();
  timeRange[1] = this->TimeStepValues.back();
  
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);
  
  //END TIME SERIES
  
  return 1; 
}

/**
 * Note: We do nothing special with the periodic boundary on the grid
 * at z=0 and z=max! This discussion from the ParaView mailing list
 * suggests that there is littl we can do for now:
 * http://www.paraview.org/pipermail/paraview/2009-July/012750.html
 */
int vtkLFMReader::RequestData(vtkInformation* request,
                              vtkInformationVector** inputVector,
                              vtkInformationVector* outputVector)
{  
  vtkDebugMacro(<<"Reading LFM HDF file as a vtkStructuredGrid...");
  vtkDebugMacro(<< "GridScaleType is \"" << this->GetGridScaleType() << "\".");
  vtkDebugMacro(<< "GridScaleFactor is \"" << GRID_SCALE::ScaleFactor[this->GetGridScaleType()] << "\"");
  
  
  ///////////////////
  // Set sub extents
  Hdf4 f;
  f.open(string(this->GetFileName()), IO::READ);
  
  int rank;
  int *dims = NULL;
  
  float *X_grid = NULL;
  f.readVariable("X_grid", X_grid, rank, dims);   delete []dims;
  float *Y_grid = NULL;
  f.readVariable("Y_grid", Y_grid, rank, dims);   delete []dims;
  float *Z_grid = NULL;
  f.readVariable("Z_grid", Z_grid, rank, dims);   delete []dims;
  
  float *rho = NULL;
  f.readVariable("rho_",   rho,    rank, dims);   delete []dims;
  
  float *c = NULL;
  f.readVariable("c_",     c,      rank, dims);   delete []dims; 
  
  float *vx = NULL;
  f.readVariable("vx_",    vx,     rank, dims);   delete []dims;
  float *vy = NULL;
  f.readVariable("vy_",    vy,     rank, dims);   delete []dims;
  float *vz = NULL;
  f.readVariable("vz_",    vz,     rank, dims);   delete []dims;
  
  float *bx = NULL;
  f.readVariable("bx_",    bx,     rank, dims);   delete []dims;
  float *by = NULL;
  f.readVariable("by_",    by,     rank, dims);   delete []dims;
  float *bz = NULL;
  f.readVariable("bz_",    bz,     rank, dims);   
    
    
  //TODO: This will need to be a dynamic system, as to not look for data that isn't there
  //Disabled untile we can check existence
  //    delete []dims;
    
  //    float *avgbx = NULL;
  //    f.readVariable("avgBx",    avgbx,     rank, dims);   delete []dims;
  //    float *avgby = NULL;
  //    f.readVariable("avgBy",    avgby,     rank, dims);   delete []dims;
  //    float *avgbz = NULL;
  //    f.readVariable("avgBz",    avgbz,     rank, dims); 
  
  //FIXME:  Read bi,bj,bk on cell faces
  //FIXME:  Read ei,ej,ek on cell edges
  //FIXME:  Read avgEi, avgEj, avgEk on cell edges
  
  f.close();
  
  const int nip1 = dims[2];
  const int ni = nip1-1;
  const int nim1 = ni-1;
  const int njp1 = dims[1];
  const int njp2 = njp1+1;
  const int nj = njp1-1;
  const int nkp1 = dims[0];
  const int nkp2 = nkp1+1;
  const int nk = nkp1-1;
  
  int subext[6] = {0, nim1,
		   0, njp1,
		   0, nk};
  
  vtkDebugMacro(<< "sub extents: "
                << subext[0] << ", " << subext[1] << ", "
                << subext[2] << ", " << subext[3] << ", "
                << subext[4] << ", " << subext[5]); 
  
  
  //Start getting file information
  
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), subext);
  
  ///////////////////////////////////////////////////////////////////////////
  //read that part of the data in from the file and put it in the output data
  
  vtkStructuredGrid *output = 
    vtkStructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  //BEGIN UPDATE TIEM STEP
  //Return the Current Time to the Calling Application
  //Added by Joshua Murphy 1 DEC 2011
  //==================================================
  int myTime=0;
  
  int numRequestedTimeSteps; 
  double * requestedTimeValues;
  
  double firstTime; 
  double lastTime;
  
  
  //This imports time step data into the system.
  //TODO: Will Need to be adjusted for files with Multiple Time Steps
  if(outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS()))
    {
    
      numRequestedTimeSteps = outInfo->Length(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS());
      requestedTimeValues = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS());
    
      firstTime = requestedTimeValues[0];
      lastTime = requestedTimeValues[numRequestedTimeSteps-1]; 
    
      double myAnswerTime = this->TimeStepValues[0];
    
      output->GetInformation()->Set(vtkDataObject::DATA_TIME_STEPS(), &myAnswerTime, 1);
    
    }
  
  //END UPDATE TIME STEP

  
  // Fix x-axis caps (nj++ in nose, nj++ in tail)
  // close off grid (nk++)
  output->SetDimensions(ni, njp2, nkp1);
  
  //////////////////////
  // Point-centered data
  vtkPoints *points = vtkPoints::New();
  // Fix x-axis caps (nj++ in nose, nj++ in tail)
  // close off grid (nk++)
  points->SetNumberOfPoints(ni*njp2*nkp1);
  
  int offset;
  int oi;  // offset(i+1,j,  k)
  int oj;  // offset(i,  j+1,k)
  int ok;  // offset(i,  j,  k+1)
  int oij; // offset(i+1,j+1,k+1)
  int ojk; // offset(i,  j+1,k+1)
  int oik; // offset(i+1,j,  k+1)
  int oijk;// offset(i+1,j+1,k+1)
  float xyz[3];
  float r2[ni*nj*nk];
  
  for (int k=0; k < nk; k++){
    for (int j=0; j < nj; j++){
      for (int i=0; i < ni; i++){
	// Data is stored in Fortran column-major ordering
        offset = i + j*nip1 + k*nip1*njp1;
        oi  = i+1 +     j*nip1 +     k*nip1*njp1;
        oj  = i   + (j+1)*nip1 +     k*nip1*njp1;
        ok  = i   +     j*nip1 + (k+1)*nip1*njp1;
        oij = i+1 + (j+1)*nip1 +     k*nip1*njp1;
        ojk = i   + (j+1)*nip1 + (k+1)*nip1*njp1;
        oik = i+1 +     j*nip1 + (k+1)*nip1*njp1;
        oijk= i+1 + (j+1)*nip1 + (k+1)*nip1*njp1;
        
        xyz[0] = (X_grid[offset] + X_grid[oi] + X_grid[oj] + X_grid[ok] + 
                  X_grid[oij] + X_grid[ojk] + X_grid[oik] + X_grid[oijk]) / 8.0;
        xyz[0] /= GRID_SCALE::ScaleFactor[this->GetGridScaleType()];
        
        xyz[1] = (Y_grid[offset] + Y_grid[oi] + Y_grid[oj] + Y_grid[ok] + 
                  Y_grid[oij] + Y_grid[ojk] + Y_grid[oik] + Y_grid[oijk]) / 8.0;
        xyz[1] /= GRID_SCALE::ScaleFactor[this->GetGridScaleType()];
        
        xyz[2] = (Z_grid[offset] + Z_grid[oi] + Z_grid[oj] + Z_grid[ok] + 
                  Z_grid[oij] + Z_grid[ojk] + Z_grid[oik] + Z_grid[oijk]) / 8.0;
        xyz[2] /= GRID_SCALE::ScaleFactor[this->GetGridScaleType()];
        
        
	// j+1 because we set data along j=0 in "Fix x-axis singularity", below.
        offset = i + (j+1)*ni + k*ni*njp2;
        
        
        points->SetPoint(offset, xyz);
      }
    }
  }
  
  vtkDebugMacro(<< "NumberOfPoints after mesh read: " << points->GetNumberOfPoints());
  
  // Fix x-axis singularity at j=0 and j=nj+1
  int jAxis = 0;
  double axisCoord[3] = {0.0, 0.0, 0.0};
  xyz[1] = 0.0;
  xyz[2] = 0.0;
  for (int j=0; j < njp2; j+=njp1){
    jAxis = max(1, min(nj, j));
    for (int i=0; i < ni; i++){
      xyz[0] = 0.0;
      for (int k=0; k < nk; k++){	 
        points->GetPoint(i + jAxis*ni + k*ni*njp2, axisCoord);
        xyz[0] += (float) axisCoord[0];
      }
      xyz[0] /= float( nk );
      for (int k=0; k < nk; k++){
        points->SetPoint(i + j*ni + k*ni*njp2, xyz);
      }
    }
  }
  
  for (int j=0; j < njp2; j++){
    for (int i=0; i < ni; i++){
      // Close off the grid.
      points->SetPoint(i + j*ni +   nk*ni*njp2, points->GetPoint(i+j*ni) );
      
      // Set periodic boundary
      //points->SetPoint(i + j*ni + nkp1*ni*njp2, points->GetPoint(i + j*ni + 1*ni*njp2) );
    }
  }
  
  
  vtkDebugMacro(<< "NumberOfPoints after closing grid & setting periodic boundary: "
                << points->GetNumberOfPoints());
  
  output->SetPoints(points);
  points->Delete();
  
  /*****************************
   * Cell-centered scalar data *
   ****************************************************************************/
  vtkFloatArray *cellScalar_rho = vtkFloatArray::New();
  cellScalar_rho->SetName("Density");
  cellScalar_rho->SetNumberOfComponents(1);
  cellScalar_rho->SetNumberOfTuples(ni*njp2*nkp1);
  
  vtkFloatArray *cellScalar_c = vtkFloatArray::New();
  cellScalar_c->SetName("Sound Speed");
  cellScalar_c->SetNumberOfComponents(1);
  cellScalar_c->SetNumberOfTuples(ni*njp2*nkp1);
  
  /*****************************
   * Cell-centered Vector data *
   ****************************************************************************/
  vtkFloatArray *cellVector_v = vtkFloatArray::New();
  cellVector_v->SetName("Velocity");
  cellVector_v->SetNumberOfComponents(3);
  cellVector_v->SetNumberOfTuples(ni*njp2*nkp1);
  
  vtkFloatArray *cellVector_b = vtkFloatArray::New();
  cellVector_b->SetName("Magnetic Field");
  cellVector_b->SetNumberOfComponents(3);
  cellVector_b->SetNumberOfTuples(ni*njp2*nkp1);
  
  //TODO: average B values
  //Disabled until we can check existence
  //    vtkFloatArray *cellVector_avgb = vtkFloatArray::New();
  //    cellVector_avgb->SetName("Average Magnetic Field");
  //    cellVector_avgb->SetNumberOfComponents(3);
  //    cellVector_avgb->SetNumberOfTuples(ni*njp2*nkp1);
  
  
  
  
  // Store values in VTK objects:
  int offsetData, offsetCell;
  float tuple[3];
  for (int k=0; k < nk; k++){
    for (int j=0; j < nj; j++){
      for (int i=0; i < ni; i++){
        offsetData = i + j*nip1 + k*nip1*njp1;
        
	// j+1 because we set data along j=0 in "Fix x-axis singularity", below.
        offsetCell = i + (j+1)*ni   + k*ni*njp2;
        
        cellScalar_rho->SetTupleValue(offsetCell, &rho[offsetData]);
        cellScalar_c->SetTupleValue(offsetCell, &c[offsetData]);
        
        tuple[0] = vx[offsetData];
        tuple[1] = vy[offsetData];
        tuple[2] = vz[offsetData];
        cellVector_v->SetTupleValue(offsetCell, tuple);
        
        tuple[0] = bx[offsetData];
        tuple[1] = by[offsetData];
        tuple[2] = bz[offsetData];
        cellVector_b->SetTupleValue(offsetCell, tuple);
        
	//TODO: average B Values
	//Disabled until we can check existence
	//                tuple[0] = avgbx[offsetData];
	//                tuple[1] = avgby[offsetData];
	//                tuple[2] = avgbz[offsetData];
	//                cellVector_avgb->SetTupleValue(offsetCell, tuple);
        
        
        
      }
    }
  }
  
  // Fix x-axis singularity at j=0 and j=nj+1
  double tupleDbl[3];
  float rhoValue, cValue;
  float vValue[3], bValue[3], avgBvalue[3], avgBvalue_r2[3];
  for (int j=0; j < njp2; j+=njp1){
    jAxis = max(1, min(nj, j));
    for (int i=0; i < ni; i++){
      rhoValue = 0.0;
      cValue = 0.0;
      vValue[0] = 0.0;
      vValue[1] = 0.0;
      vValue[2] = 0.0;
      bValue[0] = 0.0;
      bValue[1] = 0.0;
      bValue[2] = 0.0;
      
      //TODO: average B values
      //disabled until we can check existence
      //            avgBvalue[0] = 0.0;
      //            avgBvalue[1] = 0.0;
      //            avgBvalue[2] = 0.0;
      
      
      for (int k=0; k < nk; k++){
        cellScalar_rho->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);
        rhoValue += (float) tupleDbl[0];
        
        cellScalar_c->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);
        cValue += (float) tupleDbl[0];
        
        cellVector_v->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);       
        vValue[0] += (float) tupleDbl[0];
        vValue[1] += (float) tupleDbl[1];
        vValue[2] += (float) tupleDbl[2];
        
        cellVector_b->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);       
        bValue[0] += (float) tupleDbl[0];
        bValue[1] += (float) tupleDbl[1];
        bValue[2] += (float) tupleDbl[2];
        
	//TODO: average B Values
	//Disabled until we can check existence
	//                cellVector_avgb->GetTuple(i+jAxis*ni + k*ni*njp2, tupleDbl);
	//                avgBvalue[0] += (float) tupleDbl[0];
	//                avgBvalue[1] += (float) tupleDbl[1];
	//                avgBvalue[2] += (float) tupleDbl[2];
        
        
      }
      
      rhoValue /= float(nk);
      
      cValue /= float(nk);
      
      vValue[0] /= float(nk);
      vValue[1] /= float(nk);
      vValue[2] /= float(nk);
      
      bValue[0] /= float(nk);
      bValue[1] /= float(nk);
      bValue[2] /= float(nk);
      
      
      //TODO: average B Values
      //Disabled until we can check existence
      //            avgBvalue[0] /= float(nk);
      //            avgBvalue[1] /= float(nk);
      //            avgBvalue[2] /= float(nk);
      
      
      
      for (int k=0; k < nk; k++){
        cellScalar_rho->SetTupleValue(i + j*ni + k*ni*njp2, &rhoValue);
        cellScalar_c->SetTupleValue(i + j*ni + k*ni*njp2, &cValue);
        cellVector_v->SetTupleValue(i + j*ni + k*ni*njp2, vValue);
        cellVector_b->SetTupleValue(i + j*ni + k*ni*njp2, bValue);
        
	//TODO: average B Values
	//until we can check if these values exist, they are disabled
	//                cellVector_avgb->SetTupleValue(i + j*ni + k*ni*njp2, avgBvalue);
        
        
      }
    }
  }
  
  for (int j=0; j < njp2; j++){
    for (int i=0; i < ni; i++){
      // Close off the grid
      cellScalar_rho->GetTuple(i + j*ni, tupleDbl);
      rhoValue = (float) tupleDbl[0];
      cellScalar_rho->SetTupleValue(i + j*ni +   nk*ni*njp2, &rhoValue);
      
      cellScalar_c->GetTuple(i + j*ni, tupleDbl);
      cValue = (float) tupleDbl[0];
      cellScalar_c->SetTupleValue(i + j*ni +   nk*ni*njp2, &cValue);
      
      cellVector_v->GetTuple(i + j*ni, tupleDbl);
      vValue[0] = (float) tupleDbl[0];
      vValue[1] = (float) tupleDbl[1];
      vValue[2] = (float) tupleDbl[2];
      cellVector_v->SetTupleValue(i + j*ni +   nk*ni*njp2, vValue);
      
      cellVector_b->GetTuple(i + j*ni, tupleDbl);
      bValue[0] = (float) tupleDbl[0];
      bValue[1] = (float) tupleDbl[1];
      bValue[2] = (float) tupleDbl[2];
      cellVector_b->SetTupleValue(i + j*ni +   nk*ni*njp2, bValue);
      
      //TODO: Average B Values
      //Until we can check to see if these values exist, they are disabled.
      //            cellVector_avgb->GetTuple(i + j*ni, tupleDbl);
      //            avgBvalue[0] = (float) tupleDbl[0];
      //            avgBvalue[1] = (float) tupleDbl[1];
      //            avgBvalue[2] = (float) tupleDbl[2];
      //            cellVector_avgb->SetTupleValue(i + j*ni + nk*ni*njp2, avgBvalue);
      //            
      
      // FIXME: Set periodic cells:
      //cellScalar_rho->GetTuple(i + j*ni + 1*ni*njp2, tupleDbl);
      //rhoValue = (float) tupleDbl[0];
      //cellScalar_rho->SetTupleValue(i + j*ni + nkp1*ni*njp2, &rhoValue);
      //
      //cellScalar_c->GetTuple(i + j*ni + 1*ni*njp2, tupleDbl);
      //cValue = (float) tupleDbl[0];
      //cellScalar_c->SetTupleValue(i + j*ni + nkp1*ni*njp2, &cValue);
      //
      //cellVector_v->GetTuple(i + j*ni + 1*ni*njp2, tupleDbl);
      //vValue[0] = (float) tupleDbl[0];
      //vValue[1] = (float) tupleDbl[1];
      //vValue[2] = (float) tupleDbl[2];
      //cellVector_v->SetTupleValue(i + j*ni + nkp1*ni*njp2, vValue);
      //
      //cellVector_b->GetTuple(i + j*ni + 1*ni*njp2, tupleDbl);
      //bValue[0] = (float) tupleDbl[0];
      //bValue[1] = (float) tupleDbl[1];
      //bValue[2] = (float) tupleDbl[2];
      //cellVector_b->SetTupleValue(i + j*ni + nkp1*ni*njp2, bValue);
    }
  }  
  
  output->GetPointData()->AddArray(cellScalar_rho);
  cellScalar_rho->Delete();
  
  output->GetPointData()->AddArray(cellScalar_c);
  cellScalar_c->Delete();
  
  output->GetPointData()->AddArray(cellVector_v);
  cellVector_v->Delete();
  
  output->GetPointData()->AddArray(cellVector_b);
  cellVector_b->Delete();
  
  //TODO: Average B Values
  //Disabled until can check existence
  //    output->GetPointData()->AddArray(cellVector_avgb);
  //    cellVector_avgb->Delete();
  
  if (dims){      delete [] dims;      dims = NULL;  }   
  if (X_grid){    delete [] X_grid;    X_grid = NULL;  }
  if (Y_grid){    delete [] Y_grid;    Y_grid = NULL;  }
  if (Z_grid){    delete [] Z_grid;    Z_grid = NULL;  }
  if (rho){       delete [] rho;       rho = NULL;  }
  if (c){         delete [] c;         c = NULL; }
  if (vx){        delete [] vx;        vx = NULL; }
  if (vy){        delete [] vy;        vy = NULL; }
  if (vz){        delete [] vz;        vz = NULL; }
  if (bx){        delete [] bx;        bx = NULL; }
  if (by){        delete [] by;        by = NULL; }
  if (bz){        delete [] bz;        bz = NULL; }
  
  //TODO: Average B Values
  //Disabled until can check existence
  //    if (avgbx){     delete [] avgbx;     avgbx = NULL; }
  //    if (avgby){     delete [] avgby;     avgby = NULL; }
  //    if (avgbz){     delete [] avgbz;     avgbz = NULL; }
  
  return 1;
}

void vtkLFMReader::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkLFMReader says \"Hello, World!\" " << "\n";
}
