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

#ifndef _SERVICE_H_
#define _SERVICE_H_

#include "util/winhdr.h"
#include "util/StringStorage.h"
#include "util/Exception.h"
#include "thread/Thread.h"

class Service
{
public:
  Service(const TCHAR *name);

  virtual ~Service();

  void run() throw(Exception);

protected:
  virtual void onStart() = 0;
  virtual void main() = 0;
  virtual void onStop() = 0;

  static void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
  static void WINAPI ServiceControlHandler(DWORD dwCtrlCode);

protected:
  bool reportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode,
                    DWORD dwWaitHint);

protected:
  StringStorage m_name;

  SERVICE_STATUS m_status;
  SERVICE_STATUS_HANDLE m_statusHandle;

  volatile bool m_isTerminating;

  static Service *g_service;
};

#endif
