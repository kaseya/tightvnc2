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

#ifndef __DESKTOP_SERVER_APPLICATION_H__
#define __DESKTOP_SERVER_APPLICATION_H__

#include "win-system/WindowsApplication.h"
#include "util/AnEventListener.h"
#include "win-system/AnonymousPipe.h"
#include "desktop-ipc/BlockingGate.h"
#include "desktop-ipc/DesktopSrvDispatcher.h"
#include "desktop-ipc/UpdateHandlerServer.h"
#include "desktop-ipc/UserInputServer.h"
#include "desktop-ipc/ConfigServer.h"
#include "desktop-ipc/GateKickHandler.h"
#include "SessionChangesWatcher.h"
#include "win-system/LocalWindowsApplication.h"
#include "util/FileLog.h"
#include "server-config-lib/ConfigReloadListener.h"
class DesktopServerApplication : public LocalWindowsApplication,
                                 public AnEventListener,
                                 public ConfigReloadListener
{
public:
  DesktopServerApplication(HINSTANCE appInstance, const TCHAR *commandLine);

  virtual ~DesktopServerApplication();

  virtual int run();

protected:
  virtual void onAnObjectEvent();
  virtual void onConfigReload(ServerConfig *serverConfig);

private:
  void freeResources();

  AnonymousPipe *m_clToSrvChan;
  AnonymousPipe *m_srvToClChan;
  BlockingGate *m_clToSrvGate;
  BlockingGate *m_srvToClGate;

  DesktopSrvDispatcher *m_dispatcher;

  UpdateHandlerServer *m_updHandlerSrv;
  UserInputServer *m_uiSrv;
  ConfigServer *m_cfgServer;
  GateKickHandler *m_gateKickHandler;

  SessionChangesWatcher *m_sessionChangesWatcher;

  FileLog *m_log;
};

#endif 
