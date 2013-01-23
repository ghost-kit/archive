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
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkSmartPointer.h"
#include "vtkSMDomain.h"
#include "vtkSMDomainIterator.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMOutputPort.h"
#include "vtkSMPropertyIterator.h"

#include "pqActiveObjects.h"
#include "pqNonEditableStyledItemDelegate.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqTimeKeeper.h"


#include <QVariant>

swftStatusWindowWidget::swftStatusWindowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::swftStatusWindowWidget)
{
    ui->setupUi(this);

    ui->modelNameLabel->setText("[___] Model Information");

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

    vtkPVCompositeDataInformation *compositeInformation = dataInformation->GetCompositeDataInformation();

    if(compositeInformation->GetDataIsComposite())
    {
        //todo
    }

    //

}
