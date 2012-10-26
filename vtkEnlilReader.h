#ifndef vtkEnlilReader_H
#define vtkEnlilReader_H


#include "vtkStructuredGridAlgorithm.h"
#include "cxform.h"
#include<vtkstd/map>
#include<vtkstd/string>
#include<vtkstd/vector>


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
class VTK_PARALLEL_EXPORT vtkEnlilReader : public vtkStructuredGridAlgorithm
{

public:
  static vtkEnlilReader* New();

  vtkTypeMacro(vtkEnlilReader, vtkStructuredGridAlgorithm)
  void PrintSelf(ostream &os, vtkIndent indent);

  // Set/get macros
  void SetGridScaleType(int value)
  {
    this->GridScaleType = value;
    this->gridClean = false;
    this->Modified();
  }

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

  void AddFileName(const char* fname);
  void RemoveAllFileNames();
  unsigned int GetNumberOfFileNames();
  const char *GetFileName(unsigned int idx);

  vtkGetStringMacro(CurrentFileName);


protected:

  vtkEnlilReader();
  ~vtkEnlilReader();

  char* FileName;            // Base file name
  int GridScaleType;
  bool gridClean;
  int numberOfArrays;

  // Extent information
  vtkIdType NumberOfTuples;  // Number of tuples in subextent

  // Field
  int WholeExtent[6];       // Extents of entire grid
  int SubExtent[6];         // Processor grid extent
  int UpdateExtent[6];
  int Dimension[3];         // Size of entire grid
  int SubDimension[3];      // Size of processor grid

  // Check to see if info is clean
  bool infoClean;

  //Data interface information
  vtkPoints* Points;        // Structured grid geometry
  vtkDoubleArray* Radius;   // Radius Grid Data

  //BTX
  vtkstd::vector<vtkstd::string> MetaDataNames;
  vtkstd::map<vtkstd::string, vtkstd::string> ScalarVariableMap;
  vtkstd::map<vtkstd::string, vtkstd::vector<vtkstd::string> > VectorVariableMap;
  vtkstd::vector<vtkstd::vector<double> > sphericalGridCoords;

  vtkstd::string dateString;
  vtkstd::vector<vtkstd::string> fileNames;

  vtkstd::map<double,vtkstd::string> time2fileMap;
  //ETX

  // Time step information
  int NumberOfTimeSteps;    // Number of time steps
  double* TimeSteps;        // Actual times available for request
  double timeRange[2];
  double physicalTime;
  bool timesCalulated;

  char* CurrentFileName;
  void SetCurrentFileName(const char* fname);

  // Selected field of interest
  vtkDataArraySelection* PointDataArraySelection;
  vtkDataArraySelection* CellDataArraySelection;

  // Observer to modify this object when array selections are modified
  vtkCallbackCommand* SelectionObserver;

  // Load a variable from data file

  int GenerateGrid();
  int LoadVariableData(vtkInformationVector *outputVector);
  int LoadArrayValues(vtkstd::string array, vtkInformationVector* outputVector);

  void PopulateGridData();

  int getSerialNumber()
  {
    static int number = 0;
    return ++number;
  }

 inline void clearString(char* string, int size)
  {
    for(int x = 0; x < size; x++)
      {
        string[x] = '\0';
      }
  }


  // Request Information Helpers
  double getRequestedTime(vtkInformationVector *outputVector);
  int PopulateArrays();
  int LoadMetaData(vtkInformationVector* outputVector);
  int calculateTimeSteps();
  int checkStatus(void* Object, char* name);

  double* read3dPartialToArray(char *array, int extents[]);
  double* readGridPartialToArray(char *arrayName, int subExtents[], bool periodic);
  void loadArrayMetaData(const char *array,
                         const char *title,
                         vtkInformationVector* outputVector,
                         bool vector = false);


  void addPointArray(char* name);
  void addPointArray(char* name1, char* name2, char* name3);
  void extractDimensions(int dims[], int extent[]);
  void setMyExtents(int extentToSet[], int sourceExtent[]);
  void setMyExtents(int extentToSet[], int dim1, int dim2, int dim3, int dim4, int dim5, int dim6);
  void printExtents(int extent[], char* description);

  bool eq(int extent1[], int extent2[]);
  bool ExtentOutOfBounds(int extToCheck[], int extStandard[]);

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

  virtual int FillOutputPortInformation(int, vtkInformation*);


private:
  vtkEnlilReader(const vtkEnlilReader&);  // Not implemented.
  void operator=(const vtkEnlilReader&);  // Not implemented.
};



#endif // vtkEnlilReader_H
