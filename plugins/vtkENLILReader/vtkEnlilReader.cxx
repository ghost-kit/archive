#include "vtkEnlilReader.h"

#include "vtkCallbackCommand.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCellType.h"
#include "vtkDataArraySelection.h"
#include "vtkFloatArray.h"
#include "vtkFloatArray.h"
#include "vtkTable.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "vtkStringArray.h"
#include "vtkFloatArray.h"
#include "vtkIntArray.h"
#include "vtkPoints.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtksys/SystemTools.hxx"

#include <string>
#include <sstream>
#include <iostream>

#include "vtkMultiProcessController.h"
#include "vtkToolkits.h"

#include "vtk_netcdfcpp.h"
#include <iostream>


#include "DateTime.h"
//#include "cxform.h"
#include "readerCache.h"
#include "vtkNew.h"
#include <QString>
vtkStandardNewMacro(vtkEnlilReader)


//---------------------------------------------------------------
//    Constructors and Destructors
//---------------------------------------------------------------
vtkEnlilReader::vtkEnlilReader()
{
    int nulExtent[6] = {0,0,0,0,0,0};
    this->FileName = NULL;

    //set the number of output ports you will need
    this->SetNumberOfOutputPorts(1);

    //set the number of input ports (Default 0)
    this->SetNumberOfInputPorts(0);

    //configure array status selectors
    this->PointDataArraySelection = vtkDataArraySelection::New();
    this->CellDataArraySelection  = vtkDataArraySelection::New();
    this->PointDataArraySelection->DisableAllArrays();
    this->CellDataArraySelection->DisableAllArrays();

    this->numberOfArrays = 0;

    //Configure sytem array interfaces
    this->Points = NULL;
    this->Radius = NULL;
    this->gridClean = false;
    this->infoClean = false;

    this->timesCalulated = false;
    this->timeRange[0] = 0;
    this->timeRange[1] = 0;

    this->fileNames.clear();

    this->setMyExtents(this->WholeExtent, nulExtent);
    this->setMyExtents(this->SubExtent, nulExtent);

    this->SelectionObserver = vtkCallbackCommand::New();
    this->SelectionObserver->SetCallback(&vtkEnlilReader::SelectionCallback);
    this->SelectionObserver->SetClientData(this);
    this->PointDataArraySelection->AddObserver(vtkCommand::ModifiedEvent, this->SelectionObserver);
    this->CellDataArraySelection->AddObserver(vtkCommand::ModifiedEvent, this->SelectionObserver);


}

//---------------------------------------------------------------------------------------------
vtkEnlilReader::~vtkEnlilReader()
{
    this->PointDataArraySelection->Delete();
    this->CellDataArraySelection->Delete();
    this->SelectionObserver->Delete();


}

//---------------------------------------------------------------------------------------------
//-------------------------------------------------------------
//    The following Methods provide basic functionality for
//  Selective read arrays.  These methods provide the list of
//  arrays in Paraview.  You MUST populate the arrays in you
//  RequestInformation routine.  This can be done with the
//  *****DataArraySlection->AddArray(char* name) routines
//  where ***** represents the type of arrays you are
//  populating. (i.e. Point or Cell)
//-------------------------------------------------------------

/*
 * The Number of Point Arrays in current selection
 *  This is an internal function
 */
int vtkEnlilReader::GetNumberOfPointArrays()
{
    return this->PointDataArraySelection->GetNumberOfArrays();
}
//---------------------------------------------------------------------------------------------
/*
 * The Number of Cell Arrays in current selection
 *  This is an internal function
 */
int vtkEnlilReader::GetNumberOfCellArrays()
{
    return this->CellDataArraySelection->GetNumberOfArrays();
}
//---------------------------------------------------------------------------------------------
/*
 *Return the NAME (characters) of the Point array at index
 *   This is an internal function
 */
const char* vtkEnlilReader::GetPointArrayName(int index)
{
    return this->PointDataArraySelection->GetArrayName(index);
}

//---------------------------------------------------------------------------------------------
/*
 *Return the NAME (characters) of the Cell array at index
 *   This is an internal function
 */
const char* vtkEnlilReader::GetCellArrayName(int index)
{
    return this->CellDataArraySelection->GetArrayName(index);
}

//---------------------------------------------------------------------------------------------
/*
 *Get the status of the Point Array of "name"
 *   This is an internal function
 */
int vtkEnlilReader::GetPointArrayStatus(const char *name)
{
    return this->PointDataArraySelection->GetArraySetting(name);
}

//---------------------------------------------------------------------------------------------
/*
 *Get the status of the Cell Array of "name"
 *   This is an internal function
 */
int vtkEnlilReader::GetCellArrayStatus(const char *name)
{
    return this->CellDataArraySelection->GetArraySetting(name);
}

//---------------------------------------------------------------------------------------------
/*
 *Set the status of the Point Array of "name"
 *   This is an internal function
 */
void vtkEnlilReader::SetPointArrayStatus(const char *name, int status)
{
    //  std::cout << __FUNCTION__ << " Called with status: " << status << std::endl;

    if(status)
    {
        //        std::cout << "Enabling " << name << std::endl;
        this->PointDataArraySelection->EnableArray(name);
    }
    else
    {
        //        std::cout << "Disabling " << name << std::endl;

        this->PointDataArraySelection->DisableArray(name);
    }

    this->Modified();

    //    std::cout /*<< __FILE__ << " " << __LINE__ << " " <<  __FUNCTION__*/
    //            << " Status of Array " << name << ": "
    //            << this->PointDataArraySelection->ArrayIsEnabled(name) << std::endl;

}

//---------------------------------------------------------------------------------------------
/*
 *Set the status of the Cell Array of "name"
 *   This is an internal function
 */
void vtkEnlilReader::SetCellArrayStatus(const char *name, int status)
{
    if(status == 1)
        this->CellDataArraySelection->EnableArray(name);
    else
        this->CellDataArraySelection->DisableArray(name);

    //  std::cout << __FUNCTION__ << " Called with status: " << status << std::endl;


    this->Modified();

}

//---------------------------------------------------------------------------------------------
/*
 *Disables ALL Point arrays registered in system
 *   This is an internal function
 */
void vtkEnlilReader::DisableAllPointArrays()
{
    this->PointDataArraySelection->DisableAllArrays();

    std::cout << __FUNCTION__ << " Called " << std::endl;

    this->Modified();
}

//---------------------------------------------------------------------------------------------
/*
 *Disables ALL Cell arrays registered in system
 *   This is an internal function
 */
void vtkEnlilReader::DisableAllCellArrays()
{
    this->CellDataArraySelection->DisableAllArrays();

    std::cout << __FUNCTION__ << " Called " << std::endl;


    this->Modified();
}

//---------------------------------------------------------------------------------------------
/*
 *Enables ALL Point arrays registered in system
 *   This is an internal function
 */
void vtkEnlilReader::EnableAllPointArrays()
{
    this->PointDataArraySelection->EnableAllArrays();

    std::cout << __FUNCTION__ << " Called " << std::endl;


    this->Modified();
}

//---------------------------------------------------------------------------------------------
/*
 *Enables ALL Cell arrays registered in system
 *   This is an internal function
 */
