#include "swftMainControlsToolbar.h"
#include "swft_controler_ui.h"

#include "pqHelpReaction.h"
#include "pqLoadDataReaction.h"
#include "pqSaveDataReaction.h"
#include "pqServerConnectReaction.h"
#include "pqServerDisconnectReaction.h"
#include "pqUndoRedoReaction.h"
#include "pqAutoApplyReaction.h"
//-----------------------------------------------------------------------------
void swftMainControlsToolbar::constructor()
{
  Ui::swftMainControlsToolbar ui;
  ui.setupUi(this);
  new pqLoadDataReaction(ui.actionOpenData);
//  new pqSaveDataReaction(ui.actionSaveData);
//  new pqServerConnectReaction(ui.actionServerConnect);
//  new pqServerDisconnectReaction(ui.actionServerDisconnect);
//  new pqUndoRedoReaction(ui.actionUndo, true);
//  new pqUndoRedoReaction(ui.actionRedo, false);
//  new pqHelpReaction(ui.actionHelp);
//  new pqAutoApplyReaction(ui.actionAutoApply);
}



