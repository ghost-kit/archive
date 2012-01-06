#ifndef __vtkLFMReader_H__
#define __vtkLFMReader_H__

#include "vtkStructuredGridReader.h"
#include "vtkDelimitedTextReader.h"
#include "vtkToolkits.h"
#include "vtkPolyDataAlgorithm.h"
#include <vtkstd/string>
#include <vtkstd/vector>
#include <vtkstd/map>
#include "Hdf4.h"

  //simple macros to point to dimensions
#define NI this->dims["x"]-1
#define NJ this->dims["y"]-1
#define NK this->dims["z"]-1

  //Calculates the FORTRAN offset a grid
  //WARNING: the Variable ni and nj MUST be defined in the local domain to
  //    use this macro
#define gridOffset(i,j,k) i + (j)*(ni+1) + (k)*(ni+1)*(nj+1)

  //Calculates the corrent VTK Array offset
  //WARNING: the Variable ni and nj MUST be defined in the local domain to
  //    use this macro
#define ArrayOffset(i,j,k) (i) + (j)*(ni) + (k)*(ni)*(nj+2)


  //Electric Field Macros
#define cellWallAverage(array, o1, o2, o3, o4)\
           ((array[o1] + array[o2] + array[o3] + array[o4]))/4.0

#define cell_AxisAverage(array, o1,o2, o3, o4, m1, m2, m3, m4)\
           ((array[o1]  +  array[o2] +  array[o3] + array[o4]) - \
            (array[m1]  +  array[m2] +  array[m3] + array[m4]))/4.0

  //This Macro calculates the cell centered value for the 8 points at the corners
  //  This macro works with, but does not require the setCellGridPointOffsetMacro.
  //  Use the offset macro if you are using Fortran Arrays in C/C++
#define cell8PointAverage(array, o1, o2, o3, o4, o5, o6, o7, o8) \
        ((array[o1]  +  array[o2]  +  array[o3]  +  array[o4]  + \
          array[o5]  +  array[o6]  +  array[o7]  +  array[o8])/8.0)

  //This Macro sets the values of offset, oi, oj, ok, oij, oik, ojk, oijk
  //  for when you need access to the points at the corners of the cell.
  //  The Above Mentioned variables MUST exist before calling this macro.
  //  THIS IS FOR FORTRAN OFFSETS IN C/C++ Code ONLY
#define setFortranCellGridPointOffsetMacro         \
      offset  = gridOffset(i,   j,    k);   \
      oi      = gridOffset(i+1, j,    k);   \
      oj      = gridOffset(i,   j+1,  k);   \
      ok      = gridOffset(i,   j,    k+1); \
      oij     = gridOffset(i+1, j+1,  k);   \
      oik     = gridOffset(i+1, j,    k+1); \
      ojk     = gridOffset(i,   j+1,  k+1); \
      oijk    = gridOffset(i+1, j+1,  k+1); 

namespace GRID_SCALE
{
  enum ScaleType{
    NONE   = 0,
    REARTH = 1,
    RSOLAR = 2,
    AU     = 3   
  };
  static const float ScaleFactor[4] = { 1.0,
    6.5e8,
    6.955e10,
    1.5e13 };
}

class VTK_EXPORT vtkLFMReader : public vtkStructuredGridReader
{
public:
  static vtkLFMReader *New();
  vtkTypeMacro(vtkLFMReader, vtkStructuredGridReader);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  /**
   * This method allows you to specify the name of the data file to be
   * loaded by your reader. The method is not required to have this
   * exact name, but a method with this functionality must be
   * implemented. The easiest way to implement SetFileName is with a
   * vtkSetStringMacro in the header file for this class. (There is
   * also an associated vtkGetStringMacro for implementing
   * GetFileName.) This method handles allocating an array to contain
   * the file name and lets the reader know that the pipeline should
   * be updated when the name is changed.
   *
   * vtkSetStringMacro(FileName); When using this macro, you must also
   * add a FileName instance variable of type char* in the protected
   * section of this class. In the constructor for your reader, assign
   * FileName the value NULL before you use SetFileName for the first
   * time. In the destructor for your reader, call SetFileName(0)to
   * free the file name storage.
   */
  vtkSetStringMacro(HdfFileName);
  virtual void SetFileName(const char *fileName) { this->SetHdfFileName(fileName); }
  vtkGetStringMacro(HdfFileName);
  virtual char *GetFileName() { return this->GetHdfFileName(); }
  
