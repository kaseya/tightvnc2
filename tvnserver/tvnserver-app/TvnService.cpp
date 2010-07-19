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

#include "TvnService.h"

#include "ServerCommandLine.h"

#include "win-system/SCMClient.h"
#include "win-system/Environment.h"

const TCHAR TvnService::SERVICE_COMMAND_LINE_KEY[] = _T("-service");

const TCHAR TvnService::SERVICE_NAME[]             = _T("tvnserver");
const TCHAR TvnService::SERVICE_NAME_TO_DISPLAY[]  = _T("TightVNC Server");

TvnService::TvnService()
: Service(SERVICE_NAME), m_tvnServer(0)
{
}

TvnService::~TvnService()
{
}

void TvnService::onStart()
{
  m_tvnServer = new TvnServer(true);

  m_tvnServer->addListener(this);
}

void TvnService::main()
{
  m_shutdownEvent.waitForEvent();

  m_tvnServer->removeListener(this);

  delete m_tvnServer;

  m_tvnServer = 0;
}

void TvnService::onStop()
{
  m_shutdownEvent.notify();
}

void TvnService::onTvnServerShutdown()
{
  TvnService::stop();
}

void TvnService::install()
{
  StringStorage binPath;

  TvnService::getBinPath(&binPath);

  SCMClient scManager;

  scManager.installService(SERVICE_NAME, SERVICE_NAME_TO_DISPLAY,
                           binPath.getString(), _T(""));
}

void TvnService::remove()
{
  SCMClient scManager;

  scManager.removeService(SERVICE_NAME);
}

void TvnService::reinstall()
{
  try {
    remove();
  } catch (...) { }

  TvnService::install();
}

void TvnService::start(bool waitCompletion)
{
  SCMClient scManager;

  scManager.startService(SERVICE_NAME, waitCompletion);
}

void TvnService::stop(bool waitCompletion)
{
  SCMClient scManager;

  scManager.stopService(SERVICE_NAME, waitCompletion);
}

bool TvnService::getBinPath(StringStorage *binPath)
{
  StringStorage pathToServiceBinary;

  if (!Environment::getCurrentModuleFolderPath(&pathToServiceBinary)) {
    return false;
  }

  pathToServiceBinary.appendString(_T("\\tvnserver.exe"));

  binPath->format(_T("\"%s\" %s"),
                  pathToServiceBinary.getString(),
                  SERVICE_COMMAND_LINE_KEY);

  return true;
}
