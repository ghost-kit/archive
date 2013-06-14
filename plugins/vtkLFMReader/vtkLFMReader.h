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
    REARTH = 1, // Earth Radius
    RSOLAR = 2, // Solar Radius
    AU     = 3  // Astronomical Unit
  };
  static const float ScaleFactor[4] = { 1.0,      // NONE
					6.38e8,   // REARTH
					6.955e10, // RSOLAR
					1.5e13 }; // AU
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
    
  /** The following methods keep track of which arrays the user has
   * set ParaView to read.
   *
   * Calls to these routines are only valid after HdfFileName is set & UpdateInformation has been called.
   */
  // @{  

  /** For a given cell-centered variable (ie. "Velocity Vector",
   *  "Plasma Density", etc), return whether it should be read (1) or
   *  not (0).
   *
   * \note must match properties in vtkLFMReader.xml
   */
  //@{
  int GetCellArrayStatus(const char* name);
  void SetCellArrayStatus(const char* name, int status);  
  //@}

  /** For a given point-centered variable, return whether it should be
   *  read (1) or not (0).
   *
   * \note must match properties in vtkLFMReader.xml
   */
  //@{
  int GetPointArrayStatus(const char* name);
  void SetPointArrayStatus(const char* name, int status);  
  //@}

  ///  How many point & cell-centered arrays are in the file?
  //@{
  int GetNumberOfCellArrays() { return CellArrayName.size(); }  
  int GetNumberOfPointArrays() { return PointArrayName.size(); }
  //@}

  /** Returns the name of variable at index (ie. GetCellArrayName(0) == "Plasma Density")
   * 
   * \FIXME: What if index is invalid?
   */
  const char* GetCellArrayName(int index) { return this->CellArrayName[index].c_str(); }
  // @}
  
protected:
  vtkLFMReader();
  ~vtkLFMReader();
  
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
       
private:  
  char *HdfFileName;
  /// \see GRID_SCALE::ScaleType
  int GridScaleType;

  /// TimeStepValues must match property in vtkLFMReader.xml
  std::vector<double> TimeStepValues;
    
  //keep track of the master dimensions of the arrays.
  std::vector<int> globalDims;
 
  /** Map short variable name to something more descriptive 
   * For example:
   *    describeVariable["vx_"]  == "Velocity Vector"
   *    describeVariable["vy_"]  == "Velocity Vector"
   *    describeVariable["vz_"]  == "Velocity Vector"
   *    describeVariable["rho_"] == "Plasma Density"
   *    ...
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

  vtkLFMReader(const vtkLFMReader&); // Not implemented
  void operator=(const vtkLFMReader&); // Not implemented

  /** Add variable to list of available variables in the data set.
   * 
   * Sets describeVariable, CellArrayName, CellArrayStatus   
   */
  //@{
  void addScalarInformation(const std::string &scalarName, const std::string &scalarDescription);
  void addVectorInformation(const std::string &x, const std::string &y, const std::string &z,
			    const std::string &vectorDescription);
  //@}

  /**
   * The following must be surrouned by //BTX ... //ETX because it
   * references vtk data types.  If these tags are omitted, building
   * the plugin will fail with an error message like the following: ***
   * SYNTAX ERROR found in parsing the header file <something>.h before
   * line <line number> ***
   */
  //BTX
  //@{
  /** Given native (x,y,z) grid points at cell edges, calculate
   * cell-centers.
   *  
   * Grid points for the LFM grid are at cell edges, while data is
   * stored at cell centers.  It has been determined that
   * visualization packages like ParaView generate the best images
   * when we visualize data as point-centered on cell centered grid.
   * 
   * This creates a singularities in the grid:
   *   - on the x-axis 
   *   - at j=0 in the nose and j=nj+2 in the tail
   *   - periodic point for k-index
   * Because of this, the grid returned is dimensioned (ni, nj+2, nk+1)
   */
  vtkPoints *point2CellCenteredGrid(const int &nip1, const int &njp1, const int &nkp1,
				    const float *const X_grid, const float *const Y_grid, const float *const Z_grid);

  /**
   * Returns scalar on grid at cell centers.  This will become a
   * point-centered grid.
   *
   * See point2CellCenteredGrid for dimensioning information.
   */
  vtkFloatArray *point2CellCenteredScalar(const int &nip1, const int &njp1, const int &nkp1,  const float *const data);


  /**
   * Returns scalar on grid at cell centers.  This will become a
   * point-centered grid.
   *
   * See point2CellCenteredGrid for dimensioning information.
   */
  vtkFloatArray *point2CellCenteredVector(const int &nip1, const int &njp1, const int &nkp1,
				    const float *const xData, const float *const yData, const float *const zData);
  //}@
  //ETX

  /// Methods to calculate electric field.
  //@{
  /**
   * Calculate cell-centered electric field.
   *
   * ei,ej,ek = electric field on cell edges
   * ex,ey,ez = electric field on cell centers
   */
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
  
  /** Returns average value of hexahedron cell from 8 corner points.
   * o[1-8] are indices to 1-d array representing 8 corners of a cell.
   */
  inline float calcCellCenter(const float *const array, 
			      const int &o1, const int &o2, const int &o3, const int &o4, 
			      const int &o5, const int &o6, const int &o7, const int &o8) 
  {
  return ((array[o1]  +  array[o2]  +  array[o3]  +  array[o4]  +
           array[o5]  +  array[o6]  +  array[o7]  +  array[o8])/8.0);
  }
 
  /** Returns average value of array at four corners of a cell
   *  o1,o2,o3,o4 = index to 1d array representing four edges of a
   *  cell face.
   */
  inline float calcFaceCenter(const float *const array, 
			      const int &o1, const int &o2, const int &o3, const int &o4)
  {
  return ((array[o1] + array[o2] + array[o3] + array[o4]))/4.0;
  }
  
  /** Returns vector component through cell center that stores the
   *  width of hexahedron cell between opposite faces.
   *
   * o1,o2,o3,o4: four indices that make up face on cell
   * m1,m2,m3,m4: four indices that make up opposite face on cell
   */
  inline float calcCellWidth(const float *const array, 
			     const int &o1,const int &o2, const int &o3, const int &o4, 
			     const int &m1, const int &m2, const int &m3, const int &m4)
  {
  return ((array[o1]  +  array[o2] +  array[o3] + array[o4]) - 
          (array[m1]  +  array[m2] +  array[m3] + array[m4]))/4.0;
  }

  /**
   * Given an (i,j,k) index, calculate the offset to an unrolled 1d
   * array.  Assumes 3d ni x nj x nk array has i index moving fastest.
   */  
  inline int index3to1(const int &i, const int &j, const int &k,
		       const int &ni, const int &nj)
  {
    return ( i + j*ni + k*ni*nj );
  }
  ///@}
};

#endif
