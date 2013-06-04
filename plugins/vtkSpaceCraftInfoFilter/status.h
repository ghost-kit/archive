#ifndef STATUS_H
#define STATUS_H

#include <QDialog>

namespace Ui {
class Status;
}

class Status : public QDialog
{
    Q_OBJECT
    
public:
    explicit Status(QWidget *parent = 0);
    ~Status();

    void setStatusBarMessage(QString message);
    void updateAll();

public slots:
    void closeStatusBar();
    void showStatusBar();
    void setStatus(int status);

private:
    Ui::Status *ui;
};

#endif // STATUS_H
