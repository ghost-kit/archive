#ifndef __vtkLFMReader_H__
#define __vtkLFMReader_H__

#include "vtkStructuredGridReader.h"
#include "vtkDelimitedTextReader.h"
#include "vtkToolkits.h"
#include "vtkPolyDataAlgorithm.h"
#include "vtkFloatArray.h"

#include <vtksys/stl/string>
#include <vtksys/stl/vector>
#include <vtksys/stl/map>
#include "DeprecatedHdf4.h"

namespace GRID_SCALE
{
  enum ScaleType{
    NONE   = 0,
    REARTH = 1,
    RSOLAR = 2,
    AU     = 3   
  };
  static const float ScaleFactor[4] = { 1.0,
    6.38e8,
    6.955e10,
    1.5e13 };
}

class VTK_EXPORT vtkLFMReader : public vtkStructuredGridReader
{
public:
  static vtkLFMReader *New();
  vtkTypeMacro(vtkLFMReader, vtkStructuredGridReader);
  void PrintSelf(ostream& os, vtkIndent indent);

  
  vtkSetStringMacro(HdfFileName);
  /// SetFileName must match property in vtkLFMReader.xml
  virtual void SetFileName(const char *fileName) { this->SetHdfFileName(fileName); }
  vtkGetStringMacro(HdfFileName);
  virtual char *GetFileName() { return this->GetHdfFileName(); }

  /// SetGridScaleType must match property in vtkLFMReader.xml
  vtkSetMacro(GridScaleType, int);
  vtkGetMacro(GridScaleType, int);
    
  /// routines for Cell Array Info
  int GetNumberOfPointArrays() { return PointArrayName.size(); }
  int GetNumberOfCellArrays() { return CellArrayName.size(); }  
  
  /**
   * The purpose of this method is to determine whether
   * this reader can read a specified data file. Its input parameter
   * is a const char* specifying the name of a data file. In this
   * method you should not actually read the data but determine
   * whether it is the correct format to be read by this reader. This
   * method should return an integer value: 1 indicates that the
   * specified file is of the correct type; 0 indicates it is not. It
   * is not absolutely required that this method be implemented, but
   * ParaView will make use of it if it exists.
   */
  static int CanReadFile(const char *filename);
  
  /**
   * Get information about cell-based arrays.  As is typical with
   * readers this in only valid after the filename is set and
   * UpdateInformation() has been called.
   */
    //    int GetNumberOfCellArrays();
  const char* GetCellArrayName(int index);
  
  

  /** Methods to keep track of which arrays the user has set ParaView to read.
   *
   * \note these function names must match properties in vtkLFMReader.xml!
   */
  // @{
  int GetCellArrayStatus(const char* name);
  void SetCellArrayStatus(const char* name, int status);  

  int GetPointArrayStatus(const char* name);
  void SetPointArrayStatus(const char* name, int status);  
  // @}
  
protected:
  vtkLFMReader();
  ~vtkLFMReader();
  
  char *HdfFileName;
  /// \see GRID_SCALE::ScaleType
  int GridScaleType;

  /// TimeStepValues must match property in vtkLFMReader.xml
  std::vector<double> TimeStepValues;
  
  /** Map short variable name to something more descriptive 
   * For example:
   *    describeVariable["vx_"]  == "Velocity Vector"
   *    describeVariable["vy_"]  == "Velocity Vector"
   *    describeVariable["vz_"]  == "Velocity Vector"
   *    describeVariable["rho_"] == "Plasma Density"
   */
  std::map<std::string, std::string> describeVariable;

  /** Stores names of all variables that can be read into
   *  ParaView. These values are displayed on the GUI for variable
   *  name & on colorbar
   *
   * eg. "Plasma Density", "Velocity Vectory", etc.
   */
  std::vector<std::string> CellArrayName;
  std::vector<std::string> PointArrayName;
  
  /** bit to decide whether or not variable is enabled & should be read into ParaView.
   *   == 0: Variable disabled. Skip.
   *   == 1: Variable enabled. Load into ParaView
   */
  std::map<std::string,int> CellArrayStatus;
  std::map<std::string,int> PointArrayStatus;
  
  // helper values to clean up code
  //keep track of the master dimensions of the arrays.
  std::map<std::string, int> dims;
 
