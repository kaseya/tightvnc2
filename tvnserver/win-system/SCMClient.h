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

#ifndef _SCM_CLIENT_H_
#define _SCM_CLIENT_H_

#include "util/CommonHeader.h"
#include "util/Exception.h"

#include "SystemException.h"

class SCMClientException : public SystemException {
public:
  const static int ERROR_ALREADY_STOPPED = 0;
  const static int ERROR_STOP_TIMEOUT = 1;
  const static int ERROR_ALREADY_RUNNING = 2;
  const static int ERROR_START_TIMEOUT = 3;
public:
  SCMClientException(int scmErrCode);
  int getSCMErrorCode() const;
private:
  int m_scmErrCode;
};

class SCMClient
{
public:
  SCMClient(DWORD desiredAccess = SC_MANAGER_ALL_ACCESS) throw(SystemException);
  virtual ~SCMClient();
  void installService(const TCHAR *name, const TCHAR *nameToDisplay,
                      const TCHAR *binPath, const TCHAR *dependencies = _T("")) throw(SystemException);
  void removeService(const TCHAR *name) throw(SystemException);
  void startService(const TCHAR *name, bool waitCompletion = false)
    throw(SystemException, SCMClientException);
  void stopService(const TCHAR *name, bool waitCompletion = false)
    throw(SystemException, SCMClientException);

private:
  DWORD getServiceState(SC_HANDLE hService) const throw(SystemException);

private:
  SC_HANDLE m_managerHandle;
};

#endif
