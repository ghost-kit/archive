//==============================================================================//
// vtkSpaceCraftInfo filter.  This filter requests data from CDA-Web and        //
//  processes it for use with ParaView time-dependent data sets.  The filter    //
//  will also interpolate the requested information over time to get the proper //
//  time positions.                                                             //
//                                                                              //
//  Author: Joshua Murphy                                                       //
//  Date:   01 Apr 2013                                                         //
//==============================================================================//

#include "vtkSpaceCraftInfo.h"

#include "vtkCommand.h"
#include "vtkAbstractArray.h"
#include "vtkCallbackCommand.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDataSetAttributes.h"
#include "vtkDataArraySelection.h"
#include "vtkCompositeDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkDataArray.h"
#include "vtkDataObject.h"
#include "vtkFieldData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGridAlgorithm.h"
#include "vtkStructuredGrid.h"
#include "vtkStructuredData.h"
#include "vtkTable.h"
#include "vtksys/SystemTools.hxx"


#include "QObject"
#include "QtXml"
#include "QNetworkAccessManager"
#include "QNetworkReply"
#include "QURL"
#include "QStringList"
#include "qeventloop.h"
#include "QtAlgorithms"

#include <vector>
#include <vtkSmartPointer.h>
#include <vtksys/ios/sstream>

#include "cdf.h"
#include "cdflib.h"
#include "status.h"
#include "filterNetworkAccessModule.h"

#include "DateTime.h"
#include "filedownloader.h"

#include <iomanip>

vtkStandardNewMacro(vtkSpaceCraftInfo)

//=========================================================================================//
vtkSpaceCraftInfo::vtkSpaceCraftInfo()
{
    this->NumberOfTimeSteps = 0;
    this->processed = false;

    this->tempFilePath = "/tmp/";
}

//=========================================================================================//
vtkSpaceCraftInfo::~vtkSpaceCraftInfo()
{

}

//----- required overides -----//
//=========================================================================================//
//Process Request
int vtkSpaceCraftInfo::ProcessRequest(vtkInformation *request, vtkInformationVector ** inputVector, vtkInformationVector *outputVector)
{
    if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
    {
        return this->RequestInformation(request, inputVector, outputVector);
    }
    else if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_DATA()))
    {
        return this->RequestData(request, inputVector, outputVector);
    }

    return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//=========================================================================================//
//Request Inoformation
int vtkSpaceCraftInfo::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{

    std::cout << __FUNCTION__ << " on line " << __LINE__ << std::endl;

    this->inInfo = inputVector[0]->GetInformationObject(0);
    if(inInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS()))
    {
        std::cout << "Getting Number of Time steps" << std::flush << std::endl;
        this->NumberOfTimeSteps = this->inInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
        double *timeValues = this->inInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());

        //push time steps into list
        this->timeSteps.clear();
        for (int y = 0; y < this->NumberOfTimeSteps; y++)
        {
            this->timeSteps.push_back(timeValues[y]);
        }
    }
    else
    {
        this->NumberOfTimeSteps = 0;
    }

    //Provide information to PV on how many time steps for which we will be providing information
    this->outInfo = outputVector->GetInformationObject(0);

    this->outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
                       this->getTimeSteps(),
                       this->NumberOfTimeSteps);

    double timeRange[2] = {this->timeSteps.first(), this->timeSteps.last()};

    this->outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
                       timeRange,
                       2);

    this->inInfo = NULL;
    this->outInfo = NULL;
    return 1;
}

//=========================================================================================//
//Request Data
int vtkSpaceCraftInfo::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    //Get the output Data object
    this->outInfo = outputVector->GetInformationObject(0);
    this->output = vtkTable::GetData(outInfo);

    //get time request data
    if(this->outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
    {
        this->requestedTimeValue = this->outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
    }

    //if the data still needs to be loaded, load it...
    if(!this->processed)
    {
        this->LoadCDFData();
    }

    //process to return the needed information
    this->processCDAWeb(this->output);
    return 1;
}


//=========================================================================================//
//------ Port Information ------//
int vtkSpaceCraftInfo::FillInputPortInformation(int port, vtkInformation *info)
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    return 1;
}

//=========================================================================================//
int vtkSpaceCraftInfo::FillOutputPortInformation(int port, vtkInformation *info)
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTable");
    return 1;
}


