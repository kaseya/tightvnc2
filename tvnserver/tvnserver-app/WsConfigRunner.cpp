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

#include "WsConfigRunner.h"
#include "win-system/CurrentConsoleProcess.h"
#include "util/Log.h"
#include "win-system/Environment.h"
#include "server-config-lib/Configurator.h"
#include "tvncontrol-app/ControlCommandLine.h"

WsConfigRunner::WsConfigRunner(bool serviceMode)
: m_serviceMode(serviceMode)
{
  resume();
}

WsConfigRunner::~WsConfigRunner()
{
  terminate();
  wait();
}

void WsConfigRunner::execute()
{
  Process *process = 0;

  try {
    StringStorage folder;
    Environment::getCurrentModuleFolderPath(&folder);
    StringStorage path;
    path.format(_T("\"%s\\tvnserver.exe\""), folder.getString());
    StringStorage args;
    args.format(_T("%s %s"),
      m_serviceMode ? ControlCommandLine::CONTROL_SERVICE :
                      ControlCommandLine::CONTROL_APPLICATION,
      ControlCommandLine::SLAVE_MODE);
    process = new Process(path.getString(), args.getString());
    process->start();
  } catch (Exception &e) {
    Log::error(_T("Cannot start the WsControl process (%s)"), e.getMessage());
  }

  if (process != 0) {
    delete process;
  }
}
