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

#ifndef _CONFIGURATOR_H_
#define _CONFIGURATOR_H_

#include "config-lib/SettingsManager.h"

#include "PortMappingContainer.h"
#include "IpAccessControl.h"
#include "ServerConfig.h"
#include "ConfigReloadListener.h"
#include "RegistrySecurityAttributes.h"

#include "util/ListenerContainer.h"

class Configurator : public ListenerContainer<ConfigReloadListener *>
{
public:
  Configurator();
public:

  bool getServiceFlag() { return m_isConfiguringService; }
  void setServiceFlag(bool asService) { m_isConfiguringService = asService; }

  bool load();
  bool save();

  bool isConfigLoadedPartly() { return m_isConfigLoadedPartly; }

  static Configurator *getInstance();
  static void setInstance(Configurator *conf);

  void notifyReload();

  ServerConfig *getServerConfig() { return &m_serverConfig; }

private:

  bool savePortMappingContainer(SettingsManager *sm);
  bool loadPortMappingContainer(SettingsManager *sm, PortMappingContainer *portMapping);

  bool saveIpAccessControlContainer(SettingsManager *sm);
  bool loadIpAccessControlContainer(SettingsManager *sm,
                                    IpAccessControl *ipContainer);

  bool saveServerConfig(SettingsManager *sm);
  bool loadServerConfig(SettingsManager *sm, ServerConfig *config);

  bool saveQueryConfig(SettingsManager *sm);
  bool loadQueryConfig(SettingsManager *sm, ServerConfig *config);

  bool saveInputHandlingConfig(SettingsManager *sm);
  bool loadInputHandlingConfig(SettingsManager *sm, ServerConfig *config);

  bool saveVideoRegionConfig(SettingsManager *sm);
  bool loadVideoRegionConfig(SettingsManager *sm, ServerConfig *config);

  bool save(SettingsManager *sm);
  bool load(SettingsManager *sm);

  bool load(bool forService);
  bool save(bool forService);
protected:

  ServerConfig m_serverConfig;

  bool m_isConfiguringService;
  bool m_isConfigLoadedPartly;

  RegistrySecurityAttributes *m_regSA;

  bool m_isFirstLoad;

  static Configurator *s_instance;
};

#endif
