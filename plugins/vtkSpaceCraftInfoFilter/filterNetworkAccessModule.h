#ifndef FILTERNETWORKACCESSMODULE_H
#define FILTERNETWORKACCESSMODULE_H
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QURL>
#include <QObject>
#include <QXmlSimpleReader>

class filterNetworkAccessModule : public QObject
{
    Q_OBJECT
public:
    filterNetworkAccessModule();
    ~filterNetworkAccessModule();

    void setRequestURL(QString URL);
    QNetworkReply* Get();
    QNetworkReply* Get(QString URL);

protected:


private:
    QString requestURL;
    QNetworkAccessManager *netManager;
    QNetworkReply *reply;
    int networkAccessStatus;



protected slots:
    void networkReply();
};

#endif // FILTERNETWORKACCESSMODULE_H
