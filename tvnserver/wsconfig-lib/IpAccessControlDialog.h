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

#ifndef _IP_ACCESS_CONTROL_DIALOG_H_
#define _IP_ACCESS_CONTROL_DIALOG_H_

#include "gui/BaseDialog.h"
#include "gui/ListView.h"
#include "gui/SpinControl.h"
#include "gui/BalloonTip.h"

#include "server-config-lib/Configurator.h"

#include "EditIpAccessRuleDialog.h"

class IpAccessControlDialog : public BaseDialog
{
public:
  IpAccessControlDialog();
  virtual ~IpAccessControlDialog();

  void setParentDialog(BaseDialog *dialog);

  virtual BOOL onInitDialog();
  virtual BOOL onCommand(UINT controlID, UINT notificationID);
  virtual BOOL onNotify(UINT controlID, LPARAM data);
  virtual BOOL onDestroy() { return TRUE; }

  bool validateInput();
  void updateUI();
  void apply();

private:
  void initControls();

  void onAddButtonClick();
  void onEditButtonClick();
  void onRemoveButtonClick();
  void onMoveUpButtonClick();
  void onMoveDownButtonClick();
  void onListViewSelChange();
  void onListViewSelChangeDblClick();
  void onAcceptRadioClick();
  void onRefuseRadioClick();
  void onAllowLoopbackConnectionsClick();
  void onAllowOnlyLoopbackConnectionsClick();
  void onIpCheckUpdate();
  void onQueryTimeoutUpdate();

  void updateButtonsState();
  void updateCheckBoxesState();
  void setListViewItemText(int index, IpAccessRule *control);

private:
  IpAccessControl *m_container;
  ServerConfig *m_config;
  EditIpAccessRuleDialog m_editDialog;
  ListView m_list;
  Control m_addButton;
  Control m_editButton;
  Control m_removeButton;
  Control m_moveUpButton;
  Control m_moveDownButton;
  CheckBox m_defaultActionAccept;
  CheckBox m_defaultActionRefuse;
  CheckBox m_allowLoopbackConnections;
  CheckBox m_onlyLoopbackConnections;
  TextBox m_queryTimeout;
  TextBox m_ip;
  Control m_ipCheckResult;
  SpinControl m_queryTimeoutSpin;
  BaseDialog *m_parentDialog;
};

#endif