//=========================================================================================//
//----- helper function -----//
//getTimeSteps() returns a NEW double array that *must*
//  be managed by the caller. TODO: Fix this to manage memory slef.
double *vtkSpaceCraftInfo::getTimeSteps()
{
    double *ret = new double[this->NumberOfTimeSteps];

    for(int i = 0; i < this->NumberOfTimeSteps; i++)
    {
        ret[i] = this->timeSteps[i];
    }

    return ret;
}

//=========================================================================================//
//------ get actuall space craft data -----//
bool vtkSpaceCraftInfo::getSCData()
{

    return true;
}


//=========================================================================================//
bool vtkSpaceCraftInfo::processCDAWeb(vtkTable *output)
{

    QMap<QString, QString>::Iterator iter;
    for(iter = this->CacheFileName.begin(); iter != this->CacheFileName.end(); ++iter)
    {


        QString DataSet = this->CacheFileName.key(*iter);
        double time = this->requestedTimeValue;
        QStringList keys = this->DataCache[DataSet][time].keys();

        for(int q=0; q<keys.size(); q++)
        {
            std::cout << ": DATA[" << time << "][" << keys[q].toAscii().data() << "]: " << this->DataCache[DataSet][time][keys[q]][0].first.toDouble()
                    << " " << this->DataCache[DataSet][time][keys[q]][0].second.toAscii().data() << std::endl;

            long numElements = this->DataCache[DataSet][time][keys[q]].size();

            vtkDoubleArray *newArray = vtkDoubleArray::New();
            newArray->SetName(keys[q].toAscii().data());
            newArray->SetNumberOfComponents(numElements);

            double *newData = new double[numElements];
            for(int a=0; a < numElements; a++)
            {
                newData[a] = this->DataCache[DataSet][time][keys[q]][a].first.toDouble();
                newArray->SetComponentName(a, this->DataCache[DataSet][time][keys[q]][a].second.toAscii().data());
            }

            newArray->InsertNextTupleValue(newData);

            output->AddColumn(newArray);

            newArray->Delete();
            delete [] newData;

        }

    }

    return true;
}

//=========================================================================================//
void vtkSpaceCraftInfo::checkCDFstatus(CDFstatus status)
{
    char text[CDF_STATUSTEXT_LEN+1];
    if(status != CDF_OK)
    {
        CDFgetStatusText(status, text);
        std::cerr << "ERROR: " << text << std::endl;
    }
}

//=========================================================================================//
void vtkSpaceCraftInfo::LoadCDFData()
{

    QMap<QString, QString>::Iterator iter;
    for(iter = this->CacheFileName.begin(); iter != this->CacheFileName.end(); ++iter)
    {
        std::cout << "Reading File: " << (*iter).toAscii().data() << " for Data Set: "
                  << this->CacheFileName.key((*iter)).toAscii().data() << std::endl;

        //if not EPOCH, load the data THAT WE NEED
        for(int x =0; x < this->timeSteps.size(); x++)
        {
            QString DataSet = this->CacheFileName.key(*iter);
            double time = this->timeSteps[x];
            //get the data
            this->getDataForEpoch(DataSet, time, this->DataCache[DataSet][time]);
        }

        this->processed = true;
    }

}

//=========================================================================================//
bool vtkSpaceCraftInfo::cToQVector(double *data, long dataSize, QVector<double> &vector)
{
    for(int x = 0; x < dataSize; x++)
    {
        vector.push_back(data[x]);
    }

    return true;
}

//=========================================================================================//
void vtkSpaceCraftInfo::getCDFUnits(CDFstatus status, CDFid id, int VarNum, QString &UnitText)
{

    long attrN;
    long dataType;
    long numElems;
    char *Units = NULL;

    attrN = CDFgetAttrNum (id, (char*)"UNITS");

    status = CDFinquireAttrzEntry(id, attrN, VarNum, &dataType, &numElems);
    this->checkCDFstatus(status);
    if(status == CDF_OK)
    {
        Units = new char[numElems + 1];
        status = CDFgetAttrzEntry (id, attrN, VarNum, Units);
        this->checkCDFstatus(status);
        Units[numElems] = '\0';

        UnitText = QString(Units);
    }
    else
    {
        UnitText = QString("N/A");
    }

#ifdef DEBUG
    std::cout << "UNITS: " << UnitText.toAscii().data() << std::endl;
#endif

}

