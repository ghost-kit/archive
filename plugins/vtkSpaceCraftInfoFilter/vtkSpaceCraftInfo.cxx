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

#include "cdfDataReader.h"

//Data Handlers
#include "BadDataHandler.h"
#include "omitBDHandler.h"


//=========================================================================================//
vtkSpaceCraftInfoHandler::vtkSpaceCraftInfoHandler()
{
    this->NumberOfTimeSteps = 0;
    this->processed = false;

    this->tempFilePath = "/tmp/";

    //set the generic (non-bound) handlers
    //handlers must be bound to a reader before use
    this->BDhandler = new BadDataHandler();
    this->TFhandler = new timeFitHandler();
    this->TimeRange[0] = 0;
    this->TimeRange[1] = 0;


}

//=========================================================================================//
vtkSpaceCraftInfoHandler::~vtkSpaceCraftInfoHandler()
{

}


//=========================================================================================//
//----- helper function -----//
//getTimeSteps() returns a NEW double array that *must*
//  be managed by the caller. TODO: Fix this to manage memory slef.
double *vtkSpaceCraftInfoHandler::getTimeSteps()
{
    double *ret = new double[this->NumberOfTimeSteps];

    for(int i = 0; i < this->NumberOfTimeSteps; i++)
    {
        ret[i] = this->timeSteps[i];
    }

    return ret;
}

