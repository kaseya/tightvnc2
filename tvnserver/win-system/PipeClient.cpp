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

#include "PipeClient.h"
#include "util/Exception.h"

PipeClient::PipeClient(const TCHAR *pipeName, bool needToConnect)
{
  if (needToConnect) {
    openConnection();
  }
}

PipeClient::PipeClient()
{
}

Pipe *PipeClient::connect(const TCHAR *name)
{
  StringStorage pipeName;
  pipeName.format(_T("\\\\.\\pipe\\%s"), name);

  HANDLE hPipe;
  hPipe = CreateFile(pipeName.getString(),  
                     GENERIC_READ |         
                     GENERIC_WRITE,
                     0,                     
                     NULL,                  
                     OPEN_EXISTING,         
                     0,                     
                     NULL);                 

  if (hPipe == INVALID_HANDLE_VALUE) {
    int errCode = GetLastError();
    StringStorage errMess;
    errMess.format(_T("Connect to pipe server failed, error code = %d"), errCode);
    throw Exception(errMess.getString());
  }

  DWORD dwMode = PIPE_READMODE_BYTE;
  if (!SetNamedPipeHandleState(hPipe,   
                               &dwMode,   
                               NULL,      
                               NULL)      
                               ) {
    int errCode = GetLastError();
    StringStorage errMess;
    errMess.format(_T("SetNamedPipeHandleState failed, error code = %d"), errCode);
    throw Exception(errMess.getString());
  }

  return new Pipe(hPipe, false);
}

void PipeClient::breakConnect()
{
}

PipeClient::~PipeClient()
{
  closeConnection();
}

void PipeClient::openConnection()
{
}

void PipeClient::closeConnection()
{
}