//=========================================================================================//
void vtkSpaceCraftInfo::convertEpochToDateTime(QVector<DateTime> &convertedFileEpoch, double *EpochBuffer, long numRecords)
{
    long year;
    long month;
    long day;
    long hour;
    long minute;
    long second;
    long msec;

    for(int c = 0; c < numRecords; c++)
    {
        //break down the epoch
        EPOCHbreakdown(EpochBuffer[c], &year, &month, &day, &hour, &minute, &second, &msec);

        //create DATE_TIME OBJECT from epoch
        DateTime convert(year, month, day, hour, minute, second);
        convertedFileEpoch.push_back( convert);

    }
}

//=========================================================================================//
long vtkSpaceCraftInfo::getNearestLowerIndex(DateTime &neededEpoch, QVector<DateTime> &convertedFileEpoch)
{
    QVector<DateTime>::ConstIterator i = qLowerBound(convertedFileEpoch.constBegin(), convertedFileEpoch.constEnd(), neededEpoch);

    //assign the values
    long indexOfFound;

    if(i != convertedFileEpoch.begin() )
    {
        //get the previous value
        if((*i) > neededEpoch && i != convertedFileEpoch.end())
        {
            --i;
        }

    }

    //make sure we are using a valid location
    if(i != convertedFileEpoch.end())
    {
        indexOfFound = convertedFileEpoch.indexOf((*i));
    }
    else
    {
        indexOfFound = convertedFileEpoch.indexOf(convertedFileEpoch.last());
    }

#ifdef DEBUG
    std::cout << "Requested Date:       " << neededEpoch.getDateTimeString() << std::endl;
    std::cout << "Nearest Lower Found:  " << convertedFileEpoch[indexOfFound].getDateTimeString() << std::endl;
    std::cout << "Found Record:         " << indexOfFound << std::endl;
    std::cout << "---------------------------------" << std::endl;
#endif

    return indexOfFound;
}

