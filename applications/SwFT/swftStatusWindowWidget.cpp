#include "swftStatusWindowWidget.h"
#include "ui_swftStatusWindowWidget.h"

#include <QHeaderView>
#include <QLineEdit>
#include <QStringList>

#include "vtkCommand.h"
#include "vtkEventQtSlotConnect.h"
#include <vtksys/SystemTools.hxx>

#include "vtkPVArrayInformation.h"
#include "vtkPVCompositeDataInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkExecutive.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkSmartPointer.h"
#include "vtkSMDomain.h"
#include "vtkSMDomainIterator.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMOutputPort.h"
#include "vtkSMPropertyIterator.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSelectionRepresentation.h"

#include "pqActiveObjects.h"
#include "pqNonEditableStyledItemDelegate.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqTimeKeeper.h"


#include <QVariant>

class swftStatusWindowWidget::pqUi
        : public QObject, public Ui::swftStatusWindowWidget
{
public:
    pqUi(QObject* p) : QObject(p) {}
};



swftStatusWindowWidget::swftStatusWindowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::swftStatusWindowWidget)
{
    ui->setupUi(this);

    ui->modelNameLabel->setText("Enlil Model");

    this->VTKConnect = vtkEventQtSlotConnect::New();
    this->updateInformation();

    this->connect(&pqActiveObjects::instance(),
                  SIGNAL (portChanged(pqOutputPort*)),
                  this,
                  SLOT(setOutputPort(pqOutputPort*)));


}

swftStatusWindowWidget::~swftStatusWindowWidget()
{
    this->VTKConnect->Disconnect();
    this->VTKConnect->Delete();
    delete ui;
}

pqOutputPort *swftStatusWindowWidget::getOutputPort()
{

    return this->OutputPort;
}


void swftStatusWindowWidget::setOutputPort(pqOutputPort *source)
{
    if(this->OutputPort == source)
    {
        return;
    }

    this->VTKConnect->Disconnect();
    if (this->OutputPort)
    {
        QObject::disconnect((this->OutputPort->getSource(),
                             SIGNAL (dataUdated(pqPipelineSource*)),
                             this, SLOT(updateInformation())));
    }

    this->OutputPort = source;
    if(this->OutputPort)
    {
        QObject::connect(this->OutputPort->getSource(),
                         SIGNAL(dataUpdated(pqPipelineSource*)),
                         this, SLOT(updateInformation()));
    }

    this->updateInformation();

}

void swftStatusWindowWidget::fillDataInformation(vtkPVDataInformation *info)
{

    std::cout << __FUNCTION__ << " " << __LINE__ << " has been triggered" << std::endl;
}


void swftStatusWindowWidget::updateInformation()
{

    std::cout << "update Information triggered" << std::endl;

    vtkPVDataInformation *dataInformation = NULL;
    pqPipelineSource * source = NULL;

    if(this->OutputPort)
    {
        source = this->OutputPort->getSource();
        if(this->OutputPort->getOutputPortProxy())
        {
            dataInformation = this->OutputPort->getDataInformation();
        }
    }

    if(!source || !dataInformation)
    {
        this->fillDataInformation(0);
        return;
    }

    this->fillDataInformation(dataInformation);

    //need to get the required information

    //find the filename
    vtkSmartPointer<vtkSMPropertyIterator> piter;
    piter.TakeReference(source->getProxy()->NewPropertyIterator());
    for(piter->Begin(); !piter->IsAtEnd(); piter->Next())
    {
        vtkSMProperty *prop = piter->GetProperty();
        if(prop->IsA("vtkSMStringVectorProperty"))
        {

            vtkSmartPointer<vtkSMDomainIterator> diter;
            diter.TakeReference(prop->NewDomainIterator());


            for(diter->Begin(); !diter->IsAtEnd(); diter->Next())
            {
                if(diter->GetDomain()->IsA("vtkSMFileListDomain"))
                {
                    vtkSMProperty* smprop = piter->GetProperty();
                    if(smprop->GetInformationProperty())
                    {
                        smprop = smprop->GetInformationProperty();
                        source->getProxy()->UpdatePropertyInformation(smprop);
                    }

                    QString filename = pqSMAdaptor::getElementProperty(smprop).toString();
                    QString path = vtksys::SystemTools::GetFilenamePath(filename.toAscii().data()).c_str();

                    std::cout << "Path: " << path.toAscii().data() << std::endl;
                    std::cout << "filename: " << filename.toAscii().data() << std::endl;

                    ui->currentFileInfo->setText(vtksys::SystemTools::GetFilenameName(filename.toAscii().data()).c_str());
                    ui->currentFilePathInfo->setText(path);

                }

                if(!diter->IsAtEnd())
                {
                    break;
                }
            }
        }
    }


    vtkPVDataSetAttributesInformation* fieldInfo;

    fieldInfo = dataInformation->GetFieldDataInformation();


    for(int q=0; q < fieldInfo->GetNumberOfArrays(); q++)
    {
        vtkPVArrayInformation* arrayInfo;
        arrayInfo = fieldInfo->GetArrayInformation(q);

        int numComponents = arrayInfo->GetNumberOfComponents();

        QString name = arrayInfo->GetName();
        QString value;

        std::cout << "Name: " << name.toAscii().data() << std::endl;

        for(int j = 0; j < numComponents; j++)
        {
            //look for the correct values
        }



    }





}
