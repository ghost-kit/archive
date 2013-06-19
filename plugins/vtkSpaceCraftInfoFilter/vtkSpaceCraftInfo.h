//==============================================================================//
// vtkSpaceCraftInfo filter.  This filter requests data from CDA-Web and        //
//  processes it for use with ParaView time-dependent data sets.  The filter    //
//  will also interpolate the requested information over time to get the proper //
//  time positions.                                                             //
//                                                                              //
//  Author: Joshua Murphy                                                       //
//  Date:   01 Apr 2013                                                         //
//==============================================================================//

#define DEBUG 1


#ifndef _vtkSpaceCraftInfo_h
#define _vtkSpaceCraftInfo_h
#include "vtkStructuredGridAlgorithm.h"
#include "vtkFiltersExtractionModule.h"
#include "vtkDataArraySelection.h"
#include "vtkTableAlgorithm.h"
#include "vtkSmartPointer.h"
#include "vtkTable.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkInformation.h"

#include "qmap.h"
#include "qstring.h"
#include "qlist.h"
#include "qurl.h"
#include "cdf.h"

#include "QNetworkAccessManager"

#include "filterNetworkAccessModule.h"

#include "DateTime.h"
#include "cdfDataReader.h"

class vtkDataSet;
class vtkTable;
class vtkDataSetAttributes;
class vtkDataArraySelection;
class vtkCallbackCommand;

class spaceCraftDataElement
{
public:
    QString  varName;
    QVariant data;
    QVariant badDataValue;
    QString  units;
    QString  notes;

    long dataType;

};


class  vtkSpaceCraftInfoHandler : public  QObject
{
    Q_OBJECT

    typedef QMap<QString, QVector <QPair < QVariant, QPair<QString, QVariant > > > >   epochDataEntry;
    typedef QMap< double, QMap<QString, spaceCraftDataElement > > varDataEntry;
public:
    vtkSpaceCraftInfoHandler();
    ~vtkSpaceCraftInfoHandler();
  void PrintSelf(ostream& os, vtkIndent indent);

  long getNearestLowerIndex(DateTime &neededEpoch, QVector<DateTime> &convertedFileEpoch);
  bool findEpochVar(cdfDataReader &cdfFile, QStringList &varsAvailable, QString &EpochVar);

  //Callbacks
  virtual void SetSCIData(const char *group, const char *observatory, const char *list);
  virtual void SetTimeFitHandler(int handler);
  virtual void SetBadDataHandler(int handler);

  //call for getting time range
//  vtkGetVector2Macro(TimeRange, double);
  virtual double getStartTime()
  {
      return this->TimeRange[0];
  }

  virtual double getEndTime()
  {
      return this->TimeRange[1];
  }

  vtkInformation *getInInfo() const;
  void setInInfo(vtkInformation *value);

  vtkInformation *getOutInfo() const;
  void setOutInfo(vtkInformation *value);

  vtkTable *getOutput() const;
  void setOutput(vtkTable *value);

  int getNumberOfTimeSteps() const;
  void setNumberOfTimeSteps(int value);

  int RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector);
  int RequestInfoFilter(vtkInformation * request, vtkInformationVector ** inputVector, vtkInformationVector * outputVector);

  bool getProcessed() const;
  void setProcessed(bool value);

protected:



  //----- data -----//
  // Records the number of timesteps present
  int NumberOfTimeSteps;

  double requestedTimeValue;
  double *getTimeSteps();
  double startTime;
  double endTime;

  // This Data Structure yeilds the value of the timestep by integer step
  //    timeSteps[timeStep] = time step value
  QList <double> timeSteps;
  double TimeRange[2];

  //SpaceCraft Data Collection utilities
  bool processCDAWeb(vtkTable *output);

  //Requested Data
  QStringList requestedData;
  QString group;
  QString observatory;

  void LoadCDFData();

  //temp file paths
  QString tempFilePath;

  //Cached Data
  //DataCache[DataSet][EPOCH][Variable][element][component] !!OUTDATED!!
  QMap <QString, QMap< double, QMap<QString, spaceCraftDataElement > > >  DataCache;
  void cleanData();

  QMap<QString, QVector<double> > Epoch;
  bool processed;

  bool cToQVector(double* data, long dataSize, QVector<double> &vector);
  void checkCDFstatus(CDFstatus status);
  void getCDFUnits(cdfDataReader &reader, QString &VarName, QString &UnitText);
  void convertEpochToDateTime(QVector<DateTime> &convertedFileEpoch, cdfDataSet Epoch);

  bool getDataForEpoch(QString &DataSet, double Epoch, epochDataEntry  &data);
  bool getDataForEpochList(QString &DataSet, QVector<double> &EpochList, varDataEntry &data);

  //------ gui attributes pannel ------//

  //gui data objects
  QMap<QString, QString> CacheFileName;
  QMap<QString, filterNetworkObject*> uriList;

  //Info items
  vtkInformation* inInfo;
  vtkInformation* outInfo;
  vtkTable* output;

  //Selected Handlers
  BadDataHandler *BDhandler;
  timeFitHandler *TFhandler;


};
#endif

