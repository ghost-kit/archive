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

class xmlTree
{
    QMap <QString, QString> map;
    QList<xmlTree*> childern;
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

    //xml parser
    void parseXMLBlock();

    //xml tree


protected slots:
    void networkReply();
};

#endif // FILTERNETWORKACCESSMODULE_H
