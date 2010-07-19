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

#ifndef _DESKTOP_SERVER_COMMAND_LINE_H_
#define _DESKTOP_SERVER_COMMAND_LINE_H_

#include "util/CommandLine.h"
#include "util/Exception.h"

class DesktopServerCommandLine : private CommandLine
{
public:
  DesktopServerCommandLine();
  virtual ~DesktopServerCommandLine();

  virtual void parse(const TCHAR *cmdLine) throw(Exception);

  void getLogDir(StringStorage *logDir);

  int getLogLevel();

  void getSharedMemName(StringStorage *shMemName);

public:
  const static TCHAR DESKTOP_SERVER_KEY[];

private:
  const static TCHAR LOG_DIR_KEY[];
  const static TCHAR LOG_LEVEL_KEY[];
  const static TCHAR SHARED_MEMORY_NAME_KEY[];
};

#endif
