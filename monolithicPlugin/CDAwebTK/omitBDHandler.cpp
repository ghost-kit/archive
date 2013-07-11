#include "omitBDHandler.h"

omitBDHandler::omitBDHandler()
    :Superclass()
{
}

omitBDHandler::omitBDHandler(cdfDataReader *reader)
    :Superclass(reader)
{
}

//fix the value
QVector<QVariant> omitBDHandler::fixBadData(int64_t variable, int64_t record)
{

    //set value to NaN
    QVector<QVariant> retVal;
    retVal.push_back(QVariant(std::numeric_limits<double>::quiet_NaN()));
    return retVal;
}

//set the bad data to the new fixed value
cdfDataSet omitBDHandler::getGoodDataRecord(int64_t variable, int64_t record)
{
    cdfDataSet origDataSet = this->reader->getZVariableRecord(variable, record);

    QVariant badData = origDataSet.getInvalidData();

    if(origDataSet.getData().contains(badData))
    {
        origDataSet.setVector(this->fixBadData(variable, record));
    }

    return origDataSet;
}
