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
#ifndef _LISTENER_CONTAINER_H_
#define _LISTENER_CONTAINER_H_

#include <vector>

#include "thread/LocalMutex.h"
#include "thread/AutoLock.h"

using namespace std;

template<class T> class SafeVector : public vector<T>,
                                     public LocalMutex {
public:
  virtual ~SafeVector() { };
};

template<class T> class ListenerContainer {
public:

  void addListener(T listener) {
    AutoLock l(&m_listeners);

    vector<T>::iterator it;
    for (it = m_listeners.begin(); it != m_listeners.end(); it++) {
      T current = *it;
      if (current == listener) {
        return ;
      } 
    } 
    m_listeners.push_back(listener);
  } 

  void removeAllListeners() {
    AutoLock l(&m_listeners);

    m_listeners.clear();
  }

  void removeListener(T listener) {
    AutoLock l(&m_listeners);

    vector<T>::iterator it;
    for (it = m_listeners.begin(); it != m_listeners.end(); it++) {
      T current = *it;
      if (current == listener) {
        m_listeners.erase(it);
        return ;
      } 
    } 
  } 

protected:
  SafeVector<T> m_listeners;
};

#endif
