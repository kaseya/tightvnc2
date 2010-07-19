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

#include "DesktopServerApplication.h"
#include "DesktopServerCommandLine.h"
#include "util/FileLog.h"
#include "util/ResourceLoader.h"
#include "desktop/WallpaperUtil.h"
#include "win-system/WTS.h"
#include "win-system/Environment.h"
#include "win-system/SharedMemory.h"

DesktopServerApplication::DesktopServerApplication(HINSTANCE appInstance, const TCHAR *commandLine)
: LocalWindowsApplication(appInstance),
  m_clToSrvChan(0),
  m_srvToClChan(0),
  m_clToSrvGate(0),
  m_srvToClGate(0),
  m_dispatcher(0),
  m_updHandlerSrv(0),
  m_uiSrv(0),
  m_cfgServer(0),
  m_gateKickHandler(0),
  m_sessionChangesWatcher(0)
{
  DesktopServerCommandLine cmdLineParser;

  cmdLineParser.parse(commandLine);

  DWORD baseSessionId = WTS::getActiveConsoleSessionId();

  StringStorage pathToLog;

  cmdLineParser.getLogDir(&pathToLog);

  if (!pathToLog.endsWith(_T('\\'))) {
    pathToLog.appendChar(_T('\\'));
  }
  pathToLog.appendString(_T("dtserver.log"));
  m_log = new FileLog(pathToLog.getString(), true);
  Configurator::getInstance()->addListener(this);

  m_log->changeLevel(cmdLineParser.getLogLevel());
  Log::message(_T("Log level has been changed to %d"),
               cmdLineParser.getLogLevel());

  try {
    StringStorage shMemName;
    cmdLineParser.getSharedMemName(&shMemName);
    SharedMemory shMem(shMemName.getString(), 72);
    UINT64 *mem = (UINT64 *)shMem.getMemPointer();
    HANDLE hWrite, hRead;

    DateTime startTime = DateTime::now();

    while (mem[0] == 0) {
      unsigned int timeForWait = max((int)10000 - 
                                     (int)(DateTime::now() -
                                           startTime).getTime(),
                                     0);
      if (timeForWait == 0) {
        throw Exception(_T("The desktop server time out expired"));
      }
    }

    hWrite = (HANDLE)mem[1];
    hRead  = (HANDLE)mem[2];
    m_clToSrvChan = new AnonymousPipe(hWrite, hRead);
    Log::info(_T("Client->server hWrite = %u; hRead = %u"), hWrite, hRead);

    hWrite = (HANDLE)mem[3];
    hRead  = (HANDLE)mem[4];
    m_srvToClChan = new AnonymousPipe(hWrite, hRead);
    Log::info(_T("Server->client hWrite = %u; hRead = %u"), hWrite, hRead);

    m_clToSrvGate = new BlockingGate(m_clToSrvChan);
    m_srvToClGate = new BlockingGate(m_srvToClChan);

    m_dispatcher = new DesktopSrvDispatcher(m_clToSrvGate, this);

    m_updHandlerSrv = new UpdateHandlerServer(m_srvToClGate, m_dispatcher, this);
    m_uiSrv = new UserInputServer(m_srvToClGate, m_dispatcher, this);
    m_cfgServer = new ConfigServer(m_dispatcher);
    m_gateKickHandler = new GateKickHandler(m_dispatcher);

    m_dispatcher->resume();

    m_sessionChangesWatcher = new SessionChangesWatcher(this);
  } catch (Exception &e) {
    Log::error(e.getMessage());
    freeResources();
    throw;
  }
}

DesktopServerApplication::~DesktopServerApplication()
{
  freeResources();
  Log::info(_T("Desktop server application has been terminated"));
  delete m_log;
}

void DesktopServerApplication::freeResources()
{
  try {
    if (m_clToSrvChan) m_clToSrvChan->close();
  } catch (Exception &e) {
    Log::error(_T("Cannot close client->server channel: %s"),
               e.getMessage());
  }
  try {
    if (m_srvToClChan) m_srvToClChan->close();
  } catch (Exception &e) {
    Log::error(_T("Cannot close server->client channel: %s"),
               e.getMessage());
  }

  if (m_sessionChangesWatcher) delete m_sessionChangesWatcher;

  if (m_gateKickHandler) delete m_gateKickHandler;
  if (m_cfgServer) delete m_cfgServer;
  if (m_uiSrv) delete m_uiSrv;
  if (m_updHandlerSrv) delete m_updHandlerSrv;

  if (m_dispatcher) delete m_dispatcher;

  if (m_srvToClGate) delete m_srvToClGate;
  if (m_clToSrvGate) delete m_clToSrvGate;
  if (m_srvToClChan) delete m_srvToClChan;
  if (m_clToSrvChan) delete m_clToSrvChan;
}

void DesktopServerApplication::onAnObjectEvent()
{
  WindowsApplication::shutdown();
}

void DesktopServerApplication::onConfigReload(ServerConfig *serverConfig)
{
  m_log->changeLevel(serverConfig->getLogLevel());
}

int DesktopServerApplication::run()
{
  try {
    WallpaperUtil wp;

    return WindowsApplication::run();
  } catch (Exception &e) {
    Log::error(_T("Desktop server has been terminated with error: %s"),
               e.getMessage());
    return 0;
  }
}