void vtkEnlilReader::EnableAllCellArrays()
{
    this->CellDataArraySelection->EnableAllArrays();

    std::cout << __FUNCTION__ << " Called " << std::endl;

    this->Modified();
}
//=============== END SELECTIVE READER METHODS================

//---------------------------------------------------------------------------------------------
//------------------------------------------------------------
//    These functions are the meat of the readers... i.e. they
//  are the calls that ParaView uses to get information from
//  your data source.   This is where the logic of the reader
//  is implemented.
//------------------------------------------------------------

int vtkEnlilReader::CanReadFiles(const char *filename)
{
    //This doesn't really do anything right now...
    return 1;
}

//---------------------------------------------------------------------------------------------
int vtkEnlilReader::RequestInformation(
        vtkInformation* request,
        vtkInformationVector** inputVector,
        vtkInformationVector* outputVector)
{
    int status = 0;

    // Array names and extents
    vtkInformation* DataOutputInfo = outputVector->GetInformationObject(0);
    status = this->checkStatus(
                DataOutputInfo,
                (char*)" Array Name: Data Info Output Information");

    //Set the Whole Extents and Time
    this->calculateTimeSteps();

    //Setup the grid date
    this->PopulateGridData();

    //get information from the first file in the list... it has to come from somewhere...
    this->CurrentFileName = (char*) this->fileNames[0].c_str();
    this->FileName = CurrentFileName;

    if(status)
    {
        //Work Around for restore state problems
        if(this->numberOfArrays == 0)
        {

            //Set the Names of the Arrays
            this->PopulateArrays();

        }

        /*Set Information*/
        //Set Time

        // Special case: if the time range is a single value, supress it.  This is
        // most likely from a data set that is a single file with no time anyway.
        // Even if it is not, how much value added is there for a single time value?
        //  This section is adapted from the ParaView vtkFileSeriesReader
        if (this->timeRange[0] >= this->timeRange[1])
        {
            DataOutputInfo->Remove(vtkStreamingDemandDrivenPipeline::TIME_RANGE());
            DataOutputInfo->Remove(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
        }
        else
        {
            DataOutputInfo->Set(
                        vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
                        this->TimeSteps.data(),
                        this->NumberOfTimeSteps);

            DataOutputInfo->Set(
                        vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
                        this->timeRange,
                        2);

            //            std::cout << "time Range: " << this->timeRange[0] << " to " << this->timeRange[1] << std::endl;

        }

        //Set Extents
        DataOutputInfo->Set(
                    vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
                    this->WholeExtent,
                    6);


    }
    return 1;
}


//---------------------------------------------------------------------------------------------
int vtkEnlilReader::RequestData(
        vtkInformation* request,
        vtkInformationVector** inputVector,
        vtkInformationVector* outputVector)
{
    //    std::cout << __FUNCTION__ << " Start" << std::endl;

    this->SetProgress(0);

    //need to determine the current requested file
    double requestedTimeValue = this->getRequestedTime(outputVector);

    //    std::cout << "Requested Time Value in Request Data: " << requestedTimeValue << std::endl;

    this->CurrentFileName = (char*)this->time2fileMap[requestedTimeValue].c_str();
    this->CurrentPhysicalTime = this->time2physicaltimeMap[requestedTimeValue];
    this->CurrentDateTimeString = (char*) this->time2datestringMap[requestedTimeValue].c_str();

    //hack to be fixed
    this->FileName = this->CurrentFileName;

    //Import the MetaData
    this->LoadMetaData(outputVector);

    //    std::cout << __FUNCTION__ <<  " Loaded MetaData" << std::endl;
    this->SetProgress(.05);

    //Import the actual Data
    this->LoadVariableData(outputVector);

    this->SetProgress(1.00);

    //    std::cout << __FUNCTION__ << " Stop" << std::endl;
    return 1;

}

//---------------------------------------------------------------------------------------------
//Get the Requested Time Step
double vtkEnlilReader::getRequestedTime(vtkInformationVector* outputVector)
{
    vtkInformation* outInfo = outputVector->GetInformationObject(0);

    double requestedTimeValue = this->TimeSteps[0];

    if(outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
    {
        requestedTimeValue = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());

        double upper =0;
        double lower =0;
        int x=0;

        //find the first time step where the value is greater than requested
        while(this->TimeSteps[x] < requestedTimeValue && x < this->NumberOfTimeSteps-1)
        {
            x++;
        }
        //        std::cout << "x counter: " << x << std::endl;
        //        std::cout << "number of time steps " << this->NumberOfTimeSteps << std::endl;

        upper = this->TimeSteps[x];

        if(TimeSteps[x] == requestedTimeValue)
        {
            //don't need to do anything
        }
        else if(x > 0)
        {
            //we will need to decide which one to use, so we need the lower level
            lower = this->TimeSteps[x-1];

            //lets determine which value to use
            double midpoint = (lower+upper)/2;
            if(requestedTimeValue > midpoint)
            {
                //requested time step in the upper half
                requestedTimeValue = upper;
            }
            else
            {
                //requested time step in the lower half
                requestedTimeValue = lower;
            }
        }
        else
        {
            //if the first value is greater, we use the first value.
            requestedTimeValue = this->TimeSteps[0];
        }

        //set the modified julian date
        this->current_MJD = requestedTimeValue;

        //        std::cout << "Requested Time Step: " << setprecision(12) << requestedTimeValue << std::endl;
    }

    return requestedTimeValue;
}


//---------------------------------------------------------------------------------------------
//Methods for file series

void vtkEnlilReader::AddFileName(const char *fname)
{
    this->fileNames.push_back(fname);
    this->Modified();
}

const char* vtkEnlilReader::GetFileName(unsigned int idx)
{
    return this->fileNames[idx].c_str();
}

void vtkEnlilReader::RemoveAllFileNames()
{
    this->fileNames.clear();
    this->Modified();
}

unsigned int vtkEnlilReader::GetNumberOfFileNames()
{
    return this->fileNames.size();
}

//=================== END CORE METHODS =======================

//---------------------------------------------------------------------------------------------
//-- Callback
void vtkEnlilReader::SelectionCallback(
        vtkObject* object,
        unsigned long vtkNotUsed(eventid),
        void* clientdata,
        void* vtkNotUsed(calldata))
{

    static_cast<vtkEnlilReader*>(clientdata)->Modified();
}


//---------------------------------------------------------------------------------------------
//------------------------------------------------------------
//    These methods to load the requested variables.
//  These are provided so that we can abstract out the reading
//  of the data from the rest of the reader.
//
//  Override these methods for your reader
//------------------------------------------------------------


//-- Return 0 for failure, 1 for success --//
int vtkEnlilReader::LoadVariableData(vtkInformationVector* outputVector)
{
    int newExtent[6];

    vtkStructuredGrid* Data = vtkStructuredGrid::GetData(outputVector, 0);
    vtkInformation* fieldInfo = outputVector->GetInformationObject(0);

    int status = this->checkStatus(Data, (char*)"Data Array Structured Grid");

    if(status)
    {
        //get new extent request
        fieldInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), newExtent);

        //check to see if exents have changed
        if(!this->eq(this->SubExtent, newExtent))
        {
            // Set the SubExtents to the NewExtents
            this->setMyExtents(this->SubExtent, newExtent);

            // The extents have changes, so mark grid dirty.
            this->gridClean = false;
        }

        //set the extents provided to Paraview
        Data->SetExtent(this->SubExtent);

        //Calculate Sub Dimensions
        this->extractDimensions(this->SubDimension, this->SubExtent);

        //Generate the Grid
        this->GenerateGrid();

        //set points and radius
        Data->SetPoints(this->Points);
        Data->GetPointData()->AddArray(this->Radius);

        //Load Variables
        int c = 0;
        double progress = 0.05;

        //Load Cell Data
        for(c = 0; c < this->CellDataArraySelection->GetNumberOfArrays(); c++)
        {
            //Load the current Cell array
            std::string array = std::string(this->CellDataArraySelection->GetArrayName(c));
            if(this->CellDataArraySelection->ArrayIsEnabled(array.c_str()))
            {
                this->LoadArrayValues(array, outputVector);
            }
        }

        //Load Point Data
        for(c=0; c < this->PointDataArraySelection->GetNumberOfArrays(); c++)
        {
            std::string array = std::string(this->PointDataArraySelection->GetArrayName(c));

            //Load the current Point array
            if(this->PointDataArraySelection->ArrayIsEnabled(array.c_str()))
            {
                //                                std::cout << "Loading Array " << array << std::endl;
                //                                std::cout << "   ArrayStatusSelection: " << this->PointDataArraySelection->ArrayIsEnabled(array.c_str())
                //                                             << std::endl;

                //when loading from state fiile, we may get some junk marking us to read bad data
//                if(this->ExtentOutOfBounds(this->SubExtent, this->WholeExtent))
//                {
//                    //                    std::cout << "Bad SubExtents" << std::endl;
////                    this->printExtents(this->WholeExtent, (char*)"Whole Extents: ");
////                    this->printExtents(this->SubExtent, (char*)"Bad SubExtent: ");

//                }

                this->LoadArrayValues(array, outputVector);
                this->SetProgress(progress);
                progress += 0.1;
            }
        }
    }

    return 1;
}

