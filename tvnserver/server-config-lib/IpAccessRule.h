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

#ifndef _IP_ACCESS_RULE_H_
#define _IP_ACCESS_RULE_H_

#include "util/StringStorage.h"
#include "thread/AutoLock.h"
#include "thread/LocalMutex.h"

class IpAccessRule : public Lockable
{
public:

  enum ActionType {
    ACTION_TYPE_ALLOW = 0,
    ACTION_TYPE_DENY  = 1,
    ACTION_TYPE_QUERY = 2
  };

public:
  IpAccessRule();
  virtual ~IpAccessRule();
public:

  virtual void lock() {
    m_objectCS.lock();
  }

  virtual void unlock() {
    m_objectCS.unlock();
  }

  void toString(StringStorage *output) const;

  static bool parse(const TCHAR *string, IpAccessRule *rule);
  static bool parseIp(const TCHAR *string, IpAccessRule *rule);
  static bool parseIpRange(const TCHAR *string, IpAccessRule *rule);
  static bool parseSubnet(const TCHAR *string, IpAccessRule *rule);

  ActionType getAction() const {
    return m_action;
  }

  void setAction(ActionType value) {
    m_action = value;
  }

  void getFirstIp(StringStorage *firstIp) const;
  void getLastIp(StringStorage *lastIp) const;

  void setFirstIp(const TCHAR *firstIp);
  void setLastIp(const TCHAR *lastIp);

  bool isEqualTo(IpAccessRule *other) const;

  bool isIncludingAddress(unsigned long ip) const;
  static bool isIpAddressStringValid(const TCHAR *string);

  static int compareIp(unsigned long ip1, unsigned long ip2);

protected:
  static bool tryParseIPPart(const TCHAR *string);
  static void getIpRange(const TCHAR *ip, const TCHAR *netmask,
                         StringStorage *firstIp, StringStorage *lastIp);
protected:
  ActionType m_action;

  StringStorage m_firstIp;
  StringStorage m_lastIp;

  LocalMutex m_objectCS;
};

#endif
