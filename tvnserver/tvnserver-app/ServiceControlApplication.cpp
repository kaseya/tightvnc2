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

#include "ServiceControlApplication.h"
#include "ServiceControlCommandLine.h"

#include "util/ResourceLoader.h"
#include "util/StringTable.h"

#include "tvnserver/resource.h"

#include "WsConfigRunner.h"
#include "TvnServerHelp.h"

#include "tvncontrol-app/ControlCommandLine.h"

#include "win-system/Registry.h"
#include "win-system/RegistryKey.h"
#include "win-system/Environment.h"
#include "win-system/Shell.h"
#include "win-system/SCMClient.h"

ServiceControlApplication::ServiceControlApplication(HINSTANCE hInstance, const TCHAR *commandLine)
: WindowsApplication(hInstance), m_commandLine(commandLine)
{
}

ServiceControlApplication::~ServiceControlApplication()
{
}

int ServiceControlApplication::run()
{
  const int RET_OK = 0;
  const int RET_ERR = 1;

  ServiceControlCommandLine cmdLine;
  try {
    cmdLine.parse(m_commandLine.getString());
  } catch (Exception &) {
    TvnServerHelp::showUsage();
    return RET_ERR;
  }

  bool success = false;

  if (!IsUserAnAdmin() && !cmdLine.beSilent() && !cmdLine.dontElevate()) {
    try {
      runElevatedInstance();
      success = true;
    } catch (SystemException &sysEx) {
      if (sysEx.getErrorCode() != ERROR_CANCELLED) {
        reportError(&cmdLine, sysEx.getMessage());
      }
    }
  } else {
    try {
      executeCommand(&cmdLine);
      success = true;
    } catch (SCMClientException &scmEx) {
      reportError(&cmdLine, &scmEx);
    } catch (SystemException &servEx) {
      reportError(&cmdLine, &servEx);
    } catch (Exception &ex) {
      _ASSERT(FALSE);
      reportError(&cmdLine, ex.getMessage());
    }
  }

  if (cmdLine.startRequested() && !cmdLine.dontElevate()) {
    try {
      WsConfigRunner tvncontrol(true);
    } catch (...) { }
  }

  return success ? RET_OK : RET_ERR;
}

void ServiceControlApplication::runElevatedInstance() const
{
  StringStorage executablePath;
  Environment::getCurrentModulePath(&executablePath);

  StringStorage commandLine;
  commandLine.format(_T("%s %s"), m_commandLine.getString(),
                     ServiceControlCommandLine::DONT_ELEVATE);

  Shell::runAsAdmin(executablePath.getString(), commandLine.getString());
}

void ServiceControlApplication::executeCommand(const ServiceControlCommandLine *cmdLine) const
{
  if (cmdLine->installationRequested()) {
    TvnService::install();
    setTvnControlStartEntry();
    reportSuccess(cmdLine);
  } else if (cmdLine->removalRequested()) {
    TvnService::remove();
    removeTvnControlStartEntry();
    reportSuccess(cmdLine);
  } else if (cmdLine->reinstallRequested()) {
    TvnService::reinstall();
    setTvnControlStartEntry();
    reportSuccess(cmdLine);
  } else if (cmdLine->startRequested()) {
    TvnService::start(true);
  } else if (cmdLine->stopRequested()) {
    TvnService::stop(true);
  }
}

void ServiceControlApplication::setTvnControlStartEntry() const
{
  StringStorage currentModuleFolderPath;
  Environment::getCurrentModuleFolderPath(&currentModuleFolderPath);
  StringStorage pathToTvnControl;
  pathToTvnControl.format(_T("\"%s\\tvnserver.exe\" %s %s"),
                          currentModuleFolderPath.getString(),
                          ControlCommandLine::CONTROL_SERVICE,
                          ControlCommandLine::SLAVE_MODE);

  RegistryKey runKey(Registry::getCurrentLocalMachineKey(),
                     _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
                     false);
  runKey.setValueAsString(_T("tvncontrol"), pathToTvnControl.getString());
}

