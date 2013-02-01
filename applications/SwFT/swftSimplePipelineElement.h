#ifndef SWFTSIMPLEPIPELINEELEMENT_H
#define SWFTSIMPLEPIPELINEELEMENT_H

#include <QWidget>
#include <QString>

namespace Ui {
class swftSimplePipelineElement;
}

class swftSimplePipelineElement : public QWidget
{
    Q_OBJECT

public:
    explicit swftSimplePipelineElement(QWidget *parent = 0);
    ~swftSimplePipelineElement();

    void setToolName(const QString &title);


    
private:
    Ui::swftSimplePipelineElement *ui;
};

#endif // SWFTSIMPLEPIPELINEELEMENT_H
