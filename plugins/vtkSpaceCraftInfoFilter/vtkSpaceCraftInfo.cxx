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

vtkStandardNewMacro(vtkSpaceCraftInfo)

vtkSpaceCraftInfo::vtkSpaceCraftInfo()
{
    this->NumberOfTimeSteps = 0;
    this->processed = false;

    this->tempFilePath = "/tmp/";
}

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

bool vtkSpaceCraftInfo::cToQVector(double *data, long dataSize, QVector<double> &vector)
{
    for(int x = 0; x < dataSize; x++)
    {
        vector.push_back(data[x]);
    }

    return true;
}

bool vtkSpaceCraftInfo::getDataForEpoch(QString DataSet, double requestedEpoch, QMap<QString, QVector<QVector<double> > > &data)
{
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

    std::cout << "DataSet: " << DataSet.toAscii().data() << std::endl;
    std::cout << "FileName: " << this->CacheFileName[DataSet].toAscii().data() << std::endl;


    //open the file
    status = CDFopenCDF(this->CacheFileName[DataSet].toAscii().data(), &id);
    this->checkCDFstatus(status);

    status = CDFinquireCDF(id, &numDims, dimSizes,
                           &encoding, &majority, &maxrRec,
                           &numrVars, &maxzRec, &numzVars,
                           &numAttrs);
    this->checkCDFstatus(status);

#ifdef DEBUG
    std::cout << "Number of Dims: " <<  numDims << std::endl << "encoding: " << encoding << std::endl << "majority: " << majority
              << std::endl << "maxRrec: " << maxrRec <<  std::endl << "numRvars: " << numrVars << std::endl
              << "maxZrec: " << maxzRec << std::endl << "numZvars: " << numzVars << std::endl << "numAttrs: " << numAttrs << std::endl;
#endif

    //get epoch (required)
    QVector<double> fileEpochList;
    long numElements;
    long epochVarNum = CDFgetVarNum(id, (char*)"Epoch");

    status = CDFgetzVarMaxWrittenRecNum(id, epochVarNum, &numElements);
    checkCDFstatus(status);

    //get the actual data
    double *EpochBuffer = new double[numElements];

    status = CDFgetzVarAllRecordsByVarID(id, epochVarNum, EpochBuffer);
    this->checkCDFstatus(status);
    this->cToQVector(EpochBuffer, numElements, fileEpochList);

    double required_epoch = 0;

    QVector<DateTime> convertedFileEpoch;
    DateTime neededEpoch(requestedEpoch);
    std::cout << "Requested Epoch: " << requestedEpoch << std::endl;
    std::cout << "DateTime: " << neededEpoch.getDateTimeString() << std::endl;

    //convert epoch to DateTime
    long year;
    long month;
    long day;
    long hour;
    long minute;
    long second;
    long msec;
    for(int c = 0; c < numElements; c++)
    {
        //break down the epoch
        EPOCHbreakdown(EpochBuffer[c], &year, &month, &day, &hour, &minute, &second, &msec);

        //create DATE_TIME OBJECT from epoch
        convertedFileEpoch.push_back( DateTime(year, month, day, hour, minute, second));
     }

    QVector<DateTime>::Iterator i = qLowerBound(convertedFileEpoch.begin(), convertedFileEpoch.end(), neededEpoch);

    if((*i) > neededEpoch)
    {
        //get the time BEFORE
        //TODO: at some point we will need to interpolate the data for a better fit.
        --i;
    }

    //if we get a bad object, do this...
    if(!convertedFileEpoch.contains((*i)))
    {
        std::cerr << "BAD DATE CONVERSION... WE HAVE AN ERROR IN "
                  << __FUNCTION__ << " near line " << __LINE__ << std::endl;
    }

    std::cout << "Requested: " << neededEpoch.getDateTimeString().c_str() << std::endl;
    std::cout << "Found:     " << (*i).getDateTimeString().c_str() << std::endl;
    std::cout << "===============================================" << std::endl;


    //dont need the buffer anymore
    delete [] EpochBuffer;
    EpochBuffer = NULL;

    //get the data for the required epoch
    for(int c = 0; c < numzVars; c++)
    {

    }




    CDFcloseCDF(id);



    //    char varName[CDF_VAR_NAME_LEN256];
    //    status = CDFgetzVarName(id, i, varName);
    //    this->checkCDFstatus(status);

    //#ifdef DEBUG
    //    std::cout << "Var Name: " << varName << std::endl;
    //#endif




    return true;
}


//=========================================================================================//
void vtkSpaceCraftInfo::SetSCIData(const char *group, const char *observatory, const char *list)
{

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
                    DateTime endTime(this->timeSteps.last());

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
//----- other stuff needed ------//
void vtkSpaceCraftInfo::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
    os << indent << "NumberOfTimeSteps: " << this->NumberOfTimeSteps << std::endl;
}



