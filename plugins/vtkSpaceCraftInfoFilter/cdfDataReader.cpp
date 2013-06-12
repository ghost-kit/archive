//this is a general C++ reader wrapper for some features of
// the CDF reader I need to make my CDAWeb filter work.

#include "cdfDataReader.h"
#include "qstringlist.h"

//===============================================================//
cdfDataReader::cdfDataReader(QString FileName)
{
    CDFstatus status;
    //record the file name
    this->FileName = FileName;

    this->openFile();
    status = CDFgetMajority(this->fileId, &majority);

    //print error message if thre is one
    this->CDFstatusOK(status);

}

//===============================================================//
cdfDataReader::~cdfDataReader()
{
    //currently the file is staying open the entire time the object exists
    //we may want to change this behavior in the future
    this->closeFile();
}

//===============================================================//
QStringList cdfDataReader::getAttributeList()
{
    long numAtts=0;
    char attName[CDF_ATTR_NAME_LEN256 +1];


    QStringList AttNameList;

    //get the number of attributes in the file
    CDFstatus status = CDFgetNumAttributes(this->fileId, &numAtts);

    if(this->CDFstatusOK(status))
    {
        for(long x = 0; x < numAtts; x++)
        {
            //inquire as to what the attribute is
            status = CDFgetAttrName(this->fileId, x, attName);
            if(this->CDFstatusOK(status))
            {
                //add the name to the list of attributes
                AttNameList.push_back(QString(attName));
            }
        }
    }

    //this list will be empty if an error occured
    return AttNameList;
}

//===============================================================//
// this provides all entries and all elements in a given global attribute
QList<QVector<QVariant> > cdfDataReader::getGlobalAttribute(QString Attribute)
{
    QList<QVector<QVariant> > returnVal;

    returnVal = this->getGlobalAttribute(CDFgetAttrNum(this->fileId, Attribute.toAscii().data()));

    return returnVal;
}