//=========================================================================================//
bool vtkSpaceCraftInfo::getDataForEpoch(QString &DataSet, double requestedEpoch, epochDataEntry  &data)
{

    //A Whole Mess of needed Variables...
    QVector<DateTime> convertedFileEpoch;
    DateTime neededEpoch(requestedEpoch);
    double *EpochBuffer;
    long indexOfFound;

    double  *dataUnitD;
    float   *dataUnitF;
    long    *dataUnitL;


    long epochVarNum;
    QVector<double> fileEpochList;
    long numRecords;

    CDFid id;
    CDFstatus status;

    long numDims =0;
    long dimSizes[CDF_MAX_DIMS];

    long encoding =0;
    long majority =0;
    long maxrRec =0;
    long numrVars =0;
    long maxzRec =0;
    long numzVars =0;
    long numAttrs =0;



    //open the file
    status = CDFopenCDF(this->CacheFileName[DataSet].toAscii().data(), &id);
    this->checkCDFstatus(status);

    status = CDFinquireCDF(id, &numDims, dimSizes,
                           &encoding, &majority, &maxrRec,
                           &numrVars, &maxzRec, &numzVars,
                           &numAttrs);
    this->checkCDFstatus(status);

    //get epoch (required)
    epochVarNum = CDFgetVarNum(id, (char*)"Epoch");

    status = CDFgetzVarNumRecsWritten(id, epochVarNum, &numRecords);
    checkCDFstatus(status);

    //get the actual data
    EpochBuffer = new double[numRecords];

    status = CDFgetzVarAllRecordsByVarID(id, epochVarNum, EpochBuffer);
    this->checkCDFstatus(status);
    this->cToQVector(EpochBuffer, numRecords, fileEpochList);

    //convert epoch to DateTime and get nearest index
    convertEpochToDateTime(convertedFileEpoch, EpochBuffer, numRecords);
    indexOfFound = getNearestLowerIndex(neededEpoch, convertedFileEpoch);

    //dont need the buffer anymore
    delete [] EpochBuffer;
    EpochBuffer = NULL;

    //====================================
    //get the data for the required epoch
    //====================================
    //get the variable info
    // TODO: Check to see if MetaVar exists for multi-dimensionsonal

    for(int c = 0; c < numzVars; c++)
    {
        char varName[CDF_VAR_NAME_LEN256 +1];
        long dataType;
        long numElements;
        long recVary;
        long numDims;
        long dimSizes[CDF_MAX_DIMS];
        long dimVarys[CDF_MAX_DIMS];
        //get Variable stats
        status = CDFinquirezVar(id, c, varName, &dataType, &numElements, &numDims,
                                dimSizes, &recVary, dimVarys);
        this->checkCDFstatus(status);

#ifdef DEBUG
        std::cout << "VarName:     " << varName << std::endl;
        std::cout << "Dims:        " << numDims << std::endl;

#endif

        //get the units
        QString Units;
        getCDFUnits(status, id, c, Units);

        //get the data
        //we don't have to worry about multi-dimensional data
        switch(dataType)
        {
        case CDF_FLOAT:
            //allocate space
            dataUnitF = new float[numElements];

            //get data record
            status = CDFgetzVarRecordData(id, c, indexOfFound ,dataUnitF);

#ifdef DEBUG
            std::cout << "================================="  << std::endl;
            std::cout << "DataUnit[" << indexOfFound << "]: " << dataUnitF[0] << std::endl;
#endif

            //inserting the data into the cache
            //we insert each element for the given variable.
            for(int k=0; k < numElements; k++)
            {
                QPair<QVariant, QString> newDataPoint;
                newDataPoint.first = QVariant(dataUnitF[k]);
                newDataPoint.second = Units;

                data[varName].push_back(newDataPoint);
            }

            delete [] dataUnitF;

            break;

        case CDF_DOUBLE:
            //allocate space
            dataUnitD = new double[numElements];

            //get data record
            status = CDFgetzVarRecordData(id, c, indexOfFound ,dataUnitD);


#ifdef DEBUG
            std::cout << "================================="  << std::endl;
            std::cout << "DataUnit[" << indexOfFound << "]: " << dataUnitD[0] << std::endl;
#endif

            //inserting the data into the cache
            //we insert each element for the given variable.
            for(int k=0; k < numElements; k++)
            {
                QPair<QVariant, QString> newDataPoint;
                newDataPoint.first = QVariant(dataUnitD[k]);
                newDataPoint.second = Units;

                data[varName].push_back(newDataPoint);
            }

            delete [] dataUnitD;

            break;

        case CDF_EPOCH:
        case CDF_EPOCH16:


            break;

        case CDF_INT1:
        case CDF_INT2:
        case CDF_INT4:
        case CDF_INT8:

            //allocate space
            dataUnitL = new long[numElements];
            //get data record
            status = CDFgetzVarRecordData(id, c, indexOfFound ,dataUnitL);

#ifdef DEBUG
            std::cout << "================================="  << std::endl;
            std::cout << "DataUnit[" << indexOfFound << "]: " << dataUnitL[0] << std::endl;
#endif

            //inserting the data into the cache
            //we insert each element for the given variable.
            for(int k=0; k < numElements; k++)
            {
                QPair<QVariant, QString> newDataPoint;
                newDataPoint.first = QVariant::fromValue(dataUnitL[k]);
                newDataPoint.second = Units;

                data[varName].push_back(newDataPoint);
            }

            delete [] dataUnitL;

            break;

        default:
            break;

        }


#ifdef DEBUG
        std::cout << "---------------------------------" << std::endl;
#endif





    }

    CDFcloseCDF(id);

    return true;
}


