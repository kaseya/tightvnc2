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

#ifndef __THREAD_H__
#define __THREAD_H__

#include "util/CommonHeader.h"
#include "LocalMutex.h"
#include "DesktopSelector.h"

enum THREAD_PRIORITY
{
  PRIORITY_IDLE,
  PRIORITY_LOWEST,
  PRIORITY_BELOW_NORMAL,
  PRIORITY_NORMAL,
  PRIORITY_ABOVE_NORMAL,
  PRIORITY_HIGHEST,
  PRIORITY_TIME_CRITICAL
};

class Thread
{
public:
  Thread();
  virtual ~Thread();

  bool wait();
  bool suspend();
  bool resume();
  virtual void terminate();

  bool isActive() const;

  DWORD getThreadId() const;

  bool setPriority(THREAD_PRIORITY value);

  static void sleep(DWORD millis);

protected:
  bool isTerminating();

  virtual void onTerminate();

  virtual void execute() = 0;

private:
  static DWORD WINAPI threadProc(LPVOID pThread);

private:
  HANDLE m_hThread;
  DWORD m_threadID;
  HDESK m_hDesk;
  bool m_active;
  volatile bool m_terminated;
};

#endif 
