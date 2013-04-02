#ifndef _vtkSpaceCraftInfo_h
#define _vtkSpaceCraftInfo_h
#include "vtkFiltersExtractionModule.h"
#include "vtkTableAlgorithm.h"

#include "qmap.h"
#include "qstring.h"
#include "qlist.h"

class vtkDataSet;
class vtkTable;
class vtkDataSetAttributes;

class VTKFILTERSEXTRACTION_EXPORT vtkSpaceCraftInfo : public vtkTableAlgorithm
{
public:
  static vtkSpaceCraftInfo *New();
  vtkTypeMacro(vtkSpaceCraftInfo,vtkTableAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetMacro(NumberOfTimeSteps, int)

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

  //----- support copying of data arrays over -----//
  void CopyDataToOutput(vtkInformation* inInfo, vtkDataSet* input, vtkTable* output);

  //----- data -----//
  // Records the number of timesteps present
  int NumberOfTimeSteps;
  double requestedTimeValue;
  double* getTimeSteps();

  // This Data Structure yeilds the value of the timestep by integer step
  //    timeSteps[timeStep] = time step value
  QList <double> timeSteps;

  // this data structure is as follows
  //    satPositions ["sat Name"] [timeStep] [x|y|z] = position[x|y|z]
  QMap <QString, QMap< double, double* > >  satPositions;

  //SpaceCraft Data Collection utilities
  bool getSCList();
  bool processCDAWeb(vtkTable *output);

private:
  vtkSpaceCraftInfo(const vtkSpaceCraftInfo&);
  void operator = (const vtkSpaceCraftInfo&);
};
#endif

