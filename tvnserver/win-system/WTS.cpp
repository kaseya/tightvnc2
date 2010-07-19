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

#include "WTS.h"

#include "DynamicLibrary.h"
#include "SystemException.h"

#include "thread/AutoLock.h"

#include <crtdbg.h>

pWTSGetActiveConsoleSessionId WTS::m_WTSGetActiveConsoleSessionId = 0;
pWTSQueryUserToken WTS::m_WTSQueryUserToken = 0;

volatile bool WTS::m_initialized = false;

HANDLE WTS::m_userProcessToken = 0;

LocalMutex WTS::m_mutex;

DWORD WTS::getActiveConsoleSessionId()
{
  AutoLock l(&m_mutex);

  if (!m_initialized) {
    initialize();
  }

  if (m_WTSGetActiveConsoleSessionId == 0) {
    return 0;
  }

  return m_WTSGetActiveConsoleSessionId();
}

void WTS::queryConsoleUserToken(HANDLE *token) throw(SystemException)
{
  {
    AutoLock l(&m_mutex);

    if (!m_initialized) {
      initialize();
    }
  }

  DWORD sessionId = getActiveConsoleSessionId();

  AutoLock l(&m_mutex);

  if (m_WTSQueryUserToken != 0) {
    if (!m_WTSQueryUserToken(sessionId, token)) {
      throw SystemException();
    }
    return;
  } else {
    if (m_userProcessToken == 0) {
      throw SystemException(_T("No console user process id specified"));
    }
    *token = m_userProcessToken;
  }
}

void WTS::defineConsoleUserProcessId(DWORD userProcessId)
{
  HANDLE procHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, userProcessId);

  if (procHandle == 0) {
    throw SystemException();
  }

  HANDLE userProcessToken;

  if (!OpenProcessToken(procHandle, TOKEN_ALL_ACCESS, &userProcessToken)) {
    CloseHandle(procHandle);
    throw SystemException();
  }

  CloseHandle(procHandle);

  AutoLock l(&m_mutex);

  if (m_userProcessToken != 0) {
    CloseHandle(m_userProcessToken);
  }

  m_userProcessToken = userProcessToken;
}

void WTS::initialize()
{
  _ASSERT(!m_initialized);

  try {
    DynamicLibrary kernel32(_T("Kernel32.dll"));

    m_WTSGetActiveConsoleSessionId = (pWTSGetActiveConsoleSessionId)kernel32.getProcAddress("WTSGetActiveConsoleSessionId");
  } catch (SystemException &) {
    _ASSERT(FALSE);
  }

  try {
    DynamicLibrary 	wtsapi32(_T("Wtsapi32.dll"));

    m_WTSQueryUserToken = (pWTSQueryUserToken)wtsapi32.getProcAddress("WTSQueryUserToken");
  } catch (SystemException &) { }

  m_initialized = true;
}

WTS::WTS()
{
}
