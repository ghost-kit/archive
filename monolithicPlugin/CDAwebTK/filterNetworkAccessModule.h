#ifndef FILTERNETWORKACCESSMODULE_H
#define FILTERNETWORKACCESSMODULE_H
#include <iostream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <qurl.h>
#include <QObject>
#include <QtXml>
#include <QMap>
#include <QMultiMap>
#include <QString>
#include <QXmlStreamReader>
#include <QStack>

typedef QList<QMultiMap<QString, QString> *> filterNetworkList;
typedef QMap<QString,QString> filterNetworkObject;

class filterNetworkAccessModule : public QObject
{
    Q_OBJECT
public:
    filterNetworkAccessModule();
    ~filterNetworkAccessModule();

    void setRequestURL(QString URL);
    void setAccessStep(int step);
    QNetworkReply* Get();
    QNetworkReply* Get(QString URL, QString TopLevel, QString ObjectLevel);

    void setTopLevel(QString topLevel)
    {
        this->TopLevel = topLevel;
    }

    void setObjectLevel(QString ObjectLevel)
    {
        this->ObjectLevel = ObjectLevel;
    }

    QList< QMultiMap <QString, QString>* > *getFinalOjects()
    {
        return this->finalObjects;
    }

    int getNetworkAccessStatus() {return this->networkAccessStatus;}

protected:
    QString TopLevel;
    QString ObjectLevel;

private:
    //URL parsing
    QString requestURL;
    QNetworkAccessManager *netManager;
    QNetworkReply *reply;
    int networkAccessStatus;
    int accessStep;

    //xml parsing
    QXmlStreamReader xmlReader;

    //xml parse stack
    QStack<QXmlStreamReader::TokenType> parseTypeStack;
    QStack<QString> parseQnStack;
    QStack<QString> parseTextStack;

    //final parse breakdown
    QList< QMultiMap < QString, QString>* > *finalObjects;

    //parsing functions
    void consolodateStacks();
    void extractObjects();

protected slots:
    void networkReply();
    void networkError(QNetworkReply::NetworkError error);
    void dataHasBeenProcessed();

signals:
    void dataRetrieved();
    void dataProcessed();
};

#endif // FILTERNETWORKACCESSMODULE_H