//---------------------------------------------------------------------------------------------
//-- Return 0 for Failure, 1 for Success --//

//This method will load the data and convert to the assigned "DataUnits" value.
//Currently, this process includes 2 types of units: Native and SWPC. As this changes,
//we will need to incorporate the changes here.
void vtkEnlilReader::readVector(std::string array, vtkFloatArray *DataArray,  vtkInformationVector* outputVector, const int &dataID)
{

    double xyz[3] = {0.0, 0.0, 0.0};

    double* newArrayR;
    double* newArrayT;
    double* newArrayP;

    int currentSize = 0;
    double radius = 0.0;

    //configure DataArray
    DataArray->SetNumberOfComponents(3);  //3-Dim Vector

    //read in the arrays
    newArrayR
            = this->read3dPartialToArray((char*)this->VectorVariableMap[array][0].c_str(), this->SubExtent);

    newArrayT
            = this->read3dPartialToArray((char*)this->VectorVariableMap[array][1].c_str(), this->SubExtent);

    newArrayP
            = this->read3dPartialToArray((char*)this->VectorVariableMap[array][2].c_str(), this->SubExtent);

    //get vector meta-data
    this->loadVarMetaData((char*)this->VectorVariableMap[array][0].c_str(), array.c_str(), outputVector);


    // convert from spherical to cartesian
    int loc=0;
    int i,j,k;
    for(k=0; k<this->SubDimension[2]; k++)
    {
        for(j=0; j<this->SubDimension[1]; j++)
        {
            for(i=0; i<this->SubDimension[0]; i++)

            {

                xyz[0] =newArrayR[loc]*sin(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][k]);
                xyz[1] =newArrayR[loc]*sin(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][k]);
                xyz[2] =newArrayR[loc]*cos(this->sphericalGridCoords[1][j]);

                xyz[0] += newArrayT[loc]*cos(this->sphericalGridCoords[1][j])*cos(this->sphericalGridCoords[2][k]);
                xyz[1] += newArrayT[loc]*cos(this->sphericalGridCoords[1][j])*sin(this->sphericalGridCoords[2][k]);
                xyz[2] += -1.0*newArrayT[loc]*sin(this->sphericalGridCoords[1][j]);

                xyz[0] += -1.0*newArrayP[loc]*sin(this->sphericalGridCoords[2][k]);
                xyz[1] += newArrayP[loc]*cos(this->sphericalGridCoords[2][k]);

                //adjust units:
                if(this->DataUnits)
                {
                    switch(this->DataUnits)
                    {
                    case 1:
                        switch(dataID)
                        {
                        case DATA_TYPE::VELOCITY:
                            xyz[0] = xyz[0] / UNITS::km2m;
                            xyz[1] = xyz[1] / UNITS::km2m;
                            xyz[2] = xyz[2] / UNITS::km2m;
                            break;

                        default:
                            break;
                        }
                        break;

                    default:
                        break;
                    }
                }


                DataArray->InsertNextTuple(xyz);

                loc++;

            }
        }



    }

    //free temporary memory
    delete [] newArrayR; newArrayR = NULL;
    delete [] newArrayP; newArrayP = NULL;
    delete [] newArrayT; newArrayT = NULL;
}

//---------------------------------------------------------------------------------------------
void vtkEnlilReader::readScalar(vtkStructuredGrid *Data, vtkFloatArray *DataArray, std::string array, vtkInformationVector* outputVector, int dataID)
{
    DataArray->SetNumberOfComponents(1);  //Scalar

    //array pointers
    double* newArray;

    //get data array
    newArray
            = this->read3dPartialToArray((char*)this->ScalarVariableMap[array].c_str(), this->SubExtent);

    //Load meta data for array
    this->loadVarMetaData((char*)this->ScalarVariableMap[array].c_str(), array.c_str(), outputVector);


    //insert points
    int k;
    for(k=0; k<this->SubDimension[2]*this->SubDimension[1]*this->SubDimension[0]; k++)
    {

        //adjust units:
        if(this->DataUnits)
        {
            switch(this->DataUnits)
            {
            case 1:
                switch(dataID)
                {
                case DATA_TYPE::PDENSITY:
                    newArray[k] = (newArray[k] * (this->Radius->GetValue(k)*this->Radius->GetValue(k)))/(UNITS::emu*UNITS::km2cm);
                    break;

                case DATA_TYPE::TEMP:
                    break;

                case DATA_TYPE::CDENSITY:
                    break;

                default:
                    break;
                }
                break;

            default:
                break;
            }
        }


        DataArray->InsertNextValue(newArray[k]);
    }

    //free temporary memory
    delete [] newArray; newArray = NULL;
}


