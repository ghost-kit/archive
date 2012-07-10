#ifndef VTKGENERICREADER_H
#define VTKGENERICREADER_H


#include "vtkStructuredGridAlgorithm.h"
#include "vtknetcdf/cxx/netcdfcpp.h"

#define CALL_NETCDF(call)\
{\
  int errorcode = call;\
  if(errorcode != NC_NOERR)\
{\
  vtkErrorMacro(<< "netCDF Error: " << nc_strerror(errorcode));\
  return 0;\
  }\
  }

#define CALL_NETCDF_NO_FEEDBACK(call)\
{\
  int errorcode = call;\
  if(errorcode != NC_NOERR)\
{\
  return 0;\
  }\
  }


class vtkDataArraySelection;
class vtkCallbackCommand;
class vtkMultibockProcessController;
class vtkStringArray;
class vtkFloatArray;
class vtkDoubleArray;
class vtkIntArray;
class vtkPoints;
class vtkTable;
class vtkStructuredGrid;
class vtkUnstructuredGrid;
class vtkMultiBlockDataSetAlgorithm;
class vtkStructuredGridAlgorithm;



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


/** READER PRIME **/
class VTK_PARALLEL_EXPORT vtkGenericReader : public vtkStructuredGridAlgorithm
{

public:
  static vtkGenericReader* New();

  vtkTypeMacro(vtkGenericReader, vtkStructuredGridAlgorithm)
  void PrintSelf(ostream &os, vtkIndent indent);

  // Set/get macros
  vtkSetMacro(GridScaleType, int)
  vtkGetMacro(GridScaleType, int)

  vtkSetStringMacro(FileName)
  vtkGetStringMacro(FileName)

  vtkSetVector6Macro(WholeExtent, int)
  vtkGetVector6Macro(WholeExtent, int)

  vtkSetVector6Macro(SubExtent, int)
  vtkGetVector6Macro(SubExtent, int)

  // Description:
  // The following methods allow selective reading of solutions fields.
  int GetNumberOfPointArrays();
  int GetNumberOfCellArrays();

  const char* GetPointArrayName(int index);
  const char* GetCellArrayName(int index);

  int  GetPointArrayStatus(const char* name);
  int  GetCellArrayStatus(const char* name);

  void SetPointArrayStatus(const char* name, int status);
  void SetCellArrayStatus(const char* name, int status);

  void DisableAllPointArrays();
  void DisableAllCellArrays();

  void EnableAllPointArrays();
  void EnableAllCellArrays();

  // Description:
  // We intercept the requests to check for which port
  // information is being requested for and if there is
  // a REQUEST_DATA_NOT_GENERATED request then we mark
  // which ports won't have data generated for that request.
  virtual int ProcessRequest(vtkInformation *request,
                             vtkInformationVector **inInfo,
                             vtkInformationVector *outInfo);

protected:

  vtkGenericReader();
  ~vtkGenericReader();

  char* FileName;            // Base file name
  int GridScaleType;

  // Extent information
  vtkIdType NumberOfTuples;  // Number of tuples in subextent

  // Field
  int WholeExtent[6];       // Extents of entire grid
  int SubExtent[6];         // Processor grid extent
  int UpdateExtent[6];
  int Dimension[3];         // Size of entire grid
  int SubDimension[3];      // Size of processor grid

  //Data interface information
  vtkPoints* Points;        // Structured grid geometry
  vtkStructuredGrid* Data;  // Structured Grid Data

  // Time step information
  int NumberOfTimeSteps;    // Number of time steps
  double* TimeSteps;        // Actual times available for request

  // Selected field of interest
  vtkDataArraySelection* PointDataArraySelection;
  vtkDataArraySelection* CellDataArraySelection;

  // Observer to modify this object when array selections are modified
  vtkCallbackCommand* SelectionObserver;

  // Load a variable from data file
  int GenerateGrid();
  int LoadVariableData(vtkInformationVector *outputVector);
  int LoadGridValues(const char *array);


  // Request Information Helpers
  int PopulateArrays();
  int PopulateMetaData(vtkInformationVector* outputVector);
  int PopulateDataInformation();
  int checkStatus(vtkObject* Object, char* name);
  void printWholeExtents();

  // Required Paraview Functions
  static int CanReadFile(const char* filename);

  virtual int RequestData(
      vtkInformation* request,
      vtkInformationVector** inputVector,
      vtkInformationVector* outputVector);

  virtual int RequestInformation(
      vtkInformation* request,
      vtkInformationVector** inputVector,
      vtkInformationVector* outputVector);

  static void SelectionCallback(
      vtkObject *caller,
      unsigned long eid,
      void *clientdata,
      void *calldata);

  static void EventCallback(
      vtkObject* caller,
      unsigned long eid,
      void* clientdata, void* calldata);

  virtual int FillOutputPortInformation(int, vtkInformation*);


private:
  vtkGenericReader(const vtkGenericReader&);  // Not implemented.
  void operator=(const vtkGenericReader&);  // Not implemented.
};



#endif // VTKGENERICREADER_H
