#ifndef SWFTPARAVIEWMENUBUILDERS_H
#define SWFTPARAVIEWMENUBUILDERS_H

#include "pqApplicationComponentsModule.h"

class QMenu;
class QWidget;
class QMainWindow;

class PQAPPLICATIONCOMPONENTS_EXPORT swftParaViewMenuBuilders
{
public:

    /// Builds standard File menu.
    static void buildFileMenu(QMenu& menu);

    /// Builds the standard Edit menu.
    static void buildEditMenu(QMenu& menu);

    /// overide build toolbars
    static void buildToolbars(QMainWindow &mainWindow);

    /// Builds "Sources" menu.
    /// If you want to automatically add toolbars for sources as requested in the
    /// configuration pass in a non-null main window.
    static void buildSourcesMenu(QMenu& menu, QMainWindow* mainWindow =0);

    /// Builds "Filters" menu.
    /// If you want to automatically add toolbars for filters as requested in the
    /// configuration pass in a non-null main window.
    static void buildFiltersMenu(QMenu& menu, QMainWindow* mainWindow=0);

    /// Builds the "Tools" menu.
    static void buildToolsMenu(QMenu& menu);

    /// Builds the "View" menu.
    static void buildViewMenu(QMenu& menu, QMainWindow& window);

    /// Builds the "Macros" menu. This menu is automatically hidden is python
    /// support is not enabled.
    static void buildMacrosMenu(QMenu& menu);

    /// Builds the help menu.
    static void buildHelpMenu(QMenu& menu);

    /// Builds the context menu shown over the pipeline browser for some common
    /// pipeline operations.
    static void buildPipelineBrowserContextMenu(QWidget&);


};

#endif // SWFTPARAVIEWMENUBUILDERS_H
