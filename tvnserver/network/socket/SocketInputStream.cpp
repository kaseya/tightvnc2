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

#include "SocketInputStream.h"

SocketInputStream::SocketInputStream(SOCKET sock)
: m_sock(sock)
{
}

SocketInputStream::~SocketInputStream()
{
}

size_t SocketInputStream::read(void *buffer, size_t len)
{
  int read = ::recv(m_sock, (char *)buffer, len, 0);

  if (read == SOCKET_ERROR) {
    throw IOException(_T("Cannot read from socket"));
  }

  if (read == 0) {
    throw IOException(_T("Connection refused"));
  }

  if (read < 0) {
    throw IOException(_T("::recv function return negative value"));
  }

  return (size_t)read;
}