  /**
   * This method is invoked by the superclass's ProcessRequest
   * implementation when it receives a REQUEST_INFORMATION request. In
   * the output port, it should store information about the data in the
   * input file. For example, if the reader produces structured data,
   * then the whole extent should be set here.
   *
   * This method is necessary when configuring a reader to operate in
   * parallel. (This will be further discussed later in this chapter.)
   * It should be placed in the protected section of the reader
   * class. The method should return 1 for success and 0 for failure.
   */
  int RequestInformation(vtkInformation*,vtkInformationVector**,vtkInformationVector* outVec);
  
  
  /**
   * This method is invoked by the superclass's ProcessRequest
   * implementation when it receives a REQUEST_DATA request. It should
   * read data from the file and store it in the corresponding data
   * object in the output port. The output data object will have already
   * been created by the pipeline before this request is made. The
   * amount of data to read may be specified by keys in the output port
   * information. 
   *
   * The method should be placed in the protected section of the reader
   * class. It should return 1 for success and 0 for failure.
   */
  int RequestData(vtkInformation*,vtkInformationVector**,vtkInformationVector* outVec);
  
  /**
   *  This function will check to see if the variable (VarName) exists, and if it does, set
   *    associated variables to correct values.
   *    
   *    This Method does NOTHING if the variable does not exist.
   */
  
    //These methods will add an ARRAY to the available list if the its associated
    //  variables exist within the file.
    //  VarDescription will be indexed on VarName or (xVar & yVar & zVar)
    //  Can only be used to add existing variables in the file.
    //  if existence query fails, NOTHING happens
  void SetIfExists(DeprecatedHdf4 &filePointer, std::string VarName, std::string VarDescription);
  void SetIfExists(DeprecatedHdf4 &filePointer, std::string xVar, std::string yVar, std::string zVar, std::string VarDescription);
     
private:  
  vtkLFMReader(const vtkLFMReader&); // Not implemented
  void operator=(const vtkLFMReader&); // Not implemented

  /**
   * The following must be surrouned by //BTX ... //ETX because it
   * references vtk data types.  If these tags are omitted, building
   * the plugin will fail with an error message like the following: ***
   * SYNTAX ERROR found in parsing the header file <something>.h before
   * line <line number> ***
   */
  //BTX
  //@{
  vtkPoints *point2CellCenteredGrid(const int &nip1, const int &njp1, const int &nkp1,
				    const float *const X_grid, const float *const Y_grid, const float *const Z_grid);


  vtkFloatArray *point2CellCenteredScalar(const int &nip1, const int &njp1, const int &nkp1,  const float *const data);


  vtkFloatArray *point2CellCenteredVector(const int &nip1, const int &njp1, const int &nkp1,
				    const float *const xData, const float *const yData, const float *const zData);
  //}@
  //ETX

  /// Methods to calculate electric field.
  //@{
  void calculateElectricField(const int &nip1, const int &njp1, const int &nkp1,
			      const float *const X_grid, const float *const Y_grid, const float *const Z_grid,
			      const float *const ei, const float *const ej, const float *const ek,
			      float *ex, float *ey, float *ez);


  /// \returns dot product (x.y) for 3d vectors x & y.
  float p3d_dot(const float *const x, const float *const y)  {return x[0]*y[0] + x[1]*y[1] + x[2]*y[2];}
  /// z = cross(x,y) cross prodcut for 3d vectors x & y.
  void p3d_cross(const float *const x, const float *const y, float *const z){
    z[0] = x[1]*y[2] - x[2]*y[1];
    z[1] = x[2]*y[0] - x[0]*y[2];
    z[2] = x[0]*y[1] - x[1]*y[0];
  }
  /// Scalar triple product: (x . (y x z)) in 3d.
  float p3d_triple(float *x, float *y, float *z){
    float dum[3];
    p3d_cross(y,z,dum);
    return p3d_dot(x,dum);
  }
  
  /// Average array values at 8 different indicies.
  inline float cell8PointAverage(const float *const array, 
				 const int &o1, const int &o2, const int &o3, const int &o4, const int &o5, const int &o6, const int &o7, const int &o8) 
  {
  return ((array[o1]  +  array[o2]  +  array[o3]  +  array[o4]  +
           array[o5]  +  array[o6]  +  array[o7]  +  array[o8])/8.0);
  }
 
  /// Average array values at 4 different indices
  inline float cellWallAverage(const float *const array, const int &o1, const int &o2, const int &o3, const int &o4)
  {
  return ((array[o1] + array[o2] + array[o3] + array[o4]))/4.0;
  }
  
  inline float cell_AxisAverage(const float *const array, const int &o1,const int &o2, const int &o3, const int &o4, const int &m1, const int &m2, const int &m3, const int &m4)
  {
  return ((array[o1]  +  array[o2] +  array[o3] + array[o4]) - 
          (array[m1]  +  array[m2] +  array[m3] + array[m4]))/4.0;
  }

  /**
   * \brief Given an (i,j,k) index, calculate the offset to an unrolled 1d array.
   * Assumes 3d ni x nj x nk array has i index moving fastest.
   */  
  inline int index3to1(const int &i, const int &j, const int &k,
		       const int &ni, const int &nj)
  {
    return ( i + j*ni + k*ni*nj );
  }

  ///@}
};

#endif
