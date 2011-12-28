#ifndef __vtkLFMReader_H__
#define __vtkLFMReader_H__

#include "vtkStructuredGridReader.h"
#include "vtkDelimitedTextReader.h"
#include "vtkToolkits.h"
#include "vtkPolyDataAlgorithm.h"
#include <vtkstd/string>
#include <vtkstd/vector>
#include <vtkstd/map>

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
  
//  vtkSetMacro(CellArrayName, vtkstd::vector<vtkstd::string>);
//  vtkGetMacro(CellArrayName, vtkstd::vector<vtkstd::string>);
  
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
//  int GetPointArrayStatus(const char* name);
  int GetCellArrayStatus(const char* name);
//  void SetPointArrayStatus(const char* name, int status);  
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
  
  
  
private:
  vtkLFMReader(const vtkLFMReader&); // Not implemented
  void operator=(const vtkLFMReader&); // Not implemented
};

#endif
