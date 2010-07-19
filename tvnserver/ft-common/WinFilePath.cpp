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

#include "WinFilePath.h"

WinFilePath::WinFilePath()
: m_parentPathIsRoot(false)
{
  StringStorage::StringStorage();
}

WinFilePath::WinFilePath(const TCHAR *string)
: m_parentPathIsRoot(false)
{
  StringStorage::StringStorage(string);
}

WinFilePath::WinFilePath(const StringStorage &stringBuffer)
: m_parentPathIsRoot(false)
{
  StringStorage::StringStorage(stringBuffer);
}

WinFilePath::~WinFilePath()
{
}

bool WinFilePath::parentPathIsRoot()
{
  return m_parentPathIsRoot;
}

void WinFilePath::setString(const TCHAR *string)
{
  StringStorage::setString(string);
  if (!isEmpty()) {
    if (findLast('/') == 0) {
      m_parentPathIsRoot = true;
    }
    convertToWindowsPath();
  }
}

void WinFilePath::convertToWindowsPath()
{
  size_t i = 0;
  size_t j = 0;
  TCHAR *newBuffer = new TCHAR[m_length + 1];
  for (i = 0; i < m_length; i++) {
    TCHAR c = m_buffer[i];
    if (c == '/' && ((i == 0) || (i == m_length - 1))) {
      continue;
    }
    if (c == '/') {
      c = '\\';
    }
    newBuffer[j++] = c;
  }
  newBuffer[j] = '\0';

  delete[] m_buffer;

  m_buffer = newBuffer;
  m_length = _tcslen(newBuffer);
}
