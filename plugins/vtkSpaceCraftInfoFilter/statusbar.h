#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QWidget>

namespace Ui {
class statusbar;
}

class statusbar : public QWidget
{
    Q_OBJECT
    
public:
    explicit statusbar(QWidget *parent = 0);
    ~statusbar();

    void setStatusBarMessage(QString message);

public slots:
    void closeStatusBar();
    void showStatusBar();
    void setStatus(int status);

signals:
    void displayUI();
    void closeUI();
    void changeStatus(int status);
    
private:
    Ui::statusbar *ui;
};

#endif // STATUSBAR_H
