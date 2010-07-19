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

#ifndef __TVNSERVERAPP_EXTRA_RFB_SERVERS_H__
#define __TVNSERVERAPP_EXTRA_RFB_SERVERS_H__

#include <list>
#include "RfbServer.h"

class ExtraRfbServers
{
  struct Conf
  {
    bool acceptConnections;
    bool loopbackOnly;
    PortMappingContainer extraPorts;

    Conf();
    Conf(const Conf &other);
    Conf &operator=(const Conf &other);
    bool equals(const Conf *other);
  };

public:
  ExtraRfbServers();
  virtual ~ExtraRfbServers();

  bool reload(bool asService, RfbClientManager *mgr);

  void shutDown();

protected:
  bool startUp(bool asService, RfbClientManager *mgr);

  static void getConfiguration(Conf *out);

protected:
  std::list<RfbServer *> m_servers;
  Conf m_effectiveConf;

private:
  ExtraRfbServers(const ExtraRfbServers &);
  ExtraRfbServers &operator=(const ExtraRfbServers &);
};

#endif 
