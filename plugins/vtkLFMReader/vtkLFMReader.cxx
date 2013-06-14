#include "vtkLFMReader.h"

#include "io/Io.hpp"

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

bool isStrInList(const list<string> &listOfStrings, const string &searchString){
  list<string>::const_iterator it;
  it = find(listOfStrings.begin(), listOfStrings.end(), searchString);
  if (it != listOfStrings.end() )
    return true;
  else
    return false;
}

//----------------------------------------------------------------

int vtkLFMReader::CanReadFile(const char *filename)
{
  Io *io = Io::extensionSelector("hdf");
  io->openRead(string(filename));

  list<string> attributeNames = io->getAttributeNames();
  if( //(isStrInList(attributeNames, "mjd")) &&
      (isStrInList(attributeNames, "time_step")) &&
      (isStrInList(attributeNames, "time")) &&
      (isStrInList(attributeNames, "tilt_angle")) &&
      //(isStrInList("I/O Revision")) &&
      //(isStrInList("Repository Revision")) &&
      (isStrInList(attributeNames, "file_contents")) &&
      (isStrInList(attributeNames, "dipole_moment")) &&
      (isStrInList(attributeNames, "written_by")) ){
    return 1;
  }

  //if we made it this far, assume attribute is not in list.
  return 0;
}

//----------------------------------------------------------------


