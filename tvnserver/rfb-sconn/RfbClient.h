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

#ifndef __RFBCLIENT_H__
#define __RFBCLIENT_H__

#include <list>
#include "network/socket/SocketIPv4.h"
#include "win-system/WindowsEvent.h"
#include "thread/Thread.h"
#include "network/RfbOutputGate.h"
#include "desktop/WinDesktop.h"
#include "fb-update-sender/UpdateSender.h"

#include "RfbDispatcher.h"
#include "ClipboardExchange.h"
#include "ClientInputHandler.h"
#include "ClientTerminationListener.h"
#include "ClientInputEventListener.h"

class ClientAuthListener;

enum ClientState
{
  IN_NONAUTH,
  IN_AUTH,
  IN_NORMAL_PHASE,
  IN_PENDING_TO_REMOVE,
  IN_READY_TO_REMOVE
};

class RfbClient: public Thread, ClientInputEventListener
{
public:
  RfbClient(SocketIPv4 *socket, ClientTerminationListener *extTermListener,
            ClientAuthListener *extAuthListener, bool viewOnly,
            bool isOutgoing, unsigned int id, const Rect *viewPort = 0);
  virtual ~RfbClient();

  void disconnect();

  ClientState getClientState();

  unsigned int getId() const;
  void getPeerHost(StringStorage *address);
  void getSocketAddr(SocketAddressIPv4 *addr) const;

  bool isOutgoing() const;

  bool getSharedFlag() const { return m_shared; }
  bool getViewOnlyAuth() const { return m_viewOnlyAuth; }

  void setViewOnlyFlag(bool value);

  bool clientIsReady() const { return m_updateSender->clientIsReady(); }
  void sendUpdate(const UpdateContainer *updateContainer,
                  const FrameBuffer *frameBuffer,
                  const CursorShape *cursorShape);
  void sendClipboard(const StringStorage *newClipboard);

protected:
  virtual void execute();
  virtual void onTerminate();

private:
  void notifyAbStateChanging(ClientState state);

  virtual void onKeyboardEvent(UINT32 keySym, bool down);
  virtual void onMouseEvent(UINT16 x, UINT16 y, UINT8 buttonMask);

  void setClientState(ClientState newState);

  ClientState m_clientState;
  bool m_isMarkedOk;
  LocalMutex m_clientStateMut;
  ClientTerminationListener *m_extTermListener;

  SocketIPv4 *m_socket;

  ClientAuthListener *m_extAuthListener;

  ViewPort m_viewPort;
  UpdateSender *m_updateSender;
  ClipboardExchange *m_clipboardExchange;
  ClientInputHandler *m_clientInputHandler;
  WinDesktop *m_desktop;

  bool m_viewOnly;
  bool m_isOutgoing;
  bool m_viewOnlyAuth;
  bool m_shared;

  unsigned int m_id;
};

#endif 
