#ifndef VTKENLILREADERMETADATAKEYS_H
#define VTKENLILREADERMETADATAKEYS_H

#include "vtkinformation.h"
#include "vtkExecutive.h"

class vtkInformationDoubleKey;
class vtkInformationIntegerKey;
class vtkInformationStringKey;

class vtkENLILMetaDataKeys
{
public:
  /*GLOBAL ATTRIBUTES*/
  //Strings
  static vtkInformationStringKey* TYPE();
  static vtkInformationStringKey* TITLE();
  static vtkInformationStringKey* LABEL();
  static vtkInformationStringKey* PROGRAM();
  static vtkInformationStringKey* E_VERSION();
  static vtkInformationStringKey* PROJECT();
  static vtkInformationStringKey* CODE();
  static vtkInformationStringKey* MODEL();
  static vtkInformationStringKey* GEOMETRY();
  static vtkInformationStringKey* GRID();
  static vtkInformationStringKey* COORDINATES();
  static vtkInformationStringKey* CASE();
  static vtkInformationStringKey* DATA();
  static vtkInformationStringKey* OBSERVATORY();
  static vtkInformationStringKey* CORONA();
  static vtkInformationStringKey* CRPOS();
  static vtkInformationStringKey* INITIAL();
  static vtkInformationStringKey* BOUNDARY();
  static vtkInformationStringKey* RUN();
  static vtkInformationStringKey* PARAMETERS();
  static vtkInformationStringKey* RESRUN();
  static vtkInformationStringKey* INITIAL_OLD();
  static vtkInformationStringKey* BOUNDARY_OLD();
  static vtkInformationStringKey* PARAMETERS_OLD();
  static vtkInformationStringKey* OBSDATE_CAL();
  static vtkInformationStringKey* REFDATE_CAL();

  //Doubles
  static vtkInformationDoubleKey* SHIFT_DEG();
  static vtkInformationDoubleKey* OBSDATE_MJD();
  static vtkInformationDoubleKey* REFDATE_MJD();
  static vtkInformationDoubleKey* RBND();
  static vtkInformationDoubleKey* GAMMA();
  static vtkInformationDoubleKey* XALPHA();

  //Integers
  static vtkInformationIntegerKey* MEVO();
  static vtkInformationIntegerKey* MFLD();
  static vtkInformationIntegerKey* MTIM();

  /*VARIABLE ATTRIBUTES*/
  static vtkInformationStringKey* LONG_NAME();
  static vtkInformationStringKey* UNITS();
  static vtkInformationStringKey* COORDINATE_AXIS_TYPE();

  static void add_global_attributes(vtkInformation* request);
  static void add_variable_attributes(vtkInformation * request);

  static void populate_meta_data(vtkInformation* request, vtkInformationStringKey* key, char* value);
  static void populate_meta_data(vtkInformation *request, vtkInformationDoubleKey *key, double &value);
  static void populate_meta_data(vtkInformation *request, vtkInformationIntegerKey *key, int &value);

};


#endif // VTKENLILREADERMETADATAKEYS_H
