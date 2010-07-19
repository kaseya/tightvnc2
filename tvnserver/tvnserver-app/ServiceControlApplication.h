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

#ifndef _SERVICE_CONTROL_APPLICATION_H_
#define _SERVICE_CONTROL_APPLICATION_H_

#include "util/CommonHeader.h"

#include "win-system/WindowsApplication.h"
#include "win-system/SCMClient.h"

#include "TvnService.h"
#include "ServiceControlCommandLine.h"

class ServiceControlApplication : public WindowsApplication
{
public:
  ServiceControlApplication(HINSTANCE hInstance, const TCHAR *commandLine);
  virtual ~ServiceControlApplication();

  virtual int run();

private:
  void runElevatedInstance() const throw(SystemException);
  void executeCommand(const ServiceControlCommandLine *cmdLine) const
    throw(SystemException);
  void setTvnControlStartEntry() const throw(SystemException);
  void removeTvnControlStartEntry() const throw(SystemException);
  void reportError(const ServiceControlCommandLine *cmdLine,
                   const SCMClientException *ex) const;
  void reportError(const ServiceControlCommandLine *cmdLine,
                   const SystemException *ex) const;
  void reportError(const ServiceControlCommandLine *cmdLine,
                   const TCHAR *errorMessage) const;
  void reportSuccess(const ServiceControlCommandLine *cmdLine) const;

private:
  StringStorage m_commandLine;
};

#endif