void ServiceControlApplication::removeTvnControlStartEntry() const
{
  RegistryKey runKey(Registry::getCurrentLocalMachineKey(),
                     _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
                     false);
  runKey.deleteValue(_T("tvncontrol"));
}

void ServiceControlApplication::reportError(const ServiceControlCommandLine *cmdLine,
                                            const SCMClientException *ex) const
{
  StringStorage errorMessage;

  switch (ex->getSCMErrorCode()) {
  case SCMClientException::ERROR_ALREADY_RUNNING:
    errorMessage.setString(StringTable::getString(IDS_SERVICE_ALREADY_RUNNING));
    break;
  case SCMClientException::ERROR_ALREADY_STOPPED:
    errorMessage.setString(StringTable::getString(IDS_SERVICE_ALREADY_STOPPED));
    break;
  case SCMClientException::ERROR_START_TIMEOUT:
    errorMessage.setString(StringTable::getString(IDS_SERVICE_START_TIMEOUT));
    break;
  case SCMClientException::ERROR_STOP_TIMEOUT:
    errorMessage.setString(StringTable::getString(IDS_SERVICE_STOP_TIMEOUT));
    break;
  default:
    errorMessage.setString(ex->getMessage());
  }

  reportError(cmdLine, errorMessage.getString());
}

void ServiceControlApplication::reportError(const ServiceControlCommandLine *cmdLine,
                                            const SystemException *ex) const
{
  StringStorage errorMessage;

  switch (ex->getErrorCode()) {
  case ERROR_SERVICE_DOES_NOT_EXIST:
    errorMessage.setString(StringTable::getString(IDS_1060_ERROR_DESCRIPTION));
    break;
  case ERROR_SERVICE_EXISTS:
    errorMessage.setString(StringTable::getString(IDS_1073_ERROR_DESCRIPTION));
    break;
  default:
    errorMessage.setString(ex->getMessage());
  }

  reportError(cmdLine, errorMessage.getString());
}

void ServiceControlApplication::reportError(const ServiceControlCommandLine *cmdLine,
                                            const TCHAR *errorMessage) const
{
  UINT stringId = 0;

  if (cmdLine->installationRequested()) {
    stringId = IDS_FAILED_TO_INSTALL_SERVICE_FORMAT;
  } else if (cmdLine->removalRequested()) {
    stringId = IDS_FAILED_TO_REMOVE_SERVICE_FORMAT;
  } else if (cmdLine->reinstallRequested()) {
    stringId = IDS_FAILED_TO_INSTALL_SERVICE_FORMAT;
  } else if (cmdLine->startRequested()) {
    stringId = IDS_FAILED_TO_START_SERVICE_FORMAT;
  } else if (cmdLine->stopRequested()) {
    stringId = IDS_FAILED_TO_STOP_SERVICE_FORMAT;
  } else {
    _ASSERT(FALSE);
    return;
  }

  if (!cmdLine->beSilent()) {
    const TCHAR *caption = StringTable::getString(IDS_MBC_TVNSERVER);
    StringStorage text;
    text.format(StringTable::getString(stringId), errorMessage);
    MessageBox(NULL, text.getString(), caption, MB_OK | MB_ICONERROR);
  }
}

void ServiceControlApplication::reportSuccess(const ServiceControlCommandLine *cmdLine) const
{
  UINT stringId = 0;

  if (cmdLine->installationRequested()) {
    stringId = IDS_SERVICE_INSTALLED;
  } else if (cmdLine->removalRequested()) {
    stringId = IDS_SERVICE_REMOVED;
  } else if (cmdLine->reinstallRequested()) {
    stringId = IDS_SERVICE_REINSTALLED;
  } else {
    _ASSERT(FALSE);
    return;
  }

  if (!cmdLine->beSilent()) {
    const TCHAR *caption = StringTable::getString(IDS_MBC_TVNSERVER);
    const TCHAR *text = StringTable::getString(stringId);
    MessageBox(NULL, text, caption, MB_OK | MB_ICONINFORMATION);
  }
}
