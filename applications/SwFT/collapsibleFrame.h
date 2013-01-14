#ifndef COLLAPSIBLEFRAME_H
#define COLLAPSIBLEFRAME_H

#include <QWidget>
#include <QVBoxLayout>
#include "extendedQlabel.h"
#include "extendedQlabel.h"


class CollapsibleFrame : public QWidget
{
    Q_OBJECT
public:
    ~CollapsibleFrame();
    CollapsibleFrame(QObject *parent, QString headerText);
    void addWidget(QWidget *widget);
    void addLayout(QLayout *layout);
    enum { OPEN, CLOSED };
private:
    QWidget* m_headerArea;
    QPixmap m_collapseIcon;
    QString m_headerText;
    QWidget* m_widgetArea;
    extendedQlabel* icon;
    QLabel* headerTextLabel;
    QVBoxLayout* widgetLayout;
    int m_frameState;
    void determineIcon();
    QObject* m_parent;
    
public slots:
    void changeState();

};

#endif // COLLAPSIBLEFRAME_H
