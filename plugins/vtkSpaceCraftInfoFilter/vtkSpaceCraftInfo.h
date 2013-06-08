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

#include "qmap.h"
#include "qstring.h"
#include "qlist.h"
#include "qurl.h"
#include "cdf.h"

#include "QNetworkAccessManager"

#include "filterNetworkAccessModule.h"

#include "DateTime.h"

class vtkDataSet;
class vtkTable;
class vtkDataSetAttributes;
class vtkDataArraySelection;
class vtkCallbackCommand;

class VTKFILTERSEXTRACTION_EXPORT vtkSpaceCraftInfo : public vtkTableAlgorithm, QObject
{

    typedef QMap<QString, QVector <QPair < QVariant, QString> > >   epochDataEntry;

public:
  static vtkSpaceCraftInfo *New();
  vtkTypeMacro(vtkSpaceCraftInfo,vtkTableAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetMacro(NumberOfTimeSteps, int)

  //Callbacks
  void SetSCIData(const char *group, const char *observatory, const char *list);
  long getNearestLowerIndex(DateTime &neededEpoch, QVector<DateTime> &convertedFileEpoch);
protected:
  vtkSpaceCraftInfo();
  ~vtkSpaceCraftInfo();

  //----- required overides -----//
  int ProcessRequest(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector);
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  //------ Port Information ------//
  int FillInputPortInformation(int port, vtkInformation *info);
  int FillOutputPortInformation(int port, vtkInformation *info);

  //----- data -----//
  // Records the number of timesteps present
  int NumberOfTimeSteps;
  double requestedTimeValue;
  double *getTimeSteps();

  // This Data Structure yeilds the value of the timestep by integer step
  //    timeSteps[timeStep] = time step value
  QList <double> timeSteps;

  // this data structure is as follows
  //    satPositions ["sat Name"] [timeStep] [x|y|z] = position[x|y|z]
  QMap <QString, QMap< double, double* > >  satPositions;

  //SpaceCraft Data Collection utilities
  bool getSCData();
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
  void getCDFUnits(CDFstatus status, CDFid id, int VarNum, QString &UnitText);
  void convertEpochToDateTime(QVector<DateTime> &convertedFileEpoch, double *EpochBuffer, long numRecords);
  //IN: DataSet IN: Epoch OUT: data OUT: bool success
  //NOTE: method will ADD TO the data list provided, not replace.
  bool getDataForEpoch(QString &DataSet, double Epoch, epochDataEntry  &data);


  //------ gui attributes pannel ------//

  //gui data objects
  QMap<QString, QString> CacheFileName;
  QMap<QString, filterNetworkObject*> uriList;

  //Info items
  vtkInformation* inInfo;
  vtkInformation* outInfo;
  vtkTable* output;


private:
  vtkSpaceCraftInfo(const vtkSpaceCraftInfo&);
  void operator = (const vtkSpaceCraftInfo&);
};
#endif

