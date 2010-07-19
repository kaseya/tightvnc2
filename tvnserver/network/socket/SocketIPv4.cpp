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

#include <stdlib.h>
#include "SocketAddressIPv4.h"
#include "SocketAddressIPv4.h"
#include "SocketIPv4.h"

#include "thread/AutoLock.h"

#include <crtdbg.h>

SocketIPv4::SocketIPv4()
: m_localAddr(NULL), m_peerAddr(NULL), m_isBound(false)
{
  m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (m_socket == INVALID_SOCKET) {
    throw SocketException();
  }
}

SocketIPv4::~SocketIPv4()
{
  try { close(); } catch (...) { }
}

void SocketIPv4::connect(const TCHAR *host, unsigned short port)
{
  SocketAddressIPv4 address(host, port);

  connect(address);
}

void SocketIPv4::connect(const SocketAddressIPv4 &addr)
{
  struct sockaddr_in targetSockAddr = addr.getSockAddr();

  if (::connect(m_socket, (const sockaddr *)&targetSockAddr, addr.getAddrLen()) == SOCKET_ERROR) {
    throw SocketException();
  }

  AutoLock l(&m_mutex);

  if (m_peerAddr) {
    delete m_peerAddr;
  }

  m_peerAddr = new SocketAddressIPv4(*(struct sockaddr_in *)&targetSockAddr);

  m_isBound = false;
}

void SocketIPv4::close()
{
  int result;
  
#ifdef _WIN32
  result = ::closesocket(m_socket);
#else
  result = ::close(m_socket);
#endif

  m_socket = INVALID_SOCKET;

  AutoLock l(&m_mutex);

  if (m_peerAddr) {
    delete m_peerAddr;
  }

  if (m_localAddr) {
    delete m_localAddr;
  }

  m_peerAddr = m_localAddr = NULL;

  if (result == SOCKET_ERROR) {
    throw SocketException();
  }
}

void SocketIPv4::shutdown(int how)
{
  if (::shutdown(m_socket, how) == SOCKET_ERROR) {
    throw SocketException();
  }
}

void SocketIPv4::bind(const TCHAR *bindHost, unsigned int bindPort)
{
  SocketAddressIPv4 address(bindHost, bindPort);

  bind(address);
}

void SocketIPv4::bind(const SocketAddressIPv4 &addr)
{
  struct sockaddr_in bindSockaddr = addr.getSockAddr();

  if (::bind(m_socket, (const sockaddr *)&bindSockaddr, addr.getAddrLen()) == SOCKET_ERROR) {
    throw SocketException();
  }

  AutoLock l(&m_mutex);

  if (m_localAddr) {
    delete m_localAddr;
  }

  m_localAddr = new SocketAddressIPv4(*(struct sockaddr_in*)&bindSockaddr);

  m_isBound = true;
}

bool SocketIPv4::isBound()
{
  AutoLock l(&m_mutex);

  return m_isBound;
}

void SocketIPv4::listen(int backlog)
{
  if (::listen(m_socket, backlog) == SOCKET_ERROR) {
    throw SocketException();
  }
}

SocketIPv4 *SocketIPv4::accept()
{
  SOCKET result;

  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(struct sockaddr_in);

  SocketIPv4 *accepted;

  fd_set afd;

  timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 200;

  while (true) {
    FD_ZERO(&afd);
    FD_SET(m_socket, &afd);

    int ret = select(m_socket + 1, &afd, NULL, NULL, &timeout);

    if (ret == SOCKET_ERROR) {
      throw SocketException();
    } else if (ret == 0) {
      continue;
    } else if (ret > 0) {
      if (FD_ISSET(m_socket, &afd)) {
        result = ::accept(m_socket, (struct sockaddr*)&addr, &addrlen);
        if (result == INVALID_SOCKET) {
          throw SocketException();
        }
        break;
      } 
    } 
  } 

  try {
    accepted = new SocketIPv4(); 
    accepted->close();
  } catch(...) {
#ifdef _WIN32
    ::closesocket(result);
#else
    ::close(result);
#endif
    throw SocketException();
  }

  AutoLock l(&accepted->m_mutex);

  accepted->m_socket = result;

  accepted->m_peerAddr = new SocketAddressIPv4(*(struct sockaddr_in *)&addr);

  AutoLock l2(&m_mutex);

  if (m_localAddr) {
    sockaddr_in tsa = m_localAddr->getSockAddr();

    accepted->m_localAddr = new SocketAddressIPv4(*(struct sockaddr_in*)&tsa);
  }

  return accepted; 
}

int SocketIPv4::send(const char *data, int size, int flags)
{
  int result;
  
  result = ::send(m_socket, data, size, flags);

  if (result == -1) {
    throw IOException(_T("Failed to send data to socket."));
  }
  
  return result;
}

int SocketIPv4::recv(char *buffer, int size, int flags)
{
  int result;

  result = ::recv(m_socket, buffer, size, flags);

  if (result == 0) {
    throw IOException(_T("Connection has been gracefully closed"));
  }

  if (result == SOCKET_ERROR) {
    throw IOException(_T("Failed to recv data from socket."));
  }

  return result;
}

bool SocketIPv4::getLocalAddr(SocketAddressIPv4 *addr)
{
  AutoLock l(&m_mutex);

  if (m_localAddr == 0) {
    return false;
  }

  *addr = *m_localAddr;

  return true;
}

bool SocketIPv4::getPeerAddr(SocketAddressIPv4 *addr)
{
  AutoLock l(&m_mutex);

  if (m_peerAddr == 0) {
    return false;
  }

  *addr = *m_peerAddr;

  return true;
}

void SocketIPv4::setSocketOptions(int level, int name, void *value, socklen_t len)
{
  if (setsockopt(m_socket, level, name, (char*)value, len) == SOCKET_ERROR) {
    throw SocketException();
  }
}

void SocketIPv4::getSocketOptions(int level, int name, void *value, socklen_t *len)
{
  if (getsockopt(m_socket, level, name, (char*)value, len) == SOCKET_ERROR) {
    throw SocketException();
  }
}

void SocketIPv4::enableNaggleAlgorithm(bool enabled)
{
  BOOL disabled = enabled ? 0 : 1;

  setSocketOptions(IPPROTO_TCP, TCP_NODELAY, &disabled, sizeof(disabled));
}

void SocketIPv4::setExclusiveAddrUse()
{
  int val = 1;

  setSocketOptions(SOL_SOCKET, SO_EXCLUSIVEADDRUSE, &val, sizeof(val));
}
