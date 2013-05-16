#ifndef SCINFOPROP_H
#define SCINFOPROP_H

#include "pqPropertyWidget.h"
#include <iostream>


class  scInfoProp : public pqPropertyWidget
{
        Q_OBJECT
        typedef pqPropertyWidget Superclass;

public:
    scInfoProp(vtkSMProxy *smproxy, vtkSMProperty *smgroup, QWidget *parentObject = 0);
    virtual ~scInfoProp();



private:
    Q_DISABLE_COPY(scInfoProp)
};

#endif // SCINFOPROP_H
