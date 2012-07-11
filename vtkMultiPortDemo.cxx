#include "vtkMultiPortDemo.h"

#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkPointData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"

#include <vtkstd/map>
#include <vtkstd/string>

using namespace std;

vtkStandardNewMacro(vtkMultiPortDemo);

 vtkMultiPortDemo::vtkMultiPortDemo()
 {
   this->fileName = NULL;
   this->SetNumberOfInputPorts(0);
   this->SetNumberOfOutputPorts(2);

   // print vtkDebugMacro messages by turning debug mode on:
   this->DebugOn();
 }

vtkMultiPortDemo::~vtkMultiPortDemo()
{
  if ( this->fileName ){
    delete [] this->fileName;
    this->fileName = NULL;
  }
}

int vtkMultiPortDemo::CanReadFile(const char *filename)
{
  return 1;
}

int vtkMultiPortDemo::RequestInformation (vtkInformation* request,
				      vtkInformationVector** inputVector,
				      vtkInformationVector* outputVector)
{ 
  int port = request->Get(vtkDemandDrivenPipeline::FROM_OUTPUT_PORT());
  vtkDebugMacro(<< "Requested Port=" << port);

  //////////////////////////////////////
  // output port 0: vtkStructuredGrid //
  //////////////////////////////////////////////////////////////////////
  int extent[6] = { 0, 8,
		    0, 8,
		    0, 8 };
  vtkInformation* outInfo = outputVector->GetInformationObject(0); 
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),extent,6);

  //////////////////////////////////
  // output port 1: metadta table //
  //////////////////////////////////////////////////////////////////////
  vtkInformation* MetaDataOutInfo = outputVector->GetInformationObject(1);
  MetaDataOutInfo->Set(vtkTable::FIELD_ASSOCIATION(), vtkTable::FIELD_ASSOCIATION_ROWS);

  return 1; 
}

int vtkMultiPortDemo::RequestData(vtkInformation* request,
 			      vtkInformationVector** inputVector,
 			      vtkInformationVector* outputVector)
{  

  //////////////////////////////////////
  // output port 0: vtkStructuredGrid //
  //////////////////////////////////////////////////////////////////////
  int subext[6] = {0, 8,
		   0, 8,
		   0, 8};
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), subext);

  vtkStructuredGrid *output = 
    vtkStructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  output->SetDimensions(9, 9, 9);

  vtkPoints *points = vtkPoints::New();
  points->SetNumberOfPoints(9*9*9);

  for (int k=0; k < 9; k++){
    for (int j=0; j < 9; j++){
      for (int i=0; i < 9; i++){
	float xyz[3] = { i,j,k };
	int offset = i + j*9 + k*9*9;
	points->SetPoint(offset, xyz);
      }
    }
  }
  output->SetPoints(points);
  points->Delete();

  //////////////////////////////////
  // output port 1: metadta table //
  //////////////////////////////////////////////////////////////////////
  vtkTable* MetaData = vtkTable::GetData(outputVector,1);
  vtkStringArray *MetaString = vtkStringArray::New();
  MetaString->SetName("Meta Data");
  MetaString->SetNumberOfComponents(1);
  MetaString->InsertNextValue("This is a Test");
  MetaString->InsertNextValue("Test 2");
  MetaString->InsertNextValue("Test 3");
  
  MetaData->AddColumn(MetaString);
  
  vtkStringArray *MetaString2 = vtkStringArray::New();
  MetaString2->SetName("Other Data");
  MetaString2->SetNumberOfComponents(1);
  MetaString2->InsertNextValue("Test 2,1");
  MetaString2->InsertNextValue("Test 2,2");
  MetaString2->InsertNextValue("Test 2,3");
  
  MetaData->AddColumn(MetaString2);
  
  MetaString->Delete();  

  return 1;
}

void vtkMultiPortDemo::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkMultiPortDemo says \"Hello, World!\" " << "\n";
}

int vtkMultiPortDemo::FillOutputPortInformation(int port, vtkInformation *info)
{
  if (port == 0)
    return this->Superclass::FillOutputPortInformation(port, info);
  if (port == 1)
      info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTable");
    return 1;
}
