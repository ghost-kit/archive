#include "vtkEnlilReaderHeader.h"

vtkStandardNewMacro(vtkEnlilReader)


//---------------------------------------------------------------
//    Constructors and Destructors
//---------------------------------------------------------------
vtkEnlilReader::vtkEnlilReader()
{
  int nulExtent[6] = {0,0,0,0,0,0};
  this->FileName = NULL;

  //set the number of output ports you will need
  this->SetNumberOfOutputPorts(1);

  //set the number of input ports (Default 0)
  this->SetNumberOfInputPorts(0);

  //configure array status selectors
  this->PointDataArraySelection = vtkDataArraySelection::New();
  this->CellDataArraySelection  = vtkDataArraySelection::New();

  //Configure sytem array interfaces
  this->Points = NULL;
  this->Radius = NULL;
  this->gridClean = false;
  this->infoClean = false;

  this->setExtents(this->WholeExtent, nulExtent);
  this->setExtents(this->SubExtent, nulExtent);

  this->SelectionObserver = vtkCallbackCommand::New();
  this->SelectionObserver->SetCallback(&vtkEnlilReader::SelectionCallback);
  this->SelectionObserver->SetClientData(this);
  this->PointDataArraySelection->AddObserver(vtkCommand::ModifiedEvent, this->SelectionObserver);
  this->CellDataArraySelection->AddObserver(vtkCommand::ModifiedEvent, this->SelectionObserver);
}

//=========================================================================
vtkEnlilReader::~vtkEnlilReader()
{
  this->PointDataArraySelection->Delete();
  this->CellDataArraySelection->Delete();

  if(this->gridClean)
    this->Points->Delete();

  this->SelectionObserver->Delete();
}


//------------------------------------------------------------
//    These functions are the meat of the readers... i.e. they
//  are the calls that ParaView uses to get information from
//  your data source.   This is where the logic of the reader
//  is implemented.
//------------------------------------------------------------

int vtkEnlilReader::CanReadFile(const char *filename)
{
  //This doesn't really do anything right now...
  return 1;
}

//=========================================================================
int vtkEnlilReader::ProcessRequest(
    vtkInformation *reqInfo,
    vtkInformationVector **inInfo,
    vtkInformationVector *outInfo)
{
  return this->Superclass::ProcessRequest(reqInfo, inInfo, outInfo);
}

//=========================================================================
int vtkEnlilReader::RequestInformation(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{
  int status = 0;

  // Array names and extents
  vtkInformation* DataOutputInfo = outputVector->GetInformationObject(0);
  status = this->checkStatus(
        DataOutputInfo,
        (char*)" Array Name: Data Info Output Information");

  if(status)
    {
      //If Information has not yet been loaded, load it.
      if(this->CellDataArraySelection->GetNumberOfArrays() == 0 &&
         this->PointDataArraySelection->GetNumberOfArrays() == 0)
        {
          //Set the Names of the Arrays
          this->PopulateArrays();
        }

      //Set the Whole Extents and Time
      this->PopulateDataInformation();


      /*Set Information*/
      //Set Time
      double timeRange[2]
          = {this->TimeSteps[0], this->TimeSteps[0]};

      DataOutputInfo->Set(
            vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
            this->TimeSteps,
            1);

      DataOutputInfo->Set(
            vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
            timeRange,
            2);

      //Set Extents
      DataOutputInfo->Set(
            vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
            this->WholeExtent,
            6);
    }
  return 1;
}

//=========================================================================
int vtkEnlilReader::RequestData(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{
  this->SetProgress(0);

  //Import the MetaData
  this->LoadMetaData(outputVector);

  this->SetProgress(.05);

  //Import the actual Data
  this->LoadVariableData(outputVector);

  this->SetProgress(1.00);

  return 1;

}

//=================== END CORE METHODS =======================


//--------------------------------------------------------------
//    Output Port Configuration
//--------------------------------------------------------------
int vtkEnlilReader::FillOutputPortInformation(int port, vtkInformation* info)
{

  if (port==0)
    {
      info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkStructuredGrid");

      return this->Superclass::FillInputPortInformation(port, info);

    }

  return 1;
}

//================= END PORT CONFIGURATION ===================

//------------------------------------------------------------
//    Internal functions -- required for system to work
//------------------------------------------------------------
void vtkEnlilReader::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
