#include "status.h"
#include "ui_status.h"

Status::Status(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Status)
{
    ui->setupUi(this);
}

Status::~Status()
{
    delete ui;
}

void Status::setStatusBarMessage(QString message)
{
    ui->message->setText(message);
}

void Status::updateAll()
{
    ui->status->hide();
    ui->status->update();
    ui->status->show();

    ui->message->hide();
    ui->message->update();
    ui->message->show();

    this->update();
    this->repaint();
}

void Status::closeStatusBar()
{
    emit this->hide();
}

void Status::showStatusBar()
{
    emit this->show();
}

void Status::setStatus(int status)
{
    ui->status->setValue(status);
}
