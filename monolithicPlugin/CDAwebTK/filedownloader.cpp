#include "filedownloader.h"
#include <QEventLoop>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <iostream>

FileDownloader::FileDownloader(QUrl imageUrl, QObject *parent) :
    QObject(parent)
{
    connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)),
                SLOT(fileDownloaded(QNetworkReply*)));

    QNetworkRequest request(imageUrl);
    QNetworkReply *reply = m_WebCtrl.get(request);

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(downloadError(QNetworkReply::NetworkError)));

    m_WebCtrl.get(request);

    //this will wait until download has finished before returning to calling
    // application
    {
        QEventLoop loop;
        connect(this, SIGNAL(downloaded()), &loop, SLOT(quit()));
        loop.exec();
    }
}

FileDownloader::~FileDownloader()
{

}

void FileDownloader::fileDownloaded(QNetworkReply* pReply)
{
    m_DownloadedData = pReply->readAll();
    //emit a signal
    pReply->deleteLater();
    emit downloaded();
}

void FileDownloader::downloadError(QNetworkReply::NetworkError error)
{
    std::cout << "Download Error occured" << std::endl;
}

QByteArray FileDownloader::downloadedData() const
{
    return m_DownloadedData;
}
