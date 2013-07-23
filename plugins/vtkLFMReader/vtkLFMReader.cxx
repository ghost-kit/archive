#include "vtkLFMReader.h"

#include "io/Io.hpp"

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

vtkLFMReader::vtkLFMReader() : HdfFileName(NULL), GridScaleType(GRID_SCALE::NONE), nSpecies(1)
{
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

/// Returns true if searchString is in listOfStrings
bool isStrInList(const list<string> &listOfStrings, const string &searchString){
  list<string>::const_iterator it;
  it = find(listOfStrings.begin(), listOfStrings.end(), searchString);
  if (it != listOfStrings.end() )
    return true;
  else
    return false;
}

//----------------------------------------------------------------

/** Returns true if attName is in attributes
 * FIXME: Add as a member of the Io base class!
 */
bool hasAttribute(const list<string> &attributes, const string &attName)
{
  return isStrInList(attributes, attName);
}

//----------------------------------------------------------------

/** Returns true if varName is in variables
 * FIXME: Add as a member of the Io base class!
 */
bool hasVariable(const list<string> &variables, const string &varName)
{
  return isStrInList(variables, varName);
}

//----------------------------------------------------------------

int vtkLFMReader::CanReadFile(const char *filename)
{
  Io *io = Io::extensionSelector("hdf");
  io->openRead(string(filename));

  bool isValidFile = false;

  list<string> attributeNames = io->getAttributeNames();
  if( //(hasAttribute(attributeNames, "mjd")) &&
      (hasAttribute(attributeNames, "time_step")) &&
      (hasAttribute(attributeNames, "time")) &&
      //(hasAttribute(attributeNames, "tilt_angle")) &&
      //(hasAttribute("I/O Revision")) &&
      //(hasAttribute("Repository Revision")) &&
      //(hasAttribute(attributeNames, "file_contents")) &&
      (hasAttribute(attributeNames, "dipole_moment")) &&
      (hasAttribute(attributeNames, "written_by")) ){
    isValidFile = true;
  }

  io->close();
  if (io){
    delete io;
    io = NULL;
  }

  //if we made it this far, assume attribute is not in list.
  return isValidFile;
}

//----------------------------------------------------------------


int vtkLFMReader::RequestInformation (vtkInformation* request,
                                      vtkInformationVector** inputVector,
                                      vtkInformationVector* outputVector)
{ 
  Io *io = Io::extensionSelector("hdf");
  io->openRead(this->GetFileName());
  array_info_t xGrid_info = io->getArrayInfo("X_grid");  
  list<string> variables = io->getVariableNames();
  list<string> attributes = io->getAttributeNames();

  if(this->globalDims.size() == 0){
    this->globalDims.push_back( xGrid_info.localDims[2] ); // nip1
    this->globalDims.push_back( xGrid_info.localDims[1] ); // njp1
    this->globalDims.push_back( xGrid_info.localDims[0] ); // nkp1

    vtkDebugMacro(<< "Dimensions: "
		  << this->globalDims[0] << ", "
		  << this->globalDims[1] << ", "
		  << this->globalDims[2]);
  }

  /********************************************************************/
  // Determine which variables are available to the GUI
  /********************************************************************/

  // Scalars
  if (hasVariable(variables, "rho_"))
    addScalarInformation("rho_", "Plasma Density");
  if (hasVariable(variables, "c_"))
    addScalarInformation("c_", "Sound Speed");
  
  // Vectors
  if (hasVariable(variables, "vx_") && hasVariable(variables, "vy_") && hasVariable(variables, "vz_"))
    addVectorInformation("vx_", "vy_", "vz_", "Velocity Vector");
  if (hasVariable(variables, "bx_") && hasVariable(variables, "by_") && hasVariable(variables, "bz_"))
    addVectorInformation("bx_", "by_", "bz_", "Magnetic Field Vector");
  if (hasVariable(variables, "avgBx") && hasVariable(variables, "avgBy") && hasVariable(variables, "avgBz"))
    addVectorInformation("avgBx", "avgBy", "avgBz", "Magnetic Field Vector (avg)");

  // Derived Quantities
  if (hasVariable(variables, "ei_") && hasVariable(variables, "ej_") && hasVariable(variables, "ek_"))
    addVectorInformation("ei_", "ej_", "ek_", "Electric Field Vector");
  if (hasVariable(variables, "avgEi") && hasVariable(variables, "avgEj") && hasVariable(variables, "avgEk"))
    addVectorInformation("avgEi", "avgEj", "avgEk", "Electric Field Vector (avg)"); 
  // placeholder for calculating the Current vector.  See Pjcalc2.F from CISM_DX reader.
  //if (hasVariable(variables, "bi_") && hasVariable(variables, "bj_") && hasVariable(variables, "bk_"))
  //  addVectorInformation("bi_", "bj_", "bk_", "Current Vector");
  

  // Multifluid Variables
  if(hasAttribute(attributes, "n_species")){
    io->readAttribute("n_species", nSpecies);

    for(int fluidNumber=1; fluidNumber <= nSpecies; fluidNumber++){
      stringstream ssVarName;
      string varName;

      ssVarName << "rho_" << fluidNumber << "_";
      varName = ssVarName.str();
      if (hasVariable(variables, varName)){
	stringstream description;
	description << "Plasma Density #" << fluidNumber;
	addScalarInformation(varName, description.str());
      }
      
      ssVarName.str(string());
      ssVarName << "c_" << fluidNumber << "_";
      varName = ssVarName.str();
      if (hasVariable(variables, varName)){
	stringstream description;
	description << "Sound Speed #" << fluidNumber;
	addScalarInformation(varName, description.str());
      }

      ssVarName.str(string());
      ssVarName << "vx_" << fluidNumber << "_";
      string xVarName = ssVarName.str();
      ssVarName.str(string());
      ssVarName << "vy_" << fluidNumber << "_";
      string yVarName = ssVarName.str();
      ssVarName.str(string());
      ssVarName << "vz_" << fluidNumber << "_";
      string zVarName = ssVarName.str();
      if (hasVariable(variables, xVarName) && hasVariable(variables, yVarName) && hasVariable(variables, zVarName)){
	stringstream description;
	description << "Velocity Vector #" << fluidNumber;
	addVectorInformation(xVarName, yVarName, zVarName, description.str());
      }      
    }
  }
  
  /********************************************************************/
  // Set WHOLE_EXTENT
  /********************************************************************/
  //Navigation helpers
  const int nim1 = this->globalDims[0]-2;
  const int njp1 = this->globalDims[1];
  const int nk   = this->globalDims[2]-1;
  
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
  

  /********************************************************************/
  // Set Time information
  /********************************************************************/

  // Currently 1 time step per file.  Append to a vector in case we want to extend this in the future.
  if (hasAttribute(attributes, "mjd")){
    // modified julian date
    double mjd;
    io->readAttribute("mjd", mjd);      
    this->TimeStepValues.push_back( mjd );
  }
  else if (hasAttribute(attributes, "time")){
    // Slava Merkin's LFM-Helio doesn't have the "mjd" parameter, but it does have "time":
    //time = number of seconds since beginning of simluation    
    float time;
    io->readAttribute("time", time);
    this->TimeStepValues.push_back( time );
  }
  else{
    vtkWarningMacro("Could not find time information in file (attribute \"mjd\" or \"time\")! Defaulting to 0.0");
    this->TimeStepValues.push_back( 0.0 );
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
  
  vtkDebugMacro(<< "number of timesteps in file=" << this->TimeStepValues.size());
  vtkDebugMacro(<< "Modified julian date in file=" << this->TimeStepValues[0] << endl
                << "TimeStepValues=" << this->TimeStepValues[0] << " " << this->TimeStepValues[1] << endl
                << "timeRange[0]=" << timeRange[0] <<" timeRange[1]=" << timeRange[1]);
  
  io->close();
  if (io){
    delete io;
    io = NULL;
  }

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
    
  const int nip1 = this->globalDims[0];
  const int ni = nip1-1;
  const int nim1 = ni-1;
  const int njp1 = this->globalDims[1];
  const int njp2 = njp1+1;
  const int nj = njp1-1;
  const int nkp1 = this->globalDims[2];
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


  Io *io = Io::extensionSelector("hdf");
  io->openRead(string( this->GetFileName() ));
  array_info_t lfmGridInfo = io->getArrayInfo("X_grid");  

  const int nPoints = lfmGridInfo.globalDims[0] * lfmGridInfo.globalDims[1] * lfmGridInfo.globalDims[2];

  /*******************
   * Set grid points *
   ****************************************************************************/
    
  float *X_grid = new float [nPoints];
  io->readVariable("X_grid", "", lfmGridInfo, X_grid);
  float *Y_grid = new float [nPoints];
  io->readVariable("Y_grid", "", lfmGridInfo, Y_grid);
  float *Z_grid = new float [nPoints];
  io->readVariable("Z_grid", "", lfmGridInfo, Z_grid);

  vtkPoints *points = point2CellCenteredGrid(nip1,njp1,nkp1,  X_grid,Y_grid,Z_grid);
  output->SetPoints(points);
  points->Delete();

  // Do not delete X_grid, Y_grid, Z_grid.  We may need them to
  // calculate derived quantities (ie. electric field).

  /*****************************
   * Cell-centered scalar data *
   ****************************************************************************/

  //Density Selective Read
  if(this->CellArrayStatus[describeVariable["rho_"]]){
    vtkDebugMacro(<<"Plasma Density Selected");
    float *rho = new float [nPoints];
    io->readVariable("rho_", "", lfmGridInfo, rho);
    
    if(rho != NULL){
      vtkFloatArray *cellScalar_rho = NULL;
      cellScalar_rho = point2CellCenteredScalar(nip1,njp1,nkp1, rho);
      cellScalar_rho->SetName(describeVariable["rho_"].c_str());
      output->GetPointData()->AddArray(cellScalar_rho);
      cellScalar_rho->Delete();

      delete [] rho;
      rho = NULL;
    }
  }
  
  //Sound Speed Selective Read
  if(this->CellArrayStatus[describeVariable["c_"]]){
    vtkDebugMacro(<< "Sound Speed Selected");
    float *c = new float [nPoints];
    io->readVariable("c_", "", lfmGridInfo, c);
    if(c != NULL){
      vtkFloatArray *cellScalar_c = NULL;
      cellScalar_c = point2CellCenteredScalar(nip1,njp1,nkp1, c);
      cellScalar_c->SetName(describeVariable["c_"].c_str());
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
  if(this->CellArrayStatus[describeVariable["vx_"]]){
    vtkDebugMacro(<< "Velocity Selected");    
    float *vx = new float [nPoints];
    float *vy = new float [nPoints];
    float *vz = new float [nPoints];
    io->readVariable("vx_", "", lfmGridInfo, vx);
    io->readVariable("vy_", "", lfmGridInfo, vy);
    io->readVariable("vz_", "", lfmGridInfo, vz);

    if(vx != NULL && vy != NULL && vz != NULL){
      vtkFloatArray *cellVector_v = NULL;
      cellVector_v = point2CellCenteredVector(nip1,njp1,nkp1, vx,vy,vz);
      cellVector_v->SetName(describeVariable["vx_"].c_str());
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
  if(this->CellArrayStatus[describeVariable["bx_"]]){
    vtkDebugMacro(<< "Magnetic Field Vector Selected");    
    float *bx = new float [nPoints];
    float *by = new float [nPoints];
    float *bz = new float [nPoints];
    io->readVariable("bx_", "", lfmGridInfo, bx);
    io->readVariable("by_", "", lfmGridInfo, by);
    io->readVariable("bz_", "", lfmGridInfo, bz);
    // Magnetic Field      
    if(bx != NULL && by != NULL && bz != NULL){
      vtkFloatArray *cellVector_b = NULL;
      cellVector_b = point2CellCenteredVector(nip1,njp1,nkp1, bx,by,bz);
      cellVector_b->SetName(describeVariable["bx_"].c_str());
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
  if(this->CellArrayStatus[describeVariable["avgBx"]]){
    vtkDebugMacro(<< "Averaged Magnetic Field Vector Selected");     
    float *avgbz = new float [nPoints];
    float *avgby = new float [nPoints];
    float *avgbx = new float [nPoints];
    io->readVariable("avgBx", "", lfmGridInfo, avgbx);
    io->readVariable("avgBy", "", lfmGridInfo, avgby);
    io->readVariable("avgBz", "", lfmGridInfo, avgbz);

    if(avgbx != NULL && avgby != NULL && avgbz != NULL){
      vtkFloatArray *cellVector_avgb = NULL;
      cellVector_avgb = point2CellCenteredVector(nip1,njp1,nkp1, avgbx, avgby, avgbz);
      cellVector_avgb->SetName(describeVariable["avgBx"].c_str());
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
  if(this->CellArrayStatus[describeVariable["ei_"]]){
    vtkDebugMacro(<< "Electric Field vector Selected");
    float *ei = new float [nPoints];
    float *ej = new float [nPoints];
    float *ek = new float [nPoints];
    io->readVariable("ei_", "", lfmGridInfo, ei);
    io->readVariable("ej_", "", lfmGridInfo, ej);
    io->readVariable("ek_", "", lfmGridInfo, ek);

    //Read Electric Field
    if(ei != NULL && ej != NULL && ek != NULL){
      vtkFloatArray *cellVector_e = NULL;  
      float *ex = new float [nip1*njp1*nkp1];
      float *ey = new float [nip1*njp1*nkp1];
      float *ez = new float [nip1*njp1*nkp1];
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
      cellVector_e->SetName(describeVariable["ei_"].c_str());
      output->GetPointData()->AddArray(cellVector_e);
      cellVector_e->Delete();
    }
  }

    
  //Averaged E(ijk) Fields
  if(this->CellArrayStatus[describeVariable["avgEi"]]){
    vtkDebugMacro(<< "Averaged Electric Field Vector Selected");
  
    float *avgei = new float [nPoints];
    float *avgej = new float [nPoints];
    float *avgek = new float [nPoints];
    io->readVariable("avgEi", "", lfmGridInfo, avgei);
    io->readVariable("avgEj", "", lfmGridInfo, avgej);
    io->readVariable("avgEk", "", lfmGridInfo, avgek);
    //Reading Averaged Electric Field
    if(avgei != NULL && avgej != NULL && avgek != NULL){
      vtkFloatArray *cellVector_avge = NULL;
      float *avgEx = new float [nip1*njp1*nkp1];
      float *avgEy = new float [nip1*njp1*nkp1];
      float *avgEz = new float [nip1*njp1*nkp1];
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
      cellVector_avge->SetName(describeVariable["avgEi_"].c_str());
      output->GetPointData()->AddArray(cellVector_avge);
      cellVector_avge->Delete();
    }
  }

  /*************************
   * Multi-fluid variables *
   ****************************************************************************/

  for (int fluidNumber=1; fluidNumber <= nSpecies; fluidNumber++){
    stringstream ssVarName;
    string varName;

    ////////////////////////////////////////////////////////////////////////////

    // Density scalar
    ssVarName << "rho_" << fluidNumber << "_";
    varName = ssVarName.str();
    if (this->CellArrayStatus[describeVariable[varName]]){
      vtkDebugMacro(<< describeVariable[varName] << " Selected");
      float *rho = new float [nPoints];
      vtkFloatArray *multifluid_rho = NULL;
      multifluid_rho = point2CellCenteredScalar(nip1,njp1,nkp1,rho);
      multifluid_rho->SetName(describeVariable[varName].c_str());
      output->GetPointData()->AddArray(multifluid_rho);
      multifluid_rho->Delete();
      delete [] rho;
      rho = NULL;
    }

    // Density scalar
    ssVarName.str(string());
    ssVarName << "c_" << fluidNumber << "_";
    varName = ssVarName.str();
    if (this->CellArrayStatus[describeVariable[varName]]){
      vtkDebugMacro(<< describeVariable[varName] << " Selected");
      float *c = new float [nPoints];
      vtkFloatArray *multifluid_c = NULL;
      multifluid_c = point2CellCenteredScalar(nip1,njp1,nkp1,c);
      multifluid_c->SetName(describeVariable[varName].c_str());
      output->GetPointData()->AddArray(multifluid_c);
      multifluid_c->Delete();
      delete [] c;
      c = NULL;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Velocity Vector
    ssVarName.str(string());
    ssVarName << "vx_" << fluidNumber << "_";
    string xVarName = ssVarName.str();
    ssVarName.str(string());
    ssVarName << "vy_" << fluidNumber << "_";
    string yVarName = ssVarName.str();
    ssVarName.str(string());
    ssVarName << "vz_" << fluidNumber << "_";
    string zVarName = ssVarName.str();
    if(this->CellArrayStatus[describeVariable[xVarName]]){
      vtkDebugMacro(<< describeVariable[xVarName] << " Selected");
      float *vx = new float [nPoints];
      float *vy = new float [nPoints];
      float *vz = new float [nPoints];
      io->readVariable(xVarName, "", lfmGridInfo, vx);
      io->readVariable(yVarName, "", lfmGridInfo, vy);
      io->readVariable(zVarName, "", lfmGridInfo, vz);
      
      if(vx != NULL && vy != NULL && vz != NULL){
	vtkFloatArray *multifluid_v = NULL;
	multifluid_v = point2CellCenteredVector(nip1,njp1,nkp1, vx,vy,vz);
	multifluid_v->SetName(describeVariable[xVarName].c_str());
	output->GetPointData()->AddArray(multifluid_v);
	multifluid_v->Delete();
	
	delete [] vx;
	vx = NULL;
	delete [] vy;
	vy = NULL;
	delete [] vz;
	vz = NULL;
      }
    }
  }  
    
  //Clean up Memory
  if (X_grid){    delete [] X_grid;    X_grid = NULL;  }
  if (Y_grid){    delete [] Y_grid;    Y_grid = NULL;  }
  if (Z_grid){    delete [] Z_grid;    Z_grid = NULL;  }
    
  io->close();
  if (io){
    delete io;
    io = NULL;
  }
  
  return 1;
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
void vtkLFMReader::addScalarInformation(const std::string &scalarName, const std::string &scalarDescription)
{
  this->describeVariable[scalarName] = scalarDescription;
  
  this->CellArrayName.push_back(scalarDescription);
  this->CellArrayStatus[scalarDescription] = 1;
  vtkDebugMacro(<< scalarName << ": " << scalarDescription);
}

//----------------------------------------------------------------
void vtkLFMReader::addVectorInformation(const std::string &x, const std::string &y, const std::string &z,
					const std::string &vectorDescription)
{
  this->describeVariable[x] = vectorDescription;
  this->describeVariable[y] = vectorDescription;
  this->describeVariable[z] = vectorDescription;
  
  this->CellArrayName.push_back(vectorDescription);
  this->CellArrayStatus[vectorDescription] = 1;
  vtkDebugMacro(<< x << "," << y << "," << z << ": " << vectorDescription);
}
//----------------------------------------------------------------
void vtkLFMReader::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkLFMReader says \"Hello, World!\" " << "\n";
}

//----------------------------------------------------------------

vtkPoints *vtkLFMReader::point2CellCenteredGrid(const int &nip1, const int &njp1, const int &nkp1,
						const float *const X_grid, const float *const Y_grid, const float *const Z_grid)
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
	offset  = index3to1(i,   j,    k,   nip1, njp1);
	oi      = index3to1(i+1, j,    k,   nip1, njp1);
	oj      = index3to1(i,   j+1,  k,   nip1, njp1);
	ok      = index3to1(i,   j,    k+1, nip1, njp1);
	oij     = index3to1(i+1, j+1,  k,   nip1, njp1);
	oik     = index3to1(i+1, j,    k+1, nip1, njp1);
	ojk     = index3to1(i,   j+1,  k+1, nip1, njp1);
	oijk    = index3to1(i+1, j+1,  k+1, nip1, njp1);
        
        xyz[0] = calcCellCenter(X_grid, offset, oi, oj, ok, oij, ojk, oik, oijk);
        xyz[0] /= GRID_SCALE::ScaleFactor[GridScaleType];
        
        xyz[1] = calcCellCenter(Y_grid, offset, oi, oj, ok, oij, ojk, oik, oijk);
        xyz[1] /= GRID_SCALE::ScaleFactor[GridScaleType];
        
        xyz[2] = calcCellCenter(Z_grid, offset, oi, oj, ok, oij, ojk, oik, oijk);
        xyz[2] /= GRID_SCALE::ScaleFactor[GridScaleType];
        
        
	// j+1 because we set data along j=0 in "Fix x-axis singularity", below.
	//        offset = i + (j+1)*ni + k*ni*njp2;
        points->SetPoint(index3to1(i, j+1, k, ni,njp2), xyz);
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
	points->GetPoint(index3to1(i, jAxis, k,  ni,njp2), axisCoord);
	xyz[0] += (float) axisCoord[0];
      }
      xyz[0] /= float( nk );
      for (int k=0; k < nk; k++){
        points->SetPoint(index3to1(i,j,k,  ni,njp2), xyz);
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

vtkFloatArray *vtkLFMReader::point2CellCenteredScalar(const int &nip1, const int &njp1, const int &nkp1,  const float *const data)
{
  return point2CellCenteredVector(nip1,njp1,nkp1, data, NULL, NULL);
}

//----------------------------------------------------------------


vtkFloatArray *vtkLFMReader::point2CellCenteredVector(const int &nip1, const int &njp1, const int &nkp1,
						      const float *const xData, const float *const yData, const float *const zData)
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
					  const float *const X_grid, const float *const Y_grid, const float *const Z_grid,
					  const float *const ei, const float *const ej, const float *const ek,
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
	offset  = index3to1(i,   j,    k,   nip1, njp1);
	oi      = index3to1(i+1, j,    k,   nip1, njp1);
	oj      = index3to1(i,   j+1,  k,   nip1, njp1);
	ok      = index3to1(i,   j,    k+1, nip1, njp1);
	oij     = index3to1(i+1, j+1,  k,   nip1, njp1);
	oik     = index3to1(i+1, j,    k+1, nip1, njp1);
	ojk     = index3to1(i,   j+1,  k+1, nip1, njp1);
	oijk    = index3to1(i+1, j+1,  k+1, nip1, njp1);
	
	// X_grid Cell edge vector
	cx[0] = calcCellWidth(X_grid, oi, oij, oik, oijk, offset, oj, ok, ojk);
	cx[1] = calcCellWidth(X_grid, oj, oij, ojk, oijk, offset, oi, ok, oik);
	cx[2] = calcCellWidth(X_grid, ok, oik, ojk, oijk, offset, oj, oi, oij);

	// Y_grid Cell edge vector       
	cy[0] = calcCellWidth(Y_grid, oi, oij, oik, oijk, offset, oj, ok, ojk);
	cy[1] = calcCellWidth(Y_grid, oj, oij, ojk, oijk, offset, oi, ok, oik);
	cy[2] = calcCellWidth(Y_grid, ok, oik, ojk, oijk, offset, oj, oi, oij);
        
	// Z_grid Cell edge vector
	cz[0] = calcCellWidth(Z_grid, oi, oij, oik, oijk, offset, oj, ok, ojk);
	cz[1] = calcCellWidth(Z_grid, oj, oij, ojk, oijk, offset, oi, ok, oik);
	cz[2] = calcCellWidth(Z_grid, ok, oik, ojk, oijk, offset, oj, oi, oij);
                          
	// Now calculate electric field through cell center
          
	// <ei,ej,ek> = electric field along edge of cells
	// et =  face-centered electric field
	et[0] = calcFaceCenter(ei, offset, ok, oj, ojk);
	et[1] = calcFaceCenter(ej, offset, ok, oi, oik);
	et[2] = calcFaceCenter(ek, offset, oi, oj, oij);
          
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
