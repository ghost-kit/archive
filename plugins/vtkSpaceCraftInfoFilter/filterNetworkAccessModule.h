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
#include <QStack>


class filterNetworkAccessModule : public QObject
{
    Q_OBJECT
public:
    filterNetworkAccessModule();
    ~filterNetworkAccessModule();

    void setRequestURL(QString URL);
    void setAccessStep(int step);
    QNetworkReply* Get();
    QNetworkReply* Get(QString URL, QString TopLevel);

    void setTopLevel(QString topLevel)
    {
        this->TopLevel = topLevel;
    }

protected:
    QString TopLevel;

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

    //parsing functions
    void consolodate_stacks();

protected slots:
    void networkReply();
};

#endif // FILTERNETWORKACCESSMODULE_H
