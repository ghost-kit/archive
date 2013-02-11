#include "swftLoadModelReaction.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqFileDialog.h"
#include "pqRecentlyUsedResourcesList.h"
#include "pqServer.h"
#include "pqServerResource.h"
#include "vtkPVXMLParser.h"

#include <QFileInfo>
#include <QResource>
#include <QByteArray>



swftLoadModelReaction::swftLoadModelReaction(QAction *parent)
    : Superclass(parent)
{
    pqActiveObjects * activeObjects = &pqActiveObjects::instance();

    QObject::connect(activeObjects, SIGNAL(serverChanged(pqServer*)), this, SLOT(updateEnableState()));
    this->updateEnableState();
}



void swftLoadModelReaction::loadModelState(const QString &filename)
{
    pqActiveObjects *activeObjects = &pqActiveObjects::instance();
    pqServer *server = activeObjects->activeServer();

    //Read in XML file to restore
    vtkPVXMLParser *xmlParser = vtkPVXMLParser::New();
    xmlParser->SetFileName(filename.toAscii().data());
    xmlParser->Parse();

    //Get Root Element
    vtkPVXMLElement *root = xmlParser->GetRootElement();
    if(root)
    {
        pqApplicationCore::instance()->loadState(root,server);
    }
    else
    {
        qCritical("Root Does not exist. Not a Valid State, or file could not be opened");
    }
    xmlParser->Delete();

}

void swftLoadModelReaction::loadModelState()
{
    QResource r("/resources/standardview.pvsm");

    std::cout << "FileName: " << r.isValid() << std::endl;

    swftLoadModelReaction::loadModelState(":/resources/standardview.pvsm");
}

void swftLoadModelReaction::updateEnableState()
{
    pqActiveObjects *activeObjects = &pqActiveObjects::instance();
    this->parentAction()->setEnabled(activeObjects->activeServer() != NULL);
}
