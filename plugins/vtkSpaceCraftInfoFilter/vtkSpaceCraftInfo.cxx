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
#include "vtkStringArray.h"
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
#include "vtkTimeStamp.h"

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

    this->startTime =0;
    this->endTime   =0;


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
bool vtkSpaceCraftInfoHandler::processCDAWeb(vtkMultiBlockDataSet *mb)
{
    int numCurrBlocks = mb->GetNumberOfBlocks();
    //clean the block for re-processing.
    if(numCurrBlocks > 0)
    {
        for(int x = 0; x < numCurrBlocks; x++)
        {
            mb->RemoveBlock(x);
        }
    }

    mb->SetNumberOfBlocks(this->DownloadedFileNames.size());

    QMap<QString,spaceCraftDataElement>::Iterator elementsIter;


    long count = 0;

    QMap<QString, QString>::Iterator iter;
    for(iter = this->DownloadedFileNames.begin(); iter != this->DownloadedFileNames.end(); ++iter)
    {

        vtkTable* output = vtkTable::New();

        QString DataSet = this->DownloadedFileNames.key(*iter);
        double time = this->requestedTimeValue;
        QStringList keys = this->DataCache[DataSet][time].keys();

        for(elementsIter = this->DataCache[DataSet][time].begin(); elementsIter != this->DataCache[DataSet][time].end(); ++elementsIter)
        {
            std::cout << "DATA[" << time << "][" << (*elementsIter).varName.toStdString()
                      << "]: " << (*elementsIter).data.toDouble() << " " << (*elementsIter).units.toStdString() << " :Bad Data: "
                      << (*elementsIter).badDataValue.toDouble() << std::endl;


            vtkDoubleArray *newDataElement = vtkDoubleArray::New();

            newDataElement->SetName((*elementsIter).varName.toAscii().data());
            newDataElement->SetNumberOfComponents(1);

            newDataElement->SetComponentName(0, (*elementsIter).units.toAscii().data());

            newDataElement->InsertNextValue( (*elementsIter).data.toDouble());
            output->AddColumn(newDataElement);

            newDataElement->Delete();
        }

        //set block name
        mb->GetMetaData(count)->Set(vtkCompositeDataSet::NAME(), DataSet.toAscii().data());

        //set block output
        mb->SetBlock(count, output);
        output->Delete();
        count++;
    }

    return true;
}

