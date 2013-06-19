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

class  vtkSpaceCraftInfoHandler : public  QObject
{
    Q_OBJECT

    typedef QMap<QString, QVector <QPair < QVariant, QPair<QString, QVariant > > > >   epochDataEntry;
    typedef QMap< double, epochDataEntry > varDataEntry;
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

protected:

  //----- data -----//
  // Records the number of timesteps present
  int NumberOfTimeSteps;
  double requestedTimeValue;
  double *getTimeSteps();

  // This Data Structure yeilds the value of the timestep by integer step
  //    timeSteps[timeStep] = time step value
  QList <double> timeSteps;
  double TimeRange[2];

  // this data structure is as follows
  //    satPositions ["sat Name"] [timeStep] [x|y|z] = position[x|y|z]
  QMap <QString, QMap< double, double* > >  satPositions;

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
  //DataCache[DataSet][EPOCH][Variable][element][component]
  QMap <QString, QMap< double, epochDataEntry > >   DataCache;
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

