#include "vtkEnlilReaderHeader.h"

//-------------------------------------------------------------
//    The following Methods provide basic functionality for
//  Selective read arrays.  These methods provide the list of
//  arrays in Paraview.  You MUST populate the arrays in you
//  RequestInformation routine.  This can be done with the
//  *****DataArraySlection->AddArray(char* name) routines
//  where ***** represents the type of arrays you are
//  populating. (i.e. Point or Cell)
//-------------------------------------------------------------

/*
 * The Number of Point Arrays in current selection
 *  This is an internal function
 */
int vtkEnlilReader::GetNumberOfPointArrays()
{
  return this->PointDataArraySelection->GetNumberOfArrays();
}

/*
 * The Number of Cell Arrays in current selection
 *  This is an internal function
 */
int vtkEnlilReader::GetNumberOfCellArrays()
{
  return this->CellDataArraySelection->GetNumberOfArrays();
}

/*
 *Return the NAME (characters) of the Point array at index
 *   This is an internal function
 */
const char* vtkEnlilReader::GetPointArrayName(int index)
{
  return this->PointDataArraySelection->GetArrayName(index);
}

/*
 *Return the NAME (characters) of the Cell array at index
 *   This is an internal function
 */
const char* vtkEnlilReader::GetCellArrayName(int index)
{
  return this->CellDataArraySelection->GetArrayName(index);
}

/*
 *Get the status of the Point Array of "name"
 *   This is an internal function
 */
int vtkEnlilReader::GetPointArrayStatus(const char *name)
{
  return this->PointDataArraySelection->GetArraySetting(name);
}

/*
 *Get the status of the Cell Array of "name"
 *   This is an internal function
 */
int vtkEnlilReader::GetCellArrayStatus(const char *name)
{
  return this->CellDataArraySelection->GetArraySetting(name);
}

/*
 *Set the status of the Point Array of "name"
 *   This is an internal function
 */
void vtkEnlilReader::SetPointArrayStatus(const char *name, int status)
{
  if(status == 1)
    this->PointDataArraySelection->EnableArray(name);
  else
    this->PointDataArraySelection->DisableArray(name);

}

/*
 *Set the status of the Cell Array of "name"
 *   This is an internal function
 */
void vtkEnlilReader::SetCellArrayStatus(const char *name, int status)
{
  if(status == 1)
    this->CellDataArraySelection->EnableArray(name);
  else
    this->CellDataArraySelection->DisableArray(name);

}

/*
 *Disables ALL Point arrays registered in system
 *   This is an internal function
 */
void vtkEnlilReader::DisableAllPointArrays()
{
  this->PointDataArraySelection->DisableAllArrays();
}

/*
 *Disables ALL Cell arrays registered in system
 *   This is an internal function
 */
void vtkEnlilReader::DisableAllCellArrays()
{
  this->CellDataArraySelection->DisableAllArrays();
}

/*
 *Enables ALL Point arrays registered in system
 *   This is an internal function
 */
void vtkEnlilReader::EnableAllPointArrays()
{
  this->PointDataArraySelection->EnableAllArrays();
}

/*
 *Enables ALL Cell arrays registered in system
 *   This is an internal function
 */
void vtkEnlilReader::EnableAllCellArrays()
{
  this->CellDataArraySelection->EnableAllArrays();
  //  this->Modified();
}
//=============== END SELECTIVE READER METHODS================

//-- Callback
void vtkEnlilReader::SelectionCallback(
    vtkObject*,
    unsigned long vtkNotUsed(eventid),
    void* clientdata,
    void* vtkNotUsed(calldata))
{
  static_cast<vtkEnlilReader*>(clientdata)->Modified();
}


int vtkEnlilReader::checkStatus(void *Object, char *name)
{
  if(Object == NULL)
    {
      std::cerr << "ERROR: " << name
                << " has failed to initialize"
                << std::endl;

      return 0;
    }

  return 1;
}

//-- print extents --//
void vtkEnlilReader::printExtents(int extent[], char* description)
{
  std::cout << description << " "
            << extent[0] << " " <<
               extent[1] << " " <<
               extent[2] << " " <<
               extent[3] << " " <<
               extent[4] << " " <<
               extent[5] << std::endl;
}

void vtkEnlilReader::setExtents(int extentToSet[], int sourceExtent[])
{
  extentToSet[0] = sourceExtent[0];
  extentToSet[1] = sourceExtent[1];
  extentToSet[2] = sourceExtent[2];
  extentToSet[3] = sourceExtent[3];
  extentToSet[4] = sourceExtent[4];
  extentToSet[5] = sourceExtent[5];

}

void vtkEnlilReader::setExtents(int extentToSet[], int dim1, int dim2, int dim3, int dim4, int dim5, int dim6)
{
  extentToSet[0] = dim1;
  extentToSet[1] = dim2;
  extentToSet[2] = dim3;
  extentToSet[3] = dim4;
  extentToSet[4] = dim5;
  extentToSet[5] = dim6;
}

bool vtkEnlilReader::eq(int extent1[], int extent2[])
{
  return (extent1[0] == extent2[0] && extent1[1] == extent2[1]
          && extent1[2] == extent2[2] && extent1[3] == extent2[3]
          && extent1[4] == extent2[4] && extent1[5] == extent2[5]);
}

bool vtkEnlilReader::ExtentOutOfBounds(int extToCheck[], int extStandard[])
{
  return extToCheck[0] < 0 || (extToCheck[0] > extStandard[0])
      || extToCheck[1] < 0 || (extToCheck[1] > extStandard[1])
      || extToCheck[2] < 0 || (extToCheck[2] > extStandard[2])
      || extToCheck[3] < 0 || (extToCheck[3] > extStandard[3])
      || extToCheck[4] < 0 || (extToCheck[4] > extStandard[4])
      || extToCheck[5] < 0 || (extToCheck[5] > extStandard[5]);

}

void vtkEnlilReader::extractDimensions(int dims[], int extent[])
{
  dims[0] = extent[1] - extent[0]+1;
  dims[1] = extent[3] - extent[2]+1;
  dims[2] = extent[5] - extent[4]+1;
}