//=========================================================================================//
bool vtkSpaceCraftInfoHandler::processCDAWebSource(vtkMultiBlockDataSet *mb)
{

    int numCurrBlocks = mb->GetNumberOfBlocks();

    std::cout << "Number of blocks reported: " << numCurrBlocks << std::endl;
    //clean the block for re-processing.
    if(numCurrBlocks > 0)
    {
        for(int x = 0; x < numCurrBlocks; x++)
        {
            mb->RemoveBlock(x);
        }
    }

    long count = 0;
    mb->SetNumberOfBlocks(this->DownloadedFileNames.size());

    std::cout << "File Names being processed:" << std::endl;
    for(int h=0; h < this->DownloadedFileNames.size(); h++)
    {
        std::cout << this->DownloadedFileNames.values()[h].toAscii().data() << std::endl;
    }

    std::cout << "Number of NEW blocks: " << mb->GetNumberOfBlocks() << std::endl;

    //    std::cout << __FUNCTION__ << " at Line " << __LINE__ << " in file " << __FILE__ << std::endl;

    QStringList DataSetsAvail = this->DownloadedFileNames.keys();

    //lets now get time series data
    QStringList::Iterator dataSetIter;
    for(dataSetIter = DataSetsAvail.begin(); dataSetIter != DataSetsAvail.end(); ++dataSetIter)
    {
        vtkTable* output = vtkTable::New();

        QList<double> availTimes = this->DataCache[*dataSetIter].keys();
        QStringList varsAvail = this->DataCache[*dataSetIter][availTimes[0]].keys();

        //        std::cout << "Number of time stesp: " << availTimes.size() << std::endl;

        //lets add the MJD first
        vtkDoubleArray *newMJDcolumn = vtkDoubleArray::New();
        vtkStringArray *dateStamp = vtkStringArray::New();
        newMJDcolumn->SetName("Modified Julian Date");
        dateStamp->SetName("Date Stamp");

        QList<double>::Iterator availTimesIter;
        for(availTimesIter = availTimes.begin(); availTimesIter != availTimes.end(); ++availTimesIter)
        {
            newMJDcolumn->InsertNextValue((*availTimesIter));
            dateStamp->InsertNextValue(DateTime(*availTimesIter).getDateTimeString().c_str());
        }
        output->AddColumn(dateStamp);
        output->AddColumn(newMJDcolumn);
        newMJDcolumn->Delete();
        dateStamp->Delete();

        QStringList::Iterator VarIter;
        for(VarIter = varsAvail.begin(); VarIter != varsAvail.end(); ++VarIter)
        {
            QList<spaceCraftDataElement> varOverTime;
            this->getAllTemperalDataFromCacheByVar((*dataSetIter), (*VarIter), varOverTime);
            vtkDoubleArray *newDataColumn = vtkDoubleArray::New();

            //set the column name
            newDataColumn->SetName((*VarIter).toAscii().data());
            newDataColumn->SetNumberOfComponents(1);

            QList<spaceCraftDataElement>::Iterator elementIter;
            int c = 0;
            for(elementIter = varOverTime.begin(); elementIter != varOverTime.end(); ++elementIter)
            {
                newDataColumn->InsertNextValue((*elementIter).data.toDouble());

                c++;
            }

            //            std::cout << "Variable: " << (*VarIter).toAscii().data() << std::endl;
            //            std::cout << "Entering " << c << " values into table." << std::endl;
            output->AddColumn(newDataColumn);
            newDataColumn->Delete();
        }

        //set block name
        mb->GetMetaData(count)->Set(vtkCompositeDataSet::NAME(), (*dataSetIter).toAscii().data());

        //set block output
        mb->SetBlock(count, output);
        output->Delete();
        count++;
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
    std::cout << __FUNCTION__ << std::endl;
    this->cleanData();

    QVector<double> timeSteps;

    QMap<QString, QString>::Iterator iter;
    for(iter = this->DownloadedFileNames.begin(); iter != this->DownloadedFileNames.end(); ++iter)
    {
        //        std::cout << "Reading File: " << (*iter).toAscii().data() << " for Data Set: "
        //                  << this->CacheFileName.key((*iter)).toAscii().data() << std::endl;

        for(int x =0; x < this->timeSteps.size(); x++)
        {
            timeSteps.push_back(this->timeSteps[x]);
        }

        QString DataSet = this->DownloadedFileNames.key(*iter);
        this->getDataForEpochList(DataSet, timeSteps, this->DataCache[DataSet]);

    }

    this->processed = true;
}

//=========================================================================================//
void vtkSpaceCraftInfoHandler::LoadCDFDataSource()
{
    std::cout << __FUNCTION__ << std::endl;
    this->cleanData();

    QMap<QString, QString>::Iterator iter;
    for(iter = this->DownloadedFileNames.begin(); iter != this->DownloadedFileNames.end(); ++iter)
    {
        // get the data set name so we can organize our data
        QString DataSet = this->DownloadedFileNames.key(*iter);

        //        std::cout << "Reading File: " << (*iter).toAscii().data() << " for Data Set: "
        //                  << this->CacheFileName.key((*iter)).toAscii().data() << std::endl;

        //get the data for ALL Epochs
        this->getDataForAllEpochs(DataSet, this->DataCache[DataSet]);
    }

    this->processed = true;
}

//=========================================================================================//
void vtkSpaceCraftInfoHandler::cleanData()
{
    //clear all of the data cache elements
    QMap<double, QMap<QString,spaceCraftDataElement> >::Iterator timeIter;
    QMap <QString, QMap< double, QMap<QString, spaceCraftDataElement > > >::Iterator cacheIter;

    for(cacheIter = this->DataCache.begin(); cacheIter != this->DataCache.end(); ++cacheIter)
    {
        for(timeIter = (*cacheIter).begin(); timeIter != (*cacheIter).end(); ++timeIter)
        {
            (*timeIter).clear();
        }
        (*cacheIter).clear();
    }

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

        //        std::cerr << "Date: " << convert.getDateTimeString() << std::endl;
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

            //            std::cerr << "FOUND EPOCH VARIABLE: " << EpochVar.toAscii().data() << std::endl;
            break;
        }

    }

    if(found == false)
    {
        std::cerr << "Could not find an EPOCH variable in Data Set" << std::endl;

    }

    return found;
}