//=========================================================================================//
bool vtkSpaceCraftInfoHandler::processCDAWeb(vtkTable *output)
{

    QMap<QString, QString>::Iterator iter;
    for(iter = this->CacheFileName.begin(); iter != this->CacheFileName.end(); ++iter)
    {


        QString DataSet = this->CacheFileName.key(*iter);
        double time = this->requestedTimeValue;
        QStringList keys = this->DataCache[DataSet][time].keys();

        for(int q=0; q<keys.size(); q++)
        {
            std::cout << "DATA[" << time << "][" << keys[q].toAscii().data() << "]: " << this->DataCache[DataSet][time][keys[q]][0].first.toDouble()
                    << " " << this->DataCache[DataSet][time][keys[q]][0].second.first.toAscii().data() <<  " :Bad Data: " <<  this->DataCache[DataSet][time][keys[q]][0].second.second.toDouble() << std::endl;

            long numElements = this->DataCache[DataSet][time][keys[q]].size();

            vtkDoubleArray *newArray = vtkDoubleArray::New();
            newArray->SetName(keys[q].toAscii().data());
            newArray->SetNumberOfComponents(numElements);

            double *newData = new double[numElements];
            for(int a=0; a < numElements; a++)
            {
                newData[a] = this->DataCache[DataSet][time][keys[q]][a].first.toDouble();
                newArray->SetComponentName(a, this->DataCache[DataSet][time][keys[q]][a].second.first.toAscii().data());
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
void vtkSpaceCraftInfoHandler::checkCDFstatus(CDFstatus status)
{
    char text[CDF_STATUSTEXT_LEN+1];
    if(status != CDF_OK)
    {
        CDFgetStatusText(status, text);
        std::cerr << "ERROR: " << text << std::endl;
    }
}

//=========================================================================================//
void vtkSpaceCraftInfoHandler::LoadCDFData()
{

    QVector<double> timeSteps;

    QMap<QString, QString>::Iterator iter;
    for(iter = this->CacheFileName.begin(); iter != this->CacheFileName.end(); ++iter)
    {
        std::cout << "Reading File: " << (*iter).toAscii().data() << " for Data Set: "
                  << this->CacheFileName.key((*iter)).toAscii().data() << std::endl;

        for(int x =0; x < this->timeSteps.size(); x++)
        {
            timeSteps.push_back(this->timeSteps[x]);
        }

        QString DataSet = this->CacheFileName.key(*iter);
        this->getDataForEpochList(DataSet, timeSteps, this->DataCache[DataSet]);

    }

    this->processed = true;
}

//=========================================================================================//
bool vtkSpaceCraftInfoHandler::cToQVector(double *data, long dataSize, QVector<double> &vector)
{
    for(int x = 0; x < dataSize; x++)
    {
        vector.push_back(data[x]);
    }

    return true;
}

//=========================================================================================//
void vtkSpaceCraftInfoHandler::getCDFUnits(cdfDataReader &reader, QString &VarName, QString &UnitText)
{
    QString attName = QString("UNITS");

    QList<QVector<QVariant> >  Units = reader.getZVariableAttribute(attName, VarName);

    if(Units.size() > 0)
    {
        UnitText = Units[0][0].toString();
    }
    else
    {
        UnitText = QString("N/A");
    }


}

//=========================================================================================//
void vtkSpaceCraftInfoHandler::convertEpochToDateTime(QVector<DateTime> &convertedFileEpoch, cdfDataSet Epoch)
{
    long year;
    long month;
    long day;
    long hour;
    long minute;
    long second;
    long msec;

    QVector<QVariant> epochList = Epoch.getData();

    for(int c = 0; c < epochList.size(); c++)
    {
        //break down the epoch
        EPOCHbreakdown(epochList[c].toDouble(), &year, &month, &day, &hour, &minute, &second, &msec);

        //create DATE_TIME OBJECT from epoch
        DateTime convert(year, month, day, hour, minute, second);
        convertedFileEpoch.push_back( convert);

    }
}

//=========================================================================================//
long vtkSpaceCraftInfoHandler::getNearestLowerIndex(DateTime &neededEpoch, QVector<DateTime> &convertedFileEpoch)
{
    QVector<DateTime>::Iterator i = qLowerBound(convertedFileEpoch.begin(), convertedFileEpoch.end(), neededEpoch);

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
bool vtkSpaceCraftInfoHandler::findEpochVar(cdfDataReader &cdfFile, QStringList &varsAvailable, QString &EpochVar)
{
    //iterators
    QStringList::Iterator SLiter;
    bool found = false;

    for(SLiter = varsAvailable.begin(); SLiter != varsAvailable.end(); ++SLiter)
    {
        cdfVarInfo variableInformation = cdfFile.getZVariableInformation(*SLiter);

        if(variableInformation.dataType == CDF_EPOCH)
        {
            EpochVar = *SLiter;
            found = true;

            std::cerr << "FOUND EPOCH VARIABLE: " << EpochVar.toAscii().data() << std::endl;
            break;
        }

    }

    if(found == false)
    {
        std::cerr << "Could not find an EPOCH variable in Data Set" << std::endl;

    }

    return found;
}



bool vtkSpaceCraftInfoHandler::getDataForEpoch(QString &DataSet, double requestedEpoch, epochDataEntry  &data)
{

    //A Whole Mess of needed Variables...
    QVector<DateTime> FileEpochsAsDateTime;
    DateTime neededEpoch(requestedEpoch);
    long indexOfFound=1;
    QString EpochVar;

    //Data File
    cdfDataReader cdfFile(this->CacheFileName[DataSet]);

    //link the handler and reader to each other (Required)
    this->BDhandler->setReader(&cdfFile);
    cdfFile.setBDHandler(this->BDhandler);

    //get list of variables
    QStringList varsAvailable = cdfFile.getZVariableList();

    //Find Epoch Variable
    if(findEpochVar(cdfFile, varsAvailable, EpochVar))
    {
        //get the Epoch data
        cdfDataSet Epoch = cdfFile.getZVariable(EpochVar.toAscii().data());

        //convert to Epoch Times
        this->convertEpochToDateTime(FileEpochsAsDateTime, Epoch);

        // get the next lowest time
        indexOfFound = getNearestLowerIndex(neededEpoch, FileEpochsAsDateTime);
    }
    else
    {
        std::cerr << "Could not find Epoch Var. Setting Index Found to 1" << std::endl;

    }

    for(int c = 0; c < varsAvailable.size(); c++)
    {

        cdfVarInfo varinfo = cdfFile.getZVariableInformation(varsAvailable[c]);

        //skip the variable if it doesn't have the correct index.
        if(varinfo.numRecords < indexOfFound) continue;

        //get the units
        QString Units;
        getCDFUnits(cdfFile, varsAvailable[c], Units);

        //get the corrected value so the selected bad data handler will be applied
        cdfDataSet InData = cdfFile.getCorrectedZVariableRecord(c, indexOfFound);
        QVector<QVariant> dataSet = InData.getData();

        QVector<QVariant>::Iterator iter;

        //cycle through the items
        for(iter = dataSet.begin(); iter != dataSet.end();)
        {
            QPair<QVariant,QPair<QString, QVariant> > newElem;
            newElem.first = *iter;
            newElem.second.first = Units;
            newElem.second.second = InData.getInvalidData();

            data[InData.getName()].push_back(newElem);
            ++iter;
        }

    }
    return true;
}

//=========================================================================================//
bool vtkSpaceCraftInfoHandler::getDataForEpochList(QString &DataSet, QVector<double> &EpochList, varDataEntry &data)
{
    //we want to process an entire list of epochs at a time instead of just one

    //A Whole Mess of needed Variables...
    QVector<DateTime> FileEpochsAsDateTime;
    QVector<long> indexListOfFound;
    QString EpochVar;

    //Data File
    cdfDataReader cdfFile(this->CacheFileName[DataSet]);

    //link the handler and reader to each other (Required)
    this->BDhandler->setReader(&cdfFile);
    cdfFile.setBDHandler(this->BDhandler);

    //get list of variables
    QStringList varsAvailable = cdfFile.getZVariableList();

    //Find Epoch Variable
    if(findEpochVar(cdfFile, varsAvailable, EpochVar))
    {
        //get the Epoch data
        cdfDataSet Epoch = cdfFile.getZVariable(EpochVar.toAscii().data());

        //convert to Epoch Times
        this->convertEpochToDateTime(FileEpochsAsDateTime, Epoch);

        // get the next lowest time for all epochs
        for(int a =0; a < EpochList.size(); a++)
        {
            std:: cout << "EpochListEntry: " << EpochList[a] << std::endl;

            DateTime neededEpoch(EpochList[a]);

            indexListOfFound.push_back(getNearestLowerIndex(neededEpoch, FileEpochsAsDateTime));
        }
    }
    else
    {
        std::cerr << "Could not find Epoch Var. Setting Index Found to 1" << std::endl;
    }

    QVector<long>::Iterator timeIter;

    //get the data
    for(int c = 0; c < varsAvailable.size(); c++)
    {

        cdfVarInfo varinfo = cdfFile.getZVariableInformation(varsAvailable[c]);

        for(int v = 0; v < indexListOfFound.size(); v++)
        {
            //skip the variable if it doesn't have the correct index.
            std::cout << "Time Iterator: " << indexListOfFound[v] << " numRecords: " << varinfo.numRecords << std::endl;
            if(varinfo.numRecords < (indexListOfFound[v]))
            {
                std::cerr << "Failure..." << std::endl;
                break;
            }

            //get the units
            QString Units;
            getCDFUnits(cdfFile, varsAvailable[c], Units);

            //get the corrected value so the selected bad data handler will be applied
            cdfDataSet InData = cdfFile.getCorrectedZVariableRecord(varsAvailable[c], indexListOfFound[v]);

            std::cout << "Getting Variable: " << varsAvailable[c].toAscii().data() << " :index: " << indexListOfFound[v] << std::endl;

            QVector<QVariant> dataSet = InData.getData();

            QVector<QVariant>::Iterator iter;

            //cycle through the items

            for(int p =0; p < dataSet.size(); p++)
            {
                QPair<QVariant, QPair<QString, QVariant> >  newData;

                newData.first = dataSet[p];
                newData.second.first = Units;
                newData.second.second = InData.getInvalidData();

                QString varNameTmp = InData.getName();
                if(dataSet.size() > 1)
                {
                    varNameTmp = varNameTmp + "_" + QVariant(p).toString();
                    std::cout << "VarName: " << varNameTmp.toStdString() << std::endl;
                }

                data[EpochList[v]][varNameTmp].push_back(newData);
                std::cout << "Inserting into " << DateTime(EpochList[v]).getDateTimeString() << std::endl;

                ++iter;
            }
        }

    }


    return true;
}


//=========================================================================================//
void vtkSpaceCraftInfoHandler::SetSCIData(const char *group, const char *observatory, const char *list)
{
    //mark dirty
    this->processed = false;

    //remove list of previous files
    this->uriList.clear();

    std::cout << "Group: " << group << std::endl;
    std::cout << "Observatory: " << observatory << std::endl;
    std::cout <<  "Setting SCI Data: " << list << std::endl;

    QString dataList = QString(list);

    this->requestedData = dataList.split(";");
    this->group = QString(group);
    this->observatory = QString(observatory);

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


    statusBar.setStatus(100);
    statusBar.show();

    statusBar.hide();

}

//=========================================================================================//
//Time Fit Handler
void vtkSpaceCraftInfoHandler::SetTimeFitHandler(int handler)
{
    std::cout << "Selected a New Time Fit Handler" << std::endl;
}

//=========================================================================================//
//Bad Data Fit Handler
void vtkSpaceCraftInfoHandler::SetBadDataHandler(int handler)
{
    //mark dirty
    this->processed = false;

    //set the new handler
    switch(handler)
    {
    case 0:
        std::cout << "BDHandler: default BadDataHandler" << std::endl;
        delete [] this->BDhandler;
        this->BDhandler = new BadDataHandler();

        break;
    case 1:
        std::cerr << "Linear Interp not yet implemented. Using last selected." << std::endl;
        break;
    case 2:
        std::cout << "BDHandler: omitBDhandler" << std::endl;
        delete [] this->BDhandler;
        this->BDhandler = new omitBDHandler();

        break;

    default:
        break;
    }

}





