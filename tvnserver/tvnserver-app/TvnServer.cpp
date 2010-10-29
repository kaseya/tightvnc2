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

#include "TvnServer.h"
#include "WsConfigRunner.h"
#include "AdditionalActionApplication.h"
#include "win-system/CurrentConsoleProcess.h"

#include "server-config-lib/Configurator.h"

#include "thread/GlobalMutex.h"

#include "tvnserver/resource.h"

#include "wsconfig-lib/TvnLogFilename.h"

#include "network/socket/WindowsSocket.h"

#include "util/FileLog.h"
#include "util/StringTable.h"

#include "file-lib/File.h"

#include "tvncontrol-app/TransportFactory.h"
#include "tvncontrol-app/ControlPipeName.h"

#include "tvnserver/BuildTime.h"

#include <crtdbg.h>
#include <time.h>

TvnServer::TvnServer(bool runsInServiceContext, StringStorage vncIniDirPath /*""*/, bool runAsPortable /*=false*/ )
: Singleton<TvnServer>(),
  ListenerContainer<TvnServerListener *>(),
  m_runAsService(runsInServiceContext),
  m_runPortable(runAsPortable),
  m_rfbClientManager(0),
  m_httpServer(0), m_controlServer(0), m_rfbServer(0)
{
  Configurator *configurator = Configurator::getInstance();

  configurator->setServiceFlag(m_runAsService);
  configurator->setPortableRunFlag(m_runPortable);
  configurator->setVncIniDirectoryPath ( vncIniDirPath.getString() );

  configurator->load();

  m_config = Configurator::getInstance()->getServerConfig();

  resetLogFilePath();

  m_log.changeLevel(m_config->getLogLevel());

  Log::message(_T("TightVNC Server Build on %s"), BuildTime::DATE);

  Log::info(_T("Initialize WinSock"));

  try {
    WindowsSocket::startup(2, 1);
  } catch (Exception &ex) {
    Log::interror(_T("%s"), ex.getMessage());
  }

  ZombieKiller *zombieKiller = new ZombieKiller();

  m_rfbClientManager = new RfbClientManager(NULL);

  m_rfbClientManager->addListener(this);

  Configurator::getInstance()->addListener(this);

  {
    AutoLock l(&m_mutex);

    restartMainRfbServer();
    (void)m_extraRfbServers.reload(m_runAsService, m_rfbClientManager);
    restartHttpServer();
    restartControlServer();
  }
}

TvnServer::~TvnServer()
{
  Configurator::getInstance()->removeListener(this);

  stopControlServer();
  stopHttpServer();
  m_extraRfbServers.shutDown();
  stopMainRfbServer();

  ZombieKiller *zombieKiller = ZombieKiller::getInstance();

  zombieKiller->killAllZombies();

  m_rfbClientManager->removeListener(this);

  delete m_rfbClientManager;

  delete zombieKiller;

  Log::info(_T("Shutdown WinSock"));

  try {
    WindowsSocket::cleanup();
  } catch (Exception &ex) {
    Log::error(_T("%s"), ex.getMessage());
  }

  delete Configurator::getInstance();
}

void TvnServer::onConfigReload(ServerConfig *serverConfig)
{
  resetLogFilePath();

  m_log.changeLevel(serverConfig->getLogLevel());

  {
    AutoLock l(&m_mutex);

    bool toggleMainRfbServer =
      m_config->isAcceptingRfbConnections() != (m_rfbServer != 0);
    bool changeMainRfbPort = m_rfbServer != 0 &&
      (m_config->getRfbPort() != (int)m_rfbServer->getBindPort());

    const TCHAR *bindHost =
      m_config->isOnlyLoopbackConnectionsAllowed() ? _T("localhost") : _T("0.0.0.0");
    bool changeBindHost =  m_rfbServer != 0 &&
      _tcscmp(m_rfbServer->getBindHost(), bindHost) != 0;

    if (toggleMainRfbServer ||
        changeMainRfbPort ||
        changeBindHost) {
      restartMainRfbServer();
    }

    (void)m_extraRfbServers.reload(m_runAsService, m_rfbClientManager);
  }

  {
    AutoLock l(&m_mutex);

    bool toggleHttp =
      m_config->isAcceptingHttpConnections() != (m_httpServer != 0);
    bool changePort = m_httpServer != 0 &&
      (m_config->getHttpPort() != (int)m_httpServer->getBindPort());

    if (toggleHttp || changePort) {
      restartHttpServer();
    }
  }
}

void TvnServer::getServerInfo(TvnServerInfo *info)
{
  bool rfbServerListening = true;
  {
    AutoLock l(&m_mutex);
    rfbServerListening = m_rfbServer != 0;
  }

  StringStorage statusString;

  bool vncAuthEnabled = m_config->isUsingAuthentication();
  bool noVncPasswords = !m_config->hasPrimaryPassword() && !m_config->hasReadOnlyPassword();
  bool vncPasswordsError = vncAuthEnabled && noVncPasswords;

  if (rfbServerListening) {
    if (vncPasswordsError) {
      statusString = StringTable::getString(IDS_NO_PASSWORDS_SET);
    } else {
      char localAddressString[1024];
      getLocalIPAddrString(localAddressString, 1024);

      statusString.fromAnsiString(localAddressString);

      if (!vncAuthEnabled) {
        statusString.appendString(StringTable::getString(IDS_NO_AUTH_STATUS));
      } 
    } 
  } else {
    statusString = StringTable::getString(IDS_SERVER_NOT_LISTENING);
  } 

  UINT stringId = m_runAsService ? IDS_TVNSERVER_SERVICE : IDS_TVNSERVER_APP;

  info->m_statusText.format(_T("%s - %s"),
                            StringTable::getString(stringId),
                            statusString.getString());
  info->m_acceptFlag = rfbServerListening && !vncPasswordsError;
  info->m_serviceFlag = m_runAsService;
}

