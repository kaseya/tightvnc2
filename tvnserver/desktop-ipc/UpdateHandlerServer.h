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

#ifndef __UPDATEHANDLERSERVER_H__
#define __UPDATEHANDLERSERVER_H__

#include "DesktopServerProto.h"
#include "desktop/LocalUpdateHandler.h"
#include "DesktopSrvDispatcher.h"
#include "IpcServer.h"

class UpdateHandlerServer: public DesktopServerProto, public ClientListener,
                           public UpdateListener, public IpcServer
{
public:
  UpdateHandlerServer(BlockingGate *forwGate,
                      DesktopSrvDispatcher *dispatcher,
                      AnEventListener *extTerminationListener);
  virtual ~UpdateHandlerServer();

  virtual void onRequest(UINT8 reqCode, BlockingGate *backGate);

protected:
  virtual void onUpdate();

  void serverInit(BlockingGate *backGate);

  void extractReply(BlockingGate *backGate);
  void screenPropReply(BlockingGate *backGate);
  void receiveFullReqReg(BlockingGate *backGate);
  void receiveExcludingReg(BlockingGate *backGate);

  PixelFormat m_oldPf;

  LocalUpdateHandler *m_updateHandler;
  AnEventListener *m_extTerminationListener;
};

#endif 
