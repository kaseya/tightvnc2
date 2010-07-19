// Copyright (C) 2008, 2009, 2010 GlavSoft LLC.
// All rights reserved.
//
//-------------------------------------------------------------------------
// This file is part of the TightVNC software.  Please visit our Web site:
//
//                       http://www.tightvnc.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

#ifndef _SERVER_CONFIG_DIALOG_H_
#define _SERVER_CONFIG_DIALOG_H_

#include "gui/BaseDialog.h"
#include "gui/TextBox.h"
#include "gui/CheckBox.h"
#include "gui/SpinControl.h"
#include "gui/BalloonTip.h"

#include "server-config-lib/ServerConfig.h"
#include "PasswordControl.h"

class ServerConfigDialog : public BaseDialog
{
public:
  ServerConfigDialog();
  virtual ~ServerConfigDialog();

  void setParentDialog(BaseDialog *dialog);

public:

  virtual BOOL onInitDialog();
  virtual BOOL onNotify(UINT controlID, LPARAM data);
  virtual BOOL onCommand(UINT controlID, UINT notificationID);
  virtual BOOL onDestroy() { return TRUE; }

  bool validateInput();
  void updateUI();
  void apply();

private:
  void initControls();
  void updateControlDependencies();

  void onAcceptRfbConnectionsClick();
  void onAcceptHttpConnectionsClick();
  void onAuthenticationClick();
  void onPrimaryPasswordChange();
  void onReadOnlyPasswordChange();
  void onUnsetPrimaryPasswordClick();
  void onUnsetReadOnlyPasswordClick();
  void onShowTrayIconCheckBoxClick();
  void onPollingIntervalSpinChangePos(LPNMUPDOWN message);
  void onRfbPortUpdate();
  void onHttpPortUpdate();
  void onUrlParamsClick();
  void onPollingIntervalUpdate();
  void onFileTransferCheckBoxClick();
  void onRemoveWallpaperCheckBoxClick();
  void onGrabTransparentWindowsChanged();

  void onBlockLocalInputChanged();
  void onBlockRemoteInputChanged();
  void onLocalInputPriorityChanged();
  void onInactivityTimeoutUpdate();
  void updateCheckboxesState();

protected:
  ServerConfig *m_config;
  TextBox m_rfbPort;
  TextBox m_httpPort;
  TextBox m_pollingInterval;
  CheckBox m_grabTransparentWindows;
  CheckBox m_enableFileTransfers;
  CheckBox m_removeWallpaper;
  CheckBox m_acceptRfbConnections;
  CheckBox m_acceptHttpConnections;
  CheckBox m_showTrayIcon;
  Control m_primaryPassword;
  Control m_readOnlyPassword;
  Control m_unsetPrimaryPassword;
  Control m_unsetReadOnlyPassword;
  CheckBox m_enableAppletParamInUrl;
  CheckBox m_useAuthentication;
  SpinControl m_rfbPortSpin;
  SpinControl m_httpPortSpin;
  SpinControl m_pollingIntervalSpin;

  CheckBox m_blockRemoteInput;
  CheckBox m_blockLocalInput;
  CheckBox m_localInputPriority;
  TextBox m_localInputPriorityTimeout;
  SpinControl m_inactivityTimeoutSpin;

  BaseDialog *m_parentDialog;

  PasswordControl *m_ppControl;
  PasswordControl *m_vpControl;
};

#endif
