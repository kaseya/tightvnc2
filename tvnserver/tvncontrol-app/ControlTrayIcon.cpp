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

#include "ControlTrayIcon.h"
#include "OutgoingConnectionDialog.h"
#include "ControlApplication.h"

#include "UpdateRemoteConfigCommand.h"
#include "DisconnectAllCommand.h"
#include "ShutdownCommand.h"
#include "MakeRfbConnectionCommand.h"
#include "ControlCommand.h"
#include "UpdateLocalConfigCommand.h"

#include "util/ResourceLoader.h"
#include "util/StringTable.h"
#include "util/MacroCommand.h"

#include "tvnserver/resource.h"

#include <crtdbg.h>

ControlTrayIcon::ControlTrayIcon(ControlProxy *serverControl,
                                 Notificator *notificator,
                                 ControlApplication *appControl,
                                 bool showAfterCreation)
: NotifyIcon(showAfterCreation),
  m_serverControl(serverControl), m_notificator(notificator),
  m_appControl(appControl),
  m_inWindowProc(false),
  m_termination(false)
{
  ResourceLoader *resLoader = ResourceLoader::getInstance();

  m_iconWorking = new Icon(resLoader->loadIcon(MAKEINTRESOURCE(IDI_CONNECTED)));
  m_iconIdle = new Icon(resLoader->loadIcon(MAKEINTRESOURCE(IDI_IDLE)));
  m_iconDisabled = new Icon(resLoader->loadIcon(MAKEINTRESOURCE(IDI_DISABLED)));

  setWindowProcHolder(this);

  m_updateRemoteConfigCommand = new UpdateRemoteConfigCommand(m_serverControl);
  m_updateLocalConfigCommand = new UpdateLocalConfigCommand(m_serverControl);
  m_applyChangesMacroCommand = new MacroCommand();
  m_applyChangesMacroCommand->addCommand(m_updateRemoteConfigCommand);
  m_applyChangesMacroCommand->addCommand(m_updateLocalConfigCommand);
  m_applyChangesControlCommand = new ControlCommand(m_applyChangesMacroCommand, m_notificator);

  m_configDialog = new ConfigDialog();
  m_configDialog->setConfigReloadCommand(m_applyChangesControlCommand);

  setNotConnectedState();

  syncStatusWithServer();
}

ControlTrayIcon::~ControlTrayIcon()
{
  delete m_configDialog;

  delete m_applyChangesControlCommand;
  delete m_applyChangesMacroCommand;
  delete m_updateLocalConfigCommand;
  delete m_updateRemoteConfigCommand;

  delete m_iconDisabled;
  delete m_iconIdle;
  delete m_iconWorking;
}

LRESULT ControlTrayIcon::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                    bool *useDefWindowProc)
{
  if (m_inWindowProc) {
    *useDefWindowProc = true;
    return 0;
  }
  if (m_termination) {
    m_endEvent.notify();
    *useDefWindowProc = true;
    return 0;
  }

  m_inWindowProc = true;

  switch (uMsg) {
  case WM_USER + 1:
    switch (lParam) {
    case WM_RBUTTONUP:
      onRightButtonUp();
      break;
    case WM_LBUTTONDOWN:
      onLeftButtonDown();
      break;
    } 
    break;
  default:
    *useDefWindowProc = true;
  }

  m_inWindowProc = false;
  return 0;
}

void ControlTrayIcon::onRightButtonUp()
{
  HMENU hRoot = LoadMenu(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TRAYMENU));
  HMENU hMenu = GetSubMenu(hRoot, 0);

  SetMenuDefaultItem(hMenu, ID_CONFIGURATION, FALSE);

  if (m_appControl->m_slaveModeEnabled) {
    RemoveMenu(hMenu, ID_CLOSE_CONTROL_INTERFACE, MF_BYCOMMAND);
  }

  POINT pos;

  if (!GetCursorPos(&pos)) {
    pos.x = pos.y = 0;
  }

  SetForegroundWindow(getWindow());

  int action = TrackPopupMenu(hMenu,
                              TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON,
                              pos.x, pos.y, 0, getWindow(), NULL);

  switch (action) {
  case ID_KILLCLIENTS:
    onDisconnectAllClientsMenuItemClick();
    break;
  case ID_SHUTDOWN_SERVICE:
    onShutdownServerMenuItemClick();
    break;
  case ID_CONFIGURATION:
    onConfigurationMenuItemClick();
    break;
  case ID_OUTGOING_CONN:
    onOutgoingConnectionMenuItemClick();
    break;
  case ID_ABOUT_TIGHTVNC_MENUITEM:
    onAboutMenuItemClick();
    break;
  case ID_CLOSE_CONTROL_INTERFACE:
    onCloseControlInterfaceMenuItemClick();
    break;
  }
}

