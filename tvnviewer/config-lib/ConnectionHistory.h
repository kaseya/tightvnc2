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

#ifndef _CONNECTION_HISTORY_H_
#define _CONNECTION_HISTORY_H_

#include "win-system/RegistryKey.h"

#include <vector>

using namespace std;

class ConnectionHistory
{
public:
  ConnectionHistory(RegistryKey *key, size_t limit);
  virtual ~ConnectionHistory();

  void setLimit(size_t limit);

  size_t getLimit() const;

  void load();

  void save();

  void truncate();

  void clear();

  void addHost(const TCHAR *host);

  size_t getHostCount() const;

  const TCHAR *getHost(size_t i) const;

protected:
  void releaseHosts();
  void removeHost(const TCHAR *host);

protected:
  vector<StringStorage> m_hosts;

  size_t m_limit;

  RegistryKey *m_key;
};

#endif
