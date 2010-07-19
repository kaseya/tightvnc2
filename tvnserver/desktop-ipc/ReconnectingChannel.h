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

#ifndef __RECONNECTINGCHANNEL_H__
#define __RECONNECTINGCHANNEL_H__

#include "io-lib/Channel.h"
#include "thread/LocalMutex.h"
#include "win-system/WindowsEvent.h"

class ReconnectingChannel : public Channel
{
public:
  ReconnectingChannel(unsigned int timeOut);
  virtual ~ReconnectingChannel();

  virtual size_t read(void *buffer, size_t len);
  virtual size_t write(const void *buffer, size_t len);

  void replaceChannel(Channel *newChannel);

  virtual void close();

private:
  Channel *getChannel(const TCHAR *funName);

  void waitForReconnect(const TCHAR *funName, Channel *channel);

  bool m_isClosed;

  Channel *m_channel;
  Channel *m_oldChannel;
  bool m_chanWasChanged;
  LocalMutex m_chanMut;

  WindowsEvent m_timer;
  unsigned int m_timeOut;
};

#endif 
