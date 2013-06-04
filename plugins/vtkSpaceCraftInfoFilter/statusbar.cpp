#include "statusbar.h"
#include "ui_statusbar.h"

statusbar::statusbar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::statusbar)
{
    ui->setupUi(this);



}

statusbar::~statusbar()
{
    delete ui;
}

void statusbar::setStatusBarMessage(QString message)
{
}

void statusbar::closeStatusBar()
{
}

void statusbar::showStatusBar()
{

}

void statusbar::setStatus(int status)
{
}