void ControlTrayIcon::onLeftButtonDown()
{
  onConfigurationMenuItemClick();
}

void ControlTrayIcon::onConfigurationMenuItemClick()
{
  ControlApplication::removeModelessDialog(m_configDialog->getControl()->getWindow());

  bool isConnectedToService = false;

  try {
    isConnectedToService = m_serverControl->getServerInfo().m_serviceFlag;
  } catch (...) {
    return;
  }

  Configurator::getInstance()->setServiceFlag(isConnectedToService);

  {
    UpdateLocalConfigCommand updateLocalConfigCommand(m_serverControl);

    ControlCommand safeCommand(&updateLocalConfigCommand, m_appControl);

    safeCommand.execute();

    if (!safeCommand.executionResultOk()) {
      return;
    }
  }

  m_configDialog->setServiceFlag(isConnectedToService);
  m_configDialog->show();

  ControlApplication::addModelessDialog(m_configDialog->getControl()->getWindow());
}

void ControlTrayIcon::onDisconnectAllClientsMenuItemClick()
{
  DisconnectAllCommand unsafeCommand(m_serverControl);

  ControlCommand safeCommand(&unsafeCommand, m_notificator);

  safeCommand.execute();
}

void ControlTrayIcon::onShutdownServerMenuItemClick()
{

  bool someoneConnected = (getIcon() == m_iconWorking);

  if (someoneConnected) {
    TvnServerInfo serverInfo = {0};

    {
      AutoLock l(&m_serverInfoMutex);

      serverInfo = m_lastKnownServerInfo;
    }

    StringStorage userMessage;

    UINT stringId = serverInfo.m_serviceFlag ? IDS_TVNSERVER_SERVICE : IDS_TVNSERVER_APP;

    userMessage.format(
      StringTable::getString(IDS_SHUTDOWN_NOTIFICATION_FORMAT),
      StringTable::getString(stringId));

    if (MessageBox(
      getWindow(),
      userMessage.getString(),
      StringTable::getString(IDS_MBC_TVNCONTROL),
      MB_YESNO | MB_ICONQUESTION) == IDNO) {
        return;
    }
  }

  ShutdownCommand unsafeCommand(m_serverControl);

  ControlCommand safeCommand(&unsafeCommand, m_notificator);

  safeCommand.execute();
}

void ControlTrayIcon::onOutgoingConnectionMenuItemClick()
{
  OutgoingConnectionDialog connDialog;

  if (connDialog.showModal() == IDOK) {
    MakeRfbConnectionCommand unsafeCommand(
      m_serverControl,
      connDialog.getConnectString(),
      connDialog.isViewOnly());

    ControlCommand safeCommand(&unsafeCommand, m_notificator);

    safeCommand.execute();
  }
}

void ControlTrayIcon::onAboutMenuItemClick()
{
  m_aboutDialog.show();

  ControlApplication::addModelessDialog(m_aboutDialog.getControl()->getWindow());
}

void ControlTrayIcon::onCloseControlInterfaceMenuItemClick()
{
  m_appControl->shutdown();
}

void ControlTrayIcon::syncStatusWithServer()
{
  try {
    TvnServerInfo info = m_serverControl->getServerInfo();
    std::list<RfbClientInfo *> clients;
    m_serverControl->getClientsList(&clients);

    if (clients.size() > 0) {
      setIcon(m_iconWorking);
    } else if (info.m_acceptFlag) {
      setIcon(m_iconIdle);
    } else {
      setIcon(m_iconDisabled);
    }

    setText(info.m_statusText.getString());

    for (std::list<RfbClientInfo *>::iterator it = clients.begin(); it != clients.end(); it++) {
      delete *it;
    }

    {
      AutoLock l(&m_serverInfoMutex);

      m_lastKnownServerInfo = info;
    }
  } catch (IOException &) {
    setNotConnectedState();
  } catch (Exception &) {
    _ASSERT(FALSE);
  } 
}

void ControlTrayIcon::setNotConnectedState()
{
  setIcon(m_iconDisabled);
  setText(StringTable::getString(IDS_CONTROL_CLIENT_NOT_CONNECTED));
}

void ControlTrayIcon::terminate()
{
  m_termination = true;
  PostMessage(getWindow(), WM_USER + 1, 0, 0);
}

void ControlTrayIcon::waitForTermination()
{
  m_endEvent.waitForEvent();
}
