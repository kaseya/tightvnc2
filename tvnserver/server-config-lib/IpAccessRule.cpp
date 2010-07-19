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

#include "IpAccessRule.h"
#include "Configurator.h"
#include "util/CommonHeader.h"

IpAccessRule::IpAccessRule()
: m_action(ACTION_TYPE_DENY)
{
  m_firstIp.setString(_T(""));
  m_lastIp.setString(_T(""));
}

IpAccessRule::~IpAccessRule()
{
}

void IpAccessRule::toString(StringStorage *output) const
{
  if (!m_lastIp.isEmpty()) {
    output->format(_T("%s-%s:%d"), m_firstIp.getString(), m_lastIp.getString(),
                   (int)m_action);
  } else {
    output->format(_T("%s:%d"), m_firstIp.getString(), (int)m_action);
  }
}

bool IpAccessRule::parse(const TCHAR *string, IpAccessRule *rule)
{
  if (!parseIpRange(string, rule)) {
    if (!parseIp(string, rule)) {
      if (!parseSubnet(string, rule)) {
        return false;
      } 
    } 
  } 
  return true;
}

bool IpAccessRule::parseIp(const TCHAR *string, IpAccessRule *rule)
{
  TCHAR firstIp[16];
  int action;
  TCHAR c;

  if (_stscanf(string, _T("%15[0123456789.]:%d%c"), &firstIp, &action, &c) != 2) {
    return false;
  }

  if (!isIpAddressStringValid(&firstIp[0])) {
    return false;
  }

  if (rule != NULL) {
    rule->setAction((IpAccessRule::ActionType)action);
    rule->setFirstIp(&firstIp[0]);
    rule->setLastIp(_T(""));
  }

  return true;
}

bool IpAccessRule::parseIpRange(const TCHAR *string, IpAccessRule *rule)
{
  TCHAR firstIp[16];
  TCHAR lastIp[16];
  int action;
  TCHAR c;

  if (_stscanf(string, _T("%15[0123456789.]-%15[0123456789.]:%d%c"), &firstIp, &lastIp, &action, &c) != 3) {
    return false;
  }

  if (!isIpAddressStringValid(&firstIp[0]) ||
      !isIpAddressStringValid(&lastIp[0])) {
    return false;
  }

  if (rule != NULL) {
    rule->setAction((IpAccessRule::ActionType)action);
    rule->setFirstIp(&firstIp[0]);
    rule->setLastIp(&lastIp[0]);
  }

  return true;
}

bool IpAccessRule::parseSubnet(const TCHAR *string, IpAccessRule *rule)
{
  TCHAR pattern[16];
  TCHAR subnet[16];
  int action;
  TCHAR c;

  if (_stscanf(string, _T("%15[0123456789.]/%15[0123456789.]:%d%c"), &pattern, &subnet, &action, &c) != 3) {
    return false;
  }

  if (rule != NULL) {
    rule->setAction((IpAccessRule::ActionType)action);
    getIpRange(&pattern[0], &subnet[0], &rule->m_firstIp, &rule->m_lastIp);
  }

  return true;
}

void IpAccessRule::getFirstIp(StringStorage *firstIp) const
{
  *firstIp = m_firstIp;
}

void IpAccessRule::getLastIp(StringStorage *lastIp) const
{
  *lastIp = m_lastIp;
}

void IpAccessRule::setFirstIp(const TCHAR *firstIp)
{
  m_firstIp.setString(firstIp);
}

void IpAccessRule::setLastIp(const TCHAR *lastIp)
{
  m_lastIp.setString(lastIp);
}

bool IpAccessRule::isEqualTo(IpAccessRule *other) const
{
  if ((other->m_firstIp.isEqualTo(&m_firstIp) == 0) &&
      (other->m_lastIp.isEqualTo(&m_lastIp) == 0)) {
    return true;
  }
  return false;
}

bool IpAccessRule::isIncludingAddress(unsigned long ip) const
{
  size_t size1 = 100;
  size_t size2 = 100;

  char firstIpAnsi[100];
  char lastIpAnsi[100];

  m_firstIp.toAnsiString(&firstIpAnsi[0], size1);
  m_lastIp.toAnsiString(&lastIpAnsi[0], size2);

  unsigned long firstIp = inet_addr(&firstIpAnsi[0]);
  unsigned long lastIp  = firstIp;

  if (!m_lastIp.isEmpty()) {
    lastIp = inet_addr(&lastIpAnsi[0]);
  }

  if (ip == firstIp || ip == lastIp) {
    return true;
  }

  if (compareIp(ip, firstIp) == 1 && compareIp(ip, lastIp) == -1) {
    return true;
  }

  return false;
}

bool IpAccessRule::isIpAddressStringValid(const TCHAR *string)
{
  StringStorage ipStorage(string);
  StringStorage stringArray[4];
  size_t arraySize = 4;
  if (!ipStorage.split(_T("."), &stringArray[0], &arraySize)) {
    return false;
  }
  if (arraySize != 4) {
    return false;
  }
  for (size_t i = 0; i < arraySize; i++) {
    if (!tryParseIPPart(stringArray[i].getString())) {
      return false;
    } 
  } 
  return true;
}

int IpAccessRule::compareIp(unsigned long ip1, unsigned long ip2)
{
  if (ip1 == ip2) {
    return 0;
  }

  unsigned long l1 = htonl(ip1);
  unsigned long l2 = htonl(ip2);

  if (l1 > l2) {
    return 1;
  }

  return -1;
}

bool IpAccessRule::tryParseIPPart(const TCHAR *string)
{
  size_t len = _tcslen(string);
  if (len < 1 || _tcsspn(string, _T("0123456789")) != len)
    return false;	
  if (len > 4) 
    return false;
  int value = _ttoi(string);
  if (value > 255)
    return false;	
  return true;
}

void IpAccessRule::getIpRange(const TCHAR *ip, const TCHAR *netmask,
                              StringStorage *firstIp, StringStorage *lastIp) {
  StringStorage ipStorage(ip);
  StringStorage netmaskStorage(netmask);

  firstIp->setString(_T(""));
  lastIp->setString(_T(""));

  size_t ipAnsiLength = _tcslen(ip);
  size_t netmaskAnsiLength = _tcslen(netmask);

  char *ipAnsi = new char[ipAnsiLength + 1];
  char *netmaskAnsi = new char[netmaskAnsiLength + 1];

  ipStorage.toAnsiString(ipAnsi, ipAnsiLength);
  netmaskStorage.toAnsiString(netmaskAnsi, netmaskAnsiLength);

  unsigned long ipAddr = inet_addr(ipAnsi);
  unsigned long netmaskAddr = inet_addr(netmaskAnsi);
  unsigned long subnetAddr = ipAddr & netmaskAddr;
  unsigned long broadcastAddr = subnetAddr | (~netmaskAddr);

  in_addr addr = {0};

  memcpy(&addr, &subnetAddr, sizeof(unsigned long));
  char *subnetworkAnsi = inet_ntoa(addr);
#ifdef _UNICODE
  firstIp->fromAnsiString(subnetworkAnsi);
#else
  firstIp->setString(subnetworkAnsi);
#endif

  memcpy(&addr, &broadcastAddr, sizeof(unsigned long));
  char *broadcastAnsi = inet_ntoa(addr);

#ifdef _UNICODE
  lastIp->fromAnsiString(broadcastAnsi);
#else
  lastIp->setString(broadcastAnsi);
#endif

  delete[] ipAnsi;
  delete[] netmaskAnsi;
}
