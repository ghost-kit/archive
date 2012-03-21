#include "vtkENLILReaderMetaDataKeys.h"

#include "vtkInformationKey.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationIntegerKey.h"



//-----------------------------------------------------------------------------
/*GLOBAL ATTRIBUTES*/
//STRINGS
vtkInformationKeyMacro(vtkENLILMetaDataKeys, TYPE, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, TITLE, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, LABEL, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, PROGRAM, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, E_VERSION, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, PROJECT, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, CODE, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, MODEL, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, GEOMETRY, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, GRID, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, COORDINATES, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, CASE, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, DATA, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, OBSERVATORY, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, CORONA, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, CRPOS, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, INITIAL, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, BOUNDARY, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, RUN, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, PARAMETERS, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, RESRUN, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, INITIAL_OLD, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, BOUNDARY_OLD, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, PARAMETERS_OLD, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, OBSDATE_CAL, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, REFDATE_CAL, String)

//DOUBLES
vtkInformationKeyMacro(vtkENLILMetaDataKeys, SHIFT_DEG, Double)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, OBSDATE_MJD, Double)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, REFDATE_MJD, Double)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, RBND, Double)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, GAMMA, Double)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, XALPHA, Double)

//INTEGERS
vtkInformationKeyMacro(vtkENLILMetaDataKeys, MEVO, Integer)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, MFLD, Integer)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, MTIM, Integer)


/*VARIABLE ATTRIBUTES*/
vtkInformationKeyMacro(vtkENLILMetaDataKeys, LONG_NAME, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, UNITS, String)
vtkInformationKeyMacro(vtkENLILMetaDataKeys, COORDINATE_AXIS_TYPE, String)


//------------------------------------------------------------------------------
/*ATTRIBUTE METHODS*/
void vtkENLILMetaDataKeys::add_global_attributes(vtkInformation* request)
{
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::BOUNDARY());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::BOUNDARY_OLD());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::CASE());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::CODE());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::COORDINATES());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::CORONA());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::CRPOS());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::DATA());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::GAMMA());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::GEOMETRY());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::GRID());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::INITIAL());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::INITIAL_OLD());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::LABEL());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::MEVO());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::MFLD());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::MODEL());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::MTIM());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::OBSDATE_CAL());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::OBSDATE_MJD());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::OBSERVATORY());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::PARAMETERS());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::PARAMETERS_OLD());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::PROGRAM());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::PROJECT());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::RBND());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::REFDATE_CAL());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::REFDATE_MJD());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::RESRUN());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::RUN());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::SHIFT_DEG());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::TITLE());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::TYPE());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::E_VERSION());
  request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::XALPHA());

}


void vtkENLILMetaDataKeys::add_variable_attributes(vtkInformation* request)
{
 request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::LONG_NAME());
 request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::UNITS());
 request->AppendUnique(vtkExecutive::KEYS_TO_COPY(), vtkENLILMetaDataKeys::COORDINATE_AXIS_TYPE());

}

void vtkENLILMetaDataKeys::populate_meta_data(vtkInformation* request, vtkInformationStringKey *key, char* value)
{
  request->Set(key, value);

}

void vtkENLILMetaDataKeys::populate_meta_data(vtkInformation* request, vtkInformationDoubleKey *key, double &value)
{
  request->Set(key, value);

}

void vtkENLILMetaDataKeys::populate_meta_data(vtkInformation* request, vtkInformationIntegerKey *key, int &value)
{
  request->Set(key, value);

}
