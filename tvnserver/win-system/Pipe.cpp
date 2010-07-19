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

#include "Pipe.h"
#include "util/Exception.h"
#include <crtdbg.h>
#include "win-system/Environment.h"

#define MAX_PORTION_SIZE 512 * 1024

Pipe::Pipe(HANDLE hPipe, bool asServer)
: m_hPipe(hPipe),
  m_asServer(asServer)
{
}

Pipe::~Pipe(void)
{
  try {
    close();
  } catch (...) {
  }
}

void Pipe::close()
{
  if (m_asServer) {
    if (DisconnectNamedPipe(m_hPipe) == 0) {
      int errCode = GetLastError();
      StringStorage errMess;
      errMess.format(_T("DisconnectNamedPipe failed, error code = %d"), errCode);
      throw Exception(errMess.getString());
    }
  }

  if (m_hPipe) {
    CloseHandle(m_hPipe);
    m_hPipe = 0;
  }
  m_winEvent.notify();
}

size_t Pipe::write(const void *buffer, size_t len)
{
  DWORD result;
  OVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(OVERLAPPED));
  overlapped.hEvent = m_winEvent.getHandle();

  bool success = WriteFile(m_hPipe, 
                           buffer,    
                           len,  
                           &result, 
                           &overlapped)    
                           != 0;

  if (!success) {
    int errCode = GetLastError();

    if (errCode == ERROR_IO_PENDING) {
      m_winEvent.waitForEvent();
      DWORD cbRet;
      if (GetOverlappedResult(m_hPipe, &overlapped, &cbRet, FALSE) ||
          cbRet == 0) {
        result = cbRet;
      } else {
        StringStorage errMess;
        Environment::getErrStr(_T("The Pipe's write function failed")
                               _T(" after GetOverlappedResult calling"),
                               &errMess);
        throw IOException(errMess.getString());
      }
    } else {
      StringStorage errMess;
      Environment::getErrStr(_T("The Pipe's write function failed")
                             _T(" after WriteFile calling"), &errMess);
      throw IOException(errMess.getString());
    }
  } 

  if (result == 0) {
    throw IOException(_T("Unknown pipe error"));
  }
  return result;
}

size_t Pipe::read(void *buffer, size_t len)
{
  DWORD result = 0;
  OVERLAPPED overlapped;
  memset(&overlapped, 0, sizeof(OVERLAPPED));
  overlapped.hEvent = m_winEvent.getHandle();

  bool success = ReadFile(m_hPipe,         
                          buffer,            
                          len,          
                          &result,         
                          &overlapped)   
                          != 0;

  if (!success) {
    DWORD errCode = GetLastError();

    if (errCode == ERROR_IO_PENDING) {
      m_winEvent.waitForEvent();
      DWORD cbRet = 0;
      if (GetOverlappedResult(m_hPipe, &overlapped, &cbRet, FALSE) &&
          cbRet != 0) {
        result = cbRet;
      } else {
        StringStorage errMess;
        Environment::getErrStr(_T("The Pipe's read function failed")
                               _T(" after GetOverlappedResult calling"),
                               &errMess);
        throw IOException(errMess.getString());
      }
    } else {
      StringStorage errMess;
      Environment::getErrStr(_T("The Pipe's read function failed")
                             _T(" after WriteFile calling"), &errMess);
      throw IOException(errMess.getString());
    }
  } 

  if (result == 0) {
    throw IOException(_T("Unknown pipe error"));
  }
  return result;
}
