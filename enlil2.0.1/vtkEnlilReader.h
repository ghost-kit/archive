#ifndef vtkEnlilReader_H
#define vtkEnlilReader_H


#include "vtkStructuredGridAlgorithm.h"
#include "vtkIOParallelNetCDFModule.h" // For export macro
//#include "cxform.h"
#include "readerCache.h"
#include "vtkSmartPointer.h"
#include<map>
#include<string>
#include<vector>
#include<QString>


class vtkDataArraySelection;
class vtkCallbackCommand;
class vtkMultibockProcessController;
class vtkStringArray;
class vtkFloatArray;
class vtkFloatArray;
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

namespace UNITS
{
static const double emu = 1.67262158e-27;
static const double km2cm = 1e6;
static const double km2m = 1e3;
}

namespace DATA_TYPE
{
enum dataType{
    PDENSITY = 0,
    CDENSITY = 1,
    TEMP = 2,
    POLARITY = 3,
    BFIELD = 4,
    VELOCITY = 5
};


}


/** READER PRIME **/
class VTKIOPARALLELNETCDF_EXPORT vtkEnlilReader : public vtkStructuredGridAlgorithm
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


    void SetDataUnits(int _arg)
    {

        std::cout << "Updating Units: " << std::endl;
        this->DataUnits = _arg;
        this->cleanCache();

        this->gridClean=false;
        this->Modified();
    }

    vtkGetMacro(DataUnits, int)


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


    void readVector(std::string array, vtkFloatArray *DataArray, vtkInformationVector* outputVector, const int &dataID);
    void readScalar(vtkStructuredGrid *Data, vtkFloatArray *DataArray, std::string array, vtkInformationVector* outputVector, int dataID);
    void getDataID(std::string array, int &dataID);
protected:

    vtkEnlilReader();
    ~vtkEnlilReader();

    char* FileName;            // Base file name
    int GridScaleType;
    int DataUnits;
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
    vtkSmartPointer<vtkPoints> Points;        // Structured grid geometry
    vtkSmartPointer<vtkFloatArray> Radius;   // Radius Grid Data

    std::vector<std::string> MetaDataNames;
    std::map<std::string, std::string> ScalarVariableMap;
    std::map<std::string, std::vector<std::string> > VectorVariableMap;
    std::vector<std::vector<double> > sphericalGridCoords;

    std::string dateString;
    std::vector<std::string> fileNames;

    std::map<double,std::string> time2fileMap;
    std::map<double,double> time2physicaltimeMap;
    std::map<double,std::string> time2datestringMap;

    //this map holds the positions of artifacts based on time step
    std::map< double, std::map<std::string, std::vector<double> > > positions;

    // Time step information
    int NumberOfTimeSteps;                 // Number of time steps
    std::vector<double> TimeSteps;        // Actual times available for request

    double timeRange[2];
    double CurrentPhysicalTime;
    double current_MJD;
    char* CurrentDateTimeString;
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
    int LoadArrayValues(std::string array, vtkInformationVector* outputVector);

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

    void calculateArtifacts();

    double* read3dPartialToArray(char *array, int extents[]);
    double* readGridPartialToArray(char *arrayName, int subExtents[], bool periodic);
    void loadVarMetaData(const char *array,
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
    static int CanReadFiles(const char* filename);

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

    //Caching implimentation
    RCache::ReaderCache pDensityCache;
    RCache::ReaderCache cDensityCache;
    RCache::ReaderCache temperatureCache;
    RCache::ReaderCache polarityCache;
    RCache::ReaderCache bFieldCache;
    RCache::ReaderCache velocityCache;

    void cleanCache();


    vtkEnlilReader(const vtkEnlilReader&);  // Not implemented.
    void operator=(const vtkEnlilReader&);  // Not implemented.
};



#endif // vtkEnlilReader_H
