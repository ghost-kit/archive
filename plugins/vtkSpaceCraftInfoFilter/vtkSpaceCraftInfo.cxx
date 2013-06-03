//==============================================================================//
// vtkSpaceCraftInfo filter.  This filter requests data from CDA-Web and        //
//  processes it for use with ParaView time-dependent data sets.  The filter    //
//  will also interpolate the requested information over time to get the proper //
//  time positions.                                                             //
//                                                                              //
//  Author: Joshua Murphy                                                       //
//  Date:   01 Apr 2013                                                         //
//==============================================================================//

#include "vtkSpaceCraftInfo.h"

#include "vtkCommand.h"
#include "vtkAbstractArray.h"
#include "vtkCallbackCommand.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDataSetAttributes.h"
#include "vtkDataArraySelection.h"
#include "vtkCompositeDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkDataArray.h"
#include "vtkDataObject.h"
#include "vtkFieldData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGridAlgorithm.h"
#include "vtkStructuredGrid.h"
#include "vtkStructuredData.h"
#include "vtkTable.h"
#include "vtksys/SystemTools.hxx"


#include "QObject"
#include "QtXml"
#include "QNetworkAccessManager"
#include "QNetworkReply"
#include "QURL"
#include "QStringList"

#include <vector>
#include <vtkSmartPointer.h>
#include <vtksys/ios/sstream>


#include "cdf.h"

vtkStandardNewMacro(vtkSpaceCraftInfo)

vtkSpaceCraftInfo::vtkSpaceCraftInfo()
{
    this->NumberOfTimeSteps = 0;

}

vtkSpaceCraftInfo::~vtkSpaceCraftInfo()
{

}

//----- required overides -----//
//=========================================================================================//
//Process Request
int vtkSpaceCraftInfo::ProcessRequest(vtkInformation *request, vtkInformationVector ** inputVector, vtkInformationVector *outputVector)
{
    if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
    {
        return this->RequestInformation(request, inputVector, outputVector);
    }
    else if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_DATA()))
    {
        return this->RequestData(request, inputVector, outputVector);
    }

    return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//=========================================================================================//
//Request Inoformation
int vtkSpaceCraftInfo::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{

    std::cout << __FUNCTION__ << " on line " << __LINE__ << std::endl;

    this->inInfo = inputVector[0]->GetInformationObject(0);
    if(inInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS()))
    {
        std::cout << "Getting Number of Time steps" << std::flush << std::endl;
        this->NumberOfTimeSteps = this->inInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
        double *timeValues = this->inInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());

        //push time steps into list
        for (int y = 0; y < this->NumberOfTimeSteps; y++)
        {
            this->timeSteps.push_back(timeValues[y]);
        }
    }
    else
    {
        this->NumberOfTimeSteps = 0;
    }

    //Provide information to PV on how many time steps for which we will be providing information
    this->outInfo = outputVector->GetInformationObject(0);

    this->outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
                       this->getTimeSteps(),
                       this->NumberOfTimeSteps);

    double timeRange[2] = {this->timeSteps.first(), this->timeSteps.last()};

    this->outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
                       timeRange,
                       2);

    this->inInfo = NULL;
    this->outInfo = NULL;
    return 1;
}

//=========================================================================================//
//Request Data
int vtkSpaceCraftInfo::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    //Get the output Data object
    this->outInfo = outputVector->GetInformationObject(0);
    this->output = vtkTable::GetData(outInfo);

    //get time request data
    if(this->outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
    {
        this->requestedTimeValue = this->outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
    }

    this->processCDAWeb(this->output);
    return 1;
}


//=========================================================================================//
//------ Port Information ------//
int vtkSpaceCraftInfo::FillInputPortInformation(int port, vtkInformation *info)
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    return 1;
}

int vtkSpaceCraftInfo::FillOutputPortInformation(int port, vtkInformation *info)
{
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTable");
    return 1;
}


//=========================================================================================//
//----- helper function -----//
//getTimeSteps() returns a NEW double array that *must*
//  be managed by the caller. TODO: Fix this to manage memory slef.
double *vtkSpaceCraftInfo::getTimeSteps()
{
    double *ret = new double[this->NumberOfTimeSteps];

    for(int i = 0; i < this->NumberOfTimeSteps; i++)
    {
        ret[i] = this->timeSteps[i];
    }

    return ret;
}

//=========================================================================================//
//------ get actuall space craft data -----//
bool vtkSpaceCraftInfo::getSCData()
{

    return true;
}


//=========================================================================================//
bool vtkSpaceCraftInfo::processCDAWeb(vtkTable *output)
{
//    for(int x = 0; x < this->requestedData.size(); x++)
//    {
//        if(!this->requestedData[x].isEmpty())
//        {
//            //split the incoming data strings
//            QStringList Split = this->requestedData[x].split(":");

//            QStringList DataSets = Split[1].split(",");
//            QString     Instrument = Split[0];


//            //iterate over the data strings, get the data, and store localy
//            QStringList::Iterator iter;
//            for(iter = DataSets.begin(); iter != DataSets.end(); ++iter)
//            {
//                vtkDoubleArray *dataArray = vtkDoubleArray::New();
//                dataArray->SetNumberOfComponents(1);
//                dataArray->SetName((*iter).toAscii().data());

//                dataArray->InsertNextValue(this->requestedTimeValue);
//                output->GetRowData()->AddArray(dataArray);
//                dataArray->Delete();
//            }

//        }
//    }

    return true;
}


//=========================================================================================//
void vtkSpaceCraftInfo::SetSCIData(const char *group, const char *observatory, const char *list)
{

    std::cout << "Group: " << group << std::endl;
    std::cout << "Observatory: " << observatory << std::endl;
    std::cout <<  "Setting SCI Data: " << list << std::endl;

    QString dataList = QString(list);

    this->requestedData = dataList.split(";");

    std::cout << "Number of Data Elements: " << this->requestedData.size() << std::endl;
    for(int x = 0; x < this->requestedData.size(); x++)
    {
        if(!this->requestedData[x].isEmpty())
        {
            //split the incoming data strings
            QStringList Split = this->requestedData[x].split(":");

            QStringList DataSets = Split[1].split(",");
            QString     Instrument = Split[0];


            //iterate over the data strings, get the data, and store localy
            QStringList::Iterator iter;
            for(iter = DataSets.begin(); iter != DataSets.end(); ++iter)
            {


            }

        }
    }

    this->Modified();

}




//=========================================================================================//
//----- other stuff needed ------//
void vtkSpaceCraftInfo::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
    os << indent << "NumberOfTimeSteps: " << this->NumberOfTimeSteps << std::endl;
}



