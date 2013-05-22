#ifndef SELECTIONBOXWIDGET_H
#define SELECTIONBOXWIDGET_H

#include <QWidget>

namespace Ui {
class selectionBoxWidget;
}

class selectionBoxWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit selectionBoxWidget(QWidget *parent = 0);
    ~selectionBoxWidget();
    
    void configure(QString Name, QString Desc);

private:
    Ui::selectionBoxWidget *ui;

    QString Name;
    QString Desc;

public slots:
    void itemSelected(bool state);

signals:
    void signalSelected(QString item, bool state);

};

#endif // SELECTIONBOXWIDGET_H
