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

    this->TFHandler = new timeFitHandler(this);
    this->BDHandler = new BadDataHandler(this);

}

//===============================================================//
cdfDataReader::cdfDataReader(QString FileName, BadDataHandler *BDHandler, timeFitHandler *TFHandler)
{
    CDFstatus status;
    //record the file name
    this->FileName = FileName;

    this->openFile();
    status = CDFgetMajority(this->fileId, &majority);

    //print error message if thre is one
    this->CDFstatusOK(status);

    this->TFHandler = TFHandler;
    this->BDHandler = BDHandler;
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
void cdfDataReader::cdfAllocateMemory(long dataType, void* &data, long numValues)
{
    switch(dataType)
    {
    case CDF_REAL4:
    case CDF_FLOAT:

        data = new float[numValues];
        break;

    case CDF_EPOCH:
    case CDF_REAL8:
    case CDF_DOUBLE:
        data = new double[numValues];
        break;

    case CDF_BYTE:
    case CDF_INT1:
        data = new int8_t[numValues];
        break;

    case CDF_UINT1:
        data = new u_int8_t[numValues];
        break;

    case CDF_INT2:
        data = new int16_t[numValues];
        break;

    case CDF_UINT2:
        data = new u_int16_t[numValues];
        break;

    case CDF_INT4:
        data = new int32_t[numValues];
        break;

    case CDF_UINT4:
        data = new u_int32_t[numValues];
        break;

    case CDF_TIME_TT2000:
    case CDF_INT8:
        data = new int64_t[numValues];
        break;

    case CDF_CHAR:
        data = new char[numValues+1];
        break;

    case CDF_UCHAR:
        data = new uchar[numValues+1];
        break;

    case CDF_EPOCH16:
        data = new double[numValues*2];
        break;

    default:
        std::cerr << "INVALID DATA TYPE" << std::endl;
        exit(EXIT_FAILURE);
        break;
    }
}

//===============================================================//
cdfDataSet cdfDataReader::getZVariable(int64_t variable)
{

    std::cout << __FUNCTION__ << std::endl;

    cdfDataSet returnVal;

    cdfVarInfo VarInfo = this->getZVariableInformation(variable);

    for(int x = 0; x < VarInfo.numRecords; x++)
    {
        cdfDataSet record = this->getZVariableRecord(variable,x);

        if(!record.getData().isEmpty())
        {
            returnVal.addNextElement(record.getData()[0]);
        }
        else
        {
            std::cerr << "No value at Record # " << x << std::endl;
        }

    }

    return returnVal;
}

//===============================================================//
cdfDataSet cdfDataReader::getCorrectedZVariableRecord(QString variable, int64_t record)
{
    long varNum = CDFgetVarNum(this->fileId, variable.toAscii().data());
    return this->getCorrectedZVariableRecord(varNum, record);
}

//===============================================================//
cdfDataSet cdfDataReader::getCorrectedZVariableRecord(int64_t variable, int64_t record)
{
    return BDHandler->getGoodDataRecord(variable, record);
}

//===============================================================//
cdfDataSet cdfDataReader::getCorrectedZVariable(QString variable)
{
    long varNum = CDFgetVarNum(this->fileId,variable.toAscii().data());
    return this->getCorrectedZVariable(varNum);
}

//===============================================================//
cdfDataSet cdfDataReader::getCorrectedZVariable(int64_t variable)
{
    std::cout << __FUNCTION__ << std::endl;

    cdfDataSet returnVal;

    cdfVarInfo VarInfo = this->getZVariableInformation(variable);

    for(int x = 0; x < VarInfo.numRecords; x++)
    {
        cdfDataSet record = this->BDHandler->getGoodDataRecord(variable,x);

        if(!record.getData().isEmpty())
        {
            returnVal.addNextElement(record.getData()[0]);
        }
        else
        {
            std::cerr << "No value at Record # " << x << std::endl;
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
    //    std::cout << __FUNCTION__ << std::endl;

    CDFstatus status;
    cdfDataSet returnVal;
    QVector<QVariant> Qdata;

    cdfVarInfo VarInfo = this->getZVariableInformation(variable);

    QList<QVector<QVariant> > badData  = this->getZVariableAttribute(CDFgetAttrNum(this->fileId, (char*)"FILLVAL"), variable );

    void *data = NULL;
    long numValues = 1;


    // proceed only if the record number exists
    if(VarInfo.numRecords > record)
    {
        if(VarInfo.numDims > 0)
        {
            for(int s = 0; s < VarInfo.numDims; s++)
            {
                numValues *= VarInfo.dimSizes[s];
            }
        }


        //allocate data
        std::cout << "Number of Values: " << numValues << std::endl;

        cdfAllocateMemory(VarInfo.dataType, data, numValues);

        //get data record
        status = CDFgetzVarRecordData(this->fileId, variable, record, data);

        //process the data
        if(CDFstatusOK(status))
        {
            if(VarInfo.dataType == CDF_CHAR)
            {
                char* temp = (char*)data;
                temp[numValues] = '\0';
            }

            this->cToQVector(data, numValues, VarInfo.dataType, Qdata);

            //setup the returnvalues
            returnVal.setVector(Qdata);
            returnVal.setVarInfo(VarInfo);
            returnVal.setInvalidData(badData[0][0]);
            returnVal.setMajority(majority);
        }

        //deallocate the data
        //TODO: add remainder of cases for freeing memory
        if(data)
        {
                 if(VarInfo.dataType == CDF_FLOAT)       delete [] (float*)data;
            else if(VarInfo.dataType == CDF_DOUBLE)      delete [] (double*)data;
            else if(VarInfo.dataType == CDF_INT1)        delete [] (int8_t*)data;
            else if(VarInfo.dataType == CDF_UINT1)       delete [] (u_int8_t*)data;
            else if(VarInfo.dataType == CDF_BYTE)        delete [] (int8_t*)data;
            else if(VarInfo.dataType == CDF_INT2)        delete [] (int16_t*)data;
            else if(VarInfo.dataType == CDF_UINT2)       delete [] (u_int16_t*)data;
            else if(VarInfo.dataType == CDF_INT4)        delete [] (int32_t*)data;
            else if(VarInfo.dataType == CDF_UINT4)       delete [] (u_int32_t*)data;
            else if(VarInfo.dataType == CDF_INT8)        delete [] (int64_t*)data;
            else if(VarInfo.dataType == CDF_FLOAT)       delete [] (char*)data;
            else if(VarInfo.dataType == CDF_REAL4)       delete [] (float*)data;
            else if(VarInfo.dataType == CDF_REAL8)       delete [] (float*)data;
            else if(VarInfo.dataType == CDF_EPOCH)       delete [] (double*)data;
            else if(VarInfo.dataType == CDF_EPOCH16)     delete [] (double*)data;
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
cdfVarInfo cdfDataReader::getZVariableInformation(QString variable)
{
    int64_t varNum = CDFgetVarNum(this->fileId, variable.toAscii().data());
    return this->getZVariableInformation(varNum);
}

//===============================================================//
cdfVarInfo cdfDataReader::getZVariableInformation(int64_t variable)
{
    cdfVarInfo retVal;

    long dataType;
    long numElems;
    long numDims;
    long dimSizes[CDF_MAX_DIMS];
    long recVary;
    long dimVarys[CDF_MAX_DIMS];
    char Name[CDF_VAR_NAME_LEN256 +1];

    long numRecords=0;

    CDFstatus status = CDFinquirezVar(this->fileId, variable, Name, &dataType,
                                      &numElems, &numDims, dimSizes, &recVary,
                                      dimVarys);

    //if satus was gotten OK, put in rerturn value
    if(this->CDFstatusOK(status))
    {
        status = CDFgetzVarNumRecsWritten(this->fileId, variable, &numRecords);

        retVal.varNum = variable;
        retVal.dataType = dataType;
        retVal.numberElements = numElems;
        retVal.numDims = numDims;
        retVal.recVary = recVary;
        retVal.Name = QString(Name);
        retVal.numRecords = numRecords;

        for(int x = 0; x < numDims; x++) retVal.dimSizes.push_back(dimSizes[x]);
        for(int x = 0; x < numDims; x++) retVal.dimVarys.push_back(dimVarys[x]);

    }

    return retVal;

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

    if(status != CDF_OK )
    {
        if(status != NO_SUCH_ENTRY)
        {
            char text[CDF_STATUSTEXT_LEN +1];
            CDFgetStatusText(status, text);
            if(!suppress)
            {
                std::cerr << "ERROR: " << text << std::endl;
            }
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

    //TOODO: Refactor this to not repeat code (for easier maintanence)
    switch(dataType)
    {
    case CDF_REAL4:
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
    {
        std::cerr << "This conversion is not yet supported.  It requires 2x double for each item" << std::endl;
        break;
    }
    case CDF_EPOCH:
    case CDF_REAL8:
    case CDF_DOUBLE:
    {
        double *dataD = (double*)data;
        for(int x = 0; x < dataSize; x++)
        {
            vector.push_back(QVariant(dataD[x]));
        }

        break;
    }
    case CDF_BYTE:
    case CDF_INT1:
    {
        int8_t *dataI1 = (int8_t*)data;
        for(int x = 0; x < dataSize; x++)
        {
            vector.push_back(QVariant(dataI1[x]));
        }

        break;
    }
    case CDF_UCHAR:
    case CDF_UINT1:
    {
        u_int8_t *dataI1 = (u_int8_t*)data;
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

    case CDF_UINT2:
    {
        u_int16_t *dataI2 = (u_int16_t*)data;
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
    case CDF_UINT4:
    {
        u_int32_t *dataI4 = (u_int32_t*)data;
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
    {
        char* dataChar = (char*)data;

        vector.push_back(QVariant(QString(dataChar)));
        break;
    }

    default:
        std::cerr << "Unknown Type Conversion: " << dataType << std::endl;
        break;
    }

    return true;
}

//===============================================================//
bool cdfDataReader::containsBadData(QVector<QVariant> data, QVariant badValue)
{
    return data.contains(badValue);
}


//===============================================================//
//===============================================================//
//===============================================================//
//===============================================================//
QString cdfDataSet::getName() const
{
    return varInfo.Name;
}

//===============================================================//
int cdfDataSet::getDataType() const
{
    return varInfo.dataType;
}

//===============================================================//
int cdfDataSet::getDataDims() const
{
    return this->varInfo.numDims;
}

//===============================================================//
long cdfDataSet::getNumberElements() const
{
    return this->varInfo.numberElements;
}

//===============================================================//
cdfDataSet::cdfDataSet()
{

}

QVector<QVariant> cdfDataSet::getData()
{
    return this->data;
}

void cdfDataSet::addNextElement(QVariant item)
{
    this->data.push_back(item);
}

//===============================================================//
void cdfDataSet::setVector(QVector<QVariant> vector)
{
    this->data = vector;
}

//===============================================================//
QVector<int> cdfDataSet::getExtents() const
{
    return this->varInfo.dimSizes;
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


QVariant cdfDataSet::getInvalidData() const
{
    return invalidData;
}

void cdfDataSet::setInvalidData(const QVariant &value)
{
    invalidData = value;
}

timeFitHandler *cdfDataReader::getTFHandler() const
{
    return TFHandler;
}

void cdfDataReader::setTFHandler(timeFitHandler *value)
{
    TFHandler = value;
}

BadDataHandler *cdfDataReader::getBDHandler() const
{
    return BDHandler;
}

void cdfDataReader::setBDHandler(BadDataHandler *value)
{
    BDHandler = value;
}

CDFid cdfDataReader::getFileId() const
{
    return fileId;
}

void cdfDataReader::setFileId(const CDFid &value)
{
    fileId = value;
}

cdfVarInfo cdfDataSet::getVarInfo() const
{
    return varInfo;
}

void cdfDataSet::setVarInfo(const cdfVarInfo &value)
{
    varInfo = value;
}
