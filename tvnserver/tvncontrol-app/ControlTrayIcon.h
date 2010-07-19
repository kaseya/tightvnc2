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

#ifndef _CONTROL_TRAY_ICON_H_
#define _CONTROL_TRAY_ICON_H_

#include "gui/NotifyIcon.h"
#include "gui/WindowProcHolder.h"
#include "gui/drawing/Icon.h"

#include "thread/Thread.h"
#include "thread/LocalMutex.h"

#include "util/Command.h"
#include "util/MacroCommand.h"

#include "wsconfig-lib/ConfigDialog.h"

#include "ControlProxy.h"
#include "Notificator.h"
#include "ControlApplication.h"
#include "AboutDialog.h"

class ControlTrayIcon : public NotifyIcon,  
                        public WindowProcHolder  
{
public:
  ControlTrayIcon(ControlProxy *serverControl,
                  Notificator *notificator,
                  ControlApplication *appControl,
                  bool showAfterCreation);
  virtual ~ControlTrayIcon();

  void syncStatusWithServer();

  void terminate();

  void waitForTermination();

protected:
  void setNotConnectedState();

  virtual LRESULT windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool *useDefWindowProc);

  void onRightButtonUp();
  void onLeftButtonDown();

  void onConfigurationMenuItemClick();
  void onDisconnectAllClientsMenuItemClick();
  void onShutdownServerMenuItemClick();
  void onOutgoingConnectionMenuItemClick();
  void onAboutMenuItemClick();
  void onCloseControlInterfaceMenuItemClick();

protected:
  Notificator *m_notificator;

  ControlApplication *m_appControl;

  Icon *m_iconWorking;
  Icon *m_iconIdle;
  Icon *m_iconDisabled;

  ControlProxy *m_serverControl;

  ConfigDialog *m_configDialog;
  AboutDialog m_aboutDialog;

  TvnServerInfo m_lastKnownServerInfo;
  LocalMutex m_serverInfoMutex;

  Command *m_updateRemoteConfigCommand;
  Command *m_updateLocalConfigCommand;
  MacroCommand *m_applyChangesMacroCommand;
  Command *m_applyChangesControlCommand;

  bool m_inWindowProc;

  WindowsEvent m_endEvent;
  bool m_termination;
};

#endif
