#include "pathSelector.h"
#include "ui_pathSelector.h"

#include "newFileChooserWidget.h"

#include <vtkSMProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMGlobalPropertiesManager.h>
#include <vtkSMStringVectorProperty.h>
#include <vtkSMDoubleVectorProperty.h>

pathSelector::pathSelector(vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject) :
    Superclass(smproxy, parentObject),
    ui(new Ui::pathSelector)
{
    ui->setupUi(this);
    ui->filePath->setUseDirectoryMode(true);
    ui->filePath->setWindowTitle("Select Path");

    ui->filePath->setForceSingleFile(true);
    ui->filePath->setSingleFilename("/tmp");

    this->svp = vtkSMStringVectorProperty::SafeDownCast(smproperty);

    //add properties link
    this->addPropertyLink(ui->filePath, smproxy->GetPropertyName(smproperty), SIGNAL(filenameChanged(const QString &)), svp);


}

pathSelector::~pathSelector()
{
    delete ui;
}

void pathSelector::apply()
{
    //send the string when appply is clicked
    if(!ui->filePath->filenames().isEmpty())
    {
        this->svp->SetElement(0, ui->filePath->filenames()[0].toAscii().data());
    }

    Superclass::apply();
}