//===============================================================//
QList<QVector<QVariant> > cdfDataReader::getGlobalAttribute(int64_t Attribute)
{
    CDFstatus status;
    QList<QVector<QVariant> > returnVal;
    long scope;
    long maxgEntry;
    long maxzEntry;
    long maxrEntry;
    char attName[CDF_ATTR_NAME_LEN256 + 1];

    long numElements;
    long dataType;

    //verify scope
    status = CDFinquireAttr(this->fileId, Attribute, attName, &scope, &maxgEntry, &maxrEntry, &maxzEntry);
    if(this->CDFstatusOK(status))
    {
        //ensure tha the scope is global
        if(scope == GLOBAL_SCOPE)
        {
            for(int x=0; x < maxgEntry; x++)
            {
                status = CDFinquireAttrgEntry(this->fileId, Attribute, x, &dataType, &numElements);

                //if the attribute is valid, get the data
                if(this->CDFstatusOK(status))
                {
                    switch(dataType)
                    {
                    case CDF_FLOAT:
                    {
                        //Data is in Float Values
                        float * dataF = new float[numElements];
                        QVector<QVariant> DataRecord;

                        status = CDFgetAttrgEntry(this->fileId, Attribute, x, dataF);
                        if(this->CDFstatusOK(status))
                        {
                            for(int y = 0; y < numElements; y++)
                            {
                                //add to the Data Record
                                DataRecord.push_back(QVariant(dataF[y]));
                            }

                            //add to the return values
                            returnVal.push_back(DataRecord);
                        }
                        break;
                    }
                    case CDF_DOUBLE:
                    {
                        //Data is in Double Values
                        double * data = new double[numElements];
                        QVector<QVariant> DataRecord;

                        status = CDFgetAttrgEntry(this->fileId, Attribute, x, data);
                        if(this->CDFstatusOK(status))
                        {
                            for(int y = 0; y < numElements; y++)
                            {
                                //add to the Data Record
                                DataRecord.push_back(QVariant(data[y]));
                            }

                            //add to the return values
                            returnVal.push_back(DataRecord);
                        }
                        break;
                    }
                    case CDF_INT1:
                    {
                        int8_t *data = new int8_t[numElements];
                        QVector<QVariant> DataRecord;
                        //Data is in byte form

                        status = CDFgetAttrgEntry(this->fileId, Attribute, x, data);
                        if(this->CDFstatusOK(status))
                        {
                            for(int y = 0; y < numElements; y++)
                            {
                                //add to the Data Record
                                DataRecord.push_back(QVariant(data[y]));
                            }

                            //add to the return values
                            returnVal.push_back(DataRecord);
                        }
                        break;
                    }
                    case CDF_INT2:
                    {
                        int16_t *data = new int16_t[numElements];
                        QVector<QVariant> DataRecord;
                        //Data is 16 bits

                        status = CDFgetAttrgEntry(this->fileId, Attribute, x, data);
                        if(this->CDFstatusOK(status))
                        {
                            for(int y = 0; y < numElements; y++)
                            {
                                //add to the Data Record
                                DataRecord.push_back(QVariant(data[y]));
                            }

                            //add to the return values
                            returnVal.push_back(DataRecord);
                        }
                        break;
                    }
                    case CDF_INT4:
                    {
                        int32_t *data = new int32_t[numElements];
                        QVector<QVariant> DataRecord;
                        //Data is 32 bits

                        status = CDFgetAttrgEntry(this->fileId, Attribute, x, data);
                        if(this->CDFstatusOK(status))
                        {
                            for(int y = 0; y < numElements; y++)
                            {
                                //add to the Data Record
                                DataRecord.push_back(QVariant(data[y]));
                            }

                            //add to the return values
                            returnVal.push_back(DataRecord);
                        }
                        break;
                    }
                    case CDF_INT8:
                    {
                        int64_t *data = new int64_t[numElements];
                        QVector<QVariant> DataRecord;
                        //Data is 64 bits

                        status = CDFgetAttrgEntry(this->fileId, Attribute, x, data);
                        if(this->CDFstatusOK(status))
                        {
                            for(int y = 0; y < numElements; y++)
                            {
                                //add to the Data Record
                                DataRecord.push_back(QVariant(data[y]));
                            }

                            //add to the return values
                            returnVal.push_back(DataRecord);
                        }
                        break;
                    }
                    case CDF_CHAR:
                    {
                        //Data is a stirng
                        char * data = new char[numElements + 1];
                        QVector<QVariant> DataRecord;

                        status = CDFgetAttrgEntry(this->fileId, Attribute, x, data);
                        if(this->CDFstatusOK(status))
                        {
                            data[numElements] = '\0';

                            //add to record
                            QString attr(data);
                            DataRecord.push_back(attr);

                            returnVal.push_back(DataRecord);
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }
        else
        {
            std::cerr << "No global attribute of this type" << std::endl;
        }
    }

    return returnVal;
}

//===============================================================//
QList<QVector<QVariant > > cdfDataReader::getZVariableAttribute(const QString Attribute, const QString Variable)
{

    long attNum = CDFgetAttrNum(this->fileId, Attribute.toAscii().data());
    long varNum = CDFgetVarNum(this->fileId, Variable.toAscii().data());

    //convert the names to values, and call other version of the call
    return  this->getZVariableAttribute(attNum, varNum);
}

//===============================================================//
QList<QVector<QVariant > > cdfDataReader::getZVariableAttribute(const int64_t Attribute, const int64_t Variable)
{
    CDFstatus status;
    QList<QVector<QVariant> > returnVal;

    long scope;
    long maxgEntry;
    long maxzEntry;
    long maxrEntry;
    char attName[CDF_ATTR_NAME_LEN256 + 1];

    long numElements;
    long dataType;

    //verify scope
    status = CDFinquireAttr(this->fileId, Attribute, attName, &scope, &maxgEntry, &maxrEntry, &maxzEntry);
    if(this->CDFstatusOK(status))
    {
        //ensure tha the scope is global
        if(scope == VARIABLE_SCOPE)
        {

            long x = Variable;
            status = CDFinquireAttrzEntry(this->fileId, Attribute, x, &dataType, &numElements);

            //if the attribute is valid, get the data
            if(this->CDFstatusOK(status))
            {
                switch(dataType)
                {
                case CDF_FLOAT:
                {
                    //Data is in Float Values
                    float * dataF = new float[numElements];
                    QVector<QVariant> DataRecord;

                    status = CDFgetAttrzEntry(this->fileId, Attribute, x, dataF);
                    if(this->CDFstatusOK(status))
                    {
                        for(int y = 0; y < numElements; y++)
                        {
                            //add to the Data Record
                            DataRecord.push_back(QVariant(dataF[y]));
                        }

                        //add to the return values
                        returnVal.push_back(DataRecord);
                    }
                    break;
                }
                case CDF_DOUBLE:
                {
                    //Data is in Double Values
                    double * data = new double[numElements];
                    QVector<QVariant> DataRecord;

                    status = CDFgetAttrzEntry(this->fileId, Attribute, x, data);
                    if(this->CDFstatusOK(status))
                    {
                        for(int y = 0; y < numElements; y++)
                        {
                            //add to the Data Record
                            DataRecord.push_back(QVariant(data[y]));
                        }

                        //add to the return values
                        returnVal.push_back(DataRecord);
                    }
                    break;
                }
                case CDF_INT1:
                {
                    int8_t *data = new int8_t[numElements];
                    QVector<QVariant> DataRecord;
                    //Data is in byte form

                    status = CDFgetAttrzEntry(this->fileId, Attribute, x, data);
                    if(this->CDFstatusOK(status))
                    {
                        for(int y = 0; y < numElements; y++)
                        {
                            //add to the Data Record
                            DataRecord.push_back(QVariant(data[y]));
                        }

                        //add to the return values
                        returnVal.push_back(DataRecord);
                    }
                    break;
                }
                case CDF_INT2:
                {
                    int16_t *data = new int16_t[numElements];
                    QVector<QVariant> DataRecord;
                    //Data is 16 bits

                    status = CDFgetAttrzEntry(this->fileId, Attribute, x, data);
                    if(this->CDFstatusOK(status))
                    {
                        for(int y = 0; y < numElements; y++)
                        {
                            //add to the Data Record
                            DataRecord.push_back(QVariant(data[y]));
                        }

                        //add to the return values
                        returnVal.push_back(DataRecord);
                    }
                    break;
                }
                case CDF_INT4:
                {
                    int32_t *data = new int32_t[numElements];
                    QVector<QVariant> DataRecord;
                    //Data is 32 bits

                    status = CDFgetAttrzEntry(this->fileId, Attribute, x, data);
                    if(this->CDFstatusOK(status))
                    {
                        for(int y = 0; y < numElements; y++)
                        {
                            //add to the Data Record
                            DataRecord.push_back(QVariant(data[y]));
                        }

                        //add to the return values
                        returnVal.push_back(DataRecord);
                    }
                    break;
                }
                case CDF_INT8:
                {
                    int64_t *data = new int64_t[numElements];
                    QVector<QVariant> DataRecord;
                    //Data is 64 bits

                    status = CDFgetAttrzEntry(this->fileId, Attribute, x, data);
                    if(this->CDFstatusOK(status))
                    {
                        for(int y = 0; y < numElements; y++)
                        {
                            //add to the Data Record
                            DataRecord.push_back(QVariant(data[y]));
                        }

                        //add to the return values
                        returnVal.push_back(DataRecord);
                    }
                    break;
                }
                case CDF_CHAR:
                {
                    //Data is a stirng
                    char * data = new char[numElements + 1];
                    QVector<QVariant> DataRecord;

                    status = CDFgetAttrzEntry(this->fileId, Attribute, x, data);
                    if(this->CDFstatusOK(status))
                    {
                        data[numElements] = '\0';

                        //add to record
                        QString attr(data);
                        DataRecord.push_back(attr);

                        returnVal.push_back(DataRecord);
                    }
                    break;
                }
                default:
                    break;
                }
            }

        }
        else
        {
            std::cerr << "No global attribute of this type" << std::endl;
        }
    }

    return returnVal;
}

//===============================================================//
QMap<QString, QList<QVector< QVariant > > > cdfDataReader::getZVariableAttributes(QStringList Attributes, QString Variable)
{
    QMap< QString, QList<QVector <QVariant > > > returnVal;
    QStringList::Iterator iter;
    long varN =  CDFgetVarNum(this->fileId, Variable.toAscii().data());

    //for each Attribute in the list, get the attributes for the given variable
    for(iter = Attributes.begin(); iter != Attributes.end(); ++iter)
    {
        long attN =  CDFgetAttrNum(this->fileId, (*iter).toAscii().data());

        returnVal[*iter] = this->getZVariableAttribute(attN, varN);
    }

    return returnVal;
}

//===============================================================//
QMap<QString, QList<QVector< QVariant > > > cdfDataReader::getZVaraibleAttributes(QStringList Attributes, int64_t Variable)
{
    QMap< QString, QList<QVector <QVariant > > > returnVal;
    QStringList::Iterator iter;
    long varN =  Variable;

    //for each Attribute in the list, get the attributes for the given variable
    for(iter = Attributes.begin(); iter != Attributes.end(); ++iter)
    {
        long attN =  CDFgetAttrNum(this->fileId, (*iter).toAscii().data());

        returnVal[*iter] = this->getZVariableAttribute(attN, varN);
    }

    return returnVal;
}


//===============================================================//
QStringList cdfDataReader::getZVariableList()
{
    QStringList returnVal;

    CDFstatus status;
    long numVars;

    //get the number of variables
    status = CDFgetNumzVars(this->fileId, &numVars);

    if(this->CDFstatusOK(status))
    {
        for(int x = 0; x < numVars; x++)
        {
            char text[CDF_VAR_NAME_LEN256 + 1];
            status = CDFgetzVarName(this->fileId, x, text);
            //make sure we actual got a name
            if(this->CDFstatusOK(status))
            {
                //add the name to the list
                returnVal.push_back(QString(text));
            }
        }
    }

    return returnVal;
}

//===============================================================//
cdfDataSet cdfDataReader::getZVariable(QString variable)
{
    // get the number for the name
    long varNum = CDFgetVarNum(this->fileId, variable.toAscii().data());
    return this->getZVariable(varNum);
}

//===============================================================//
cdfDataSet cdfDataReader::getZVariable(int64_t variable)
{
    cdfDataSet returnVal;
    CDFstatus status;

    long numRecs=0;
    long numDims=0;
    long dimSizes[CDF_MAX_DIMS];
    long dimVarys[CDF_MAX_DIMS];
    long numValues=0;
    char varName[CDF_VAR_NAME_LEN256+1];

    long numElements=0;
    long recVary=0;
    long dataType=0;

    status = CDFinquirezVar(this->fileId, variable, varName, &dataType, &numElements,
                            &numDims, dimSizes, &recVary, dimVarys);

    if(CDFstatusOK(status))
    {
        status = CDFgetzVarMaxWrittenRecNum(this->fileId, variable, &numRecs);

        if(CDFstatusOK(status))
        {
            void * data = NULL;

            //calculate the number of values
            numValues = 1;
            for(int i = 0; i < numDims; ++i) numValues *= dimSizes[i];
            numValues *= numRecs;

            //allocate data
            switch(dataType)
            {
            case CDF_FLOAT:
                data = new float[numValues];
                break;
            case CDF_DOUBLE:
                data = new double[numValues];
                break;
            case CDF_INT1:
                data = new int8_t[numValues];
                break;
            case CDF_INT2:
                data = new int16_t[numValues];
                break;
            case CDF_INT4:
                data = new int32_t[numValues];
                break;
            case CDF_INT8:
                data = new int64_t[numValues];
                break;
            case CDF_CHAR:
                //variables are characters
                data = new char[numValues+1];
                break;
            case CDF_EPOCH:
            case CDF_EPOCH16:
                data = new double[numValues];
                break;
            default:
                std::cerr << "INVALID DATA TYPE" << std::endl;
                exit(EXIT_FAILURE);
                break;
            }

            //get data record
            status = CDFgetzVarAllRecordsByVarID(this->fileId, variable, data);
            QVector<QVariant> Qdata;
            this->cToQVector(data, numValues, dataType, Qdata);

            if(dataType == CDF_CHAR)
            {
                char* temp = (char*)data;
                temp[numValues] = '\0';
            }

            //process the data
            if(CDFstatusOK(status))
            {
                //setup the returnvalues
                returnVal.setVector(Qdata);
                returnVal.setName(varName);
                returnVal.setDataDims(numDims);
                returnVal.setDataType(dataType);
                returnVal.setNumberElements(numElements);

                QVector<int> Extents;
                for(int k = 0; k < numDims; k++)
                {
                    //put the extents into the extents
                    Extents.push_back(dimSizes[k]);
                }

                //set the return values
                returnVal.setExtents(Extents);
                returnVal.setMajority(majority);
            }

            //deallocate the data
            if(data)
            {
                if(dataType == CDF_FLOAT)            delete [] (float*)data;
                else if(dataType == CDF_DOUBLE)      delete [] (double*)data;
                else if(dataType == CDF_INT1)        delete [] (int8_t*)data;
                else if(dataType == CDF_INT2)        delete [] (int16_t*)data;
                else if(dataType == CDF_INT4)        delete [] (int32_t*)data;
                else if(dataType == CDF_INT8)        delete [] (int64_t*)data;
                else if(dataType == CDF_FLOAT)       delete [] (char*)data;
                else if(dataType == CDF_EPOCH)       delete [] (double*)data;

            }
        }
    }

    return returnVal;
}

//===============================================================//
cdfDataSet cdfDataReader::getZVariableRecord(QString variable, int64_t record)
{

    return this->getZVariableRecord(CDFgetVarNum(this->fileId, variable.toAscii().data()), record);

}

//===============================================================//
cdfDataSet cdfDataReader::getZVariableRecord(int64_t variable, int64_t record)
{
    CDFstatus status;
    cdfDataSet returnVal;

    char varName[CDF_VAR_NAME_LEN256 +1];
    long dataType=0;
    long numElements=0;
    long recVary=0;
    long numDims=0;
    long dimSizes[CDF_MAX_DIMS];
    long dimVarys[CDF_MAX_DIMS];
    long numValues=1;

    status = CDFinquirezVar(this->fileId, variable, varName, &dataType, &numElements, &numDims,
                            dimSizes, &recVary, dimVarys);

    if(this->CDFstatusOK(status))
    {
        void *data = NULL;
        //calculte the number of values to be retrieved
        numValues = numElements;

        switch(dataType)
        {
        case CDF_FLOAT:

            data = new float[numValues];
            break;

        case CDF_DOUBLE:
            data = new double[numValues];
            break;

        case CDF_INT1:
            data = new int8_t[numValues];
            break;

        case CDF_INT2:
            data = new int16_t[numValues];
            break;

        case CDF_INT4:
            data = new int32_t[numValues];
            break;

        case CDF_INT8:
            data = new int64_t[numValues];
            break;

        case CDF_CHAR:
            data = new char[numValues+1];
            break;

        case CDF_EPOCH:
        case CDF_EPOCH16:
            data = new double[numValues];
            break;

        default:
            std::cerr << "Unsuported data type" << std::endl;
            exit(EXIT_FAILURE);
            break;
        }

        //get data record
        status = CDFgetzVarRecordData(this->fileId, variable, record ,data);
        QVector<QVariant> Qdata;
        this->cToQVector(data, numValues, dataType, Qdata);

        if(dataType == CDF_CHAR)
        {
            char* temp = (char*)data;
            temp[numValues] = '\0';
        }
        //process the data
        if(CDFstatusOK(status))
        {
            //setup the returnvalues
            returnVal.setVector(Qdata);
            returnVal.setName(varName);
            returnVal.setDataDims(numDims);
            returnVal.setDataType(dataType);
            returnVal.setNumberElements(numElements);

            QVector<int> Extents;
            for(int k = 0; k < numDims; k++)
            {
                //put the extents into the extents
                Extents.push_back(dimSizes[k]);
            }

            //set the return values
            returnVal.setExtents(Extents);
            returnVal.setMajority(majority);
        }

        //deallocate the data
        if(data)
        {
            if(dataType == CDF_FLOAT)            delete [] (float*)data;
            else if(dataType == CDF_DOUBLE)      delete [] (double*)data;
            else if(dataType == CDF_INT1)        delete [] (int8_t*)data;
            else if(dataType == CDF_INT2)        delete [] (int16_t*)data;
            else if(dataType == CDF_INT4)        delete [] (int32_t*)data;
            else if(dataType == CDF_INT8)        delete [] (int64_t*)data;
            else if(dataType == CDF_FLOAT)       delete [] (char*)data;
            else if(dataType == CDF_EPOCH)       delete [] (double*)data;
        }
    }


    return returnVal;
}


//===============================================================//
cdfDataSet cdfDataReader::getZVariableRecords(QString variable, QVector<int64_t> recordList)
{

    std::cerr << "NOT YET IMPLEMENTED" << std::endl;
    cdfDataSet returnVal;

    return returnVal;
}

//===============================================================//
cdfDataSet cdfDataReader::getZVariableRecords(int64_t variable, QVector<int64_t> recordList)
{
    std::cerr << "NOT YET IMPLEMENTED" << std::endl;
    cdfDataSet returnVal;

    return returnVal;
}

//===============================================================//
//                         Private Methods                       //
//===============================================================//
bool cdfDataReader::openFile()
{
    CDFstatus status;
    status = CDFopenCDF(this->FileName.toAscii().data(), &this->fileId);

    return this->CDFstatusOK(status);
}

//===============================================================//
bool cdfDataReader::closeFile()
{
    CDFstatus status;
    status = CDFcloseCDF(this->fileId);

    //return the error status
    return this->CDFstatusOK(status);

}

//===============================================================//
bool cdfDataReader::CDFstatusOK(CDFstatus status, bool suppress)
{

    if(status != CDF_OK)
    {
        char text[CDF_STATUSTEXT_LEN +1];
        CDFgetStatusText(status, text);
        if(!suppress)
        {
            std::cerr << "ERROR: " << text << std::endl;
        }
        //error closing file
        return false;
    }

    //no errors
    return true;
}

//===============================================================//
bool cdfDataReader::cToQVector(void *data, long dataSize, long dataType ,QVector<QVariant> &vector)
{

    switch(dataType)
    {
    case CDF_FLOAT:
    {
        float *dataF = (float*)data;
        for(int x = 0; x < dataSize; x++)
        {
            vector.push_back(QVariant(dataF[x]));
        }

        break;
    }
    case CDF_EPOCH16:
    case CDF_EPOCH:
    case CDF_DOUBLE:
    {
        double *dataD = (double*)data;
        for(int x = 0; x < dataSize; x++)
        {
            vector.push_back(QVariant(dataD[x]));
        }

        break;
    }
    case CDF_INT1:
    {
        int8_t *dataI1 = (int8_t*)data;
        for(int x = 0; x < dataSize; x++)
        {
            vector.push_back(QVariant(dataI1[x]));
        }

        break;
    }
    case CDF_INT2:
    {
        int16_t *dataI2 = (int16_t*)data;
        for(int x = 0; x < dataSize; x++)
        {
            vector.push_back(QVariant(dataI2[x]));
        }
        break;
    }
    case CDF_INT4:
    {
        int32_t *dataI4 = (int32_t*)data;
        for(int x = 0; x < dataSize; x++)
        {
            vector.push_back(QVariant(dataI4[x]));
        }
        break;
    }
    case CDF_INT8:
    {
        int64_t *dataI8 = (int64_t*)data;
        for(int x = 0; x < dataSize; x++)
        {
            vector.push_back(QVariant(dataI8[x]));
        }
        break;
    }
    case CDF_CHAR:
        break;
    default:
        break;
    }

    return true;
}


//===============================================================//
//===============================================================//
//===============================================================//
//===============================================================//
QString cdfDataSet::getName() const
{
    return Name;
}

//===============================================================//
void cdfDataSet::setName(const QString &value)
{
    Name = value;
}

//===============================================================//
int cdfDataSet::getDataType() const
{
    return dataType;
}

//===============================================================//
void cdfDataSet::setDataType(int value)
{
    dataType = value;
}

//===============================================================//
int cdfDataSet::getDataDims() const
{
    return dataDims;
}

//===============================================================//
void cdfDataSet::setDataDims(int value)
{
    dataDims = value;
}

//===============================================================//
cdfDataSet::cdfDataSet()
{
    this->dataType = 0;
    this->dataDims = 0;
}

QVector<QVariant> cdfDataSet::getData()
{
    return this->data;
}

//===============================================================//
void cdfDataSet::setVector(QVector<QVariant> vector)
{
    this->data = vector;
}

//===============================================================//
QVector<int> cdfDataSet::getExtents() const
{
    return Extents;
}

//===============================================================//
void cdfDataSet::setExtents(const QVector<int> &value)
{
    Extents = value;
}

//===============================================================//
int cdfDataSet::getMajority() const
{
    return majority;
}

//===============================================================//
void cdfDataSet::setMajority(int value)
{
    majority = value;
}

long cdfDataSet::getNumberElements() const
{
    return numberElements;
}

void cdfDataSet::setNumberElements(long value)
{
    numberElements = value;
}