int vtkLFMReader::RequestInformation (vtkInformation* request,
                                      vtkInformationVector** inputVector,
                                      vtkInformationVector* outputVector)
{ 
  // Read entire extents from Hdf4 file.  This requires reading an
  // entire variable.  Let's arbitrarily choose X_grid:
  //Io *io = Io::extensionSelector("hdf");
  //io->openRead(this->GetFileName());
  //array_info_t xGrid_info = io->getArrayInfo("X_grid");  

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
  
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), subext);
  

  /*************************
   * Setup structured grid *
   ****************************************************************************/  
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

  /* Note the funny dimensions:
   *  - Fix x-axis caps (nj++ in nose, nj++ in tail)
   *  - close off grid (nk++)
   * 
   * Note: We do nothing special with the periodic boundary on the
   * grid at z=0 and z=max! This discussion from the ParaView mailing
   * list suggests that there is little we can do for now:
   * http://www.paraview.org/pipermail/paraview/2009-July/012750.html
   */
  output->SetDimensions(ni, njp2, nkp1);
  
  DeprecatedHdf4 f;
  f.open(string(this->GetFileName()), IO::READ);

  /*******************
   * Set grid points *
   ****************************************************************************/
  
  int rank;
  int *dims = NULL;  
  float *X_grid = NULL;    
  f.readVariable("X_grid", X_grid, rank, dims);   delete []dims;
  float *Y_grid = NULL;
  f.readVariable("Y_grid", Y_grid, rank, dims);   delete []dims;
  float *Z_grid = NULL;
  f.readVariable("Z_grid", Z_grid, rank, dims);   delete []dims;        

  vtkPoints *points = point2CellCenteredGrid(nip1,njp1,nkp1,  X_grid,Y_grid,Z_grid);
  output->SetPoints(points);
  points->Delete();

  // Do not delete X_grid, Y_grid, Z_grid.  We may need them to
  // calculate derived quantities (ie. electric field).

  /*****************************
   * Cell-centered scalar data *
   ****************************************************************************/

  //Density Selective Read
  if(this->CellArrayStatus[GetDesc("rho_")]){
    vtkDebugMacro(<<"Plasma Desnity Selected");
    float *rho = NULL;
    f.readVariable("rho_",   rho,    rank, dims);  delete []dims;   
    
    if(rho != NULL){
      vtkFloatArray *cellScalar_rho = NULL;
      cellScalar_rho = point2CellCenteredScalar(nip1,njp1,nkp1, rho);
      cellScalar_rho->SetName(GetDesc("rho_").c_str());
      output->GetPointData()->AddArray(cellScalar_rho);
      cellScalar_rho->Delete();

      delete [] rho;
      rho = NULL;
    }
  }
  
  //Sound Speed Selective Read
  if(this->CellArrayStatus[GetDesc("c_")]){
    vtkDebugMacro(<< "Sound Speed Selected");
    float *c = NULL;
    f.readVariable("c_",     c,      rank, dims);  delete []dims;    
    if(c != NULL){
      vtkFloatArray *cellScalar_c = NULL;
      cellScalar_c = point2CellCenteredScalar(nip1,njp1,nkp1, c);
      cellScalar_c->SetName(GetDesc("c_").c_str());
      output->GetPointData()->AddArray(cellScalar_c);
      cellScalar_c->Delete();

      delete [] c;
      c = NULL;
    }
  }
  
  /*****************************
   * Cell-centered Vector data *
   ****************************************************************************/
  // Velocity
  //Velocity Selective Read
  if(this->CellArrayStatus[GetDesc("vx_")]){
    vtkDebugMacro(<< "Velocity Selected");    
    float *vx = NULL;
    float *vy = NULL;
    float *vz = NULL;    
    f.readVariable("vx_",    vx,     rank, dims);   delete []dims;
    f.readVariable("vy_",    vy,     rank, dims);   delete []dims;
    f.readVariable("vz_",    vz,     rank, dims);   delete []dims;

    if(vx != NULL && vy != NULL && vz != NULL){
      vtkFloatArray *cellVector_v = NULL;
      cellVector_v = point2CellCenteredVector(nip1,njp1,nkp1, vx,vy,vz);
      cellVector_v->SetName(GetDesc("vx_").c_str());
      output->GetPointData()->AddArray(cellVector_v);
      cellVector_v->Delete();

      delete [] vx;
      vx = NULL;
      delete [] vy;
      vy = NULL;
      delete [] vz;
      vz = NULL;
    }
  }

  //Magnetic Field Selective Read
  if(this->CellArrayStatus[GetDesc("bx_")]){
    vtkDebugMacro(<< "Magnetic Field Vector Selected");    
    float *bx = NULL;
    float *by = NULL;
    float *bz = NULL;
    f.readVariable("bx_",    bx,     rank, dims);   delete []dims;
    f.readVariable("by_",    by,     rank, dims);   delete []dims;
    f.readVariable("bz_",    bz,     rank, dims);   delete []dims;
    // Magnetic Field      
    if(bx != NULL && by != NULL && bz != NULL){
      vtkFloatArray *cellVector_b = NULL;
      cellVector_b = point2CellCenteredVector(nip1,njp1,nkp1, bx,by,bz);
      cellVector_b->SetName(GetDesc("bx_").c_str());
      output->GetPointData()->AddArray(cellVector_b);
      cellVector_b->Delete();
      delete [] bx;
      bx = NULL;
      delete [] by;
      by = NULL;
      delete [] bz;
      bz = NULL;
    }
  }
  
  //Averaged Magnetic Field Selective Read
  if(this->CellArrayStatus[GetDesc("avgBx")]){
    vtkDebugMacro(<< "Averaged Magnetic Field Vector Selected");     
    float *avgbz = NULL;
    float *avgby = NULL;
    float *avgbx = NULL;
      f.readVariable("avgBx",    avgbx,     rank, dims);   delete []dims;
    f.readVariable("avgBy",    avgby,     rank, dims);   delete []dims;
    f.readVariable("avgBz",    avgbz,     rank, dims);   delete []dims;
    if(avgbx != NULL && avgby != NULL && avgbz != NULL){
      vtkFloatArray *cellVector_avgb = NULL;
      cellVector_avgb = point2CellCenteredVector(nip1,njp1,nkp1, avgbx, avgby, avgbz);
      cellVector_avgb->SetName(GetDesc("avgBx").c_str());
      output->GetPointData()->AddArray(cellVector_avgb);
      cellVector_avgb->Delete();
      delete [] avgbx;
      avgbx = NULL;
      delete [] avgby;
      avgby = NULL;
      delete [] avgbz;
      avgbz = NULL;
    }
  }

  //Electric Field Selective Read
  if(this->CellArrayStatus[GetDesc("ei_")]){
    vtkDebugMacro(<< "Electric Field vector Selected");
    float *ei = NULL;
    float *ej = NULL;
    float *ek = NULL;
    f.readVariable("ei_",   ei,   rank, dims);    delete []dims;
    f.readVariable("ej_",   ej,   rank, dims);    delete []dims;
    f.readVariable("ek_",   ek,   rank, dims);    delete []dims;    

    //Read Electric Field
    if(ei != NULL && ej != NULL && ek != NULL){
      vtkFloatArray *cellVector_e = NULL;  
      float *ex = new float[nip1*njp1*nkp1];
      float *ey = new float[nip1*njp1*nkp1];
      float *ez = new float[nip1*njp1*nkp1];
      calculateElectricField(nip1,njp1,nkp1,
			     X_grid,Y_grid,Z_grid,
			     ei,ej,ek,
			     ex,ey,ez);
      delete [] ei;
      ei = NULL;
      delete [] ej;
      ej = NULL;
      delete [] ek;
      ek = NULL;
      cellVector_e = point2CellCenteredVector(nip1,njp1,nkp1, ex,ey,ez);
      delete[] ex;
      ex=NULL;
      delete[] ey;
      ey=NULL;
      delete[] ez;
      ez=NULL;
      cellVector_e->SetName(GetDesc("ei_").c_str());
      output->GetPointData()->AddArray(cellVector_e);
      cellVector_e->Delete();
    }
  }

    
  //Averaged E(ijk) Fields
  if(this->CellArrayStatus[GetDesc("avgEi")]){
    vtkDebugMacro(<< "Averaged Electric Field Vector Selected");
  
    float *avgei = NULL;
    float *avgej = NULL;
    float *avgek = NULL;
    
    f.readVariable("avgEi",   avgei,      rank, dims);  delete []dims;
    f.readVariable("avgEj",   avgej,      rank, dims);  delete []dims;
    f.readVariable("avgEk",   avgek,      rank, dims);  delete []dims;
    //Reading Averaged Electric Field
    if(avgei != NULL && avgej != NULL && avgek != NULL){
      vtkFloatArray *cellVector_avge = NULL;
      float *avgEx = new float[nip1*njp1*nkp1];
      float *avgEy = new float[nip1*njp1*nkp1];
      float *avgEz = new float[nip1*njp1*nkp1];
      calculateElectricField(nip1,njp1,nkp1,
			     X_grid,Y_grid,Z_grid,
			     avgei,avgej,avgek,
			     avgEx,avgEy,avgEz);
      delete [] avgei;
      avgei = NULL;
      delete [] avgej;
      avgej = NULL;
      delete [] avgek;
      avgek = NULL;
      cellVector_avge = point2CellCenteredVector(nip1,njp1,nkp1, avgEx,avgEy,avgEz);
      delete[] avgEx;
      avgEx=NULL;
      delete[] avgEy;
      avgEy=NULL;
      delete[] avgEz;
      avgEz=NULL;
      cellVector_avge->SetName(GetDesc("avgEi_").c_str());
      output->GetPointData()->AddArray(cellVector_avge);
      cellVector_avge->Delete();
    }
  }
    
  //Clean up Memory
  if (X_grid){    delete [] X_grid;    X_grid = NULL;  }
  if (Y_grid){    delete [] Y_grid;    Y_grid = NULL;  }
  if (Z_grid){    delete [] Z_grid;    Z_grid = NULL;  }
    
  dims = NULL;
  f.close();
  
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