  vtkSetMacro(GridScaleType, int);
  vtkGetMacro(GridScaleType, int);
  
  
  /*
   * routines for Cell Array Info
   */
  
  vtkGetMacro(NumberOfPointArrays, int);
  vtkSetMacro(NumberOfPointArrays, int);
  
  vtkGetMacro(NumberOfCellArrays, int);
  vtkSetMacro(NumberOfCellArrays, int);
  
  
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
  
  
  
    // Description:
    // Get/Set whether the point or cell array with the given name is to
    // be read.
  int GetPointArrayStatus(const char* name);
  int GetCellArrayStatus(const char* name);
  void SetPointArrayStatus(const char* name, int status);  
  void SetCellArrayStatus(const char* name, int status);  
  
  
  
protected:
  vtkLFMReader();
  ~vtkLFMReader();
  
  char *HdfFileName;
  int GridScaleType;
  
    //THESE VALUES are needed for dealing with Time Steps
    //  Added TimeStep Variables:
    //    1) NumberOfTimeSteps
    //    2) TimeStepValues
    //
    //  The BTX and ETX comments must encompase stl calls when in a header file
    // Added by Joshua Murphy 1 DEC 2011
  int NumberOfTimeSteps;
  
    //BTX    
  vtkstd::vector<double> TimeStepValues;
  
    //Map of variable name to Description String
  vtkstd::map<vtkstd::string, vtkstd::string> ArrayNameLookup;
  
    //Point and Cell Array Status Information
  vtkstd::vector<vtkstd::string> CellArrayName;
  vtkstd::vector<vtkstd::string> PointArrayName;
  
  vtkstd::map<vtkstd::string,int> CellArrayStatus;
  vtkstd::map<vtkstd::string,int> PointArrayStatus;
    //ETX
  
  
    // The number of point/cell data arrays in the output.  Valid after
    // SetupOutputData has been called.
  int NumberOfPointArrays;
  int NumberOfCellArrays; 
  
    // helper values to clean up code
    //BTX
    //keep track of the master dimensions of the arrays.
  vtkstd::map<vtkstd::string, int> dims;
    //ETX
  
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
  
    //BTX
    //These methods will add an ARRAY to the available list if the its associated
    //  variables exist within the file.
    //  VarDescription will be indexed on VarName or (xVar & yVar & zVar)
    //  Can only be used to add existing variables in the file.
    //  if existence query fails, NOTHING happens
  void SetIfExists(Hdf4 &filePointer, vtkstd::string VarName, vtkstd::string VarDescription);
  void SetIfExists(Hdf4 &filePointer, vtkstd::string xVar, vtkstd::string yVar, vtkstd::string zVar, vtkstd::string VarDescription);
  
    //these methods will add an ARRAY to the available list indexed at "ArrayIndexName" with the 
    //  description value of "VarDescription".  This can be used to add dirived quantities to the system.
    //  
    //  If existence querry fails, NOTHING happens.
  void SetNewIfExists(Hdf4 &filePointer, vtkstd::string VarName, vtkstd::string ArrayIndexName, vtkstd::string VarDescription);
  void SetNewIfExists(Hdf4 &filePointer, vtkstd::string xVar, vtkstd::string yVar, vtkstd::string zVar, vtkstd::string ArrayIndexName,  vtkstd::string VarDescription);
  
  vtkstd::string GetDesc(vtkstd::string varName)
  { return this->ArrayNameLookup[varName];}
  
    //ETX

    //Helper Functions  
    //--------------------------------------------------------------------
  
  float p_dot(float *x, float *y)  {return x[0]*y[0] + x[1]*y[1] + x[2]*y[2];}
  
    //--------------------------------------------------------------------
  
  void p_cross3(float *x, float *y, float *z){
    z[0] = x[1]*y[2] - x[2]*y[1];
    z[1] = x[2]*y[0] - x[0]*y[2];
    z[2] = x[0]*y[1] - x[1]*y[0];
  }
  
    //--------------------------------------------------------------------
  
  float p_tripple(float *x, float *y, float *z){
    float dum[3];
    p_cross3(y,z,dum);
    return p_dot(x,dum);
  }
  
    //--------------------------------------------------------------------

private:
  vtkLFMReader(const vtkLFMReader&); // Not implemented
  void operator=(const vtkLFMReader&); // Not implemented
};

#endif
