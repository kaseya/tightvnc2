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

#include "DesktopServerWatcher.h"
#include "win-system/Environment.h"
#include "util/Exception.h"
#include "util/Log.h"
#include "server-config-lib/Configurator.h"
#include "win-system/CurrentConsoleProcess.h"
#include "win-system/AnonymousPipeFactory.h"
#include <time.h>

DesktopServerWatcher::DesktopServerWatcher(ReconnectionListener *recListener)
: m_recListener(recListener),
  m_process(0),
  m_sharedMem(0),
  m_shMemName(_T("Global\\"))
{
  StringStorage folder;
  Environment::getCurrentModuleFolderPath(&folder);

  StringStorage path;
  path.format(_T("\"%s\\tvnserver.exe\""), folder.getString());

  try {
    m_process = new CurrentConsoleProcess(path.getString());

    srand((unsigned)time(0));
    for (int i = 0; i < 20; i++) {
      m_shMemName.appendChar('a' + rand() % ('z' - 'a'));
    }
    m_sharedMem = new SharedMemory(m_shMemName.getString(), 72);
  } catch (...) {
    if (m_process) delete m_process;
    if (m_sharedMem) delete m_sharedMem;
    throw;
  }

  resume();
}

DesktopServerWatcher::~DesktopServerWatcher()
{
  terminate();
  wait();
  delete m_process;
}

void DesktopServerWatcher::execute()
{
  AnonymousPipeFactory pipeFactory;
  AnonymousPipe *ownSidePipeChanTo, *otherSidePipeChanTo,
                *ownSidePipeChanFrom, *otherSidePipeChanFrom;

  UINT64 *mem = (UINT64 *)m_sharedMem->getMemPointer();

  while (!isTerminating()) {
    try {
      mem[0] = 0;

      ownSidePipeChanTo = otherSidePipeChanTo =
      ownSidePipeChanFrom = otherSidePipeChanFrom = 0;

      pipeFactory.generatePipes(&ownSidePipeChanTo, false,
                                &otherSidePipeChanTo, false);
      pipeFactory.generatePipes(&ownSidePipeChanFrom, false,
                                &otherSidePipeChanFrom, false);

      StringStorage logDir;
      Configurator::getInstance()->getServerConfig()->getLogFileDirectory(&logDir);

      StringStorage args;
      args.format(_T("-desktopserver -logdir \"%s\" -loglevel %d -shmemname %s"),
                  logDir.getString(),
                  Configurator::getInstance()->getServerConfig()->getLogLevel(),
                  m_shMemName.getString());

      m_process->setArguments(args.getString());
      m_process->start();

      otherSidePipeChanTo->assignHandlesFor(m_process->getProcessHandle(),
                                            false);
      otherSidePipeChanFrom->assignHandlesFor(m_process->getProcessHandle(),
                                              false);

      mem[1] = (UINT64)otherSidePipeChanTo->getWriteHandle();
      mem[2] = (UINT64)otherSidePipeChanTo->getReadHandle();
      mem[3] = (UINT64)otherSidePipeChanFrom->getWriteHandle();
      mem[4] = (UINT64)otherSidePipeChanFrom->getReadHandle();
      mem[0] = 1;

      delete otherSidePipeChanTo;
      otherSidePipeChanTo = 0;
      delete otherSidePipeChanFrom;
      otherSidePipeChanFrom = 0;

      m_recListener->onReconnect(ownSidePipeChanTo, ownSidePipeChanFrom);

      m_process->waitForExit();

    } catch (Exception &e) {
      if (ownSidePipeChanTo) delete ownSidePipeChanTo;
      if (otherSidePipeChanTo) delete otherSidePipeChanTo;
      if (ownSidePipeChanFrom) delete ownSidePipeChanFrom;
      if (otherSidePipeChanFrom) delete otherSidePipeChanFrom;
      Log::error(_T("DesktopServerWatcher has failed with error: %s"), e.getMessage());
      Sleep(1000);
    }
  }
}

void DesktopServerWatcher::onTerminate()
{
  m_process->stopWait();
}
