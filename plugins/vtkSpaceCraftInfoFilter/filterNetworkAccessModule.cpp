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
    delete [] netManager;
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
            this->networkAccessStatus = 1;

            QByteArray myCustomData = reply->readAll();

            std::cout << "data: " << myCustomData.data() << std::endl;



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


QNetworkReply *filterNetworkAccessModule::Get(QString URL)
{
    //set URL (for last call info)
    this->requestURL = URL;

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
