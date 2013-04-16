

#include "filterNetworkAccessModule.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>

filterNetworkAccessModule::filterNetworkAccessModule()
{
    this->networkAccessStatus = -1;
    this->netManager = new QNetworkAccessManager();
}

filterNetworkAccessModule::~filterNetworkAccessModule()
{
    this->netManager->deleteLater();
}


void filterNetworkAccessModule::networkReply()
{
    this->reply = qobject_cast<QNetworkReply*>(sender());
    if(reply)
    {
        if(reply->error() == QNetworkReply::NoError)
        {
            //read data from reply
            std::cout << "No HTTP Error" << std::endl;

            QByteArray scInfoXML = reply->readAll();

            //parse XML
            this->xmlReader.addData(scInfoXML);

            this->parseTreeHead = new xmlTreeObject;

            this->parseTreeHead->name = QString("XML TREE HEAD");
            this->parseTreeHead->contents = QString("N/A");

            //read the first element
            this->xmlReader.readNext();

            QString topLevel = this->xmlReader.tokenString();

            while(!(this->xmlReader.isEndElement() && this->xmlReader.tokenString() == topLevel) && !this->xmlReader.atEnd())
            {
                //parse
                if(this->xmlReader.isStartElement())
                {
                    this->parseXMLBlock(this->parseTreeHead);
                    std::cout << "processing... " << topLevel.toAscii().data() << std::endl;
                }

                //advance the element
                this->xmlReader.readNext();
            }
            std::cout << "Number of Children: " << this->parseTreeHead->map.size() << std::endl;

            //mark activity as complete
            this->networkAccessStatus = 1;
        }
        else
        {
            //get http status code
            int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            //process error
            std::cout << "HTTP ERROR: " << httpStatus << std::endl;
            this->networkAccessStatus = httpStatus;
        }

        this->reply->deleteLater();
    }
}


QNetworkReply *filterNetworkAccessModule::Get(QString URL, int step)
{
    //set URL (for last call info)
    this->requestURL = URL;
    this->accessStep = step;

    //Perform the get operation
    return this->Get();
}


// XML Parser... RECURSIVE ...
void filterNetworkAccessModule::parseXMLBlock(xmlTreeObject *treeblock)
{

    //get type of current parse block
    QString levelName = this->xmlReader.tokenString();

    //set the name of current parseblock
    if(this->xmlReader.qualifiedName().toString() == "")
    {
        treeblock->name = QString("Leaf");
    }
    else
    {
        treeblock->name = this->xmlReader.qualifiedName().toString();
    }

    //set the payload for the parseblock
    if(this->xmlReader.isCharacters())
    {
        treeblock->contents = this->xmlReader.text().toString();
    }
    else
    {
        treeblock->contents = QString("N/A");
    }

    if(treeblock->parentNode != NULL) std::cout << "Parent: " << treeblock->parentNode->name.toAscii().data() << std::endl;
    std::cout << "Block Name: " << treeblock->name.toAscii().data() << std::endl;
    std::cout << "Text: " << treeblock->contents.toAscii().data() << std::endl;
    std::cout << "===================================" << std::endl;

    //populate the current parseblock
    while(!(this->xmlReader.isEndElement() && this->xmlReader.tokenString() == levelName) && !this->xmlReader.atEnd())
    {
        //parse new unit
        this->xmlReader.readNext();

        if(this->xmlReader.isStartElement() || this->xmlReader.isCharacters())
        {
            xmlTreeObject *newObject = new xmlTreeObject;
            newObject->parent = treeblock->name;
            newObject->parentNode = treeblock;
            treeblock->map.push_back(newObject);

            this->parseXMLBlock(newObject);
        }

    }
    std::cout << "Child Count for " << treeblock->name.toAscii().data() <<": " << treeblock->map.size() << std::endl;

}


QNetworkReply *filterNetworkAccessModule::Get()
{
    QUrl url(this->requestURL);
    QNetworkRequest req(url);

    this->reply = netManager->get(req);

    connect(reply, SIGNAL(finished()), this, SLOT(networkReply()));

    return this->reply;
}


void filterNetworkAccessModule::setRequestURL(QString URL)
{
    this->requestURL = URL;
}
