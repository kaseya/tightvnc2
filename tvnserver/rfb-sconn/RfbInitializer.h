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

#ifndef __RFBINITIALIZER_H__
#define __RFBINITIALIZER_H__

#include "io-lib/Channel.h"
#include "io-lib/DataOutputStream.h"
#include "io-lib/DataInputStream.h"
#include "CapContainer.h"
#include "region/Dimension.h"
#include "rfb/PixelFormat.h"
#include "ClientAuthListener.h"

class RfbInitializer
{
public:
  RfbInitializer(Channel *stream,
                 ClientAuthListener *extAuthListener,
                 RfbClient *client, bool authAllowed);
  virtual ~RfbInitializer();

  void authPhase();
  void afterAuthPhase(const CapContainer *srvToClCaps,
                      const CapContainer *clToSrvCaps,
                      const CapContainer *encCaps,
                      const Dimension *dim,
                      const PixelFormat *pf);

  bool getSharedFlag() const { return m_shared; }
  bool getViewOnlyAuth() const { return m_viewOnlyAuth; }

  bool getTightEnabledFlag() const { return m_tightEnabled; }

protected:
  void initVersion();
  void checkForLoopback();
  void initAuthenticate();
  void readClientInit();
  void sendServerInit(const Dimension *dim,
                      const PixelFormat *pf);
  void sendDesktopName();
  void sendInteractionCaps(const CapContainer *srvToClCaps,
                           const CapContainer *clToSrvCaps,
                           const CapContainer *encCaps);

  void doAuth(UINT32 authType);
  void doTightAuth();
  void doVncAuth();
  void doAuthNone();

  void checkForBan();

  unsigned int getProtocolMinorVersion(const char str[12]) throw(Exception);

  DataOutputStream *m_output;
  DataInputStream *m_input;

  bool m_shared;
  unsigned int m_minorVerNum;
  bool m_viewOnlyAuth;
  bool m_tightEnabled;
  bool m_authAllowed;

  ClientAuthListener *m_extAuthListener;
  RfbClient *m_client;
};

#endif 
