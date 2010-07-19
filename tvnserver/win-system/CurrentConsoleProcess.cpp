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

#include "CurrentConsoleProcess.h"

#include "win-system/WinStaLibrary.h"
#include "win-system/Environment.h"
#include "win-system/SystemException.h"
#include "win-system/Workstation.h"
#include "win-system/WTS.h"

#include "util/Log.h"

CurrentConsoleProcess::CurrentConsoleProcess(const TCHAR *path, const TCHAR *args, bool useXpTrick)
 : Process(path, args), m_useXpTrick(useXpTrick)
{
}

CurrentConsoleProcess::~CurrentConsoleProcess()
{
}

void CurrentConsoleProcess::start()
{
  int pipeNotConnectedErrorCount = 0;

  for (int i = 0; i < 5; i++) {
    try {
      startImpersonated();
      return ;
    } catch (SystemException &sysEx) {
      if (!m_useXpTrick) {
        throw;
      }
      if (sysEx.getErrorCode() == 233) {
        pipeNotConnectedErrorCount++;

        DWORD sessionId = WTS::getActiveConsoleSessionId();

        bool isXPFamily = Environment::isWinXP() || Environment::isWin2003Server();
        bool doXPTrick = (isXPFamily) && (sessionId > 0) && (pipeNotConnectedErrorCount >= 3);

        if (doXPTrick) {
          CurrentConsoleProcess::doXPTrick();
          startImpersonated();
          return ;
        } 
      } else {
        throw;
      } 
    }
    Sleep(3000);
  } 
}

void CurrentConsoleProcess::startImpersonated()
{
  cleanup();

  DWORD sessionId = WTS::getActiveConsoleSessionId();
  Log::info(_T("Try to start \"%s %s\" process as current user at %d session"),
            m_path.getString(),
            m_args.getString(),
            sessionId);

  PROCESS_INFORMATION pi;
  STARTUPINFO sti;
  getStartupInfo(&sti);

  HANDLE procHandle = GetCurrentProcess();

  HANDLE token, userToken;

  try {
    if (OpenProcessToken(procHandle, TOKEN_DUPLICATE, &token) == 0) {
      throw SystemException();
    }

    if (DuplicateTokenEx(token,
      MAXIMUM_ALLOWED,
      0,
      SecurityImpersonation,
      TokenPrimary,
      &userToken) == 0) {
        throw SystemException();
    }

    if (SetTokenInformation(userToken,
      (TOKEN_INFORMATION_CLASS) TokenSessionId,
      &sessionId,
      sizeof(sessionId)) == 0) {
        throw SystemException();
    }

    StringStorage commandLine = getCommandLineString();

    if (CreateProcessAsUser(userToken, 0, (LPTSTR) commandLine.getString(),
      0, 0, m_handlesIsInherited, NORMAL_PRIORITY_CLASS, 0, 0, &sti,
      &pi) == 0) {
        throw SystemException();
    }
    Log::info(_T("Created \"%s\" process at %d windows session"),
              commandLine.getString(), sessionId);
  } catch (SystemException &sysEx) {
    Log::error(_T("Failed to start process with %d error"), sysEx.getErrorCode());
    throw;
  }

  CloseHandle(userToken);
  CloseHandle(token);

  m_hThread = pi.hThread;
  m_hProcess = pi.hProcess;
}

void CurrentConsoleProcess::doXPTrick()
{
  Log::info(_T("Trying to do WindowsXP trick to start process on separate session"));

  try {
    WinStaLibrary winSta;

    WCHAR password[1];
    memset(password, 0, sizeof(password));

    if (winSta.WinStationConnectW(NULL, 0, WTS::getActiveConsoleSessionId(),
      password, 0) == FALSE) {
      throw SystemException(_T("Failed to call WinStationConnectW"));
    }

    StringStorage pathToBinary;
    Environment::getCurrentModulePath(&pathToBinary);

    CurrentConsoleProcess lockWorkstation(pathToBinary.getString(),
      _T("-lockworkstation"),
      false);
    lockWorkstation.start();
    lockWorkstation.waitForExit();

    DWORD exitCode = lockWorkstation.getExitCode();

    if (exitCode != 0) {
      throw SystemException(exitCode);
    }
  } catch (SystemException &ex) {
    Log::error(ex.getMessage());
    throw;
  }
}
