

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
                if(this->xmlReader.isStartElement() && (this->xmlReader.qualifiedName() == QString("ObservatoryDescription")))
                {
                    QString obsName;
                    QString obsShortDescrip;

                    while(!(this->xmlReader.isEndElement()
                            && this->xmlReader.qualifiedName() == QString("ObservatoryDescription")))
                    {

                        //add observatory name to map
                        if(this->xmlReader.isStartElement()
                                && this->xmlReader.qualifiedName() == QString("Name"))
                        {
                            while(!(this->xmlReader.isEndElement()
                                    && this->xmlReader.qualifiedName() == QString("Name")))
                            {
                                if(this->xmlReader.tokenString() == QString("Characters"))
                                {
                                    obsName = this->xmlReader.text().toString();

                                }

                                this->xmlReader.readNext();
                            }
                        }
                        else if (this->xmlReader.isStartElement()
                                 && this->xmlReader.qualifiedName() == QString("ShortDescription"))
                        {
                            while(!(this->xmlReader.isEndElement()
                                    && this->xmlReader.qualifiedName() == QString("ShortDescription")))
                            {
                                if(this->xmlReader.tokenString() == QString("Characters"))
                                {
                                    obsShortDescrip = this->xmlReader.text().toString();

                                }

                                this->xmlReader.readNext();
                            }
                        }

                        this->xmlReader.readNext();

                    }
                    this->xmlMap.insert(obsName,obsShortDescrip);
                    std::cout << "Name: " << obsName.toAscii().data()
                              << " Description: " << obsShortDescrip.toAscii().data()
                              << std::endl;
                }
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