//---------------------------------------------------------------------------------------------
void vtkEnlilReader::getDataID(std::string array, int &dataID)
{
    QString arrayName(array.c_str());

    if(arrayName.contains("Density") && arrayName.contains("cloud"))
    {
        dataID = DATA_TYPE::CDENSITY;
    }
    else if (arrayName.contains("Density"))
    {
        dataID = DATA_TYPE::PDENSITY;
    }
    else if (arrayName.contains("Polarity"))
    {
        dataID = DATA_TYPE::POLARITY;
    }
    else if(arrayName.contains("Magnetic"))
    {
        dataID = DATA_TYPE::BFIELD;
    }
    else if(arrayName.contains("Temperature"))
    {
        dataID = DATA_TYPE::TEMP;
    }
    else if(arrayName.contains("Velocity"))
    {
        dataID = DATA_TYPE::VELOCITY;
    }
}


//---------------------------------------------------------------------------------------------
int vtkEnlilReader::LoadArrayValues(std::string array, vtkInformationVector* outputVector)
{
    //get the dataID
    int dataID = 0;
    getDataID(array, dataID);

    bool vector
            = (this->VectorVariableMap.find(std::string(array)) != this->VectorVariableMap.end());

    //get data from system
    vtkStructuredGrid *Data = vtkStructuredGrid::GetData(outputVector,0);

    //get the Xtents to play with
    RCache::extents wholeExtents(this->WholeExtent);
    RCache::extents subExtents(this->SubExtent);

    //make the data array pointer available
    vtkFloatArray *DataArray = NULL;

    //set up array to be added

    //    std::cout << "Loading Fields" << std::endl << std::flush;
    //    std::cout << "Vector Status: " << (vector ? "TRUE" : "FALSE") << std::endl << std::flush;
    //    std::cout << "DataID: " << dataID << std::endl << std::flush;

    if(vector)  //load vector data
    {
        switch(dataID)
        {
        case DATA_TYPE::BFIELD:
            //read the fields
            if(this->bFieldCache.getExtentsFromCache(this->current_MJD, subExtents) == NULL)
            {
                //                std::cout << "BFIELD" << std::endl << std::flush;

                DataArray = vtkFloatArray::New();
                DataArray->SetName(array.c_str());
                //this means the data is not in cache, so lets get it
                readVector(array, DataArray, outputVector, dataID);

                // we really cannot cache the ENTIRE dataset at 800+ MB per step
                // so we are, for now, just limiting the cache to non-whole reads.
                // This can be further improved, I am sure.
                if(subExtents < wholeExtents)
                {
                    //add the element to the cache
                    //lets not use the cache yet.. not ready for prime time
                    //TODO: finish cache system

                    //this->bFieldCache.addCacheElement(this->current_MJD, subExtents, DataArray);

                }

            }
            else
            {
                //                std::cout << "BFIELD Cache" << std::endl << std::flush;
                //                std::cout << "Getting Data From the Cache: " << std::endl;
                //                this->printExtents(SubExtent, (char*)"Current Extents: ");
                //                std::cout << "Current Time: " << this->current_MJD << std::endl;

                //get the data array from the cache system
                DataArray = vtkFloatArray::SafeDownCast(this->bFieldCache.getExtentsFromCache(this->current_MJD, subExtents)->data);


            }

            break;

        case DATA_TYPE::VELOCITY:

            if(this->velocityCache.getExtentsFromCache(this->current_MJD, subExtents) == NULL)
            {
                //                std::cout << "VELOCITY" << std::endl << std::flush;


                DataArray = vtkFloatArray::New();
                DataArray->SetName(array.c_str());
                //this means the data is not in cache, so lets get it
                readVector(array, DataArray, outputVector, dataID);

                // we really cannot cache the ENTIRE dataset at 800+ MB per step
                // so we are, for now, just limiting the cache to non-whole reads.
                // This can be further improved, I am sure.
                if(subExtents < wholeExtents)
                {
                    //add the element to the cache
                    //lets not use the cache yet.. not ready for prime time
                    //TODO: finish cache system

                    //this->velocityCache.addCacheElement(this->current_MJD, subExtents, DataArray);

                }
            }
            else
            {
                //                std::cout << "VELOCITY Cache" << std::endl << std::flush;
                //                std::cout << "Getting Data From the Cache: " << std::endl;
                //                this->printExtents(SubExtent, (char*)"Current Extents: ");
                //                std::cout << "Current Time: " << this->current_MJD << std::endl;

                //get the data array from the cache system
                DataArray = vtkFloatArray::SafeDownCast(this->velocityCache.getExtentsFromCache(this->current_MJD, subExtents)->data);


            }
            break;

        default:
            std::cout << "DEFAULT - THIS IS AN ERROR" << std::endl << std::flush;

            break;
        }

        //Add array to grid
        if(DataArray)
        {
            Data->GetPointData()->AddArray(DataArray);

            // Call delete here. Both vtkPointData::AddArray() and now the cache::addCacheElement()
            // methods are keeping a reference to the array.
            DataArray->Delete();
        }



    }
    else    //load scalar data
    {

        switch(dataID)
        {
        case DATA_TYPE::PDENSITY:

            //            std::cout << "Looking for PDentsity" << std::endl;
            //            std::cout << "MJD: " << this->current_MJD << std::endl << std::flush;
            //            std::cout << "Extents: " << subExtents.getExtent(0) << " " << subExtents.getExtent(1)
            //                      << " " << subExtents.getExtent(2) << " " << subExtents.getExtent(3)
            //                      << " " << subExtents.getExtent(4) << " " << subExtents.getExtent(5)
            //                      << std::endl << std::flush;

            if(this->pDensityCache.getExtentsFromCache(this->current_MJD, subExtents) == NULL)
            {
                //                std::cout << "PDENSITY" << std::endl << std::flush;

                DataArray = vtkFloatArray::New();
                DataArray->SetName(array.c_str());
                readScalar(Data, DataArray, array, outputVector, dataID);

                if(subExtents < wholeExtents)
                {
                    //add the element to the cache
                    //lets not use the cache yet.. not ready for prime time
                    //TODO: finish cache system

                    //this->pDensityCache.addCacheElement(this->current_MJD, subExtents, DataArray);

                }
            }
            else
            {
                //SafeDownCast it
                //                std::cout << "PDENSITY Cache" << std::endl << std::flush;
                //                std::cout << "Getting Data From the Cache: " << std::endl;
                //                this->printExtents(SubExtent, (char*)"Current Extents: ");
                //                std::cout << "Current Time: " << this->current_MJD << std::endl;

                //get the data array from the cache system
                DataArray = vtkFloatArray::SafeDownCast(this->pDensityCache.getExtentsFromCache(this->current_MJD, subExtents)->data);

            }

            break;
        case DATA_TYPE::CDENSITY:
            if(this->cDensityCache.getExtentsFromCache(this->current_MJD, subExtents) == NULL)
            {
                //                std::cout << "CDENSITY" << std::endl << std::flush;


                DataArray = vtkFloatArray::New();
                DataArray->SetName(array.c_str());
                readScalar(Data, DataArray, array, outputVector, dataID);

                if(subExtents < wholeExtents)
                {
                    //add the element to the cache
                    //lets not use the cache yet.. not ready for prime time
                    //TODO: finish cache system

                    //this->cDensityCache.addCacheElement(this->current_MJD, subExtents, DataArray);

                }
            }
            else
            {
                //                std::cout << "CDENSITY Cache" << std::endl << std::flush;
                //                std::cout << "Getting Data From the Cache: " << std::endl;
                //                this->printExtents(SubExtent, (char*)"Current Extents: ");
                //                std::cout << "Current Time: " << this->current_MJD << std::endl;

                //get the data array from the cache system
                DataArray = vtkFloatArray::SafeDownCast(this->cDensityCache.getExtentsFromCache(this->current_MJD, subExtents)->data);

            }
            break;

        case DATA_TYPE::POLARITY:
            if(this->polarityCache.getExtentsFromCache(this->current_MJD, subExtents) == NULL)
            {
                //                std::cout << "POLARITY" << std::endl << std::flush;

                DataArray = vtkFloatArray::New();
                DataArray->SetName(array.c_str());
                readScalar(Data, DataArray, array, outputVector, dataID);

                if(subExtents < wholeExtents)
                {
                    //add the element to the cache
                    //lets not use the cache yet.. not ready for prime time
                    //TODO: finish cache system

                    //this->polarityCache.addCacheElement(this->current_MJD, subExtents, DataArray);

                }
            }
            else
            {
                //                std::cout << "POLARITY Cache" << std::endl << std::flush;
                //                std::cout << "Getting Data From the Cache: " << std::endl;
                //                this->printExtents(SubExtent, (char*)"Current Extents: ");
                //                std::cout << "Current Time: " << this->current_MJD << std::endl;

                //get the data array from the cache system
                DataArray = vtkFloatArray::SafeDownCast(this->polarityCache.getExtentsFromCache(this->current_MJD, subExtents)->data);

            }
            break;

        case DATA_TYPE::TEMP:
            if(this->temperatureCache.getExtentsFromCache(this->current_MJD, subExtents) == NULL)
            {
                //                std::cout << "TEMPURATURE" << std::endl;

                DataArray = vtkFloatArray::New();
                DataArray->SetName(array.c_str());
                readScalar(Data, DataArray, array, outputVector, dataID);

                if(subExtents < wholeExtents)
                {
                    //add the element to the cache
                    //lets not use the cache yet.. not ready for prime time
                    //TODO: finish cache system

                    //this->temperatureCache.addCacheElement(this->current_MJD, subExtents, DataArray);

                }

            }
            else
            {
                //                std::cout << "TEMPURATURE Cache" << std::endl;
                //                std::cout << "Getting Data From the Cache: " << std::endl;
                //                this->printExtents(SubExtent, (char*)"Current Extents: ");
                //                std::cout << "Current Time: " << this->current_MJD << std::endl;

                //get the data array from the cache system
                DataArray = vtkFloatArray::SafeDownCast(this->temperatureCache.getExtentsFromCache(this->current_MJD, subExtents)->data);

            }
            break;

        default:
            //            std::cout << "DEFAULT - THIS IS AN ERROR" << std::endl;

            break;

        }

        //Add array to grid
        if(DataArray)
        {
            Data->GetPointData()->AddArray(DataArray);

            // Call delete here. Both vtkPointData::AddArray() and now the cache::addCacheElement()
            // methods are keeping a reference to the array.
            DataArray->Delete();
        }


    }

    return 1;
}

