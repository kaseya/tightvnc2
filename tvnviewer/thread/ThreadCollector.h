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

#ifndef __THREADCOLLECTOR_H__
#define __THREADCOLLECTOR_H__

#include "Thread.h"
#include "LocalMutex.h"
#include "win-system/WindowsEvent.h"
#include <list>

using namespace std;

typedef list<Thread *> ThreadList;

class ThreadCollector : private Thread
{
public:
  ThreadCollector();
  virtual ~ThreadCollector();

  void addThread(Thread *thread);

  void destroyAllThreads();

protected:
  virtual void execute();

  void deleteDeadThreads();

protected:
  ThreadList m_threads;
  LocalMutex m_lockObj;

  WindowsEvent m_timer;
};

#endif 