//=========================================================================================//

bool vtkSpaceCraftInfoHandler::getDataForEpoch(QString &DataSet, double requestedEpoch, epochDataEntry  &data)
{

    //A Whole Mess of needed Variables...
    QVector<DateTime> FileEpochsAsDateTime;
    DateTime neededEpoch(requestedEpoch);
    long indexOfFound=1;
    QString EpochVar;

    //Data File
    cdfDataReader cdfFile(this->DownloadedFileNames[DataSet]);

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
    cdfDataReader cdfFile(this->DownloadedFileNames[DataSet]);

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
            //            std:: cout << "EpochListEntry: " << EpochList[a] << std::endl;

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
            //            std::cout << "Time Iterator: " << indexListOfFound[v] << " numRecords: " << varinfo.numRecords << std::endl;
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

            //            std::cout << "Getting Variable: " << varsAvailable[c].toAscii().data() << " :index: " << indexListOfFound[v] << std::endl;

            QVector<QVariant> dataSet = InData.getData();

            QVector<QVariant>::Iterator iter;

            //cycle through the items

            for(int p =0; p < dataSet.size(); p++)
            {
                QString varNameTmp = InData.getName();
                if(dataSet.size() > 1)
                {
                    varNameTmp = varNameTmp + "_" + QVariant(p).toString();
                    //                    std::cout << "VarName: " << varNameTmp.toStdString() << std::endl;
                }

                data[EpochList[v]][varNameTmp].varName = varNameTmp;
                data[EpochList[v]][varNameTmp].data = dataSet[p];
                data[EpochList[v]][varNameTmp].units = Units;
                data[EpochList[v]][varNameTmp].badDataValue = InData.getInvalidData();

                //                std::cout << "Inserting into " << DateTime(EpochList[v]).getDateTimeString() << std::endl;

                ++iter;
            }
        }

    }


    return true;
}

//=========================================================================================//
bool vtkSpaceCraftInfoHandler::getDataForAllEpochs(QString &DataSet, vtkSpaceCraftInfoHandler::varDataEntry &data)
{
    QVector<DateTime> FileEpochsAsDateTime;
    QString EpochVar;


    //Data File
    cdfDataReader cdfFile(this->DownloadedFileNames[DataSet]);

    //link the handler and reader to each other (Required)
    this->BDhandler->setReader(&cdfFile);
    cdfFile.setBDHandler(this->BDhandler);

    //get list of variables
    QStringList varsAvailable = cdfFile.getZVariableList();

    //Find Epoch Variable (we need it for our time information)
    if(findEpochVar(cdfFile, varsAvailable, EpochVar))
    {
        //get the Epoch data
        cdfDataSet Epoch = cdfFile.getZVariable(EpochVar.toAscii().data());

        //convert to Epoch Times
        this->convertEpochToDateTime(FileEpochsAsDateTime, Epoch);
    }
    else
    {
        std::cerr << "Could not find Epoch Var. Setting Index Found to 1" << std::endl;
    }

    //    std::cout << "Number of Epochs: " <<FileEpochsAsDateTime.size() << std::endl << std::flush;

    //get the data
    for(int c = 0; c < varsAvailable.size(); c++)
    {

        cdfVarInfo varinfo = cdfFile.getZVariableInformation(varsAvailable[c]);

        //skip the variable if it doesn't have the correct index.
        if(varinfo.numRecords < FileEpochsAsDateTime.size())
        {
            std::cerr << "Data Parse Failure on Variable " << varsAvailable[c].toAscii().data() << "..." << std::endl;
            break;
        }

        //get the units
        QString Units;
        getCDFUnits(cdfFile, varsAvailable[c], Units);

        for(int64_t v = 0; v < FileEpochsAsDateTime.size(); v++)
        {
            //get the corrected value so the selected bad data handler will be applied
            cdfDataSet InData = cdfFile.getCorrectedZVariableRecord(varsAvailable[c], v);

            //            std::cout << "Getting Variable: " << varsAvailable[c].toAscii().data() <<  std::endl;

            QVector<QVariant> dataSet = InData.getData();

            QVector<QVariant>::Iterator iter;

            //cycle through the items


            for(int p =0; p < dataSet.size(); p++)
            {
                QString varNameTmp = InData.getName();
                if(dataSet.size() > 1)
                {
                    varNameTmp = varNameTmp + "_" + QVariant(p).toString();
                }
                //                std::cout << "VarName: " << varNameTmp.toStdString() << std::endl;

                data[FileEpochsAsDateTime[v].getMJD()][varNameTmp].varName = varNameTmp;
                data[FileEpochsAsDateTime[v].getMJD()][varNameTmp].data = dataSet[p];
                data[FileEpochsAsDateTime[v].getMJD()][varNameTmp].units = Units;
                data[FileEpochsAsDateTime[v].getMJD()][varNameTmp].badDataValue = InData.getInvalidData();


                ++iter;
            }

        }
    }


    return true;
}

