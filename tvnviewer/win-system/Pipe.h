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

#ifndef __PIPE_H__
#define __PIPE_H__

#include "util/CommonHeader.h"
#include "WindowsEvent.h"
#include "io-lib/Channel.h"

class Pipe : public Channel
{
public:
  Pipe(HANDLE hPipe, bool asServer);
  virtual ~Pipe();

  void close();

  virtual size_t read(void *buffer, size_t len) throw(IOException);
  virtual size_t write(const void *buffer, size_t len) throw(IOException);

protected:

  HANDLE m_hPipe;
  StringStorage m_pipeName;

  WindowsEvent m_winEvent;
  bool m_asServer;
};

#endif 