void TvnServer::generateExternalShutdownSignal()
{
  AutoLock l(&m_listeners);

  vector<TvnServerListener *>::iterator it;
  for (it = m_listeners.begin(); it != m_listeners.end(); it++) {
    TvnServerListener *each = *it;

    each->onTvnServerShutdown();
  } 
}

bool TvnServer::isRunningAsService() const
{
  return m_runAsService;
}

bool TvnServer::isRunningAsPortableService() const
{
	return m_runAsService && m_runPortable;
}

void TvnServer::afterFirstClientConnect()
{
}

void TvnServer::afterLastClientDisconnect()
{
  ServerConfig::DisconnectAction action = m_config->getDisconnectAction();

  StringStorage keys;

  switch (action) {
  case ServerConfig::DA_LOCK_WORKSTATION:
    keys.format(_T("%s"), AdditionalActionApplication::LOCK_WORKSTATION_KEY);
    break;
  case ServerConfig::DA_LOGOUT_WORKSTATION:
    keys.format(_T("%s"), AdditionalActionApplication::LOGOUT_KEY);
    break;
  case ServerConfig::DA_STOP_AND_REMOVE_SERVICE:
	  if (isRunningAsPortableService())
	  {
	      keys.format(_T("%s"), AdditionalActionApplication::STOP_AND_REMOVE_PORTABLE_SERVICE_KEY);
	      break;
	  }
  default:
    return;
  }

  Process *process;

  if (isRunningAsService()) {
    process = new CurrentConsoleProcess(_T("tvnserver.exe"), keys.getString());
  } else {
    process = new Process(_T("tvnserver.exe"), keys.getString());
  }

  Log::message(_T("Execute disconnect action in separate process"));

  try {
    process->start();
  } catch (SystemException &ex) {
    Log::error(_T("Failed to start application: \"%s\""), ex.getMessage());
  }

  delete process;
}

void TvnServer::restartHttpServer()
{

  stopHttpServer();

  if (m_config->isAcceptingHttpConnections()) {
    Log::message(_T("Starting HTTP server"));
    try {
      m_httpServer = new HttpServer(_T("0.0.0.0"), m_config->getHttpPort(), m_runAsService);
    } catch (Exception &ex) {
      Log::error(_T("Failed to start HTTP server: \"%s\""), ex.getMessage());
    }
  }
}

void TvnServer::restartControlServer()
{

  stopControlServer();

  Log::message(_T("Starting control server"));

  try {
    Transport *transport = 0;

    StringStorage pipeName;
    ControlPipeName::createPipeName(isRunningAsService(), &pipeName);
    transport = TransportFactory::createPipeServerTransport(pipeName.getString());

    m_controlServer = new ControlServer(transport, m_rfbClientManager);
  } catch (Exception &ex) {
    Log::error(_T("Failed to start control server: \"%s\""), ex.getMessage());
  }
}

void TvnServer::restartMainRfbServer()
{

  stopMainRfbServer();

  if (!m_config->isAcceptingRfbConnections()) {
    return;
  }

  const TCHAR *bindHost = m_config->isOnlyLoopbackConnectionsAllowed() ? _T("localhost") : _T("0.0.0.0");
  unsigned short bindPort = m_config->getRfbPort();

  Log::message(_T("Starting main RFB server"));

  try {
    m_rfbServer = new RfbServer(bindHost, bindPort, m_rfbClientManager, m_runAsService);
  } catch (Exception &ex) {
    Log::error(_T("Failed to start main RFB server: \"%s\""), ex.getMessage());
  }
}

void TvnServer::stopHttpServer()
{
  Log::message(_T("Stopping HTTP server"));

  HttpServer *httpServer = 0;
  {
    AutoLock l(&m_mutex);
    httpServer = m_httpServer;
    m_httpServer = 0;
  }
  if (httpServer != 0) {
    delete httpServer;
  }
}

void TvnServer::stopControlServer()
{
  Log::message(_T("Stopping control server"));

  ControlServer *controlServer = 0;
  {
    AutoLock l(&m_mutex);
    controlServer = m_controlServer;
    m_controlServer = 0;
  }
  if (controlServer != 0) {
    delete controlServer;
  }
}

void TvnServer::stopMainRfbServer()
{
  Log::message(_T("Stopping main RFB server"));

  RfbServer *rfbServer = 0;
  {
    AutoLock l(&m_mutex);
    rfbServer = m_rfbServer;
    m_rfbServer = 0;
  }
  if (rfbServer != 0) {
    delete rfbServer;
  }
}

void TvnServer::resetLogFilePath()
{
  StringStorage pathToLogDirectory;

  TvnLogFilename::queryLogFileDirectory(m_runAsService,
    m_config->isSaveLogToAllUsersPathFlagEnabled(),
    &pathToLogDirectory);

  {
    File logDirectory(pathToLogDirectory.getString());

    logDirectory.mkdir();
  }

  StringStorage pathToLogFile;

  TvnLogFilename::queryLogFilePath(m_runAsService,
    m_config->isSaveLogToAllUsersPathFlagEnabled(),
    &pathToLogFile);

  m_config->setLogFilePath(pathToLogFile.getString());

  m_log.changeFilename(pathToLogFile.getString());
}