//=========================================================================================//
bool vtkSpaceCraftInfoHandler::getAllTemperalDataFromCacheByVar(const QString DataSet, const QString var, QList<spaceCraftDataElement> &data)
{
    QMap<double, QMap<QString, spaceCraftDataElement> >::Iterator TimeIter;

    int count = 0;
    for(TimeIter = this->DataCache[DataSet].begin(); TimeIter != this->DataCache[DataSet].end(); ++TimeIter)
    {
        data.push_back((*TimeIter)[var]);
        count++;
    }

    //       std::cout << "Number of Time Steps in DataSet: " << DataSet.toAscii().data()  << ": " << count << std::endl;
    return true;
}


//=========================================================================================//
void vtkSpaceCraftInfoHandler::SetSCIData(const char *group, const char *observatory, const char *list)
{
    //mark dirty
    this->processed = false;

    //remove list of previous files
    this->urlMap.clear();
    this->tempFilePath.clear();
    this->requestedData.clear();
    this->DownloadedFileNames.clear();

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

        //        std::cout << "Count: " << count << " Progress should be: " << count/totalSets * 100 << std::endl;

        if(!this->requestedData[x].isEmpty())
        {
            //split the incoming data strings
            QStringList Split = this->requestedData[x].split(":");

            QStringList DataSets = Split[1].split(",");
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


                    //set start time
                    DateTime startTime;
                    if(this->numInputPorts == 1)
                    {
                        startTime.setMJD(this->timeSteps.first());
                    }
                    else
                    {
                        startTime.setMJD(this->startTime);
                    }

                    //set the end time
                    DateTime endTime;
                    if(this->numInputPorts == 1)
                    {
                        endTime.setMJD(this->timeSteps.last());
                    }
                    else
                    {
                        endTime.setMJD(this->endTime);
                    }

                    endTime.incrementMinutes(+60);
                    startTime.incrementMinutes(-60);


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
                            this->urlMap[DSet] = currentObject;

                            std::cout << "URL: " << this->urlMap[DSet]->operator []("Name").toAscii().data() << std::endl;
                            std::cout << "Added to the URL map at key " << DSet.toAscii().data() << std::endl;
                            std::cout << "Size of Map: " << this->urlMap.size() << std::endl;

                            //Download the actual files
                            statusBar.setStatusBarMessage(("Downloading " + DSet));
                            statusBar.setStatusCount(QString("Getting " + QString::number(this->urlMap[DSet]->operator []("Length").toDouble()/1e6) + " MBs"));

                            statusBar.show();

                            FileDownloader recievedFile(this->urlMap[DSet]->operator []("Name") );

                            // Save the file to the TEMP space on disk
                            QString fileName = this->tempFilePath + DSet + "-" + QString(startTime.getISO8601DateTimeString().c_str()) +  "-" + QString(endTime.getISO8601DateTimeString().c_str()) + ".cdf";
                            QFile file(fileName);

                            if(file.open(QIODevice::WriteOnly))
                            {
                                QDataStream out(&file);
                                file.write(recievedFile.downloadedData());
                                file.close();

                                this->DownloadedFileNames[DSet] = fileName;
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

//===============================================//
int vtkSpaceCraftInfoHandler::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    //Get the output Data object

    vtkInformation* info = outputVector->GetInformationObject(0);
    vtkDataObject* doOutput = info->Get(vtkDataObject::DATA_OBJECT());
    vtkMultiBlockDataSet* mb = vtkMultiBlockDataSet::SafeDownCast(doOutput);

    this->outInfo = info;
    this->output = mb;

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

//===============================================//
int vtkSpaceCraftInfoHandler::RequestDataSource(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    vtkInformation* info = outputVector->GetInformationObject(0);
    vtkDataObject* doOutput = info->Get(vtkDataObject::DATA_OBJECT());
    vtkMultiBlockDataSet* mb = vtkMultiBlockDataSet::SafeDownCast(doOutput);

    if(!mb)
    {
        std::cerr << "Failed to create multi-block dataset... think again...  this way doesn't work..." << std::endl;
        return 0;
    }
    else
    {
        std::cout << "MB set created successfully... now what?" << std::endl;
    }

    this->outInfo = info;
    this->output = mb;

    //load the data
    if(!this->processed)
    {
        this->LoadCDFDataSource();
    }

    this->processCDAWebSource(this->output);
    return 1;
}

//===============================================//
int vtkSpaceCraftInfoHandler::RequestInfoFilter(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    //    std::cout << __FUNCTION__ << " on line " << __LINE__ << std::endl;

    this->setInInfo(inputVector[0]->GetInformationObject(0));
    if(this->getInInfo()->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS()))
    {
        //        std::cout << "Getting Number of Time steps" << std::flush << std::endl;
        this->NumberOfTimeSteps = this->inInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
        double *timeValues = this->inInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());

        std::cout << "Number of time steps: " << this->NumberOfTimeSteps  << std::endl;

        //push time steps into list
        this->timeSteps.clear();
        for (int y = 0; y < this->NumberOfTimeSteps; y++)
        {
            this->timeSteps.push_back(timeValues[y]);
        }
        this->TimeRange[0] = this->timeSteps.first();
        this->TimeRange[1] = this->timeSteps.last();
        this->startTime = this->timeSteps.first();
        this->endTime = this->timeSteps.last();
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

//===============================================//
vtkMultiBlockDataSet *vtkSpaceCraftInfoHandler::getOutput() const
{
    return output;
}

//===============================================//
void vtkSpaceCraftInfoHandler::setOutput(vtkMultiBlockDataSet *value)
{
    output = value;
}

//===============================================//
vtkInformation *vtkSpaceCraftInfoHandler::getOutInfo() const
{
    return outInfo;
}

//===============================================//
void vtkSpaceCraftInfoHandler::setOutInfo(vtkInformation *value)
{
    outInfo = value;
}

//===============================================//
vtkInformation *vtkSpaceCraftInfoHandler::getInInfo() const
{
    return inInfo;
}

//===============================================//
void vtkSpaceCraftInfoHandler::setInInfo(vtkInformation *value)
{
    inInfo = value;
}

//===============================================//
int vtkSpaceCraftInfoHandler::getNumberOfTimeSteps() const
{
    return NumberOfTimeSteps;
}

//===============================================//
void vtkSpaceCraftInfoHandler::setNumberOfTimeSteps(int value)
{
    NumberOfTimeSteps = value;
}

//===============================================//
bool vtkSpaceCraftInfoHandler::getProcessed() const
{
    return processed;
}

//===============================================//
void vtkSpaceCraftInfoHandler::setProcessed(bool value)
{
    processed = value;
}

//===============================================//
int vtkSpaceCraftInfoHandler::getNumInputPorts() const
{
    return numInputPorts;
}

//===============================================//
void vtkSpaceCraftInfoHandler::setNumInputPorts(int value)
{
    numInputPorts = value;
}

//===============================================//
int vtkSpaceCraftInfoHandler::getNumOutputPorts() const
{
    return numOutputPorts;
}

//===============================================//
void vtkSpaceCraftInfoHandler::setNumOutputPorts(int value)
{
    numOutputPorts = value;
}

//===============================================//
void vtkSpaceCraftInfoHandler::setTempPath(const char *path)
{
    std::cout << "You are trying to set your path to: " << path << std::endl;
    this->tempFilePath = QString(path);
}