//----------------------------------------------------------------

vtkPoints *vtkLFMReader::point2CellCenteredGrid(const int &nip1, const int &njp1, const int &nkp1,
						const float const *X_grid, const float const *Y_grid, const float const *Z_grid)
{
  const int ni = nip1-1;
  const int nim1 = ni-1;

  const int njp2 = njp1+1;
  const int nj = njp1-1;

  const int nkp2 = nkp1+1;
  const int nk = nkp1-1;

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

  return points;
}

//----------------------------------------------------------------

vtkFloatArray *vtkLFMReader::point2CellCenteredScalar(const int &nip1, const int &njp1, const int &nkp1,  const float const *data)
{
  return point2CellCenteredVector(nip1,njp1,nkp1, data, NULL, NULL);
}

//----------------------------------------------------------------


vtkFloatArray *vtkLFMReader::point2CellCenteredVector(const int &nip1, const int &njp1, const int &nkp1,
						      const float const *xData, const float const *yData, const float const *zData)
{
  const int ni = nip1-1;
  const int nim1 = ni-1;

  const int njp2 = njp1+1;
  const int nj = njp1-1;

  const int nkp2 = nkp1+1;
  const int nk = nkp1-1;

  vtkFloatArray *data = vtkFloatArray::New();
  if (xData && yData && zData){
    data->SetNumberOfComponents(3);
  }
  //else if ( (xData && yData && not zData) ||
  //	 (not xData && yData && zData) ||
  //	 (xData && not yData && zData) ){
  //  data->SetNumberOfComponents(2);
  //}
  else if ( (xData && not yData && not zData) ){
    data->SetNumberOfComponents(1);
  }
  data->SetNumberOfTuples(ni*njp2*nkp1);

  // Store values in VTK objects:
  
  int offsetData, offsetCell;
  float tuple[3];
  
  for (int k=0; k < nk; k++){
    for (int j=0; j < nj; j++){
      for (int i=0; i < ni; i++){        
        offsetData = i + j*nip1 + k*nip1*njp1;
        
	// j+1 because we set data along j=0 in "Fix x-axis singularity", below.
        offsetCell = i + (j+1)*ni   + k*ni*njp2;
        
	//Store Velocity Data
	tuple[0] = xData[offsetData];
	if (yData && zData){
	  tuple[1] = yData[offsetData];
	  tuple[2] = zData[offsetData];
	}
	data->SetTupleValue(offsetCell, tuple);
      }
    }
  }
  
  // Fix x-axis singularity at j=0 and j=nj+1
  double tupleDbl[3];

  int jAxis;
  for (int j=0; j < njp2; j+=njp1){
    jAxis = max(1, min(nj, j));
    for (int i=0; i < ni; i++){
      tuple[0] = 0.0;
      tuple[1] = 0.0;
      tuple[2] = 0.0;

      for (int k=0; k < nk; k++){        
	data->GetTuple(i + jAxis*ni + k*ni*njp2, tupleDbl);       
	tuple[0] += (float) tupleDbl[0];
	if (yData && zData){
	  tuple[1] += (float) tupleDbl[1];
	  tuple[2] += (float) tupleDbl[2];
	}
      }
      tuple[0] /= float(nk);
	if (yData && zData){
	  tuple[1] /= float(nk);
	  tuple[2] /= float(nk);
	}
      
      for (int k=0; k < nk; k++){
	data->SetTupleValue(i + j*ni + k*ni*njp2, tuple);
      }
    }
  }
  
  for (int j=0; j < njp2; j++){
    for (int i=0; i < ni; i++){
      // Close off the grid
      data->GetTuple(i + j*ni, tupleDbl);
      tuple[0] = (float) tupleDbl[0];
      if (yData && zData){
	tuple[1] = (float) tupleDbl[1];
	tuple[2] = (float) tupleDbl[2];
      }
      data->SetTupleValue(i + j*ni +   nk*ni*njp2, tuple);
      
      // FIXME: Set periodic cells:
      //data->GetTuple(i + j*ni + 1*ni*njp2, tupleDbl);
      //tuple[0] = (float) tupleDbl[0];
      //tuple[1] = (float) tupleDbl[1];
      //tuple[2] = (float) tupleDbl[2];
      //data->SetTupleValue(i + j*ni + nkp1*ni*njp2, tuple);
    }
  }

  return data;
}

