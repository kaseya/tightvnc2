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

#include "Environment.h"
#include "CtrlAltDelSimulator.h"

#include <shlobj.h>
#include <crtdbg.h>
#include <Wtsapi32.h>
#include "util/Log.h"
#include "win-system/AutoImpersonator.h"
#include "win-system/WTS.h"
#include "Shell.h"
#include "DynamicLibrary.h"

OSVERSIONINFO Environment::m_osVerInfo = { 0 };
typedef VOID (WINAPI *SendSas)(BOOL asUser);

Environment::Environment()
{
}

Environment::~Environment()
{
}

void Environment::getErrStr(StringStorage *out)
{
  DWORD errCode = GetLastError();
  TCHAR buffer[1024];

  if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, errCode,
                    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                    (LPTSTR)&buffer[0],
                    sizeof(buffer), NULL) == 0) {
    out->format(_T("<<Cannot get text error describing>> (%u)"), errCode);
  } else {
    out->format(_T("%s (%u)"), buffer, errCode);
  }
}

void Environment::getErrStr(const TCHAR *specification, StringStorage *out)
{
  StringStorage sysErrText;
  getErrStr(&sysErrText);
  out->format(_T("%s (%s)"), specification, sysErrText.getString());
}

bool Environment::getSpecialFolderPath(int specialFolderId, StringStorage *out)
{
  _ASSERT(out != NULL);

  int csidl = 0;

  switch (specialFolderId) {
  case APPLICATION_DATA_SPECIAL_FOLDER:
    csidl = CSIDL_APPDATA;
    break;
  case COMMON_APPLICATION_DATA_SPECIAL_FOLDER:
    csidl = CSIDL_COMMON_APPDATA;
    break;
  default:
    _ASSERT(FALSE);
    return false;
  } 

  bool returnVal = false;

  TCHAR path[MAX_PATH + 1];
  if (SHGetSpecialFolderPath(NULL, &path[0], csidl, TRUE) == TRUE) {
    out->setString(&path[0]);
    returnVal = true;
  }

  return returnVal;
}

bool Environment::getCurrentModulePath(StringStorage *out)
{
  TCHAR *buffer;
  size_t size = MAX_PATH;

  while (true) {
    buffer = new TCHAR[size + 1];
    DWORD ret = GetModuleFileName(NULL, buffer, size);

    if (ret == 0) {
      delete[] buffer;
      return false;
    } else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
      delete[] buffer;
      size += 128;
    } else {
      break;
    }
  } 

  out->setString(buffer);
  delete[] buffer;

  return true;
} 

bool Environment::getCurrentModuleFolderPath(StringStorage *out)
{
  if (!getCurrentModulePath(out)) {
    return false;
  }

  int lastPos = out->findLast(_T('\\'));

  if (lastPos >= 0) {
    out->getSubstring(out, 0, max(lastPos - 1, 0));
  }

  return true;
}

bool Environment::getCurrentUserName(StringStorage *out)
{
  TCHAR *userName;
  DWORD byteCount;
  DWORD sessionId = WTS::getActiveConsoleSessionId();
  if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, sessionId,
                                 WTSUserName, &userName, &byteCount) == 0) {
    return false;
  }
  out->setString(userName);
  WTSFreeMemory(userName);

  return true;
}

bool Environment::getComputerName(StringStorage *out)
{
  TCHAR compName[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD length = MAX_COMPUTERNAME_LENGTH + 1;
  if (GetComputerName(compName, &length) == 0) {
    return false;
  }
  out->setString(compName);
  out->toLowerCase();
  return true;
}

void Environment::restoreWallpaper()
{
  Log::info(_T("Try to restore wallpaper"));
  Impersonator imp;
  AutoImpersonator ai(&imp);

  if (SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, 0, 0) == 0) {
      SystemException(_T("Cannot restore desktop wallpaper"));
  }
}

void Environment::disableWallpaper()
{
  Log::info(_T("Try to disable wallpaper"));
  Impersonator imp;
  AutoImpersonator ai(&imp);

  if (SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, _T(""), 0) == 0) {
    SystemException(_T("Cannot disable desktop wallpaper"));
  }
}

void Environment::init()
{
  if (m_osVerInfo.dwOSVersionInfoSize == 0) {
    m_osVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (!GetVersionEx(&m_osVerInfo)) {
      m_osVerInfo.dwOSVersionInfoSize = 0;
    }
  }
}

bool Environment::isWinNTFamily()
{
  init();
  return m_osVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT;
}

bool Environment::isWinXP()
{
  init();
  return ((m_osVerInfo.dwMajorVersion == 5) && (m_osVerInfo.dwMinorVersion == 1) && isWinNTFamily());
}

bool Environment::isWin2003Server()
{
  init();
  return ((m_osVerInfo.dwMajorVersion == 5) && (m_osVerInfo.dwMinorVersion == 2) && isWinNTFamily());
}

bool Environment::isVistaOrLater()
{
  init();
  return m_osVerInfo.dwMajorVersion >= 6;
}

void Environment::simulateCtrlAltDel()
{
  Log::info(_T("Requested Ctrl+Alt+Del simulation"));

  if (!isVistaOrLater() && isWinNTFamily()) {
    CtrlAltDelSimulator cadSim;
    cadSim.wait();
  }
}

void Environment::simulateCtrlAltDelUnderVista()
{
  Log::info(_T("Requested Ctrl+Alt+Del simulation under Vista or later"));

  try {
    DynamicLibrary sasLib(_T("sas.dll"));
    SendSas sendSas = (SendSas)sasLib.getProcAddress("SendSAS");
    if (sendSas == 0) {
      throw Exception(_T("The SendSAS function has not been found"));
    }
    sendSas(FALSE); 
  } catch (Exception &e) {
    Log::error(_T("The simulateCtrlAltDelUnderVista() function failed: %s"),
               e.getMessage());
  }
}
