//======================================================================================================
// File: vtkENLILReader.cxx
// Author: Joshua Murphy
// Date Last Updated: 13 DEC 2011
// NOTES: This is a ParaView Reader for the Space Weather Prediction Center's Version of the
//      Enlil Solar Wind model as of the data above.  IF the data format of your model output is
//      the same as the SWPC model version, then you should have no trouble using this reader.
//
//      This reader will be updated to accept other versions of the Enlil files as the need arises.
//=====================================================================================================


#include "vtkENLILReader.h"

#include "vtkCallbackCommand.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCellType.h"
#include "vtkDataArraySelection.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkTable.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "vtkStringArray.h"
#include "vtkFloatArray.h"
#include "vtkIntArray.h"
#include "vtkPoints.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtksys/SystemTools.hxx"

#include <string>
#include <sstream>
#include <iostream>

#include "vtkMultiProcessController.h"
#include "vtkToolkits.h"


using namespace std;

vtkStandardNewMacro(vtkENLILReader);

vtkENLILReader::vtkENLILReader()
{


}

vtkENLILReader::~vtkENLILReader()
{

}

