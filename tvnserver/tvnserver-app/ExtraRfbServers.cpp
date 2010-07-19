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

#include "ExtraRfbServers.h"

#include "server-config-lib/Configurator.h"
#include "util/FileLog.h"

ExtraRfbServers::Conf::Conf()
: acceptConnections(false),
  loopbackOnly(false),
  extraPorts()
{
}

ExtraRfbServers::Conf::Conf(const Conf &other)
: acceptConnections(other.acceptConnections),
  loopbackOnly(other.loopbackOnly),
  extraPorts(other.extraPorts)
{
}

ExtraRfbServers::Conf &
ExtraRfbServers::Conf::operator=(const Conf &other)
{
  acceptConnections = other.acceptConnections;
  loopbackOnly = other.loopbackOnly;
  extraPorts = other.extraPorts;
  return *this;
}

bool ExtraRfbServers::Conf::equals(const Conf *other)
{
  return (acceptConnections == other->acceptConnections &&
          loopbackOnly == other->loopbackOnly &&
          extraPorts.equals(&other->extraPorts));
}

ExtraRfbServers::ExtraRfbServers()
: m_servers(),
  m_effectiveConf()
{
}

ExtraRfbServers::~ExtraRfbServers()
{
  try {
    shutDown();
  } catch (...) { }
}

bool ExtraRfbServers::reload(bool asService, RfbClientManager *mgr)
{
  Log::detail(_T("Considering to reload extra RFB servers"));

  Conf newConf;
  getConfiguration(&newConf);
  bool noConfigChanges = newConf.equals(&m_effectiveConf);
  bool enoughServers = (newConf.extraPorts.count() == m_servers.size());
  Log::detail(_T("Same Extra Ports configuration = %d, enough servers = %d"),
              (int)noConfigChanges, (int)enoughServers);

  if (noConfigChanges && enoughServers) {
    return true; 
  }

  Log::message(_T("Need to reconfigure extra RFB servers"));
  shutDown();
  return startUp(asService, mgr);
}

void ExtraRfbServers::shutDown()
{
  Log::detail(_T("Requested to shut down extra RFB servers"));

  std::list<RfbServer *>::const_iterator i;
  for (i = m_servers.begin(); i != m_servers.end(); i++) {
    int port = (*i)->getBindPort();
    Log::detail(_T("Stopping extra RFB server at port %d"), port);
    delete *i;
    Log::message(_T("Stopped extra RFB server at port %d"), port);
  }
  m_servers.clear();
}

bool ExtraRfbServers::startUp(bool asService, RfbClientManager *mgr)
{
  Log::detail(_T("Requested to start up extra RFB servers"));

  if (!m_servers.empty()) {
    Log::interror(_T("Extra RFB servers active, will have to stop them"));
    shutDown();
  }

  Conf newConf;
  getConfiguration(&newConf);
  m_effectiveConf = newConf;

  if (newConf.acceptConnections) {
    const TCHAR *bindHost =
      newConf.loopbackOnly ? _T("localhost") : _T("0.0.0.0");

    for (size_t i = 0; i < newConf.extraPorts.count(); i++) {
      PortMapping pm = *newConf.extraPorts.at(i);
      PortMappingRect rect = pm.getRect();
      int port = pm.getPort();

      Log::detail(_T("Starting extra RFB server at port %d"), port);

      try {
        RfbServer *s = new RfbServer(bindHost, port, mgr, asService, &rect);
        m_servers.push_back(s);
        Log::message(_T("Started extra RFB server at port %d"), port);
      } catch (Exception &ex) {
        Log::error(_T("Failed to start extra RFB server: \"%s\""),
                   ex.getMessage());
      }
    }
  }

  return newConf.extraPorts.count() == m_servers.size();
}

void ExtraRfbServers::getConfiguration(Conf *out)
{
  //          ConfigAccessor ca;                         
  //          ServerConfig *cfg = ca.getServerConfig();  
  //          someSetting = cfg.getSomeSetting();        
  //        }                                            
  ServerConfig *config = Configurator::getInstance()->getServerConfig();
  AutoLock l(config);

  out->acceptConnections = config->isAcceptingRfbConnections();
  out->loopbackOnly = config->isOnlyLoopbackConnectionsAllowed();
  out->extraPorts = *config->getPortMappingContainer();
}
