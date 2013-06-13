#ifndef BADDATAHANDLER_H
#define BADDATAHANDLER_H

#include "cdfDataReader.h"

class cdfDataReader;
class cdfDataSet;

class BadDataHandler
{
public:
    BadDataHandler();
    BadDataHandler(cdfDataReader *reader);
    virtual QVector<QVariant> fixBadData(int64_t variable, int64_t record);
    virtual cdfDataSet getGoodDataRecord(int64_t variable, int64_t record);

    cdfDataReader *getReader() const;
    void setReader(cdfDataReader *value);

protected:
    cdfDataSet getNextGoodRecord(int64_t variable, int64_t currentRecord);
    cdfDataSet getPreviousGoodRecord(int64_t variable, int64_t currentRecord);

    cdfDataReader *reader;
};





#endif // BADDATAHANDLER_H
