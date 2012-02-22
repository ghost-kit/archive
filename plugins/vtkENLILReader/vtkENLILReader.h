#ifndef __vtkENLILReader_H__
#define __vtkENLILReader_H__

#include "vtkStructuredGridReader.h"
#include "vtkDelimitedTextReader.h"
#include "vtkInformationVector.h"
#include "vtkPoints.h"

#include "vtkPointData.h"
#include <sys/types.h>
#include <vtkstd/string>
#include <vtkstd/vector>
#include <vtkstd/map>

//Calculates the corrent VTK Array offset
//WARNING: the Variable ni and nj MUST be defined in the local domain to
//    use this macro
#define ArrayOffset(i,j,k) i+ni*(j+nj*k)
#define gridOffset(i,j,k) i+ni*(j+nj*k)


namespace GRID_SCALE
{
  enum ScaleType{
    NONE   = 0,
    REARTH = 1,
    RSOLAR = 2,
    AU     = 3   
  };
  static const float ScaleFactor[4] = { 1.0,
					6.5e6,
					6.955e8,
					1.5e11 };
}

class VTK_EXPORT vtkENLILReader : public vtkStructuredGridReader
{
 public:
  static vtkENLILReader *New();
  vtkTypeMacro(vtkENLILReader, vtkStructuredGridReader);

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
  vtkSetStringMacro(EnlilFileName);
  virtual void SetFileName(const char *fileName) { this->SetEnlilFileName(fileName); }
  vtkGetStringMacro(EnlilFileName);
  virtual char *GetFileName() { return this->GetEnlilFileName(); }

//  vtkSetMacro(GridScaleType, int);
  void SetGridScaleType(int set)
  {
      //set the value
      this->GridScaleType = set;

      //need to mark as modified
      this->Modified();
  }

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
  //int GetNumberOfCellArrays();
   const char* GetCellArrayName(int index);
   const char* GetPointArrayName(int index);

  // Description:
  // Get/Set whether the point or cell array with the given name is to
  // be read.
int GetPointArrayStatus(const char* name);
int GetCellArrayStatus(const char* name);

//BTX
int GetCellArrayStatus(vtkstd::string);
int GetPointArrayStatus(vtkstd::string);


//ETX
void SetPointArrayStatus(const char* name, int status);
void SetCellArrayStatus(const char* name, int status);


 protected:
  vtkENLILReader();
  ~vtkENLILReader();

  char *EnlilFileName;
  int GridScaleType;
  int64_t numberOfPoints;
  int64_t dimR;
  int64_t dimTheta;
  int64_t dimPhi;

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
  void SetArrayName(vtkstd::string VarName, vtkstd::string VarDescription);
  void SetArrayName(vtkstd::string xVar, vtkstd::string yVar, vtkstd::string zVar, vtkstd::string VarDescription);


    //ETX





  void clearString(char* string, int size)
  {
    for(int x = 0; x < size; x++)
      {
        string[x] = '\0';
      }
  }


  vtkstd::string GetDesc(vtkstd::string varName)
  { return this->ArrayNameLookup[varName];}


private:
  vtkENLILReader(const vtkENLILReader&); // Not implemented
  void operator=(const vtkENLILReader&); // Not implemented
};

#endif
