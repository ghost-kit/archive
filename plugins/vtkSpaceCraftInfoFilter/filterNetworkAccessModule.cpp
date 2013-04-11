

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

            while(!this->xmlReader.atEnd())
            {
                this->xmlReader.readNext();
                if(this->xmlReader.isStartElement())
                {
                    this->parseXMLBlock();
                }

                std::cout << "Parsing next block" << std::endl;
                std::cout << "Top Level: " << this->xmlReader.qualifiedName().toAscii().data() << std::endl;

            }


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
void filterNetworkAccessModule::parseXMLBlock()
{
    if(this->xmlReader.isStartElement())
    {
        QString levelName = this->xmlReader.tokenString();

        while(!(this->xmlReader.isEndElement() && this->xmlReader.tokenString() == levelName) && !this->xmlReader.atEnd())
        {
            this->xmlReader.readNext();

            //get field information
            QString fieldText = this->xmlReader.text().toString();
            QString qualifiedName = this->xmlReader.qualifiedName().toString();
            QString tokenString = this->xmlReader.tokenString();

            std::cout << "fieldText: " << fieldText.toAscii().data() <<
                         ": qualified Name: " << qualifiedName.toAscii().data() <<
                         ": token String: " << tokenString.toAscii().data() << std::endl;

            if(this->xmlReader.isStartElement())
            {
                std::cout << "Parsing next inner block" << std::endl;
                this->parseXMLBlock();

            }


        }

    }
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
