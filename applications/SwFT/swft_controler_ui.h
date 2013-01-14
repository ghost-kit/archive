#ifndef SWFT_CONTROLER_UI_H
#define SWFT_CONTROLER_UI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QToolBar>

QT_BEGIN_NAMESPACE

class Ui_swftControlerToolbar
{
public:
    //open/reset data
    QAction *actionOpenData;
    QAction *actionResetData;

    //other actions
    QAction *actionServerConnect;
    QAction *actionServerDisconnect;

    void setupUi(QToolBar *swftMainControlsToolbar)
    {
        if(swftMainControlsToolbar->objectName().isEmpty())
            swftMainControlsToolbar->setObjectName(QString::fromUtf8("swftMainControlsToolbar"));

        swftMainControlsToolbar->resize(350, 100);
        swftMainControlsToolbar->setOrientation(Qt::Horizontal);

        actionOpenData = new QAction(swftMainControlsToolbar);
        actionOpenData->setObjectName(QString::fromUtf8("actionOpenData"));
        actionOpenData->setIconText(QString::fromUtf8("Load Data"));
//        actionOpenData->setIcon(QIcon(":/icons.png"));
        actionOpenData->setProperty("PV_MUST_BE_MASTER_TO_SHOW", QVariant(true));

        actionResetData = new QAction(swftMainControlsToolbar);
        actionResetData->setObjectName(QString::fromUtf8("actionResetData"));
        actionResetData->setIconText(QString::fromUtf8("Reset Data"));

        //QIcon icon3;
        //icon.addFile(QString::fromUtf8(":/pqWidets/Icons/pqOpen24.png"));
        //actionResetData->setIcon(icon);
        actionResetData->setProperty("PV_MUST_BE_MASTER_TO_SHOW", QVariant(true));

        actionServerConnect = new QAction(swftMainControlsToolbar);
        actionServerConnect->setObjectName(QString::fromUtf8("actionServerConnect"));
        actionServerConnect->setIconText(QString::fromUtf8("Connect Server"));
        //QIcon icon2;
        //icon2.addFile(QString::fromUtf8(":/pqWidgets/Icons/pqConnect24.png"), QSize(), QIcon::Normal, QIcon::Off);
        //actionServerConnect->setIcon(icon2);

        actionServerDisconnect = new QAction(swftMainControlsToolbar);
        actionServerDisconnect->setObjectName(QString::fromUtf8("actionServerDisconnect"));
        actionServerDisconnect->setIconText(QString::fromUtf8("Disconnect Server"));

        //QIcon icon2;
        //icon2.addFile(QString::fromUtf8(":/pqWidgets/Icons/pqConnect24.png"), QSize(), QIcon::Normal, QIcon::Off);
        //actionServerConnect->setIcon(icon2);

        swftMainControlsToolbar->addAction(actionOpenData);
        swftMainControlsToolbar->addAction(actionResetData);
        swftMainControlsToolbar->addSeparator();
        swftMainControlsToolbar->addAction(actionServerConnect);
        swftMainControlsToolbar->addAction(actionServerDisconnect);
        swftMainControlsToolbar->addSeparator();

        retranslateUi(swftMainControlsToolbar);

        QMetaObject::connectSlotsByName(swftMainControlsToolbar);

    } //setupUi

    void retranslateUi(QToolBar *swftMainControlsToolbar)
    {
        swftMainControlsToolbar->setWindowTitle(QApplication::translate("pqMainControlsToolbar", "Main Controls", 0, QApplication::UnicodeUTF8));
        actionOpenData->setText(QApplication::translate("pqMainControlsToolbar", "Open...", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_STATUSTIP
        actionOpenData->setStatusTip(QApplication::translate("pqMainControlsToolbar", "Open", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
//        actionSaveData->setText(QApplication::translate("pqMainControlsToolbar", "Save Data...", 0, QApplication::UnicodeUTF8));
        actionServerConnect->setText(QApplication::translate("pqMainControlsToolbar", "&Connect...", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_STATUSTIP
        actionServerConnect->setStatusTip(QApplication::translate("pqMainControlsToolbar", "Connect", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
       actionServerDisconnect->setText(QApplication::translate("pqMainControlsToolbar", "&Disconnect", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_STATUSTIP
       actionServerDisconnect->setStatusTip(QApplication::translate("pqMainControlsToolbar", "Disconnect", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
//        actionHelp->setText(QApplication::translate("pqMainControlsToolbar", "Help", 0, QApplication::UnicodeUTF8));
//        actionUndo->setText(QApplication::translate("pqMainControlsToolbar", "&Undo", 0, QApplication::UnicodeUTF8));
//        actionRedo->setText(QApplication::translate("pqMainControlsToolbar", "&Redo", 0, QApplication::UnicodeUTF8));
//        actionAutoApply->setText(QApplication::translate("pqMainControlsToolbar", "Auto Apply", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
//        actionAutoApply->setToolTip(QApplication::translate("swftMainControlsToolbar", "Apply changes to parameters automatically", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class pqMainControlsToolbar: public Ui_swftControlerToolbar {};
} // namespace Ui

QT_END_NAMESPACE

#endif // SWFT_CONTROLER_UI_H