//---------------------------------------------------------------------------------------------
//-- returns array read via partial IO limited by extents --//
/* This method will automatically adjust for the periodic boundary
 *  condition that does not exist sequentially in file */
double* vtkEnlilReader::read3dPartialToArray(char* arrayName, int extents[])
{
    int extDims[3] = {0,0,0};
    size_t readDims[4]   = {1,1,1,1};
    long readStart[4]  = {0,extents[4],extents[2],extents[0]};

    // get dimensions from extents
    this->extractDimensions(extDims, extents);

    // Enlil encodes in reverse, so reverse the order, add fourth dimension 1st.
    readDims[1] = extDims[2];
    readDims[2] = extDims[1];
    readDims[3] = extDims[0];

    //find all conditions that need to be accounted for
    bool periodic = false;
    bool periodicRead = false;
    bool periodicOnly = false;

    // this->printExtents(extents, (char*)"Loading Extents: ");

    if(extents[5] == this->WholeExtent[5])
    {
        periodic = true;
        // std::cout << "Set Periodic" << std::endl;

        if(extents[4] > 0)
        {
            periodicRead = true;
            // std::cout << "Set Periodic Read" << std::endl;
            if(extents[4] == this->WholeExtent[5])
            {
                periodicOnly = true;
                // std::cout << "Set Periodic Only" << std::endl;

            }
        }
    }
    else
    {
        //  std::cout << "Non-Periodic" << std::endl;
        //  dont need to do anything

    }

    // allocate memory for complete array
    double *array = new double[extDims[0]*extDims[1]*extDims[2]];

    //open file
    NcFile file(this->FileName);
    NcVar* variable = file.get_var(arrayName);

    // start to read in data
    if(periodic && !periodicOnly)
    {
        //adjust dims
        readDims[1] = readDims[1]-1;

        //adjust the start point
        variable->set_cur(readStart);

        //read the file
        variable->get(array, readDims);

    }
    else if(periodicOnly)
    {
        //set periodic only
        readDims[1] = 1;
        readStart[1] = 0;
        readStart[2] = extents[2];
        readStart[3] = extents[0];

        //set read location
        variable->set_cur(readStart);

        //read the file
        variable->get(array, readDims);

    }
    else
    {
        //set read location as stated
        variable->set_cur(readStart);

        //read as stated
        variable->get(array, readDims);

    }

    // fix periodic boundary if necesary
    if(periodic && !periodicRead && !periodicOnly)
    {
        //copy periodic data from begining to end
        size_t wedgeSize = (extDims[0]*extDims[1]);
        size_t wedgeLoc  = (extDims[0]*extDims[1])*(extDims[2]-1);

        for(int x = 0; x < wedgeSize; x++)
        {
            //copy the wedge
            array[wedgeLoc] = array[x];

            //advance index
            wedgeLoc++;
        }

    }
    else if (periodic && periodicRead && !periodicOnly)  /*periodicRead &&*/
    {
        //read in periodic data and place at end of array
        size_t wedgeSize = extDims[0]*extDims[1];
        size_t wedgeLoc  = (extDims[0]*extDims[1])*(extDims[2]-1);

        double * wedge = new double[wedgeSize];

        //start at 0,0,0
        readStart[0] = 0;
        readStart[1] = 0;
        readStart[2] = extents[2];
        readStart[3] = extents[0];

        //restrict to phi = 1 dimension
        readDims[1] = 1;

        //set start
        variable->set_cur(readStart);

        //read data
        variable->get(wedge, readDims);

        //populate wedge to array
        for(int x = 0; x < wedgeSize; x++)
        {
            //copy the wedge
            array[wedgeLoc] = wedge[x];

            //advance index
            wedgeLoc++;
        }

        //free temp memory
        delete [] wedge; wedge = NULL;
    }


    //close file
    file.close();

    return array;

}

//---------------------------------------------------------------------------------------------
//-- returns array read via partial IO limited by extents --//
/* This method will automatically adjust for the periodic boundary
 *  condition that does not exist sequentially in file */
