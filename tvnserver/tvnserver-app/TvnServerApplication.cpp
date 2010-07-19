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

#include "TvnServerApplication.h"
#include "ServerCommandLine.h"
#include "TvnServerHelp.h"

#include "thread/GlobalMutex.h"

#include "util/ResourceLoader.h"
#include "util/StringTable.h"

#include "tvnserver/resource.h"

TvnServerApplication::TvnServerApplication(HINSTANCE hInstance, const TCHAR *commandLine)
: WindowsApplication(hInstance), m_tvnServer(0), m_commandLine(commandLine)
{
}

TvnServerApplication::~TvnServerApplication()
{
}

int TvnServerApplication::run()
{

  ServerCommandLine parser;

  if (!parser.parse(m_commandLine.getString()) || parser.showHelp()) {
    TvnServerHelp::showUsage();
    return 0;
  }

  GlobalMutex *appInstanceMutex;

  try {
    appInstanceMutex = new GlobalMutex(_T("tvnserverApplication"), false, true);
  } catch (...) {
    MessageBox(0,
               StringTable::getString(IDS_SERVER_ALREADY_RUNNING),
               StringTable::getString(IDS_MBC_TVNSERVER), MB_OK | MB_ICONEXCLAMATION);
    return 1;
  }

  m_tvnServer = new TvnServer(false);

  m_tvnServer->addListener(this);

  m_tvnControlRunner = new WsConfigRunner();

  int exitCode = WindowsApplication::run();

  delete m_tvnControlRunner;

  m_tvnServer->removeListener(this);

  delete m_tvnServer;

  delete appInstanceMutex;

  return exitCode;
}

void TvnServerApplication::onTvnServerShutdown()
{
  WindowsApplication::shutdown();
}
