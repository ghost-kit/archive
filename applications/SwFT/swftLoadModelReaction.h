#ifndef SWFTLOADMODELREACTION_H
#define SWFTLOADMODELREACTION_H

#include "pqReaction.h"

class PQAPPLICATIONCOMPONENTS_EXPORT swftLoadModelReaction : public pqReaction
{
    Q_OBJECT
    typedef pqReaction Superclass;
public:
    //Constructor. Parent CANNOT be NULL
    swftLoadModelReaction(QAction *parent);

    static void loadModelState(const QString &filename);
    static void loadModelState();

public slots:
    /// Updates the enabled state. Applications need not call this explicitly
    void updateEnableState();

protected:
    //Called when action is triggered
    virtual void onTriggered()
    {
        swftLoadModelReaction::loadModelState();
    }

private:
    Q_DISABLE_COPY(swftLoadModelReaction);
};

#endif // SWFTLOADMODELREACTION_H