double* vtkEnlilReader::readGridPartialToArray(char *arrayName, int subExtents[], bool isPeriodic = false)
{
    int     extDim = subExtents[1]-subExtents[0]+1;;
    size_t  readDims[2]  = {1,extDim};
    long    readStart[2] = {0,subExtents[0]};

    //Find conditions that need to be handled
    bool periodic = false;
    bool periodicRead = false;
    bool periodicOnly = false;

    //if isPeriodic is set, then we are looking at phi
    if(isPeriodic)
    {
        if(subExtents[1] == this->WholeExtent[5])
        {
            periodic = true;
            if(subExtents[0] > 0)
            {
                periodicRead = true;
                if(subExtents[0] == this->WholeExtent[5])
                {
                    periodicOnly = true;
                }
            }
        }
    }

    //allocate Memory for complete array
    double *array = new double[extDim];

    //Open file
    NcFile file(this->FileName);
    NcVar* variable = file.get_var(arrayName);

    //start to read in data
    if(periodic && !periodicOnly)
    {

        //adjust dims
        readDims[1] = readDims[1]-1;

        //adjust the start point
        variable->set_cur(readStart);

        //read the file
        variable->get(array, readDims);

    }
    else if(periodicOnly)
    {

        //set periodic only
        readDims[1] = 1;
        readStart[1] = 0;

        //set read location
        variable->set_cur(readStart);

        //read the file
        variable->get(array, readDims);

    }
    else
    {
        //set read location as stated
        variable->set_cur(readStart);

        //read as stated
        variable->get(array, readDims);
    }

    //fix periodic boundary if necesary
    if(periodic && !periodicRead && !periodicOnly)
    {

        //copy periodic data from begining to end
        array[extDim-1] = array[0];

    }
    else if (periodic && periodicRead && !periodicOnly)
    {

        //read in periodic data and place at end of array
        size_t wedgeSize = 1;
        size_t wedgeLoc  = (extDim-1);

        double * wedge = new double[wedgeSize];

        //start at 0,0,0
        readStart[1] = 0;

        //restrict to phi = 1 dimension
        readDims[1] = 1;

        //set start
        variable->set_cur(readStart);

        //read data
        variable->get(wedge, readDims);

        //populate wedge to array

        array[wedgeLoc] = wedge[0];

        //free temp memory
        delete [] wedge; wedge = NULL;
    }

    //close the file
    file.close();

    //return completed array
    return array;
}

//---------------------------------------------------------------------------------------------
//Loading Meta-Data from Variables
void vtkEnlilReader::loadVarMetaData(const char *array, const char* title,
                                     vtkInformationVector *outputVector,
                                     bool vector)
{

    vtkStructuredGrid *Data = vtkStructuredGrid::GetData(outputVector,0);
    int status = this->checkStatus(Data, (char*)"(MetaData)Structured Grid Data Object");

    if(!status)
    {
        std::cerr << "Failed to get Data Structure in " << __FUNCTION__ << std::endl;
    }

    //open the file
    NcFile file(this->FileName);
    NcVar* variable = file.get_var(array);
    NcType attType;

    std::string* attname = NULL;
    char* attSval = NULL;

    double  attDval = 0.0;
    int     attIval = 0;

    std::string placeholder = std::string(title);
    placeholder.append(" ");

    std::string outputName;

    //determine if any meta-data exists for array
    int count = variable->num_atts();

    //if so, load the meta data into arrays
    for(int x = 0; x < count; x++)
    {
        attname = new std::string(variable->get_att(x)->name());
        attType = variable->get_att(x)->type();

        outputName.clear();
        outputName.assign(placeholder.c_str());
        outputName.append(attname->c_str());

        // create the needed arrays
        vtkNew<vtkStringArray>  MetaString;
        vtkNew<vtkIntArray>     MetaInt;
        vtkNew<vtkFloatArray>  MetaDouble;

        //        std::cout << "Adding Attribute: " << outputName << std::endl;

        switch(attType)
        {
        case ncByte:

            //            std::cout << "Type: Byte" << std::endl;
            //            std::cout << "Not implimented" << std::endl;
            break;

        case ncChar:

            attSval = variable->get_att(x)->as_string(0);

            MetaString->SetName(outputName.c_str());
            MetaString->SetNumberOfComponents(1);
            MetaString->InsertNextValue(attSval);

            Data->GetFieldData()->AddArray(MetaString.GetPointer());
            break;

        case ncShort:
            //            std::cout << "Type: Short" << std::endl;
            //            std::cout << "Not implimented" << std::endl;
            break;

        case ncInt:

            attIval = variable->get_att(x)->as_int(0);

            MetaInt->SetName(outputName.c_str());
            MetaInt->SetNumberOfComponents(1);
            MetaInt->InsertNextValue(attIval);

            Data->GetFieldData()->AddArray(MetaInt.GetPointer());
            break;

        case ncFloat:
            //            std::cout << "Type: Float" << std::endl;
            //            std::cout << "Not implimented" << std::endl;
            break;

        case ncDouble:

            attDval = variable->get_att(x)->as_double(0);

            MetaDouble->SetName(outputName.c_str());
            MetaDouble->SetNumberOfComponents(1);
            MetaDouble->InsertNextValue(attDval);

            Data->GetFieldData()->AddArray(MetaDouble.GetPointer());
            break;

        default:
            break;
        }

    }

}

//---------------------------------------------------------------------------------------------
//-- Return 0 for failure, 1 for success --//
/* You will want to over-ride this method to
 * Populate the system with your own arrays */
int vtkEnlilReader::PopulateArrays()
{

    this->addPointArray((char*)"D");
    this->addPointArray((char*)"DP");
    this->addPointArray((char*)"T");
    this->addPointArray((char*)"BP");
    this->addPointArray((char*)"B1", (char*)"B2", (char*)"B3");
    this->addPointArray((char*)"V1", (char*)"V2", (char*)"V3");

    this->numberOfArrays = this->PointDataArraySelection->GetNumberOfArrays();

    return 1;
}

