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

#include "SecurityAttributes.h"

SecurityAttributes::SecurityAttributes()
: m_isDefaultAttributes(true), m_localUsers(0), m_rules(0)
{
  ZeroMemory(&m_securityAttributes, sizeof(SECURITY_ATTRIBUTES));

  m_securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
}

SecurityAttributes::~SecurityAttributes()
{
}

void SecurityAttributes::setDefaultAttributes()
{
  m_isDefaultAttributes = true;
}

void SecurityAttributes::shareToAllUsers()
{
  if (m_localUsers == 0) {
    m_localUsers = SecurityIdentifier::createSidFromString(_T("S-1-1-0"));
  }
  if (m_rules != 0) {
    delete m_rules;
    m_rules = 0;
  }

  m_rules = new EXPLICIT_ACCESS[1];

  ZeroMemory(m_rules, 1 * sizeof(EXPLICIT_ACCESS));

  m_rules[0].grfAccessPermissions = GENERIC_ALL;
  m_rules[0].grfAccessMode = SET_ACCESS;
  m_rules[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
  m_rules[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
  m_rules[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
  m_rules[0].Trustee.ptstrName  = (LPTSTR)m_localUsers->getSid();

  try {
    m_sd.setRulesAsDacl(1, m_rules);
  } catch (...) {
    m_isDefaultAttributes = true;
  }

  m_securityAttributes.lpSecurityDescriptor = m_sd.getSD();

  m_isDefaultAttributes = false;
}

SECURITY_ATTRIBUTES *SecurityAttributes::getSecurityAttributes()
{
  if (m_isDefaultAttributes) {
    return 0;
  } else {
    return &m_securityAttributes;
  }
}

void SecurityAttributes::setInheritable()
{
  m_isDefaultAttributes = false;
  m_securityAttributes.bInheritHandle = TRUE;
}
