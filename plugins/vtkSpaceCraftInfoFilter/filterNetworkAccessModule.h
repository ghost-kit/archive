#ifndef FILTERNETWORKACCESSMODULE_H
#define FILTERNETWORKACCESSMODULE_H
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QURL>
#include <QObject>
#include <QtXml>
#include <QMap>
#include <QString>
#include <QXmlStreamReader>


class xmlTreeObject
{
public:
    xmlTreeObject()
    {
        this->parentNode = NULL;
    }
    ~xmlTreeObject()
    {
        //TODO: need to walk the tree to delete items
    }

    QString name;
    QString contents;
    QString parent;
    xmlTreeObject* parentNode;
    QList <xmlTreeObject*> map;

};


class filterNetworkAccessModule : public QObject
{
    Q_OBJECT
public:
    filterNetworkAccessModule();
    ~filterNetworkAccessModule();

    void setRequestURL(QString URL);
    void setAccessStep(int step);
    QNetworkReply* Get();
    QNetworkReply* Get(QString URL, int step);

protected:


private:
    //URL parsing
    QString requestURL;
    QNetworkAccessManager *netManager;
    QNetworkReply *reply;
    int networkAccessStatus;
    int accessStep;

    //xml parsing
    QXmlStreamReader xmlReader;

    //xml map
    xmlTreeObject* parseTreeHead;

    //xml parser
    void parseXMLBlock(xmlTreeObject *treeblock);

protected slots:
    void networkReply();
};

#endif // FILTERNETWORKACCESSMODULE_H
