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

#include "CharString.h"

#include <crtdbg.h>

CharString::CharString(const char *string)
{
  _ASSERT(string != NULL);

  m_buffer.setBytes(string, strlen(string) + 1);
}

CharString::CharString()
{
  m_buffer.setBytes("", 1);
}

CharString::~CharString()
{
}

const char *CharString::getString() const
{
  return m_buffer.getBytes();
}

size_t CharString::getLength() const
{
  return strlen(getString());
}

void CharString::format(const char *format, ...)
{
  va_list vl;

  va_start(vl, format);
  int size = _vscprintf(format, vl) + 1;
  va_end(vl);

  char *buffer = new char[size];

  va_start(vl, format);
  vsprintf_s(buffer, size, format, vl);
  va_end(vl);

  m_buffer.setBytes(buffer, size);

  delete[] buffer;
}