//---------------------------------------------------------------------------------------------
//-- Meta Data Population
int vtkEnlilReader::LoadMetaData(vtkInformationVector *outputVector)
{
    int ncFileID = 0;
    int ncSDSID = 0;
    int natts = 0;

    NcType type;

    char* attname;

    char*    attvalc;
    int     attvali;
    double  attvald;

    vtkStructuredGrid *Data = vtkStructuredGrid::GetData(outputVector,0);
    int status = this->checkStatus(Data, (char*)"(MetaData)Structured Grid Data Object");


    if(status)
    {

        //date string
        vtkNew<vtkStringArray> DateString;
        DateString->SetName("DateString");
        DateString->SetNumberOfComponents(1);
        DateString->InsertNextValue(this->CurrentDateTimeString);

        Data->GetFieldData()->AddArray(DateString.GetPointer());

        //Load Physical Time
        vtkNew<vtkFloatArray> physTime;
        physTime->SetName("PhysicalTime");
        physTime->SetNumberOfComponents(1);
        physTime->InsertNextValue(this->CurrentPhysicalTime);

        Data->GetFieldData()->AddArray(physTime.GetPointer());

        //mjd is encoded as TIME already.  Do we want to put in here as well?
        vtkNew<vtkFloatArray> currentMJD;
        currentMJD->SetName("MJD");
        currentMJD->SetNumberOfComponents(1);
        currentMJD->InsertNextValue(this->current_MJD);

        Data->GetFieldData()->AddArray(currentMJD.GetPointer());

        //get metadate from file
        NcFile file(this->FileName);
        natts = file.num_atts();

        //TODO: Need to strip spaces from meta-data names and reformat them with underscores
        for(int q=0; q < natts; q++)
        {

            vtkNew<vtkStringArray>  MetaString;
            vtkNew<vtkIntArray>     MetaInt;
            vtkNew<vtkFloatArray>  MetaDouble;

            attname = (char*)file.get_att(q)->name();
            type = file.get_att(q)->type();

            switch(type)
            {
            case 1:
                break;

            case 2: //text

                attvalc = file.get_att(q)->as_string(0);

                MetaString->SetName(attname);
                MetaString->SetNumberOfComponents(1);
                MetaString->InsertNextValue(attvalc);

                Data->GetFieldData()->AddArray(MetaString.GetPointer());
                break;

            case 3:
                break;

            case 4: //int
                attvali = file.get_att(q)->as_int(0);

                MetaInt->SetName(attname);
                MetaInt->SetNumberOfComponents(1);
                MetaInt->InsertNextValue(attvali);

                Data->GetFieldData()->AddArray(MetaInt.GetPointer());
                break;

            case 5:
                break;

            case 6: //double
                attvald = file.get_att(q)->as_double(0);

                MetaDouble->SetName(attname);
                MetaDouble->SetNumberOfComponents(1);
                MetaDouble->InsertNextValue(attvald);

                Data->GetFieldData()->AddArray(MetaDouble.GetPointer());
                break;

            default:
                break;
            }
        }

        file.close();
    }

    return 1;
}

//---------------------------------------------------------------------------------------------
//Status Check
int vtkEnlilReader::checkStatus(void *Object, char *name)
{
    if(Object == NULL)
    {
        std::cerr << "ERROR: " << name
                  << " has failed to initialize"
                  << std::endl;

        return 0;
    }

    return 1;
}

//---------------------------------------------------------------------------------------------
//this function calculates the positions of artifacts in the system
//void vtkEnlilReader::calculateArtifacts()
//{
//    DateTime time;

//    int retError;
//    int es;

//    double jd;
//    Vec pos_in, pos_out;

//    pos_in[0] = -1;
//    pos_in[1] = 0;
//    pos_in[2] = 0;

//    //lets calculate all positions, just once (TODO: check we do this only once)
//    for(int x=0; x < this->NumberOfTimeSteps; x++)
//    {
//        time.setMJD(this->TimeSteps[x]);
//        jd = gregorian_calendar_to_jd(time.getYear(),
//                                      time.getMonth(),
//                                      time.getDay(),
//                                      time.getHour(),
//                                      time.getMinute(),
//                                      time.getSecond());

//        es = date2es(time.getYear(),
//                     time.getMonth(),
//                     time.getDay(),
//                     time.getHour(),
//                     time.getMinute(),
//                     time.getSecond());

//    }
//}

//---------------------------------------------------------------------------------------------
//-- Return 0 for failure, 1 for success --//
/* Over-ride this method to provide the
 * extents of your data */
int vtkEnlilReader::calculateTimeSteps()
{

    /* Find Time Range.
     * We need to open all files, calculate the time, and store locally.
     * We need to keep track of current file
     * We don't want to re-calculate the times, just keep them available mapped to their file names
     * We also need to calculate the time range so ParaView knows what time steps we have.
     */

    if(this->timesCalulated == false)
    {

        //calculate number of time steps
        //  This is easy, as there is one time step per file.
        this->NumberOfTimeSteps = this->fileNames.size();

        //        std::cout << "Number of Time Steps: " << this->NumberOfTimeSteps << std::endl;

        //the hard part... open all of the files, map them to their calculated times

        for (int x = 0; x < this->NumberOfTimeSteps; x++)
        {
            NcFile data(this->fileNames[x].c_str());
            NcVar* time = data.get_var("TIME");
            NcAtt* mjd_start = data.get_att("refdate_mjd");

            DateTime refDate(mjd_start->as_double(0));
            double epochSeconds = refDate.getSecondsSinceEpoch();
            epochSeconds += time->as_double(0);

            refDate.incrementSeconds(time->as_double(0));

            this->TimeSteps.push_back(refDate.getMJD());

            //populate physical time map
            this->time2physicaltimeMap[this->TimeSteps[x]] = time->as_double(0);
            data.close();

            //populate file map
            this->time2fileMap[this->TimeSteps[x]] = this->fileNames[x];

            //populate datestring map
            this->time2datestringMap[this->TimeSteps[x]].assign(refDate.getDateTimeString());

            //            std::cout << "[" << x << "] MJD: " << this->TimeSteps[x] << std::endl;
        }

        //calculate time range
        this->timeRange[0] = this->TimeSteps[0];
        this->timeRange[1] = this->TimeSteps[this->NumberOfTimeSteps-1];

        this->timesCalulated = true;
    }

    return 1;
}


//---------------------------------------------------------------------------------------------
//this function populates the grid data.  used to be calcuated with time steps, but
//  the new timestep handling routine makes more sense to not include this.
void vtkEnlilReader::PopulateGridData()
{
    //get the dimensions of the grid
    NcFile grid(this->fileNames[0].c_str());
    NcDim* dims_x = grid.get_dim(0);
    NcDim* dims_y = grid.get_dim(1);
    NcDim* dims_z = grid.get_dim(2);

    //Populate Dimensions
    this->Dimension[0] = (int)dims_x->size();
    this->Dimension[1] = (int)dims_y->size();
    this->Dimension[2] = (int)dims_z->size()+1;

    //Populate Extents
    this->setMyExtents(this->WholeExtent,
                       0, this->Dimension[0]-1,
            0, this->Dimension[1]-1,
            0, this->Dimension[2]-1);

    //done with grid, thus we now close it
    grid.close();

}

//---------------------------------------------------------------------------------------------
//-- print extents --//
void vtkEnlilReader::printExtents(int extent[], char* description)
{
    std::cout << description << " "
              << extent[0] << " " <<
                 extent[1] << " " <<
                 extent[2] << " " <<
                 extent[3] << " " <<
                 extent[4] << " " <<
                 extent[5] << std::endl;
}

//---------------------------------------------------------------------------------------------
void vtkEnlilReader::setMyExtents(int extentToSet[], int sourceExtent[])
{
    extentToSet[0] = sourceExtent[0];
    extentToSet[1] = sourceExtent[1];
    extentToSet[2] = sourceExtent[2];
    extentToSet[3] = sourceExtent[3];
    extentToSet[4] = sourceExtent[4];
    extentToSet[5] = sourceExtent[5];

}

//---------------------------------------------------------------------------------------------
//set exents to given array
void vtkEnlilReader::setMyExtents(int extentToSet[], int dim1, int dim2, int dim3, int dim4, int dim5, int dim6)
{
    extentToSet[0] = dim1;
    extentToSet[1] = dim2;
    extentToSet[2] = dim3;
    extentToSet[3] = dim4;
    extentToSet[4] = dim5;
    extentToSet[5] = dim6;
}

