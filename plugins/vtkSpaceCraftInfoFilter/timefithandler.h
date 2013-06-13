#ifndef TIMEFITHANDLER_H
#define TIMEFITHANDLER_H

#include <QtAlgorithms>
#include <QString>
#include <QMap>
#include <QList>
#include <QPair>
#include <QStringList>

#include "cdfDataReader.h"

class cdfDataReader;
class cdfDataSet;

class timeFitHandler
{
public:
    timeFitHandler(cdfDataReader *reader);
    timeFitHandler();
};

#endif // TIMEFITHANDLER_H
