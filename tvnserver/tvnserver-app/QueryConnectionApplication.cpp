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

#include "QueryConnectionApplication.h"
#include "QueryConnectionCommandLine.h"
#include "QueryConnectionDialog.h"
#include "win-system/CurrentConsoleProcess.h"
#include "TvnServerHelp.h"

#include "server-config-lib/Configurator.h"
#include "win-system/Environment.h"
#include "win-system/Process.h"
#include "util/Log.h"
#include "util/ResourceLoader.h"
#include "tvnserver/resource.h"

QueryConnectionApplication::QueryConnectionApplication(HINSTANCE hInstance, const TCHAR *cmdLine)
: LocalWindowsApplication(hInstance), m_cmdLine(cmdLine)
{
}

QueryConnectionApplication::~QueryConnectionApplication()
{
}

int QueryConnectionApplication::run()
{
  QueryConnectionCommandLine parser;

  try {
    parser.parse(m_cmdLine.getString());
  } catch (Exception &) {
    TvnServerHelp::showUsage();
    return 0;
  }

  StringStorage peerAddress;

  parser.getPeerAddress(&peerAddress);

  DWORD queryTimeout = parser.isTimeoutSpecified() ? parser.getTimeout() : 30;

  QueryConnectionDialog dialog(peerAddress.getString(),
                               parser.isDefaultActionAccept(),
                               queryTimeout);

  return dialog.showModal();
}

int QueryConnectionApplication::execute(const TCHAR *peerAddr, bool acceptByDefault, DWORD timeOutSec)
{

  StringStorage curModulePath;
  StringStorage command;

  Environment::getCurrentModulePath(&curModulePath);

  command.format(_T("%s %s %s %s %s %s %d"),
                 curModulePath.getString(),
                 QueryConnectionCommandLine::QUERY_CONNECTION,
                 QueryConnectionCommandLine::PEER_ADDR,
                 peerAddr,
                 acceptByDefault ? QueryConnectionCommandLine::ACCEPT : _T(""),
                 QueryConnectionCommandLine::TIMEOUT,
                 timeOutSec);

  Process *process = 0;

  int defaultRetCode = acceptByDefault ? 0 : 1;
  int retCode = defaultRetCode;

  if (Configurator::getInstance()->getServiceFlag()) {
    process = new CurrentConsoleProcess(command.getString());
  } else {
    process = new Process(command.getString());
  }

  try {
    process->start();
    process->waitForExit();
    retCode = process->getExitCode();
  } catch (Exception &ex) {
    Log::error(ex.getMessage());
  }

  delete process;

  if (retCode != 0 && retCode != 1) {
    retCode = defaultRetCode;
  }

  return retCode;
}
