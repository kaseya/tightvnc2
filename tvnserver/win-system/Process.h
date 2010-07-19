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

#ifndef __PROCESSMANAGER_H__
#define __PROCESSMANAGER_H__

#include "util/CommonHeader.h"

#include "SystemException.h"

class Process
{
public:
  Process(const TCHAR *path = 0, const TCHAR *args = 0);

  virtual ~Process();

  void setFilename(const TCHAR *path);

  void setArguments(const TCHAR *args);

  void setStandardIoHandles(HANDLE stdIn, HANDLE stdOut, HANDLE stdErr);

  void setHandleInheritances(bool handlesIsInerited);

  virtual void start() throw(SystemException);

  virtual void kill() throw(SystemException);

  void waitForExit();

  void stopWait();

  DWORD getExitCode() throw(SystemException);

  HANDLE getProcessHandle();

protected:
  StringStorage getCommandLineString();

  void getStartupInfo(STARTUPINFO *sti);

  void cleanup();

  StringStorage m_path;
  StringStorage m_args;

  HANDLE m_hProcess;
  HANDLE m_hThread;

  HANDLE m_hStopWait;

  HANDLE m_stdIn;
  HANDLE m_stdOut;
  HANDLE m_stdErr;

  bool m_handlesIsInherited;
};

#endif 
