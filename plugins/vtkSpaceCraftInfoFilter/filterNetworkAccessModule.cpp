

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
    //get the data
    this->reply = qobject_cast<QNetworkReply*>(sender());

    while(!this->reply->isFinished()){/*spin until done*/}

    if(reply)
    {
        //get XML data
        this->xmlReader.addData(reply->readAll());

        //parse the xml
        while(!this->xmlReader.atEnd())
        {
            this->xmlReader.readNext();

            //parse XML
            std::cout << "xmlType: " << this->xmlReader.tokenString().toAscii().data() << std::endl;
            std::cout << "xmlQN: " << this->xmlReader.qualifiedName().toAscii().data() << std::endl;
            std::cout << "xmlText: " << this->xmlReader.text().toAscii().data() << std::endl;
            this->parseTypeStack.push_front(this->xmlReader.tokenType());
            this->parseQnStack.push_front(this->xmlReader.qualifiedName().toString());
            this->parseTextStack.push_front(this->xmlReader.text().toString());
        }


        this->consolodate_stacks();

    }
    else
    {
        //get http status code
        QString netStatus = reply->errorString();

        //process error
        this->networkAccessStatus = reply->error();
        std::cout << "HTTP ERROR: " << this->networkAccessStatus << " : "<< netStatus.toAscii().data() << std::endl;
    }

}


QNetworkReply *filterNetworkAccessModule::Get(QString URL, QString TopLevel)
{
    //set URL (for last call info)
    this->requestURL = URL;
    this->TopLevel = TopLevel;

    //Perform the get operation
    return this->Get();
}

void filterNetworkAccessModule::consolodate_stacks()
{
    if(this->TopLevel.isEmpty())
    {
        //there is an erros, and it needs to be addressed.
        std::cerr << "ERROR: XML Parser: TopLevel not set" << std::endl;
        exit(1);
    }
    else
    {
        //strip the top level, consolodate text to its corrosponding name type

        //these stacks are used for stripping... after done, swap them back
        QStack<QXmlStreamReader::TokenType> tempTypeStack;
        QStack<QString> tempQnStack;
        QStack<QString> tempTextStack;

        //parse!
        while(!this->parseTypeStack.isEmpty())
        {
            QXmlStreamReader::TokenType tempType = this->parseTypeStack.pop();
            QString tempQn = this->parseQnStack.pop();
            QString tempText = this->parseTextStack.pop();

            //strip out everything we don't want, then consolodate text fields
            switch(tempType)
            {
            //strip out everything we don't want
            case QXmlStreamReader::EndElement:
            case QXmlStreamReader::Invalid:
            case QXmlStreamReader::StartDocument:
            case QXmlStreamReader::EndDocument:
                break;

            //handle the text cases
            case QXmlStreamReader::Characters:
                //process characters
                std::cout << "Characters..." << std::endl;

                //this assumes that the first element is NOT a character element
                if(tempTextStack.isEmpty())
                {
                    //TODO: We will want to gracefully handle this in future so PV wont crash
                    std::cerr << "ERROR: While parsing XML, found ill-formed Character field" << std::endl;
                    exit(EXIT_FAILURE);
                }
                else
                {

                    //take off the previous
                    std::cout << "Poping off blank text from previous tag" << std::endl;
                    tempTextStack.pop();
                    tempTextStack.push(tempText);
                }

                break;

            //keep everything that we want
            default:
                std::cout << "Everything else..." << std::endl;
                if(tempQn != this->TopLevel)
                {
                    //put everything (except the top level)
                    //TODO: see if we need to eliminate everythin UP TO and INCLUDING the toplevel
                    tempTypeStack.push(tempType);
                    tempQnStack.push(tempQn);
                    tempTextStack.push(tempText);
                }
                break;
            }

        }

        //let us swap the stacks
        this->parseQnStack.swap(tempQnStack);
        this->parseTypeStack.swap(tempTypeStack);
        this->parseTextStack.swap(tempTextStack);

        //remember the stack is upside down at this point

        //DEBUG: print out...
        for(int x = 0; x < this->parseTextStack.size(); x++)
        {
            std::cout << "Element: " << this->parseQnStack[x].toAscii().data()
                      << " : " << this->parseTextStack[x].toAscii().data()
                      <<  std::endl;
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
