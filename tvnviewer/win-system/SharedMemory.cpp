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

#include "SharedMemory.h"
#include "util/Exception.h"
#include <Aclapi.h>

SharedMemory::SharedMemory(const TCHAR *name, size_t size)
: m_hToMap(0),
  m_memory(0)
{
  try {
    bool needToInit = createFile(name, size);
    mapViewOfFile();
    if (needToInit) {
      memset(m_memory, 0, size);
    }
  } catch (...) {
    freeRes();
    throw;
  }
}

SharedMemory::~SharedMemory()
{
  freeRes();
}

void SharedMemory::freeRes()
{
  if (m_memory) {
    UnmapViewOfFile(m_memory);
    m_memory = 0;
  }
  if (m_hToMap) {
    CloseHandle(m_hToMap);
    m_hToMap = 0;
  }
}

bool SharedMemory::createFile(const TCHAR *name, size_t size)
{
  m_hToMap = CreateFileMapping(INVALID_HANDLE_VALUE,  
                               0,                     
                               PAGE_READWRITE,        
                               0,                     
                               size,                  
                               name);                 
  if (m_hToMap == NULL) {
    int errCode = GetLastError();
    StringStorage errMess;
    errMess.format(_T("Cannot create file mapping with error = %d"), errCode);
    throw Exception(errMess.getString());
  }
  bool needToInit = GetLastError() != ERROR_ALREADY_EXISTS;

  if (needToInit) {
    setAllAccess(m_hToMap);
  }

  return needToInit;
}

void SharedMemory::mapViewOfFile()
{
  m_memory = MapViewOfFile(m_hToMap,       
                           FILE_MAP_WRITE, 
                           0,              
                           0,              
                           0);             
  if (m_memory == NULL) {
    int errCode = GetLastError();
    StringStorage errMess;
    errMess.format(_T("Cannot map view of file with error = %d"), errCode);
    throw Exception(errMess.getString());
  }
}

void SharedMemory::setAllAccess(HANDLE objHandle)
{
  DWORD errorCode = SetSecurityInfo(objHandle, SE_FILE_OBJECT,
                                    DACL_SECURITY_INFORMATION, 
                                    0,
                                    0,
                                    0, 
                                    0);
  if (errorCode != ERROR_SUCCESS) {
    StringStorage errMess;
    errMess.format(_T("Cannot SetSecurityInfo with error = %d"), (int)errorCode);
    throw Exception(errMess.getString());
  }
}

