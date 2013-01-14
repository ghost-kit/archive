#ifndef SWFTMAINCONTROLSTOOLBAR_H
#define SWFTMAINCONTROLSTOOLBAR_H

#include <QToolBar>
#include "pqApplicationComponentsModule.h"



class PQAPPLICATIONCOMPONENTS_EXPORT swftMainControlsToolbar : public QToolBar
{
  Q_OBJECT
  typedef QToolBar Superclass;
public:
  swftMainControlsToolbar(const QString& title, QWidget* parentObject=0)
    : Superclass(title, parentObject)
    {
    this->constructor();
    }
  swftMainControlsToolbar(QWidget* parentObject=0)
    : Superclass(parentObject)
    {
    this->constructor();
    }

private:
  Q_DISABLE_COPY(swftMainControlsToolbar)

  void constructor();
};


#endif // SWFTMAINCONTROLSTOOLBAR_H
