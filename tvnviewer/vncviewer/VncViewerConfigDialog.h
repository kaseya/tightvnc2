//  Copyright (C) 2008 GlavSoft LLC. All Rights Reserved.
//
//  This file is part of the TightVNC software.
//
//  TightVNC is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC homepage on the Web: http://www.tightvnc.com/

#ifndef _VNC_VIEWER_CONFIG_DIALOG_H_
#define _VNC_VIEWER_CONFIG_DIALOG_H_

#include "gui/BaseDialog.h"
#include "gui/TextBox.h"
#include "gui/CheckBox.h"

#include "util/Singleton.h"

class VncViewerConfigDialog : public BaseDialog
{
public:
  VncViewerConfigDialog();
  virtual ~VncViewerConfigDialog();

  bool historyWasCleared();

protected:

  //
  // Inherited from BaseDialog.
  //

  virtual BOOL onInitDialog();
  virtual BOOL onNotify(UINT controlID, LPARAM data);
  virtual BOOL onCommand(UINT controlID, UINT notificationID);
  virtual BOOL onDestroy();

  //
  // Button event handlers.
  //

  void onOkButtonClick();
  void onCancelButtonClick();
  void onClearConnectionHistoryButtonClick();

private:
  void initControls();
  bool isInputValid();
  bool testNum(TextBox *tb, const TCHAR *tbName);
  void updateControlValues();

protected:
  bool m_historyWasCleared;

  TextBox m_listenPort;
  TextBox m_logLevel;
  TextBox m_historyLimit;

  CheckBox m_showToolbar;
  CheckBox m_promptOnFullscreen;

  TextBox m_pathToLogFile;
};

extern VncViewerConfigDialog g_vncViewerConfigDialog;

#endif