//--------------------------------------------------------------------

void vtkLFMReader::calculateElectricField(const int &nip1, const int &njp1, const int &nkp1,
					  const float const *X_grid, const float const *Y_grid, const float const *Z_grid,
					  const float const *ei, const float const *ej, const float const *ek,
					  float *ex, float *ey, float *ez)
{
  const int ni = nip1-1;
  const int nj = njp1-1;
  const int nk = nkp1-1;

  float cx[3]={0,0,0}, cy[3]={0,0,0}, cz[3]={0,0,0}, et[3]={0,0,0};
  float det=0; 

  int offset = 0;
  int oi = 0, oij = 0, oik = 0;
  int oj = 0, ojk = 0, oijk = 0;
  int ok = 0;  
  
  for (int k=0; k < nk; k++){
    for (int j=0; j < nj; j++){
      for (int i=0; i < ni; i++){          
	//Store Electric Field Data
        setFortranCellGridPointOffsetMacro;
	
	// X_grid Cell edge vector
	cx[0] = cell_AxisAverage(X_grid, oi, oij, oik, oijk, offset, oj, ok, ojk);
	cx[1] = cell_AxisAverage(X_grid, oj, oij, ojk, oijk, offset, oi, ok, oik);
	cx[2] = cell_AxisAverage(X_grid, ok, oik, ojk, oijk, offset, oj, oi, oij);

	// Y_grid Cell edge vector       
	cy[0] = cell_AxisAverage(Y_grid, oi, oij, oik, oijk, offset, oj, ok, ojk);
	cy[1] = cell_AxisAverage(Y_grid, oj, oij, ojk, oijk, offset, oi, ok, oik);
	cy[2] = cell_AxisAverage(Y_grid, ok, oik, ojk, oijk, offset, oj, oi, oij);
        
	// Z_grid Cell edge vector
	cz[0] = cell_AxisAverage(Z_grid, oi, oij, oik, oijk, offset, oj, ok, ojk);
	cz[1] = cell_AxisAverage(Z_grid, oj, oij, ojk, oijk, offset, oi, ok, oik);
	cz[2] = cell_AxisAverage(Z_grid, ok, oik, ojk, oijk, offset, oj, oi, oij);
                          
	// Now calculate electric field through cell center
          
	// <ei,ej,ek> = electric field along edge of cells
	// et =  face-centered electric field
	et[0] = cellWallAverage(ei, offset, ok, oj, ojk);
	et[1] = cellWallAverage(ej, offset, ok, oi, oik);
	et[2] = cellWallAverage(ek, offset, oi, oj, oij);
          
	//FIXME:  Is this Stoke's Theorem or Green's Theorem at work?
	det = 1.e-6/p3d_triple(cx, cy, cz);
	ex[offset] = p3d_triple(et,cy,cz)*det;
	ey[offset] = p3d_triple(cx,et,cz)*det;
	ez[offset] = p3d_triple(cx,cy,et)*det;

	// Here's how you would calculate volume of cell. Use
	// parallelpiped vector identity: Volume of parallelpiped
	// determined by vectors A,B,C is the magnitude of triple
	// scalar product |a.(bxc)|
	//float volume = fabs(p3d_triple(cx, cy, cz));
      }       
    }
  }
}

//--------------------------------------------------------------------