//---------------------------------------------------------------------------------------------
//check equality of extents
bool vtkEnlilReader::eq(int extent1[], int extent2[])
{
    return (extent1[0] == extent2[0] && extent1[1] == extent2[1]
            && extent1[2] == extent2[2] && extent1[3] == extent2[3]
            && extent1[4] == extent2[4] && extent1[5] == extent2[5]);
}

//---------------------------------------------------------------------------------------------
//check bounds of extents
bool vtkEnlilReader::ExtentOutOfBounds(int extToCheck[], int extStandard[])
{
    if(extToCheck[0] >= 0)
    {
        if(extToCheck[2] >= 0)
        {
            if(extToCheck[4] >= 0)
            {
                if(extToCheck[1] <= extStandard[1] &&
                        extToCheck[3] <= extStandard[3] &&
                        extToCheck[5] <= extStandard[5])
                {
                    return false;
                }
            }
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------
//get the dimensions from the extents provided
void vtkEnlilReader::extractDimensions(int dims[], int extent[])
{
    dims[0] = extent[1] - extent[0]+1;
    dims[1] = extent[3] - extent[2]+1;
    dims[2] = extent[5] - extent[4]+1;
}

//---------------------------------------------------------------------------------------------
//add a point array
void vtkEnlilReader::addPointArray(char* name)
{
    NcFile file(this->FileName);
    try
    {
        // look up the "Long Name" of the variable
        std::string varname = file.get_var(name)->get_att("long_name")->as_string(0);
        this->ScalarVariableMap[varname] = std::string(name);

        // Add it to the point grid
        if(!this->PointDataArraySelection->ArrayExists(varname.c_str()))
        {
            this->PointDataArraySelection->AddArray(varname.c_str());
        }
    }
    catch (...)
    {
        std::cerr << "Failed to retrieve variable " << name
                  << ". Verify variable name." << std::endl;

        file.close();
        return;
    }

    file.close();
}

//---------------------------------------------------------------------------------------------
void vtkEnlilReader::addPointArray(char* name1, char* name2, char* name3)
{
    NcFile file(this->FileName);
    try
    {
        //get the long name of the first variable in the vector
        std::string varname1 = file.get_var(name1)->get_att("long_name")->as_string(0);

        //remove the vector component of the name
        size_t pos = varname1.find("-");
        std::string varname2 = varname1.substr(pos+1);

        //ensure that first work is capitalized
        varname2[0] = toupper((unsigned char) varname2[0]);

        //add components of vector to vector map
        std::vector<std::string> nameArray;
        nameArray.push_back(name1);
        nameArray.push_back(name2);
        nameArray.push_back(name3);
        this->VectorVariableMap[varname2] = nameArray;

        //add array to point array name list
        if(!this->PointDataArraySelection->ArrayExists(varname2.c_str()))
        {
            this->PointDataArraySelection->AddArray(varname2.c_str());
        }

    }
    catch(...)
    {
        std::cerr << "Failed to retrieve variable "
                  << name1 << " or " << name2 << " or " << name3
                  << ". Verify variable names." << std::endl;

        file.close();
        return;
    }
    file.close();
}

//---------------------------------------------------------------------------------------------
//-- Return 0 for failure, 1 for success --//
/* You will need to over-ride this method to provide
 * your own grid-information */
int vtkEnlilReader::GenerateGrid()
{

    int i = 0;
    int j = 0;
    int k = 0;

    const int GridScale = this->GetGridScaleType();

    double *X1;
    double *X2;
    double *X3;

    int X1_extents[2] = {this->SubExtent[0], this->SubExtent[1]};
    int X2_extents[2] = {this->SubExtent[2], this->SubExtent[3]};
    int X3_extents[2] = {this->SubExtent[4], this->SubExtent[5]};

    //build the grid if it is dirty (modified in application)
    if(!this->gridClean)
    {

        this->sphericalGridCoords.clear();


        //build the Grid
        this->Points = vtkSmartPointer<vtkPoints>::New();

        //build the Radius Array
        this->Radius = vtkSmartPointer<vtkFloatArray>::New();
        this->Radius->SetName("Radius");
        this->Radius->SetNumberOfComponents(1);

        // read data from file
        X1 = this->readGridPartialToArray((char*)"X1", X1_extents, false);
        X2 = this->readGridPartialToArray((char*)"X2", X2_extents, false);
        X3 = this->readGridPartialToArray((char*)"X3", X3_extents, true);

        // Populate the Spherical Grid Coordinates (to be used in calcs later)
        std::vector<double> R(X1, X1 + this->SubDimension[0]);
        std::vector<double> T(X2, X2 + this->SubDimension[1]);
        std::vector<double> P(X3, X3 + this->SubDimension[2]);

        this->sphericalGridCoords.push_back(R);
        this->sphericalGridCoords.push_back(T);
        this->sphericalGridCoords.push_back(P);

        // Generate the grid based on the R-P-T coordinate system.
        double xyz[3] = { 0, 0, 0 };
        for (k = 0; k < this->SubDimension[2]; k++)
        {
            for (j = 0; j < this->SubDimension[1]; j++)
            {
                for (i = 0; i < this->SubDimension[0]; i++)
                {
                    xyz[0] = (X1[i] * sin(X2[j]) * cos(X3[k]))
                            / GRID_SCALE::ScaleFactor[GridScale];
                    xyz[1] = (X1[i] * sin(X2[j]) * sin(X3[k]))
                            / GRID_SCALE::ScaleFactor[GridScale];
                    xyz[2] = (X1[i] * cos(X2[j]))
                            / GRID_SCALE::ScaleFactor[GridScale];

                    //insert point information into the grid
                    this->Points->InsertNextPoint(xyz);

                    // insert radius value into radius array.
                    // Scaled by grid scale factor
                    this->Radius->InsertNextValue(
                                X1[i] / GRID_SCALE::ScaleFactor[GridScale]);
                }
            }
        }

        //grid just created, so clean by definition.
        this->gridClean=true;
    }
    return 1;
}

//=================== END USER METHODS =========================



//---------------------------------------------------------------------------------------------
//=================== Cache Control Methods ====================
void vtkEnlilReader::cleanCache()
{

    std::cout << "Cleaning Cache..." << std::endl;

    this->pDensityCache.cleanCache();
    this->cDensityCache.cleanCache();
    this->polarityCache.cleanCache();
    this->temperatureCache.cleanCache();
    this->velocityCache.cleanCache();
    this->bFieldCache.cleanCache();
}



//---------------------------------------------------------------------------------------------
//--------------------------------------------------------------
//    Output Port Configuration
//--------------------------------------------------------------
int vtkEnlilReader::FillOutputPortInformation(int port, vtkInformation* info)
{

    if (port==0)
    {
        info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkStructuredGrid");

        return this->Superclass::FillInputPortInformation(port, info);

    }

    return 1;
}

//---------------------------------------------------------------------------------------------

//================= END PORT CONFIGURATION ===================

//------------------------------------------------------------
//    Internal functions -- required for system to work
//------------------------------------------------------------
void vtkEnlilReader::PrintSelf(ostream &os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
}
