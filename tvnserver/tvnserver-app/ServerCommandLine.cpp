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

#include "ServerCommandLine.h"
#include "util/CommandLine.h"

const TCHAR ServerCommandLine::RUN_SERVER_PORTABLE_KEY[] = _T("-runportable");
const TCHAR ServerCommandLine::RUN_SERVER_KEY[] = _T("-run");
const TCHAR ServerCommandLine::SERVER_NAME_KEY[] = _T("-servername");
const TCHAR ServerCommandLine::VNC_INI_DIRECTORY_PATH[] = _T("-inidirectory");



ServerCommandLine::ServerCommandLine()
{
}

ServerCommandLine::~ServerCommandLine()
{
}

bool ServerCommandLine::parse(const TCHAR *commandLine)
{
  CommandLineFormat format[] = {
    { _T("-help"), NO_ARG },
    { _T("-h"), NO_ARG },
    { _T("-?"), NO_ARG },
    { _T("/help"), NO_ARG },
    { _T("/h"), NO_ARG },
    { _T("/?"), NO_ARG },
    { RUN_SERVER_KEY, NO_ARG },
    { RUN_SERVER_PORTABLE_KEY, NO_ARG },
    { SERVER_NAME_KEY,  NEEDS_ARG },
    { VNC_INI_DIRECTORY_PATH,  NEEDS_ARG },



  };

  if (!CommandLine::parse(format, sizeof(format) / sizeof(CommandLineFormat), commandLine)) {
    return false;
  }
  if (showHelp() && optionSpecified(RUN_SERVER_KEY) && optionSpecified(RUN_SERVER_PORTABLE_KEY)) {
    return false;
  }

  return true;
}

bool ServerCommandLine::showHelp()
{
  return optionSpecified(_T("-help")) || optionSpecified(_T("-h")) ||
         optionSpecified(_T("-?")) || optionSpecified(_T("/help")) ||
         optionSpecified(_T("/h")) || optionSpecified(_T("/?"));
}
