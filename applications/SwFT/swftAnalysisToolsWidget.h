#ifndef SWFTANALYSISTOOLSWIDGET_H
#define SWFTANALYSISTOOLSWIDGET_H

#include <QWidget>

namespace Ui {
class swftAnalysisToolsWidget;
}

class swftAnalysisToolsWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit swftAnalysisToolsWidget(QWidget *parent = 0);
    ~swftAnalysisToolsWidget();
    
private:
    Ui::swftAnalysisToolsWidget *ui;
};

#endif // SWFTANALYSISTOOLSWIDGET_H
