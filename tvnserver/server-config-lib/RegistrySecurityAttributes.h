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

#ifndef _REGISTRY_SECURITY_ATTRIBUTES_H_
#define _REGISTRY_SECURITY_ATTRIBUTES_H_

#include "util/winhdr.h"

#include "win-system/security/SecurityIdentifier.h"
#include "win-system/security/SecurityDescriptor.h"

#include <AccCtrl.h>
#include <Aclapi.h>

class RegistrySecurityAttributes
{
public:
  RegistrySecurityAttributes() throw(SystemException);
  virtual ~RegistrySecurityAttributes();

  SECURITY_ATTRIBUTES *getServiceSA();

  SECURITY_ATTRIBUTES *getApplicationSA();

private:
  void fillExplicitAccess(EXPLICIT_ACCESS *rules,
                          const SecurityIdentifier *secIdent);
  void fillExplicitAccesses(EXPLICIT_ACCESS serviceRules[2],
                            EXPLICIT_ACCESS applicationRules[2]);

  SecurityIdentifier *m_admins;
  SecurityIdentifier *m_serviceOwner;
  SecurityIdentifier *m_applicationOwner;
  SecurityDescriptor m_serviceSD;
  SecurityDescriptor m_applicationSD;
  SECURITY_ATTRIBUTES m_serviceSA;
  SECURITY_ATTRIBUTES m_applicationSA;
};

#endif
