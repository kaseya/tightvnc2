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

#include "ByteArrayOutputStream.h"

#include <string.h>

ByteArrayOutputStream::ByteArrayOutputStream(size_t max)
: m_size(0), m_max(max), m_ownMemory(true)
{
  m_buffer = new char[m_max];
}

ByteArrayOutputStream::ByteArrayOutputStream()
: m_size(0), m_buffer(0), m_max(DEFAULT_INNER_BUFFER_CAPACITY), m_ownMemory(true)
{
  m_buffer = new char[m_max];
}

ByteArrayOutputStream::ByteArrayOutputStream(void *alienMemory)
: m_size(0), m_buffer((char *)alienMemory), m_max(0xFFFFFF), m_ownMemory(false)
{
}

ByteArrayOutputStream::~ByteArrayOutputStream()
{
  if ((m_buffer != 0) && (m_ownMemory)) {
    delete[] m_buffer;
  }
}

size_t ByteArrayOutputStream::write(const void *buffer, size_t len)
{
  bool allocateNewBuffer = (m_size + len) > m_max;

  if (allocateNewBuffer && m_ownMemory) {
    size_t reserve = DEFAULT_INNER_BUFFER_CAPACITY;
    char *newBuffer = new char[m_size + len + reserve];
    memcpy(newBuffer, m_buffer, m_size);
    delete[] m_buffer;
    m_buffer = newBuffer;
    m_max = m_size + len + reserve;
  }

  memcpy(&m_buffer[m_size], (const char *)buffer, len);
  m_size += len;

  return len;
}

size_t ByteArrayOutputStream::size() const
{
  return m_size;
}

const char *ByteArrayOutputStream::toByteArray() const
{
  return m_buffer;
}
