#include "cdfDataReader.h"

//make sure your subclass calls this (superclass) intializer
BadDataHandler::BadDataHandler()
{
    this->reader = NULL;
}

BadDataHandler::BadDataHandler(cdfDataReader *reader)
{
    this->reader = reader;
}

//overide this method to handle bad data
QVector<QVariant> BadDataHandler::fixBadData(int64_t variable, int64_t record)
{
    QVariant zero(0);
    QVector<QVariant> retVal;
    retVal.push_back(zero);
    return retVal;
}

//overide this method to change how data is gathered
cdfDataSet BadDataHandler::getGoodDataRecord(int64_t variable, int64_t record)
{

    return reader->getZVariableRecord(variable, record);
}

//this is a helper function that will return the next good value
//the data will be empty if no good data set is found
cdfDataSet BadDataHandler::getNextGoodRecord(int64_t variable, int64_t currentRecord)
{
    std::cerr << "Not Yet Implemented" << std::endl;

    return cdfDataSet();
}

//this is a helper function that will return the previous good value
//the data in the cdfDataSet will be empty if not good data is found
cdfDataSet BadDataHandler::getPreviousGoodRecord(int64_t variable, int64_t currentRecord)
{
    std::cerr << "Not Yet Implemented" << std::endl;

    return cdfDataSet();
}



cdfDataReader *BadDataHandler::getReader() const
{
    return reader;
}

void BadDataHandler::setReader(cdfDataReader *value)
{
    reader = value;
}
