//==============================================================================//
// vtkSpaceCraftInfo filter.  This filter requests data from CDA-Web and        //
//  processes it for use with ParaView time-dependent data sets.  The filter    //
//  will also interpolate the requested information over time to get the proper //
//  time positions.                                                             //
//                                                                              //
//  Author: Joshua Murphy                                                       //
//  Date:   01 Apr 2013                                                         //
//==============================================================================//

//#define DEBUG 1


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

#include "vtkMultiBlockDataSet.h"

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
      return this->startTime;
  }

  void setStartTime(double value)
  {
      this->startTime = value;
  }

  virtual double getEndTime()
  {
      return this->endTime;
  }


  void setEndTime(double value)
  {
      this->endTime = value;
  }

  vtkInformation *getInInfo() const;
  void setInInfo(vtkInformation *value);

  vtkInformation *getOutInfo() const;
  void setOutInfo(vtkInformation *value);

  vtkMultiBlockDataSet *getOutput() const;
  void setOutput(vtkMultiBlockDataSet *value);

  int getNumberOfTimeSteps() const;
  void setNumberOfTimeSteps(int value);

  int RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector);
  int RequestDataSource(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector);

  int RequestInfoFilter(vtkInformation * request, vtkInformationVector ** inputVector, vtkInformationVector * outputVector);

  bool getProcessed() const;
  void setProcessed(bool value);

  int getNumInputPorts() const;
  void setNumInputPorts(int value);

  int getNumOutputPorts() const;
  void setNumOutputPorts(int value);

  void setTempPath(const char *path);

  void setOverShoot(int value);

  void updateForOvershoot(DateTime &startTime, DateTime &endTime);
protected:

  int numInputPorts;
  int numOutputPorts;

  //----- data -----//
  // Records the number of timesteps present
  int NumberOfTimeSteps;

  double requestedTimeValue;
  double *getTimeSteps();
  double startTime;
  double endTime;

  int overShoot;

  // This Data Structure yeilds the value of the timestep by integer step
  //    timeSteps[timeStep] = time step value
  QList <double> timeSteps;
  double TimeRange[2];

  //SpaceCraft Data Collection utilities
  bool processCDAWeb(vtkMultiBlockDataSet *mb);
  bool processCDAWebSource(vtkMultiBlockDataSet *mb);

  //Requested Data
  QStringList requestedData;
  QString group;
  QString observatory;

  void LoadCDFData();
  void LoadCDFDataSource();

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
  bool getDataForAllEpochs(QString &DataSet, varDataEntry &data);

  bool getAllTemperalDataFromCacheByVar(const QString DataSet, const QString var, QList<spaceCraftDataElement> &data);


  //------ gui attributes pannel ------//

  //gui data objects
  QMap<QString, QString> DownloadedFileNames;
  QMap<QString, filterNetworkObject*> urlMap;

  //Info items
  vtkInformation* inInfo;
  vtkInformation* outInfo;
  vtkMultiBlockDataSet* output;

  //Selected Handlers
  BadDataHandler *BDhandler;
  timeFitHandler *TFhandler;


};
#endif