//=========================================================================================//
void vtkSpaceCraftInfo::SetSCIData(const char *group, const char *observatory, const char *list)
{

    //remove list of previous files
    this->uriList.clear();

    std::cout << "Group: " << group << std::endl;
    std::cout << "Observatory: " << observatory << std::endl;
    std::cout <<  "Setting SCI Data: " << list << std::endl;

    QString dataList = QString(list);

    this->requestedData = dataList.split(";");
    this->group = QString(group);
    this->observatory = QString(observatory);

    this->processed = false;

    Status statusBar;
    statusBar.setStatusBarMessage(("Downloading "));
    statusBar.setWindowTitle("Downloading Data...");
    statusBar.show();


    int count = 1;
    int totalcount = requestedData.size();
    double totalSets = 0;
    for(int c = 0; c < totalcount; c++ )
    {
        totalSets = totalSets + this->requestedData[c].split(",").size();
    }

    for(int x = 0; x < this->requestedData.size(); x++)
    {

        std::cout << "Count: " << count << " Progress should be: " << count/totalSets * 100 << std::endl;


        if(!this->requestedData[x].isEmpty())
        {
            //split the incoming data strings
            QStringList Split = this->requestedData[x].split(":");

            QStringList DataSets = Split[1].split(",");
            QString     Instrument = Split[0];


            //iterate over the data strings, get the data, and store localy
            QStringList::Iterator iter;
            for(iter = DataSets.begin(); iter != DataSets.end(); ++iter)
            {
                statusBar.setStatus(count/totalSets * 100);
                statusBar.show();

                // split the Instrument request
                QStringList parts = (*iter).split("~");

                //get the data set
                QString DSet = parts[0];
                QStringList VarSet;

                statusBar.setStatusBarMessage(("Downloading " + DSet));
                statusBar.setStatusCount(QString("Gathering Information..."));
                statusBar.updateAll();

                //get the variables we need to get data on
                if(parts[1] != "")
                {
                    VarSet = parts[1].split("|");

                    filterNetworkAccessModule manager;
                    QString url;

                    DateTime startTime(this->timeSteps.first());
                    startTime.incrementMinutes(-60);
                    DateTime endTime(this->timeSteps.last());
                    endTime.incrementMinutes(+60);

                    url = QString("http://cdaweb.gsfc.nasa.gov/WS/cdasr/1/dataviews/sp_phys")
                            + "/datasets/" + DSet
                            + "/data/"
                            + QString(startTime.getISO8601DateTimeString().c_str())
                            + ","
                            + QString(endTime.getISO8601DateTimeString().c_str()) + "/";

                    for(int a = 0; a < VarSet.size(); a++)
                    {
                        if(a != 0)
                        {
                            url = url + "," + VarSet[a];
                        }
                        else
                        {
                            url = url + VarSet[a];
                        }
                    }

                    url = url + "?format=cdf";

                    std::cerr << url.toAscii().data() << std::endl;

                    //get the data
                    manager.Get(url,QString("DataResult"),QString("FileDescription"));

                    filterNetworkList *objects = manager.getFinalOjects();

                    filterNetworkList::Iterator iter;

                    //save our uri objects for processing
                    for(iter=objects->begin(); iter != objects->end(); ++iter)
                    {
                        filterNetworkObject* currentObject = (*iter);
                        count++;
                        statusBar.setStatus(count/totalSets * 100);
                        statusBar.show();

                        if(currentObject->contains("Name"))
                        {
                            this->uriList[DSet] = currentObject;

                            std::cout << "URI: " << this->uriList[DSet]->operator []("Name").toAscii().data() << std::endl;

                            //Download the actual files
                            statusBar.setStatusBarMessage(("Downloading " + DSet));
                            statusBar.setStatusCount(QString("Getting " + QString::number(this->uriList[DSet]->operator []("Length").toDouble()/1e6) + " MBs"));
                                                             statusBar.show();

                                                     FileDownloader recievedFile(this->uriList[DSet]->operator []("Name") );

                                                     // Save the file to the TEMP space on disk
                                                     QString fileName = this->tempFilePath + DSet + "-" + QString(startTime.getISO8601DateTimeString().c_str()) +  "-" + QString(endTime.getISO8601DateTimeString().c_str()) + ".cdf";
                                    QFile file(fileName);

                            if(file.open(QIODevice::WriteOnly))
                            {
                                QDataStream out(&file);
                                file.write(recievedFile.downloadedData());
                                file.close();

                                this->CacheFileName[DSet] = fileName;
                            }
                            else
                            {
                                std::cerr << "ERROR WRITING TO DISK" << std::endl;

                            }

                        }

                    }

                }
                else
                {
                    count++;
                    statusBar.setStatus(count/totalSets * 100);
                    statusBar.show();
                }

            }

        }

    }

    this->processed = false;

    statusBar.setStatus(100);
    statusBar.show();

    statusBar.hide();

    this->Modified();

}

//=========================================================================================//
//Time Fit Handler
void vtkSpaceCraftInfo::SetTimeFitHandler(int handler)
{
    std::cout << "Selected a New Time Fit Handler" << std::endl;
    this->Modified();
}

//=========================================================================================//
//Bad Data Fit Handler
void vtkSpaceCraftInfo::SetBadDataHandler(int handler)
{
    std::cout << "Selected a New Bad Data Handler" << std::endl;
    this->Modified();
}

//=========================================================================================//
//----- other stuff needed ------//
void vtkSpaceCraftInfo::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
    os << indent << "NumberOfTimeSteps: " << this->NumberOfTimeSteps << std::endl;
}



