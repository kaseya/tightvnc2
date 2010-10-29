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

#ifndef _TVN_SERVER_H_
#define _TVN_SERVER_H_

#include "util/CommonHeader.h"

#include "RfbClientManager.h"
#include "RfbServer.h"
#include "ExtraRfbServers.h"
#include "ControlServer.h"
#include "TvnServerListener.h"

#include "http-server-lib/HttpServer.h"

#include "thread/ZombieKiller.h"
#include "thread/LocalMutex.h"

#include "util/Singleton.h"
#include "util/FileLog.h"
#include "util/ListenerContainer.h"

#include "server-config-lib/ConfigReloadListener.h"

#include "tvncontrol-app/TvnServerInfo.h"

class TvnServer : public Singleton<TvnServer>,
                  public ListenerContainer<TvnServerListener *>,
                  public ConfigReloadListener,
                  public RfbClientManagerEventListener
{
public:
  TvnServer(bool runsInServiceContext, StringStorage vncIniDirPath = _T( ""), bool runAsPortable=false);
  
  virtual ~TvnServer();

  void getServerInfo(TvnServerInfo *info);

  virtual void onConfigReload(ServerConfig *serverConfig);

  void generateExternalShutdownSignal();

  bool isRunningAsService() const;
  bool isRunningAsPortableService() const;

  virtual void afterFirstClientConnect();

  virtual void afterLastClientDisconnect();

protected:
  void restartHttpServer();
  void restartControlServer();
  void restartMainRfbServer();

  void stopHttpServer();
  void stopControlServer();
  void stopMainRfbServer();

  void resetLogFilePath();

protected:
  FileLog m_log;

  LocalMutex m_mutex;

  const bool m_runAsService;
  const bool m_runPortable;

  RfbClientManager *m_rfbClientManager;
  ControlServer *m_controlServer;
  HttpServer *m_httpServer;
  RfbServer *m_rfbServer;
  ExtraRfbServers m_extraRfbServers;

  ServerConfig *m_config;
};

#endif
