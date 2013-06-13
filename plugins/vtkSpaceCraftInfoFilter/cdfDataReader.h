#ifndef CDFDATAREADER_H
#define CDFDATAREADER_H
#include<QString>
#include<QStringList>
#include<QVariant>
#include<QList>
#include<QVector>
#include<QMap>
#include<QtAlgorithms>
#include<QStringList>

#include <iostream>

#include "cdf.h"
#include "BadDataHandler.h"
#include "timefithandler.h"

class BadDataHandler;
class timeFitHandler;

class cdfAttributeSet
{

};

class cdfAttributeElement
{

};

class cdfDataSet
{
public:
    cdfDataSet();

    QVector<QVariant> getData();
    void addElement(QVariant item);
    void setVector(QVector<QVariant> vector);

    void clean();

    int getDataType() const;
    void setDataType(int value);

    QString getName() const;
    void setName(const QString &value);

    int getDataDims() const;
    void setDataDims(int value);

    QVector<int> getExtents() const;
    void setExtents(const QVector<int> &value);

    int getMajority() const;
    void setMajority(int value);

    long getNumberElements() const;
    void setNumberElements(long value);

    QVariant getInvalidData() const;
    void setInvalidData(const QVariant &value);

protected:
    //data returned from cdf call
    QVector<QVariant> data;
    int dataType;
    QString Name;
    int dataDims;
    QVector<int> Extents;
    int majority;
    long numberElements;
    QVariant invalidData;


private:
};

class cdfDataReader
{
public:
    cdfDataReader(QString FileName);
    cdfDataReader(QString FileName, BadDataHandler *BDHandler, timeFitHandler *TFHandler);
    ~cdfDataReader();

    //Getting Attributes from CDF files
    QStringList getAttributeList();

    QList<QVector<QVariant> > getGlobalAttribute(QString Attribute);
    QList<QVector<QVariant> > getGlobalAttribute(int64_t Attribute);

    QList<QVector<QVariant> > getZVariableAttribute(const QString Attribute, const QString Variable);
    QList<QVector<QVariant> > getZVariableAttribute(const int64_t Attribute, const int64_t Variable);

    QMap<QString, QList<QVector<QVariant> > > getZVariableAttributes(QStringList Attributes, QString Variable);
    QMap<QString, QList<QVector< QVariant > > > getZVaraibleAttributes(QStringList Attributes, int64_t Variable);

    //getting Data from CDF files
    QStringList getZVariableList();

    cdfDataSet getZVariable(QString variable);
    cdfDataSet getZVariable(int64_t variable);

    cdfDataSet getCorrectedZVariableRecord(QString variable, int64_t record);
    cdfDataSet getCorrectedZVariableRecord(int64_t variable, int64_t record);

    cdfDataSet getZVariableRecord(QString variable, int64_t record);
    cdfDataSet getZVariableRecord(int64_t variable, int64_t record);

    cdfDataSet getZVariableRecords(QString variable, QVector<int64_t> recordList);
    cdfDataSet getZVariableRecords(int64_t variable, QVector<int64_t> recordList);

    QVector<QVariant> getZVariableRecords(QString variable, int64_t startRecord, int64_t stopRecord);
    QVector<QVariant> getZVariableRecords(int64_t variable, int64_t startRecord, int64_t stopRecord);

    BadDataHandler *getBDHandler() const;
    void setBDHandler(BadDataHandler *value);

    timeFitHandler *getTFHandler() const;
    void setTFHandler(timeFitHandler *value);


    bool cToQVector(void *data, long dataSize, long dataType, QVector<QVariant> &vector);

    CDFid getFileId() const;
    void setFileId(const CDFid &value);

protected:
    //File pointers
    QString FileName;
    CDFid fileId;

    long    majority;
private:
    //file handlers
    bool openFile();
    bool closeFile();

    //Error Handlers
    bool CDFstatusOK(CDFstatus status, bool suppress=false);

    bool containsBadData(QVector<QVariant> data, QVariant badValue);

    //polymorphic handlers
    BadDataHandler *BDHandler;
    timeFitHandler *TFHandler;

};



#endif // CDFDATAREADER_H
