//this is a bad data handler for our custom cdfDataReader C++ class
//you can use this class as a sample of how you might be able to go about extending
// this filter to use different time handlers.


#ifndef OMITBDHANDLER_H
#define OMITBDHANDLER_H
#include "BadDataHandler.h"
#include <limits>

class omitBDHandler : public BadDataHandler
{
    typedef BadDataHandler Superclass;
public:
    omitBDHandler();
    omitBDHandler(cdfDataReader *reader);

    QVector<QVariant> fixBadData(int64_t variable, int64_t record);
    cdfDataSet getGoodDataRecord(int64_t variable, int64_t record);

};

#endif // OMITBDHANDLER_H
