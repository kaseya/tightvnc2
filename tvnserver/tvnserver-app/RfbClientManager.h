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

#ifndef __RFBCLIENTMANAGER_H__
#define __RFBCLIENTMANAGER_H__

#include "util/ListenerContainer.h"
#include "rfb-sconn/RfbClient.h"
#include "thread/AutoLock.h"
#include "thread/Thread.h"
#include "thread/LocalMutex.h"
#include "win-system/WindowsEvent.h"
#include "desktop/WinDesktop.h"
#include "RfbClientManager.h"
#include "RfbClientManagerEventListener.h"
#include "rfb-sconn/ClientTerminationListener.h"
#include "desktop/ClipboardListener.h"
#include "desktop/UpdateSendingListener.h"
#include "rfb-sconn/ClientAuthListener.h"
#include "tvncontrol-app/RfbClientInfo.h"

typedef std::list<RfbClient *> ClientList;
typedef std::list<RfbClient *>::iterator ClientListIter;

struct BanProp
{
  unsigned int count;
  DateTime banFirstTime;
};
typedef std::map<StringStorage, BanProp> BanList;
typedef BanList::iterator BanListIter;

class RfbClientManager: public ClientTerminationListener,
                        public ClipboardListener,
                        public UpdateSendingListener,
                        public ClientAuthListener,
                        public AbnormDeskTermListener,
                        public ListenerContainer<RfbClientManagerEventListener *>
{
public:
  RfbClientManager(const TCHAR *serverName);
  virtual ~RfbClientManager();

  void getClientsInfo(RfbClientInfoList *list);

  virtual void disconnectAllClients();
  virtual void disconnectNonAuthClients();
  virtual void disconnectAuthClients();

  void addNewConnection(SocketIPv4 *socket, const Rect *viewPort,
                        bool viewOnly, bool isOutgoing);

protected:
  virtual void onClientTerminate();
  virtual WinDesktop *onClientAuth(RfbClient *client);
  virtual bool onCheckForBan(RfbClient *client);
  virtual void onAuthFailed(RfbClient *client);
  virtual void onCheckAccessControl(RfbClient *client) throw(AuthException);
  virtual void onClipboardUpdate(const StringStorage *newClipboard);
  virtual void onSendUpdate(const UpdateContainer *updateContainer,
                            const FrameBuffer *frameBuffer,
                            const CursorShape *cursorShape);
  virtual bool isReadyToSend();
  virtual void onAbnormalDesktopTerminate();

  void waitUntilAllClientAreBeenDestroyed();

private:
  void validateClientList();

  bool checkForBan(const StringStorage *ip);
  void updateIpInBan(const StringStorage *ip, bool success);
  void refreshBan();

  ClientList m_nonAuthClientList;
  ClientList m_clientList;
  LocalMutex m_clientListLocker;

  static const int MAX_BAN_COUNT = 10;
  static const int BAN_TIME = 3000 * MAX_BAN_COUNT; 
  BanList m_banList;
  WindowsEvent m_banTimer;
  LocalMutex m_banListMutex;

  WindowsEvent m_listUnderflowingEvent;

  WinDesktop *m_desktop;

  unsigned int m_nextClientId;
};

#endif 
