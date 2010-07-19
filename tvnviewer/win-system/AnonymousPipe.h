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

#ifndef __ANONYMOUSPIPE_H__
#define __ANONYMOUSPIPE_H__

#include "io-lib/Channel.h"
#include "util/CommonHeader.h"

class AnonymousPipe : public Channel
{
public:
  AnonymousPipe(HANDLE hWrite, HANDLE hRead);
  virtual ~AnonymousPipe();

  void close();

  virtual size_t read(void *buffer, size_t len) throw(IOException);

  virtual size_t write(const void *buffer, size_t len) throw(IOException);

  HANDLE getWriteHandle() const;

  HANDLE getReadHandle() const;

  void assignHandlesFor(HANDLE hTargetProc, bool neededToClose);

private:
  HANDLE m_hWrite;
  HANDLE m_hRead;
  bool m_neededToClose;
};

#endif 
