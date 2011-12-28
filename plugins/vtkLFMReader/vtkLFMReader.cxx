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

  //----------------------------------------------------------------


vtkStandardNewMacro(vtkLFMReader);


  //----------------------------------------------------------------


vtkLFMReader::vtkLFMReader()
{
  this->HdfFileName = NULL;
  this->NumberOfTimeSteps = 1;
  
  this->ReadDensityFields = 1;
  this->ReadElecFields = 0;
  this->ReadMagFields = 0;
  this->ReadVelFields = 0;
  this->ReadSoundFields = 0;
  this->ReadAVGMagFields = 0;
  this->ReadAVGElecFields = 0;
  
  this->NumberOfPointArrays = 0;
  this->NumberOfCellArrays = 0;
  
    // print vtkDebugMacro messages by turning debug mode on:
    //this->DebugOn();
}

  //----------------------------------------------------------------


vtkLFMReader::~vtkLFMReader()
{
  if ( this->HdfFileName ){
    delete [] this->HdfFileName;
    this->HdfFileName = NULL;
  }
}

  //----------------------------------------------------------------


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

  //----------------------------------------------------------------


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
  
  /**
   *  BEGIN CellArrayInfo
   *
   *  This section will check to see if possible variables exist, and if they do, set them
   *    up for readability.  This includes incrementing the number of arrays available (NumberOfCellArrays), 
   *    adding the array name to the Array Name list (CellArrayName), and setting an entry in the 
   *    Status Dictionary (CellArrayStatus)
   */
  
  if(NumberOfCellArrays == 0)
    {
    if(f.hasVariable("rho_"))
      {
      this->NumberOfCellArrays++;
      this->CellArrayName.push_back("Plasma Density");
      this->CellArrayStatus["Plasma Density"] = 0;
      
      cout << this->NumberOfCellArrays << ": " << this->CellArrayName[this->NumberOfCellArrays-1] << endl;
      
      }
    
    if(f.hasVariable("c_"))
      {
      this->NumberOfCellArrays++;
      this->CellArrayName.push_back("Sound Speed");
      this->CellArrayStatus["Sound Speed"] = 0;
      
      cout << this->NumberOfCellArrays << ": " << this->CellArrayName[this->NumberOfCellArrays-1] << endl;
      
      }
    
    if(f.hasVariable("vx_") && f.hasVariable("vy_") && f.hasVariable("vz_"))
      {
      this->NumberOfCellArrays++;
      this->CellArrayName.push_back("Velocity Vector");
      this->CellArrayStatus["Velocity Vector"] = 0;
      
      cout << this->NumberOfCellArrays << ": " << this->CellArrayName[this->NumberOfCellArrays-1] << endl;
      
      }
    
    if(f.hasVariable("bx_") && f.hasVariable("by_") && f.hasVariable("bz_"))
      {
      this->NumberOfCellArrays++;
      this->CellArrayName.push_back("Magnetic Field Vector");
      this->CellArrayStatus["Magnetic Field Vector"] = 0;
      
      cout << this->NumberOfCellArrays << ": " << this->CellArrayName[this->NumberOfCellArrays-1] << endl;
      
      }
    
    if(f.hasVariable("bi_") && f.hasVariable("bj_") && f.hasVariable("bk_"))
      {
      this->NumberOfCellArrays++;
      this->CellArrayName.push_back("B(ijk) Vector");
      this->CellArrayStatus["B(ijk) Vector"] = 0;
      
      cout << this->NumberOfCellArrays << ": " << this->CellArrayName[this->NumberOfCellArrays-1] << endl;
      
      }
    
    if(f.hasVariable("ei_") && f.hasVariable("ej_") && f.hasVariable("ek_"))
      {
      this->NumberOfCellArrays++;
      this->CellArrayName.push_back("E(ijk) Vector");
      this->CellArrayStatus["E(ijk) Vector"] = 0;
      
      cout << this->NumberOfCellArrays << ": " << this->CellArrayName[this->NumberOfCellArrays-1] << endl;
      
      }
    
    if(f.hasVariable("avgBx") && f.hasVariable("avgBy") && f.hasVariable("avgBz"))
      {
      this->NumberOfCellArrays++;
      this->CellArrayName.push_back("Averaged Magnetic Field Vector");
      this->CellArrayStatus["Averaged Magnetic Field Vector"] = 0;
      
      cout << this->NumberOfCellArrays << ": " << this->CellArrayName[this->NumberOfCellArrays-1] << endl;
      
      }
    
    if(f.hasVariable("avgEi") && f.hasVariable("avgEj") && f.hasVariable("avgEk"))
      {
      this->NumberOfCellArrays++;
      this->CellArrayName.push_back("avgE(ijk) Vector");
      this->CellArrayStatus["avgE(ijk) Vector"] = 0;
      
      cout << this->NumberOfCellArrays << ": " << this->CellArrayName[this->NumberOfCellArrays-1] << endl;
      
      }
    }
  
    //END CellArrayInfo
  
  
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
    // time is a float, mjd was a double.  Need to make sure we look at the correct
    //  meta data vector.
  this->TimeStepValues[0] = metaFloats["time"];
  
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
               &this->TimeStepValues[0],
               static_cast<int>(this->TimeStepValues.size()));
  
  timeRange[0] = this->TimeStepValues.front();
  timeRange[1] = this->TimeStepValues.back();
  
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);
  
    //END TIME SERIES
  
  
  
  
  return 1; 
}

  //----------------------------------------------------------------

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
    ///////////////////
  
    //TODO: Implement Extent Restricted Read
  Hdf4 f;
  f.open(string(this->GetFileName()), IO::READ);
  
  int rank;
  int *dims = NULL;
  
  float *X_grid = NULL;
  float *Y_grid = NULL;
  float *Z_grid = NULL;
  
  float *rho = NULL;
  float *c = NULL;
  
  float *vx = NULL;
  float *vy = NULL;
  float *vz = NULL;
  
  float *bx = NULL;
  float *by = NULL;
  float *bz = NULL;
  
  float *avgbz = NULL;
  float *avgby = NULL;
  float *avgbx = NULL;
  
  
  f.readVariable("X_grid", X_grid, rank, dims);   delete []dims;
  f.readVariable("Y_grid", Y_grid, rank, dims);   delete []dims;
  f.readVariable("Z_grid", Z_grid, rank, dims);   
  
  
    //Density Selective Read
  if(this->CellArrayStatus["Plasma Density"])
    {
    
    cout << "Plasma Desnity Selected" << endl;
      //if(dims) delete []dims;
    f.readVariable("rho_",   rho,    rank, dims);
    
    }
  
  
    //Sound Speed Selective Read
  if(this->CellArrayStatus["Sound Speed"])
    {
    cout << "Sound Speed Selected" << endl;
    if(dims) delete []dims;
    f.readVariable("c_",     c,      rank, dims);  
    
    }
  
    //Velocity Selective Read
  if(this->CellArrayStatus["Velocity Vector"])
    {
    cout << "Velocity Selected" << endl;
    
    if(dims) delete []dims;
    f.readVariable("vx_",    vx,     rank, dims);   delete []dims;
    f.readVariable("vy_",    vy,     rank, dims);   delete []dims;
    f.readVariable("vz_",    vz,     rank, dims);   
    }
  
  
    //Magnetic Field Selective Read
  if(this->CellArrayStatus["Magnetic Field Vector"])
    {
    cout << "Magnetic Field Vector Selected" << endl;
    
    if(dims) delete []dims;
    f.readVariable("bx_",    bx,     rank, dims);   delete []dims;
    f.readVariable("by_",    by,     rank, dims);   delete []dims;
    f.readVariable("bz_",    bz,     rank, dims);   
    
    }
  
    //Averaged Magnetic Field Selective Read
  if(this->CellArrayStatus["Averaged Magnetic Field Vector"])
    {
    cout << "Averaged Magnetic Field Vector Selected" << endl;
    
    if(dims) delete []dims;  
    f.readVariable("avgBx",    avgbx,     rank, dims);   delete []dims;
    f.readVariable("avgBy",    avgby,     rank, dims);   delete []dims;
    f.readVariable("avgBz",    avgbz,     rank, dims); 
    }
  
    //FIXME:  Read bi,bj,bk on cell faces
    //FIXME:  Read ei,ej,ek on cell edges
    //FIXME:  Read avgEi, avgEj, avgEk on cell edges
  
  f.close();
  
  
  /*------------------------------------------*/
  /*TODO: FIX SUB EXTENTS FOR PROPER READING  */
  /*VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV*/
  
  
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
  
  /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
  /*----------------------------------------------*/
  
  
  
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
  vtkFloatArray *cellScalar_rho = NULL;
  vtkFloatArray *cellScalar_c = NULL;
  
  
    //If we don't want to read the variables, DON'T allocate the space!
  if(rho != NULL)
    {
    cellScalar_rho = vtkFloatArray::New();
    cellScalar_rho->SetName("Density");
    cellScalar_rho->SetNumberOfComponents(1);
    cellScalar_rho->SetNumberOfTuples(ni*njp2*nkp1);
    }
  
  if(c != NULL)
    {
    cellScalar_c = vtkFloatArray::New();
    cellScalar_c->SetName("Sound Speed");
    cellScalar_c->SetNumberOfComponents(1);
    cellScalar_c->SetNumberOfTuples(ni*njp2*nkp1);
    }
  
  
  
  /*****************************
   * Cell-centered Vector data *
   ****************************************************************************/
  vtkFloatArray *cellVector_v = NULL;
  vtkFloatArray *cellVector_b = NULL;
  vtkFloatArray *cellVector_avgb = NULL;
  
  if(vx != NULL && vy != NULL && vz != NULL)
    {
    cellVector_v = vtkFloatArray::New();
    cellVector_v->SetName("Velocity");
    cellVector_v->SetNumberOfComponents(3);
    cellVector_v->SetNumberOfTuples(ni*njp2*nkp1);
    }
  
  if(bx != NULL && by != NULL && bz != NULL)
    {
    cellVector_b = vtkFloatArray::New();
    cellVector_b->SetName("Magnetic Field");
    cellVector_b->SetNumberOfComponents(3);
    cellVector_b->SetNumberOfTuples(ni*njp2*nkp1);
    }
  
  if(avgbx != NULL && avgby != NULL && avgbz != NULL)
    {    
    cellVector_avgb = vtkFloatArray::New();
    cellVector_avgb->SetName("Average Magnetic Field");
    cellVector_avgb->SetNumberOfComponents(3);
    cellVector_avgb->SetNumberOfTuples(ni*njp2*nkp1);
    }
  
  
    // Store values in VTK objects:
  int offsetData, offsetCell;
  float tuple[3];
  for (int k=0; k < nk; k++)
    {
    for (int j=0; j < nj; j++)
      {
      for (int i=0; i < ni; i++)
        {
        
        offsetData = i + j*nip1 + k*nip1*njp1;
        
          // j+1 because we set data along j=0 in "Fix x-axis singularity", below.
        offsetCell = i + (j+1)*ni   + k*ni*njp2;
        
        if(rho != NULL)
          cellScalar_rho->SetTupleValue(offsetCell, &rho[offsetData]);
        
        if(c != NULL)
          cellScalar_c->SetTupleValue(offsetCell, &c[offsetData]);
        
        if(vx != NULL && vy != NULL && vz != NULL)
          {
          tuple[0] = vx[offsetData];
          tuple[1] = vy[offsetData];
          tuple[2] = vz[offsetData];
          cellVector_v->SetTupleValue(offsetCell, tuple);
          }
        
        if(bx != NULL && by != NULL && bz != NULL)
          {
          tuple[0] = bx[offsetData];
          tuple[1] = by[offsetData];
          tuple[2] = bz[offsetData];
          cellVector_b->SetTupleValue(offsetCell, tuple);
          }
        
        if(avgbx != NULL && avgby != NULL && avgbz != NULL)
          {
          tuple[0] = avgbx[offsetData];
          tuple[1] = avgby[offsetData];
          tuple[2] = avgbz[offsetData];
          cellVector_avgb->SetTupleValue(offsetCell, tuple);
          }
        
        
        }
      }
    }
  
    // Fix x-axis singularity at j=0 and j=nj+1
  double tupleDbl[3];
  float rhoValue, cValue;
  float vValue[3], bValue[3], avgBvalue[3];
  
  for (int j=0; j < njp2; j+=njp1){
    jAxis = max(1, min(nj, j));
    for (int i=0; i < ni; i++)
      {
      rhoValue = 0.0;
      cValue = 0.0;
      vValue[0] = 0.0;
      vValue[1] = 0.0;
      vValue[2] = 0.0;
      bValue[0] = 0.0;
      bValue[1] = 0.0;
      bValue[2] = 0.0;
      avgBvalue[0] = 0.0;
      avgBvalue[1] = 0.0;
      avgBvalue[2] = 0.0;
      
      
      for (int k=0; k < nk; k++)
        {
        
        if(rho != NULL)
          {
          cellScalar_rho->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);
          rhoValue += (float) tupleDbl[0];
          }
        
        if(c != NULL)
          {
          cellScalar_c->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);
          cValue += (float) tupleDbl[0];
          }
        
        if(vx != NULL && vy != NULL && vz != NULL)
          {
          cellVector_v->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);       
          vValue[0] += (float) tupleDbl[0];
          vValue[1] += (float) tupleDbl[1];
          vValue[2] += (float) tupleDbl[2];
          }
        
        if(bx != NULL && by != NULL && bz != NULL)
          {
          cellVector_b->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);       
          bValue[0] += (float) tupleDbl[0];
          bValue[1] += (float) tupleDbl[1];
          bValue[2] += (float) tupleDbl[2];
          }
        
        if(avgbx != NULL && avgby != NULL && avgbz != NULL)
          {
          cellVector_avgb->GetTuple(i+jAxis*ni + k*ni*njp2, tupleDbl);
          avgBvalue[0] += (float) tupleDbl[0];
          avgBvalue[1] += (float) tupleDbl[1];
          avgBvalue[2] += (float) tupleDbl[2];
          }
        
        }
      
      if(rho != NULL)
        rhoValue /= float(nk);
      
      if(c != NULL)
        cValue /= float(nk);
      
      if(vx != NULL && vy != NULL && vz != NULL)
        {
        vValue[0] /= float(nk);
        vValue[1] /= float(nk);
        vValue[2] /= float(nk);
        }
      
      if(bx != NULL && by != NULL && bz != NULL)
        {
        bValue[0] /= float(nk);
        bValue[1] /= float(nk);
        bValue[2] /= float(nk);
        }
      
      if(avgbx != NULL && avgby != NULL && avgbz != NULL)
        {
        avgBvalue[0] /= float(nk);
        avgBvalue[1] /= float(nk);
        avgBvalue[2] /= float(nk);
        }
      
      
      
      for (int k=0; k < nk; k++)
        {
        if(rho != NULL)
          cellScalar_rho->SetTupleValue(i + j*ni + k*ni*njp2, &rhoValue);
        
        if(c != NULL)
          cellScalar_c->SetTupleValue(i + j*ni + k*ni*njp2, &cValue);
        
        if(vx != NULL && vy != NULL && vz != NULL)
          cellVector_v->SetTupleValue(i + j*ni + k*ni*njp2, vValue);
        
        if(bx != NULL && by != NULL && bz != NULL)
          cellVector_b->SetTupleValue(i + j*ni + k*ni*njp2, bValue);
        
        if(avgbx != NULL && avgby != NULL && avgbz != NULL)
          cellVector_avgb->SetTupleValue(i + j*ni + k*ni*njp2, avgBvalue);
        
        
        }
      }
  }
  
  for (int j=0; j < njp2; j++)
    {
    for (int i=0; i < ni; i++)
      {
        // Close off the grid
      
      if(rho != NULL)
        {
        cellScalar_rho->GetTuple(i + j*ni, tupleDbl);
        rhoValue = (float) tupleDbl[0];
        cellScalar_rho->SetTupleValue(i + j*ni +   nk*ni*njp2, &rhoValue);
        }
      
      if(c != NULL)
        {
        cellScalar_c->GetTuple(i + j*ni, tupleDbl);
        cValue = (float) tupleDbl[0];
        cellScalar_c->SetTupleValue(i + j*ni +   nk*ni*njp2, &cValue);
        }
      
      if(vx != NULL && vy != NULL && vz != NULL)
        {
        cellVector_v->GetTuple(i + j*ni, tupleDbl);
        vValue[0] = (float) tupleDbl[0];
        vValue[1] = (float) tupleDbl[1];
        vValue[2] = (float) tupleDbl[2];
        cellVector_v->SetTupleValue(i + j*ni +   nk*ni*njp2, vValue);
        }
      
      if(bx != NULL && by != NULL && bz != NULL)
        {
        cellVector_b->GetTuple(i + j*ni, tupleDbl);
        bValue[0] = (float) tupleDbl[0];
        bValue[1] = (float) tupleDbl[1];
        bValue[2] = (float) tupleDbl[2];
        cellVector_b->SetTupleValue(i + j*ni +   nk*ni*njp2, bValue);
        }
      
      if(avgbx != NULL && avgby != NULL && avgbz != NULL)
        {
        cellVector_avgb->GetTuple(i + j*ni, tupleDbl);
        avgBvalue[0] = (float) tupleDbl[0];
        avgBvalue[1] = (float) tupleDbl[1];
        avgBvalue[2] = (float) tupleDbl[2];
        cellVector_avgb->SetTupleValue(i + j*ni + nk*ni*njp2, avgBvalue);
        }            
      
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
  
  if(rho != NULL)
    {
    output->GetPointData()->AddArray(cellScalar_rho);
    cellScalar_rho->Delete();
    }
  
  if(c != NULL)
    {
    output->GetPointData()->AddArray(cellScalar_c);
    cellScalar_c->Delete();
    }
  
  if(vx != NULL && vy != NULL && vz != NULL)
    {
    output->GetPointData()->AddArray(cellVector_v);
    cellVector_v->Delete();
    }
  
  if(bx != NULL && by != NULL && bz != NULL)
    {
    output->GetPointData()->AddArray(cellVector_b);
    cellVector_b->Delete();
    }
  
  if(avgbx != NULL && avgby != NULL && avgbz != NULL)
    {
    output->GetPointData()->AddArray(cellVector_avgb);
    cellVector_avgb->Delete();
    }
  
  if (dims != NULL)   { delete [] dims;      dims = NULL;  }   
  if (X_grid != NULL) { delete [] X_grid;    X_grid = NULL;  }
  if (Y_grid != NULL) { delete [] Y_grid;    Y_grid = NULL;  }
  if (Z_grid != NULL) { delete [] Z_grid;    Z_grid = NULL;  }
  if (rho != NULL)    { delete [] rho;       rho = NULL;  }
  if (c != NULL)      { delete [] c;         c = NULL; }
  if (vx != NULL)     { delete [] vx;        vx = NULL; }
  if (vy != NULL)     { delete [] vy;        vy = NULL; }
  if (vz != NULL)     { delete [] vz;        vz = NULL; }
  if (bx != NULL)     { delete [] bx;        bx = NULL; }
  if (by != NULL)     { delete [] by;        by = NULL; }
  if (bz != NULL)     { delete [] bz;        bz = NULL; }
  if (avgbx){     delete [] avgbx;     avgbx = NULL; }
  if (avgby){     delete [] avgby;     avgby = NULL; }
  if (avgbz){     delete [] avgbz;     avgbz = NULL; }
  
  return 1;
}

  //----------------------------------------------------------------

const char * vtkLFMReader::GetCellArrayName(int index)
{
  const char* name;
  int nameSize;
  
  nameSize = this->CellArrayName[index].size();
  name = this->CellArrayName[index].c_str();
  
  
  return name;
}

  //----------------------------------------------------------------

  //Cell Array Status Retrieval
int vtkLFMReader::GetCellArrayStatus(const char *CellArray)
{
  return this->CellArrayStatus[string(CellArray)];
}

  //----------------------------------------------------------------

  //Cell Array Status Set
void vtkLFMReader::SetCellArrayStatus(const char* CellArray, int status)
{
  
  
  this->CellArrayStatus[CellArray] = status;
  
  
}

  //----------------------------------------------------------------

void vtkLFMReader::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkLFMReader says \"Hello, World!\" " << "\n";
}
