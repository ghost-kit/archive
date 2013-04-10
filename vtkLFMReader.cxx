#include "vtkLFMReader.h"

//#include "Io.hpp"
//#include "Hdf.hpp"

#include "DeprecatedHdf4.h"

#include "vtkPointData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"

#include <vtksys/SystemTools.hxx>
#include <vtksys/RegularExpression.hxx>

using namespace std;

//----------------------------------------------------------------

vtkStandardNewMacro(vtkLFMReader);


//----------------------------------------------------------------

vtkLFMReader::vtkLFMReader()
{
  this->HdfFileName = NULL;

  this->NumberOfTimeSteps = 1;
  
  this->NumberOfPointArrays = 0;
  this->NumberOfCellArrays = 0;
  
  // print vtkDebugMacro messages by turning debug mode on:
  // NOTE: This will make things VERY SLOW
  //this->DebugOn();
  this->DebugOff();
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
//  Io *io = new Hdf(0);
//  io->openRead(string(filename));

//  double mjd;
//  io->readAttribute(mjd, "mjd");
//  io->close();
//  delete io;
//  io = NULL;

  DeprecatedHdf4 f;
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
  DeprecatedHdf4 f;
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
  
  //Set Dimension Information in Class
  if(this->dims.size() == 0)
    {
      this->dims["x"] = dims[2];
      this->dims["y"] = dims[1];
      this->dims["z"] = dims[0];

      vtkDebugMacro(<< "Dimensions: "
		    << this->dims["x"] << ", "
		    << this->dims["y"] << ", "
		    << this->dims["z"]);
    }
  //local dims no longer needed
  delete [] dims;
    
  //BEGIN CellArrayInfo
  
  /**
   *  This section will check to see if possible variables exist, and if they do, set them
   *    up for readability.  This includes incrementing the number of arrays available (NumberOfCellArrays), 
   *    adding the array name to the Array Name list (CellArrayName), and setting an entry in the 
   *    Status Dictionary (CellArrayStatus)
   */
  
  //TODO #297: Also, lets do a little caching, and keep around the arrays that we are using 
  //      (if it doesn't use too much memory)  I would say keep the most recent reads in the 
  //      object.  This way, if we time-step over the objects, we don't have to read everything from 
  //      disk every time.
  
  if(NumberOfCellArrays == 0){
    //Set the Variables needed to selectively set Arrays (Scalar)
    SetIfExists(f, "rho_", "Plasma Density");
    SetIfExists(f, "c_", "Sound Speed");
    
    //Set the Variables needed to selectively set Arrays (Vector)
    SetIfExists(f, "vx_", "vy_", "vz_", "Velocity Vector");
    SetIfExists(f, "bx_", "by_", "bz_", "Magnetic Field Vector");
    SetIfExists(f, "avgBx", "avgBy", "avgBz", "Magnetic Field Vector (avg)");
    
    //Set the Variables needed to selectively set Arrays (Derived)
    SetIfExists(f, "ei_", "ej_", "ek_", "Electric Field Vector");
    //    SetNewIfExists(f, "ei_", "ej_", "ek_", "eVolume", "Electric Field Volume");
    
    SetIfExists(f, "avgEi", "avgEj", "avgEk", "Electric Field Vector (avg)");
    //    SetNewIfExists(f, "avgEi", "avgEj", "avgEk", "eAvgVolume", "Electric Field Volume (avg)");
    
    // placeholder for calculating the Current vector.  See Pjcalc2.F from CISM_DX reader.
    //SetIfExists(f, "bi_", "bj_", "bk_", "Current Vector");    
  }
  
  f.close();
  
  //Navigation helpers
  const int nip1 = NI+1;
  const int ni = nip1-1;
  const int nim1 = ni-1;
  const int njp1 = NJ+1;
  const int njp2 = njp1+1;
  const int nj = njp1-1;
  const int nkp1 = NK+1;
  const int nkp2 = nkp1+1;
  const int nk = nkp1-1;
  
  int extent[6] = {0, nim1,
		   0, njp1,
		   0, nk};
  
  vtkDebugMacro(<< "Whole extents: "
                << extent[0] << ", " << extent[1] << ", "
                << extent[2] << ", " << extent[3] << ", "
                << extent[4] << ", " << extent[5]); 
  
  //Set extents of grid
  vtkInformation* outInfo = outputVector->GetInformationObject(0); 
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),extent,6);
  

  //Set Time step Information
  this->NumberOfTimeSteps = 1; // 1 step per file
  this->TimeStepValues.assign(this->NumberOfTimeSteps, 0.0);
  if (metaDoubles.count(string("mjd")) != 0){
    this->TimeStepValues[0] = metaDoubles["mjd"];
  }
  else{
    // Slava Merkin's LFM-Helio doesn't have the "mjd" parameter, but it does have "time":
    this->TimeStepValues[0] = metaFloats["time"];
  }
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
	       &this->TimeStepValues[0],
	       static_cast<int>(this->TimeStepValues.size()));
  
  double timeRange[2];
  //Set Time Range for file
  timeRange[0] = this->TimeStepValues.front();
  timeRange[1] = this->TimeStepValues.back();
  
  //Update Pipeline
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);
  
  vtkDebugMacro(<< "number of timesteps in file=" << this->NumberOfTimeSteps);
  vtkDebugMacro(<< "Modified julian date in file=" << this->TimeStepValues[0] << endl
                << "TimeStepValues=" << this->TimeStepValues[0] << " " << this->TimeStepValues[1] << endl
                << "timeRange[0]=" << timeRange[0] <<" timeRange[1]=" << timeRange[1]);
  
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
  vtkDebugMacro(<< "Reading LFM HDF file as a vtkStructuredGrid...");
  vtkDebugMacro(<< "GridScaleType is \"" << this->GetGridScaleType() << "\".");
  vtkDebugMacro(<< "GridScaleFactor is \"" << GRID_SCALE::ScaleFactor[this->GetGridScaleType()] << "\"");  
  
  ///////////////////
  // Set sub extents
  ///////////////////
  
  //TODO: Implement Extent Restricted Read
  DeprecatedHdf4 f;
  f.open(string(this->GetFileName()), IO::READ);
  
  int rank;
  int *dims = NULL;
  
  //grid points
  float *X_grid = NULL;
  float *Y_grid = NULL;
  float *Z_grid = NULL;
  
  //scalar grids
  float *rho = NULL;
  float *c = NULL;
  
  //Vector Grids
  float *vx = NULL;
  float *vy = NULL;
  float *vz = NULL;
  
  float *bx = NULL;
  float *by = NULL;
  float *bz = NULL;
  
  float *ei = NULL;
  float *ej = NULL;
  float *ek = NULL;
  
  float *bi = NULL;
  float *bj = NULL;
  float *bk = NULL;
  
  float *avgbz = NULL;
  float *avgby = NULL;
  float *avgbx = NULL;
  
  float *avgei = NULL;
  float *avgej = NULL;
  float *avgek = NULL;
  
  f.readVariable("X_grid", X_grid, rank, dims);   delete []dims;
  f.readVariable("Y_grid", Y_grid, rank, dims);   delete []dims;
  f.readVariable("Z_grid", Z_grid, rank, dims);   delete []dims;
  
  //Density Selective Read
  if(this->CellArrayStatus[GetDesc("rho_")]){
    vtkDebugMacro(<<"Plasma Desnity Selected");
    f.readVariable("rho_",   rho,    rank, dims);  delete []dims;   
  }
  
  //Sound Speed Selective Read
  if(this->CellArrayStatus[GetDesc("c_")]){
    vtkDebugMacro(<< "Sound Speed Selected");
    f.readVariable("c_",     c,      rank, dims);  delete []dims;    
  }
  
  //Velocity Selective Read
  if(this->CellArrayStatus[GetDesc("vx_")]){
    vtkDebugMacro(<< "Velocity Selected");    
    f.readVariable("vx_",    vx,     rank, dims);   delete []dims;
    f.readVariable("vy_",    vy,     rank, dims);   delete []dims;
    f.readVariable("vz_",    vz,     rank, dims);   delete []dims;
  }
  
  //Magnetic Field Selective Read
  if(this->CellArrayStatus[GetDesc("bx_")]){
    vtkDebugMacro(<< "Magnetic Field Vector Selected");    
    f.readVariable("bx_",    bx,     rank, dims);   delete []dims;
    f.readVariable("by_",    by,     rank, dims);   delete []dims;
    f.readVariable("bz_",    bz,     rank, dims);   delete []dims;
  }
  
  //Electric Field Selective Read
  if(this->CellArrayStatus[GetDesc("ei_")]){
    vtkDebugMacro(<< "Electric Field vector Selected");
    f.readVariable("ei_",   ei,   rank, dims);    delete []dims;
    f.readVariable("ej_",   ej,   rank, dims);    delete []dims;
    f.readVariable("ek_",   ek,   rank, dims);    delete []dims;    
  }
  
  //Bijk
  if(this->CellArrayStatus[GetDesc("bi_")]){
    vtkDebugMacro(<<"Magnetic flux through the face vector selected");
    f.readVariable("bi_",   bi,   rank, dims);    delete []dims;
    f.readVariable("bj_",   bj,   rank, dims);    delete []dims;
    f.readVariable("bk_",   bk,   rank, dims);    delete []dims;    
  }
  
  //Averaged Magnetic Field Selective Read
  if(this->CellArrayStatus[GetDesc("avgBx")]){
    vtkDebugMacro(<< "Averaged Magnetic Field Vector Selected");    
    f.readVariable("avgBx",    avgbx,     rank, dims);   delete []dims;
    f.readVariable("avgBy",    avgby,     rank, dims);   delete []dims;
    f.readVariable("avgBz",    avgbz,     rank, dims);   delete []dims;
  }
  
  //Averaged E(ijk) Fields
  if(this->CellArrayStatus[GetDesc("avgEi")]){
    vtkDebugMacro(<< "Averaged Electric Field Vector Selected");
    
    f.readVariable("avgEi",   avgei,      rank, dims);  delete []dims;
    f.readVariable("avgEj",   avgej,      rank, dims);  delete []dims;
    f.readVariable("avgEk",   avgek,      rank, dims);  delete []dims;
  }
  
  dims = NULL;
  f.close();
  
  
  /*------------------------------------------*/
  /*TODO: FIX SUB EXTENTS FOR PROPER READING  */
  /*VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV*/
  
  
  const int nip1 = NI+1;
  const int ni = nip1-1;
  const int nim1 = ni-1;
  const int njp1 = NJ+1;
  const int njp2 = njp1+1;
  const int nj = njp1-1;
  const int nkp1 = NK+1;
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
  
  // Tell ParaView what the requested time is. Without this, the GUI thinks a single file loaded in displays has a time of "0.0".
  if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP())){
    double requestedTime = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
    // Hack to get time displaying correctly for single time step visualization in ParaView-3.98.1:
    if (fabs(requestedTime) <= 1e-6){
      // Set the current time to the start of the time range.
      double *timeRange=outInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_RANGE());
      requestedTime = timeRange[0];
    }
    output->GetInformation()->Set(vtkDataObject::DATA_TIME_STEP(), requestedTime);
  }

  // Fix x-axis caps (nj++ in nose, nj++ in tail)
  // close off grid (nk++)
  output->SetDimensions(ni, njp2, nkp1);
  
  //////////////////////
  // Point-centered data
  vtkPoints *points = vtkPoints::New();
  // Fix x-axis caps (nj++ in nose, nj++ in tail)
  // close off grid (nk++)
  points->SetNumberOfPoints(ni*njp2*nkp1);
  
  float xyz[3]={0.0, 0.0, 0.0};
  
  int offset = 0;
  int oi = 0, oij = 0, oik = 0;
  int oj = 0, ojk = 0, oijk = 0;
  int ok = 0;
  
  
  for (int k=0; k < nk; k++){
    for (int j=0; j < nj; j++){
      for (int i=0; i < ni; i++){
        
	//For speed efficiency, we calculate offsets once per loop.
	//
	//This Macro sets the values of offset, oi, oj, ok, oij, oik, ojk, oijk
	//  for when you need access to the points at the corners of the cell.
	//  The Above Mentioned variables MUST exist before calling this macro.
        setFortranCellGridPointOffsetMacro;
        
        xyz[0] = cell8PointAverage(X_grid, offset, oi, oj, ok, oij, ojk, oik, oijk);
        xyz[0] /= GRID_SCALE::ScaleFactor[GridScaleType];
        
        xyz[1] = cell8PointAverage(Y_grid, offset, oi, oj, ok, oij, ojk, oik, oijk);
        xyz[1] /= GRID_SCALE::ScaleFactor[GridScaleType];
        
        xyz[2] = cell8PointAverage(Z_grid, offset, oi, oj, ok, oij, ojk, oik, oijk);
        xyz[2] /= GRID_SCALE::ScaleFactor[GridScaleType];
        
        
	// j+1 because we set data along j=0 in "Fix x-axis singularity", below.
	//        offset = i + (j+1)*ni + k*ni*njp2;
        points->SetPoint(ArrayOffset(i, j+1, k), xyz);
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
	points->GetPoint(ArrayOffset(i, jAxis, k), axisCoord);
	xyz[0] += (float) axisCoord[0];
      }
      xyz[0] /= float( nk );
      for (int k=0; k < nk; k++){
        points->SetPoint(ArrayOffset(i,j,k), xyz);
      }
    }
  }
  
  for (int j=0; j < njp2; j++){
    for (int i=0; i < ni; i++){
      // Close off the grid.
      points->SetPoint(i+ j*ni + nk*ni*njp2, points->GetPoint(i+j*ni));
      
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
  //  vtkFloatArray *cellScalar_volume = NULL;
  
  
  
  //If we don't want to read the variables, DON'T allocate the space!
  
  //Read Desnity
  if(rho != NULL){
    cellScalar_rho = vtkFloatArray::New();
    cellScalar_rho->SetName(GetDesc("rho_").c_str());
    cellScalar_rho->SetNumberOfComponents(1);
    cellScalar_rho->SetNumberOfTuples(ni*njp2*nkp1);
  }
  
  //Read Sound Speed
  if(c != NULL){
    cellScalar_c = vtkFloatArray::New();
    cellScalar_c->SetName(GetDesc("c_").c_str());
    cellScalar_c->SetNumberOfComponents(1);
    cellScalar_c->SetNumberOfTuples(ni*njp2*nkp1);
  }
  
  /*****************************
   * Cell-centered Vector data *
   ****************************************************************************/
  vtkFloatArray *cellVector_v = NULL;
  vtkFloatArray *cellVector_b = NULL;
  vtkFloatArray *cellVector_e = NULL;
  vtkFloatArray *cellVector_be = NULL;
  vtkFloatArray *cellVector_avgb = NULL;
  vtkFloatArray *cellVector_avge = NULL;
  
  
  //Read Velocity
  if(vx != NULL && vy != NULL && vz != NULL){
    cellVector_v = vtkFloatArray::New();
    cellVector_v->SetName(GetDesc("vx_").c_str());
    cellVector_v->SetNumberOfComponents(3);
    cellVector_v->SetNumberOfTuples(ni*njp2*nkp1);
  }
  
  //Read Magnetic Field
  if(bx != NULL && by != NULL && bz != NULL){
    cellVector_b = vtkFloatArray::New();
    cellVector_b->SetName(GetDesc("bx_").c_str());
    cellVector_b->SetNumberOfComponents(3);
    cellVector_b->SetNumberOfTuples(ni*njp2*nkp1);
  }
  
  //Read Bijk Magnetic Field
  if(bi != NULL && bj != NULL && bk != NULL){
    cellVector_be = vtkFloatArray::New();
    cellVector_be->SetName(GetDesc("bi_").c_str());
    cellVector_be->SetNumberOfComponents(3);
    cellVector_be->SetNumberOfTuples(ni*njp2*nkp1);
  }
  
  //Read Electric Field
  if(ei != NULL && ej != NULL && ek != NULL){
    cellVector_e = vtkFloatArray::New();
    cellVector_e->SetName(GetDesc("ei_").c_str());
    cellVector_e->SetNumberOfComponents(3);
    cellVector_e->SetNumberOfTuples(ni*njp2*nkp1);
    
    //    cellScalar_volume = vtkFloatArray::New();
    //    cellScalar_volume->SetName("Cell Volume");
    //    cellScalar_volume->SetNumberOfComponents(1);
    //    cellScalar_volume->SetNumberOfTuples(ni*njp2*nkp1);        
  }
  
  
  //Reading Averaged Magnetic Field
  if(avgbx != NULL && avgby != NULL && avgbz != NULL){
    cellVector_avgb = vtkFloatArray::New();
    cellVector_avgb->SetName(GetDesc("avgBx").c_str());
    cellVector_avgb->SetNumberOfComponents(3);
    cellVector_avgb->SetNumberOfTuples(ni*njp2*nkp1);
  }
  
  //Reading Averaged Electric Field
  if(avgei != NULL && avgej != NULL && avgek != NULL){
    cellVector_avge = vtkFloatArray::New();
    cellVector_avge->SetName(GetDesc("avgEi").c_str());
    cellVector_avge->SetNumberOfComponents(3);
    cellVector_avge->SetNumberOfTuples(ni*njp2*nkp1);
    
    //    if(!cellScalar_volume)
    //      {
    //      cellScalar_volume = vtkFloatArray::New();
    //      cellScalar_volume->SetName("Electric Field Volume (avg)");
    //      cellScalar_volume->SetNumberOfComponents(1);
    //      cellScalar_volume->SetNumberOfTuples(ni*njp2*nkp1);
    //      }
  }
  
  
  // Store values in VTK objects:
  
  float cx[3]={0,0,0}, cy[3]={0,0,0}, cz[3]={0,0,0}, et[3]={0,0,0};
  float x_ijk[3]={0,0,0}, y_ijk[3]={0,0,0}, z_ijk[3]={0,0,0}, e_ijk[3]={0,0,0};
  float det=0;
  int cSet=0;
  
  int offsetData, offsetCell;
  float tuple[3];
  
  for (int k=0; k < nk; k++){
    for (int j=0; j < nj; j++){
      for (int i=0; i < ni; i++){
        
        offsetData = i + j*nip1 + k*nip1*njp1;
        
	// j+1 because we set data along j=0 in "Fix x-axis singularity", below.
        offsetCell = i + (j+1)*ni   + k*ni*njp2;
        
	//--
        
	//Store Density Data
        if(rho != NULL)
          cellScalar_rho->SetTupleValue(offsetCell, &rho[offsetData]);
        
	//--
        
	//Store sound speed data
        if(c != NULL)
          cellScalar_c->SetTupleValue(offsetCell, &c[offsetData]);
        
	//--
        
	//Store Velocity Data
        if(vx != NULL && vy != NULL && vz != NULL){
          tuple[0] = vx[offsetData];
          tuple[1] = vy[offsetData];
          tuple[2] = vz[offsetData];
          cellVector_v->SetTupleValue(offsetCell, tuple);
	}
        
	//--
        
	//Store Magnetic Field data
        if(bx != NULL && by != NULL && bz != NULL){
          tuple[0] = bx[offsetData];
          tuple[1] = by[offsetData];
          tuple[2] = bz[offsetData];
          cellVector_b->SetTupleValue(offsetCell, tuple);
	}
        
	//--
        
	//Store Electric Field Data
	//TODO: Implement Derived Quantities
        if(ei != NULL && ej != NULL && ek != NULL){          
          setFortranCellGridPointOffsetMacro;
          
	  // Get cell center positions
	  // FIXME: query cell center positions from existing
	  // vtkStructuredGrid::GetPoints(...) array.
          // output->GetPoints()->GetPoint(offsetCell);
          cx[0] = cell_AxisAverage(X_grid, oi, oij, oik, oijk, offset, oj, ok, ojk);
          cx[1] = cell_AxisAverage(X_grid, oj, oij, ojk, oijk, offset, oi, ok, oik);
          cx[2] = cell_AxisAverage(X_grid, ok, oik, ojk, oijk, offset, oj, oi, oij);
          
          cy[0] = cell_AxisAverage(Y_grid, oi, oij, oik, oijk, offset, oj, ok, ojk);
          cy[1] = cell_AxisAverage(Y_grid, oj, oij, ojk, oijk, offset, oi, ok, oik);
          cy[2] = cell_AxisAverage(Y_grid, ok, oik, ojk, oijk, offset, oj, oi, oij);
          
          cz[0] = cell_AxisAverage(Z_grid, oi, oij, oik, oijk, offset, oj, ok, ojk);
          cz[1] = cell_AxisAverage(Z_grid, oj, oij, ojk, oijk, offset, oi, ok, oik);
          cz[2] = cell_AxisAverage(Z_grid, ok, oik, ojk, oijk, offset, oj, oi, oij);
          
          
	  // Calculate Cell Volume. Use parallelpiped vector identity:
	  // Volume of parallelpiped determined by vectors A,B,C is
	  // the magnitude of triple scalar product |a.(bxc)|
          tuple[0] = fabs(p_tripple(cx, cy, cz));
          
	  //          cellScalar_volume->SetTupleValue(offsetCell, &tuple[0]);
          
	  // Now calculate electric field through cell centre
          
	  // <ei,ej,ek> = electric field along edge of cells
	  // et =  face-centered electric field
          et[0] = cellWallAverage(ei, offset, ok, oj, ojk);
          et[1] = cellWallAverage(ej, offset, ok, oi, oik);
          et[2] = cellWallAverage(ek, offset, oi, oj, oij);
          
	  //FIXME:  Is this Stoke's Theorem or Green's Theorem at work?
          det = 1.e-6/p_tripple(cx, cy, cz);
          tuple[0] = p_tripple(et,cy,cz)*det;
          tuple[1] = p_tripple(cx,et,cz)*det;
          tuple[2] = p_tripple(cx,cy,et)*det;
          
          cellVector_e->SetTupleValue(offsetCell, tuple);
	}
        
	//--
        
	//Store Bijk Data
	//TODO: Implement Derived Quantities
        if(bi != NULL && bj != NULL && bk != NULL){
          tuple[0] = bi[offsetData];
          tuple[1] = bj[offsetData];
          tuple[2] = bk[offsetData];
          cellVector_be->SetTupleValue(offsetCell, tuple);
	}       
        
	//--
        
	//Store Averaged Magnetic Field Data
        if(avgbx != NULL && avgby != NULL && avgbz != NULL){
          tuple[0] = avgbx[offsetData];
          tuple[1] = avgby[offsetData];
          tuple[2] = avgbz[offsetData];
          cellVector_avgb->SetTupleValue(offsetCell, tuple);
	}
        
	//--
        
	//Store Averaged Electric Field Data
	//TODO: Implement Derived Quantities
        if(avgei != NULL && avgej != NULL && avgek != NULL){
          
          setFortranCellGridPointOffsetMacro;
          
	  // Get cell center positions
	  // FIXME: query cell center positions from existing
	  // vtkStructuredGrid::GetPoints(...) array.
          
          cx[0] = cell_AxisAverage(X_grid, oi, oij, oik, oijk, offset, oj, ok, ojk);
          cx[1] = cell_AxisAverage(X_grid, oj, oij, ojk, oijk, offset, oi, ok, oik);
          cx[2] = cell_AxisAverage(X_grid, ok, oik, ojk, oijk, offset, oj, oi, oij);
          
          cy[0] = cell_AxisAverage(Y_grid, oi, oij, oik, oijk, offset, oj, ok, ojk);
          cy[1] = cell_AxisAverage(Y_grid, oj, oij, ojk, oijk, offset, oi, ok, oik);
          cy[2] = cell_AxisAverage(Y_grid, ok, oik, ojk, oijk, offset, oj, oi, oij);
          
          cz[0] = cell_AxisAverage(Z_grid, oi, oij, oik, oijk, offset, oj, ok, ojk);
          cz[1] = cell_AxisAverage(Z_grid, oj, oij, ojk, oijk, offset, oi, ok, oik);
          cz[2] = cell_AxisAverage(Z_grid, ok, oik, ojk, oijk, offset, oj, oi, oij);
          
          
	  // Calculate Cell Volume. Use parallelpiped vector identity:
	  // Volume of parallelpiped determined by vectors A,B,C is
	  // the magnitude of triple scalar product |a.(bxc)|
          tuple[0] = fabs(p_tripple(cx, cy, cz));
          
	  //          cellScalar_volume->SetTupleValue(offsetCell, &tuple[0]);
          
          
	  // Now calculate electric field through cell centre
          
	  // <ei,ej,ek> = electric field along edge of cells
	  // et =  face-centered electric field
          et[0] = cellWallAverage(avgei, offset, ok, oj, ojk);
          et[1] = cellWallAverage(avgej, offset, ok, oi, oik);
          et[2] = cellWallAverage(avgek, offset, oi, oj, oij);
          
	  //FIXME:  Is this Stoke's Theorem or Green's Theorem at work?
          det = 1.e-6/p_tripple(cx, cy, cz);
          tuple[0] = p_tripple(et,cy,cz)*det;
          tuple[1] = p_tripple(cx,et,cz)*det;
          tuple[2] = p_tripple(cx,cy,et)*det;
          
          cellVector_avge->SetTupleValue(offsetCell, tuple);
	}
      }
    }
  }
  
  // Fix x-axis singularity at j=0 and j=nj+1
  double tupleDbl[3];
  float rhoValue, cValue, volumeValue, avgEVvalue;
  float vValue[3], bValue[3], eValue[3], beValue[3], avgBvalue[3], avgEvalue[3];
  
  for (int j=0; j < njp2; j+=njp1){
    jAxis = max(1, min(nj, j));
    for (int i=0; i < ni; i++){
      rhoValue = 0.0;
      cValue = 0.0;
      volumeValue = 0.0;
      avgEVvalue = 0.0;
      
      vValue[0] = 0.0;
      vValue[1] = 0.0;
      vValue[2] = 0.0;
      
      bValue[0] = 0.0;
      bValue[1] = 0.0;
      bValue[2] = 0.0;
      
      beValue[0] = 0.0;
      beValue[1] = 0.0;
      beValue[2] = 0.0;
      
      eValue[0] = 0.0;
      eValue[1] = 0.0;
      eValue[2] = 0.0;
      
      avgBvalue[0] = 0.0;
      avgBvalue[1] = 0.0;
      avgBvalue[2] = 0.0;
      
      avgEvalue[0] = 0.0;
      avgEvalue[1] = 0.0;
      avgEvalue[2] = 0.0;
                  
      for (int k=0; k < nk; k++){        
	//Fix Density
        if(rho != NULL){
	  cellScalar_rho->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);
          rhoValue += (float) tupleDbl[0];
	}
        
	//Fix Sound Speed
        if(c != NULL){
          cellScalar_c->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);
          cValue += (float) tupleDbl[0];
	}
        
        
	//Fix Velocity
        if(vx != NULL && vy != NULL && vz != NULL){
          cellVector_v->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);       
          vValue[0] += (float) tupleDbl[0];
          vValue[1] += (float) tupleDbl[1];
          vValue[2] += (float) tupleDbl[2];
	}
        
	//Fix Magnetic Field
        if(bx != NULL && by != NULL && bz != NULL){
          cellVector_b->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);       
          bValue[0] += (float) tupleDbl[0];
          bValue[1] += (float) tupleDbl[1];
          bValue[2] += (float) tupleDbl[2];
	}
        
	//Fix Electric Field
        if(ei != NULL && ej != NULL && ek != NULL){
          cellVector_e->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);
          eValue[0] += (float) tupleDbl[0];
          eValue[1] += (float) tupleDbl[1];
          eValue[2] += (float) tupleDbl[2];
          
	  //          cellScalar_volume->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);
	  //          volumeValue += (float) tupleDbl[0];
	}
        
	//Fix Bijk Field
        if(bi != NULL && bj != NULL && bk != NULL){
          cellVector_e->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);
          beValue[0] += (float) tupleDbl[0];
          beValue[1] += (float) tupleDbl[1];
          beValue[2] += (float) tupleDbl[2];
	}
               
	//Fix Average Magnetic Field
        if(avgbx != NULL && avgby != NULL && avgbz != NULL){
          cellVector_avgb->GetTuple(i+jAxis*ni + k*ni*njp2, tupleDbl);
          avgBvalue[0] += (float) tupleDbl[0];
          avgBvalue[1] += (float) tupleDbl[1];
          avgBvalue[2] += (float) tupleDbl[2];
	}
        
	//Fix Average Electric Field
        if(avgei != NULL && avgej != NULL && avgek != NULL){
          cellVector_avge->GetTuple(i+jAxis*ni + k*ni*njp2, tupleDbl);
          avgEvalue[0] += (float) tupleDbl[0];
          avgEvalue[1] += (float) tupleDbl[1];
          avgEvalue[2] += (float) tupleDbl[2];
          
	  //          cellScalar_volume->GetTuple(i+jAxis*ni + k*ni*njp2, tupleDbl);
	  //          avgEVvalue += (float) tupleDbl[0];
          
	}
      }
      
      //Adjust Density
      if(rho != NULL)
        rhoValue /= float(nk);
      
      //Adjust Sound Speed
      if(c != NULL)
        cValue /= float(nk);
            
      //Adjust Velocity
      if(vx != NULL && vy != NULL && vz != NULL){
        vValue[0] /= float(nk);
        vValue[1] /= float(nk);
        vValue[2] /= float(nk);
      }
      
      //Adjust Magnetic Field
      if(bx != NULL && by != NULL && bz != NULL){
        bValue[0] /= float(nk);
        bValue[1] /= float(nk);
        bValue[2] /= float(nk);
      }
      
      //adjust Electric Field
      if(ei != NULL && ej != NULL && ek != NULL){
        eValue[0] /= float(nk);
        eValue[1] /= float(nk);
        eValue[2] /= float(nk);
        
        volumeValue /= float(nk);
      }
      
      //Adjust Averaged Magnetic Field
      if(avgbx != NULL && avgby != NULL && avgbz != NULL){
        avgBvalue[0] /= float(nk);
        avgBvalue[1] /= float(nk);
        avgBvalue[2] /= float(nk);
      }
      
      //Adjust Averaged Electric Field
      if(avgei != NULL && avgej != NULL && avgek != NULL){
        avgEvalue[0] /= float(nk);
        avgEvalue[1] /= float(nk);
        avgEvalue[2] /= float(nk);
        
        avgEVvalue /= float(nk);
      }
      
      for (int k=0; k < nk; k++){
	//Commit Fixes
	//Scalars
        if(rho != NULL)
          cellScalar_rho->SetTupleValue(i + j*ni + k*ni*njp2, &rhoValue);
        
        if(c != NULL)
          cellScalar_c->SetTupleValue(i + j*ni + k*ni*njp2, &cValue);
        
	//Vectors
        if(vx != NULL && vy != NULL && vz != NULL)
          cellVector_v->SetTupleValue(i + j*ni + k*ni*njp2, vValue);
        
        if(bx != NULL && by != NULL && bz != NULL)
          cellVector_b->SetTupleValue(i + j*ni + k*ni*njp2, bValue);
        
        if(avgbx != NULL && avgby != NULL && avgbz != NULL)
          cellVector_avgb->SetTupleValue(i + j*ni + k*ni*njp2, avgBvalue);
        
        if(avgei != NULL && avgej != NULL && avgek != NULL){
          cellVector_avge->SetTupleValue(i + j*ni + k*ni*njp2, avgEvalue);
	  //          cellScalar_volume->SetTupleValue(i + j*ni + k*ni*njp2, &avgEVvalue);
	}
        if(ei != NULL && ej != NULL && ek != NULL){
          cellVector_e->SetTupleValue(i + j*ni + k*ni*njp2, eValue);
	  //          cellScalar_volume->SetTupleValue(i + j*ni + k*ni*njp2, &volumeValue);
        }
        if(bi != NULL && bj != NULL && bk != NULL)
          cellVector_be->SetTupleValue(i + j*ni + k*ni*njp2, beValue);          
        
        
      }
    }
  }
  
  for (int j=0; j < njp2; j++){
    for (int i=0; i < ni; i++){
      // Close off the grid
      
      if(rho != NULL){
        cellScalar_rho->GetTuple(i + j*ni, tupleDbl);
        rhoValue = (float) tupleDbl[0];
        cellScalar_rho->SetTupleValue(i + j*ni +   nk*ni*njp2, &rhoValue);
      }
      
      if(c != NULL){
        cellScalar_c->GetTuple(i + j*ni, tupleDbl);
        cValue = (float) tupleDbl[0];
        cellScalar_c->SetTupleValue(i + j*ni +   nk*ni*njp2, &cValue);
      }
      
      
      if(vx != NULL && vy != NULL && vz != NULL){
        cellVector_v->GetTuple(i + j*ni, tupleDbl);
        vValue[0] = (float) tupleDbl[0];
        vValue[1] = (float) tupleDbl[1];
        vValue[2] = (float) tupleDbl[2];
        cellVector_v->SetTupleValue(i + j*ni +   nk*ni*njp2, vValue);
      }
      
      if(bx != NULL && by != NULL && bz != NULL){
        cellVector_b->GetTuple(i + j*ni, tupleDbl);
        bValue[0] = (float) tupleDbl[0];
        bValue[1] = (float) tupleDbl[1];
        bValue[2] = (float) tupleDbl[2];
        cellVector_b->SetTupleValue(i + j*ni +   nk*ni*njp2, bValue);
      }
      
      if(avgbx != NULL && avgby != NULL && avgbz != NULL){
        cellVector_avgb->GetTuple(i + j*ni, tupleDbl);
        avgBvalue[0] = (float) tupleDbl[0];
        avgBvalue[1] = (float) tupleDbl[1];
        avgBvalue[2] = (float) tupleDbl[2];
        cellVector_avgb->SetTupleValue(i + j*ni + nk*ni*njp2, avgBvalue);
      }            
      
      if(avgei != NULL && avgej != NULL && avgek != NULL){
        cellVector_avge->GetTuple(i + j*ni, tupleDbl);
        avgEvalue[0] = (float) tupleDbl[0];
        avgEvalue[1] = (float) tupleDbl[1];
        avgEvalue[2] = (float) tupleDbl[2];
        cellVector_avge->SetTupleValue(i + j*ni + nk*ni*njp2, avgEvalue);                
	//        cellScalar_volume->GetTuple(i + j*ni, tupleDbl);
	//        avgEVvalue = (float)tupleDbl[0];
	//        cellScalar_volume->SetTupleValue(i + j*ni + nk*ni*njp2, &avgEVvalue);
      }
      
      if(ei != NULL && ej != NULL && ek != NULL){
        cellVector_e->GetTuple(i + j*ni, tupleDbl);
        eValue[0] = (float) tupleDbl[0];
        eValue[1] = (float) tupleDbl[1];
        eValue[2] = (float) tupleDbl[2];
        cellVector_e->SetTupleValue(i + j*ni +   nk*ni*njp2, eValue);
        
	//        cellScalar_volume->GetTuple(i + j*ni, tupleDbl);
	//        volumeValue = (float)tupleDbl[0];
	//        cellScalar_volume->SetTupleValue(i + j*ni + nk*ni*njp2, &volumeValue);
      }
      
      if(bi != NULL && bj!= NULL && bk != NULL){
        cellVector_be->GetTuple(i + j*ni, tupleDbl);
        beValue[0] = (float) tupleDbl[0];
        beValue[1] = (float) tupleDbl[1];
        beValue[2] = (float) tupleDbl[2];
        cellVector_be->SetTupleValue(i + j*ni +   nk*ni*njp2, beValue);
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
  
  //Commit Density
  if(rho != NULL){
    output->GetPointData()->AddArray(cellScalar_rho);
    cellScalar_rho->Delete();
  }
  
  //Commit Sound Speed
  if(c != NULL){
    output->GetPointData()->AddArray(cellScalar_c);
    cellScalar_c->Delete();
  }
  
  //Commit Velocity
  if(vx != NULL && vy != NULL && vz != NULL){
    output->GetPointData()->AddArray(cellVector_v);
    cellVector_v->Delete();
  }
  
  //Commit Magnetic Field
  if(bx != NULL && by != NULL && bz != NULL){
    output->GetPointData()->AddArray(cellVector_b);
    cellVector_b->Delete();
  }
  
  //Commit Averaged Magnetic Field
  if(avgbx != NULL && avgby != NULL && avgbz != NULL){
    output->GetPointData()->AddArray(cellVector_avgb);
    cellVector_avgb->Delete();
  }
  
  //Commit Averaged Electric Field
  if(avgei != NULL && avgej != NULL && avgek != NULL){
    output->GetPointData()->AddArray(cellVector_avge);
    cellVector_avge->Delete();    
    
    //    if(cellScalar_volume)
    //      {
    //      output ->GetPointData()->AddArray(cellScalar_volume);
    //      cellScalar_volume->Delete();
    //      }
  }
  
  //Commit Electric Field Data
  if(ei != NULL && ej != NULL && ek != NULL){
    output->GetPointData()->AddArray(cellVector_e);
    cellVector_e->Delete();
    
    //    if(cellScalar_volume)
    //      {
    //      output->GetPointData()->AddArray(cellScalar_volume);
    //      cellScalar_volume->Delete();
    //      }
  }
  
  //Commit Bijk Field Data
  if(bi != NULL && bj != NULL && bk != NULL){
    output->GetPointData()->AddArray(cellVector_be);
    cellVector_be->Delete();
  }
  
  //Clean up Memory
  if (X_grid){    delete [] X_grid;    X_grid = NULL;  }
  if (Y_grid){    delete [] Y_grid;    Y_grid = NULL;  }
  if (Z_grid){    delete [] Z_grid;    Z_grid = NULL;  }
  if (rho){       delete [] rho;       rho = NULL;  }
  if (c) {        delete [] c;         c = NULL; }
  if (vx){        delete [] vx;        vx = NULL; }
  if (vy){        delete [] vy;        vy = NULL; }
  if (vz){        delete [] vz;        vz = NULL; }
  if (bx){        delete [] bx;        bx = NULL; }
  if (by){        delete [] by;        by = NULL; }
  if (bz){        delete [] bz;        bz = NULL; }
  if (bi){        delete [] bi;        bi = NULL;}
  if (bj){        delete [] bj;        bj = NULL;}
  if (bk){        delete [] bk;        bk = NULL;}
  if (ei){        delete [] ei;        ei = NULL;}
  if (ej){        delete [] ej;        ej = NULL;}
  if (ek){        delete [] ek;        ek = NULL;}
  if (avgbx){     delete [] avgbx;     avgbx = NULL; }
  if (avgby){     delete [] avgby;     avgby = NULL; }
  if (avgbz){     delete [] avgbz;     avgbz = NULL; }
  if (avgei){     delete [] avgei;     avgei = NULL; }
  if (avgej){     delete [] avgej;     avgej = NULL; }
  if (avgek){     delete [] avgek;     avgek = NULL; }
  
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

int vtkLFMReader::GetPointArrayStatus(const char *PointArray)
{
  return this->PointArrayStatus[string(PointArray)];
}

//----------------------------------------------------------------

//Cell Array Status Set
void vtkLFMReader::SetCellArrayStatus(const char* CellArray, int status)
{
  
  this->CellArrayStatus[CellArray] = status;
  this->Modified();
  
}

//----------------------------------------------------------------

void vtkLFMReader::SetPointArrayStatus(const char* PointArray, int status)
{
  this->PointArrayStatus[PointArray] = status;
  this->Modified();
}

//----------------------------------------------------------------
//This version of SetIfExists is for scalars
void vtkLFMReader::SetIfExists(DeprecatedHdf4 &f, std::string VarName, std::string VarDescription)
{
  if(f.hasVariable(VarName)){
    //Set Variable->description map
    this->ArrayNameLookup[VarName] = VarDescription;
    
    //Set other Array Variables
    this->NumberOfCellArrays++;
    this->CellArrayName.push_back(VarDescription);
    this->CellArrayStatus[VarDescription] = 1;
  }
  
  vtkDebugMacro(<< VarName << ": " << VarDescription);
}

//----------------------------------------------------------------
//This Version of SetIfExists is for Vectors (3D)
void vtkLFMReader::SetIfExists(DeprecatedHdf4 &f, std::string xVar, std::string yVar, std::string zVar, std::string VarDescription)
{
  if (f.hasVariable(xVar) && f.hasVariable(yVar) && f.hasVariable(zVar)){
    //Set variable->desciption map
    this->ArrayNameLookup[xVar] = VarDescription;
    this->ArrayNameLookup[yVar] = VarDescription;
    this->ArrayNameLookup[zVar] = VarDescription;
    
    //Set other Array Variables
    this->NumberOfCellArrays++;
    this->CellArrayName.push_back(VarDescription);
    this->CellArrayStatus[VarDescription] = 1;
  }
  
  vtkDebugMacro(<< xVar << "," << yVar << "," << zVar << ": " << VarDescription);
}

//----------------------------------------------------------------
//This Version adds a new array based on existence of a scalar
void vtkLFMReader::SetNewIfExists(DeprecatedHdf4 &f, std::string VarName, std::string ArrayIndexName, std::string VarDescription)
{
  if(f.hasVariable(VarName)){
    //Set Variable->description map
    this->ArrayNameLookup[ArrayIndexName] = VarDescription;
    
    //Set other Array Variables
    this->NumberOfCellArrays++;
    this->CellArrayName.push_back(VarDescription);
    this->CellArrayStatus[VarDescription] = 1;
  }
  
  vtkDebugMacro(<< ArrayIndexName << ": " << VarDescription);
}

//----------------------------------------------------------------
// This version adds a new Array based on existence of a vector
void vtkLFMReader::SetNewIfExists(DeprecatedHdf4 &f, std::string xVar, std::string yVar, std::string zVar, std::string ArrayIndexName,  std::string VarDescription)
{
  if (f.hasVariable(xVar) && f.hasVariable(yVar) && f.hasVariable(zVar)){
    //Set variable->desciption map
    this->ArrayNameLookup[ArrayIndexName] = VarDescription;
    
    //Set other Array Variables
    this->NumberOfCellArrays++;
    this->CellArrayName.push_back(VarDescription);
    this->CellArrayStatus[VarDescription] = 1;
  }
  
  vtkDebugMacro(<< ArrayIndexName << ":  " << VarDescription);
  
}

//----------------------------------------------------------------
void vtkLFMReader::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkLFMReader says \"Hello, World!\" " << "\n";
}
