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

#include "util/CommonHeader.h"
#include "util/winhdr.h"
#include "util/CommandLine.h"

#include "tvnserver-app/TvnService.h"
#include "tvnserver-app/TvnServerApplication.h"
#include "tvnserver-app/QueryConnectionApplication.h"
#include "tvnserver-app/DesktopServerApplication.h"
#include "tvnserver-app/AdditionalActionApplication.h"
#include "tvnserver-app/ServiceControlApplication.h"
#include "tvnserver-app/ServiceControlCommandLine.h"
#include "tvnserver-app/QueryConnectionCommandLine.h"
#include "tvnserver-app/DesktopServerCommandLine.h"

#include "tvncontrol-app/ControlApplication.h"
#include "tvncontrol-app/ControlCommandLine.h"

#include "tvnserver/resource.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPTSTR lpCmdLine, int nCmdShow)
{
  ResourceLoader resourceLoaderSingleton(hInstance);

  CommandLineFormat format[] = {
    { TvnService::SERVICE_COMMAND_LINE_KEY, NO_ARG },

    { ControlCommandLine::CONFIG_APPLICATION, NO_ARG },
    { ControlCommandLine::CONFIG_SERVICE, NO_ARG },
    { ControlCommandLine::SET_CONTROL_PASSWORD, NO_ARG },
    { ControlCommandLine::SET_PRIMARY_VNC_PASSWORD, NO_ARG },
    { ControlCommandLine::CONTROL_SERVICE, NO_ARG },
    { ControlCommandLine::CONTROL_APPLICATION, NO_ARG },

    { DesktopServerCommandLine::DESKTOP_SERVER_KEY, NO_ARG },
    { QueryConnectionCommandLine::QUERY_CONNECTION, NO_ARG },

    { AdditionalActionApplication::LOCK_WORKSTATION_KEY, NO_ARG },
    { AdditionalActionApplication::LOGOUT_KEY, NO_ARG },

    { ServiceControlCommandLine::INSTALL_SERVICE },
    { ServiceControlCommandLine::REMOVE_SERVICE },
    { ServiceControlCommandLine::REINSTALL_SERVICE },
    { ServiceControlCommandLine::START_SERVICE },
    { ServiceControlCommandLine::STOP_SERVICE }
  };

  CommandLine parser;

  StringStorage firstKey(_T(""));

  parser.parse(format,  sizeof(format) / sizeof(CommandLineFormat), lpCmdLine);
  parser.getOption(0, &firstKey);

  if (firstKey.isEqualTo(TvnService::SERVICE_COMMAND_LINE_KEY)) {
    TvnService tvnService;
    try {
      tvnService.run();
    } catch (Exception &) {
      return 1;
    }
    return 0;
  } else if (firstKey.isEqualTo(ControlCommandLine::CONFIG_APPLICATION) ||
    firstKey.isEqualTo(ControlCommandLine::CONFIG_SERVICE) ||
    firstKey.isEqualTo(ControlCommandLine::SET_CONTROL_PASSWORD) ||
    firstKey.isEqualTo(ControlCommandLine::SET_PRIMARY_VNC_PASSWORD) ||
    firstKey.isEqualTo(ControlCommandLine::CONTROL_SERVICE) ||
    firstKey.isEqualTo(ControlCommandLine::CONTROL_APPLICATION)) {
    try {
      ControlApplication tvnControl(hInstance, lpCmdLine);

      return tvnControl.run();
    } catch (Exception &fatalException) {
      MessageBox(0,
        fatalException.getMessage(),
        StringTable::getString(IDS_MBC_TVNCONTROL),
        MB_OK | MB_ICONERROR);
      return 1;
    }
  } else if (firstKey.isEqualTo(AdditionalActionApplication::LOCK_WORKSTATION_KEY) ||
    firstKey.isEqualTo(AdditionalActionApplication::LOGOUT_KEY)) {
    try {
      AdditionalActionApplication actionApp(hInstance, lpCmdLine);

      return actionApp.run();
    } catch (SystemException &ex) {
      return ex.getErrorCode();
    }
  } else if (firstKey.isEqualTo(DesktopServerCommandLine::DESKTOP_SERVER_KEY)) {
    try {
      DesktopServerApplication desktopServerApp(hInstance, lpCmdLine);

      return desktopServerApp.run();
    } catch (...) {
      return 1;
    }
  } else if (firstKey.isEqualTo(QueryConnectionCommandLine::QUERY_CONNECTION)) {
    try {
      QueryConnectionApplication app(hInstance, lpCmdLine);

      return app.run();
    } catch (...) {
      return 1;
    }
  } else if (firstKey.isEqualTo(ServiceControlCommandLine::INSTALL_SERVICE) ||
    firstKey.isEqualTo(ServiceControlCommandLine::REMOVE_SERVICE) ||
    firstKey.isEqualTo(ServiceControlCommandLine::REINSTALL_SERVICE) ||
    firstKey.isEqualTo(ServiceControlCommandLine::START_SERVICE) ||
    firstKey.isEqualTo(ServiceControlCommandLine::STOP_SERVICE)) {
    ServiceControlApplication tvnsc(hInstance, lpCmdLine);

    return tvnsc.run();
  }

  TvnServerApplication tvnServer(hInstance, lpCmdLine);

  return tvnServer.run();
}
