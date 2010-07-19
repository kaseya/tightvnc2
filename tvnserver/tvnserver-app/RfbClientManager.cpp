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

#include "RfbClientManager.h"
#include "thread/ZombieKiller.h"
#include "QueryConnectionApplication.h"
#include "util/Log.h"

RfbClientManager::RfbClientManager(const TCHAR *serverName)
: m_nextClientId(0),
  m_desktop(0)
{
  Log::info(_T("Starting rfb client manager"));
}

RfbClientManager::~RfbClientManager()
{
  Log::info(_T("~RfbClientManager() has been called"));
  disconnectAllClients();
  waitUntilAllClientAreBeenDestroyed();
  Log::info(_T("~RfbClientManager() has been completed"));
}

void RfbClientManager::onClientTerminate()
{
  validateClientList();
}

WinDesktop *RfbClientManager::onClientAuth(RfbClient *client)
{
  StringStorage ip;
  client->getPeerHost(&ip);
  updateIpInBan(&ip, true);

  AutoLock al(&m_clientListLocker);

  ServerConfig *servConf = Configurator::getInstance()->getServerConfig();
  bool isAlwaysShared = servConf->isAlwaysShared();
  bool isNeverShared = servConf->isNeverShared();

  bool isResultShared;
  if (isAlwaysShared) {
    isResultShared = true;
  } else if (isNeverShared) {
    isResultShared = false;
  } else {
    isResultShared = client->getSharedFlag();
  }

  if (!isResultShared) {
    if (servConf->isDisconnectingExistingClients()) {
      disconnectAuthClients();
    } else {
      if (!m_clientList.empty()) {
        return 0; 
      }
    }
  }

  for (ClientListIter iter = m_nonAuthClientList.begin();
       iter != m_nonAuthClientList.end(); iter++) {
    RfbClient *clientOfList = *iter;
    if (clientOfList == client) {
      m_nonAuthClientList.erase(iter);
      break;
    }
  }

  m_clientList.push_back(client);

  if (m_desktop == 0 && !m_clientList.empty()) {
    try {
      m_desktop = new WinDesktop(this, this, this);
      vector<RfbClientManagerEventListener *>::iterator iter;
      for (iter = m_listeners.begin(); iter != m_listeners.end(); iter++) {
        (*iter)->afterFirstClientConnect();
      }
    } catch (...) {
    }
  }
  return m_desktop;
}

bool RfbClientManager::onCheckForBan(RfbClient *client)
{
  StringStorage ip;
  client->getPeerHost(&ip);

  return checkForBan(&ip);
}

void RfbClientManager::onAuthFailed(RfbClient *client)
{
  StringStorage ip;
  client->getPeerHost(&ip);

  updateIpInBan(&ip, false);
}

void RfbClientManager::onCheckAccessControl(RfbClient *client)
{
  SocketAddressIPv4 peerAddr;

  try {
    client->getSocketAddr(&peerAddr);
  } catch (...) {
    throw AuthException(_T("Failed to get IP address of the RFB client"));
  }

  struct sockaddr_in addr_in = peerAddr.getSockAddr();

  ServerConfig *config = Configurator::getInstance()->getServerConfig();

  IpAccessRule::ActionType action;

  if (!client->isOutgoing()) {
    action = config->getActionByAddress((unsigned long)addr_in.sin_addr.S_un.S_addr);
  } else {
    action = IpAccessRule::ACTION_TYPE_ALLOW;
  }

  if (action == IpAccessRule::ACTION_TYPE_QUERY) {
    StringStorage peerHost;

    peerAddr.toString(&peerHost);

    int queryRetVal = QueryConnectionApplication::execute(peerHost.getString(),
                                                          config->isDefaultActionAccept(),
                                                          config->getQueryTimeout());
    if (queryRetVal == 1) {
      throw AuthException(_T("Connection has been rejected"));
    }
  }
}

void RfbClientManager::onClipboardUpdate(const StringStorage *newClipboard)
{
  AutoLock al(&m_clientListLocker);
  for (ClientListIter iter = m_clientList.begin();
       iter != m_clientList.end(); iter++) {
    if ((*iter)->getClientState() == IN_NORMAL_PHASE) {
      (*iter)->sendClipboard(newClipboard);
    }
  }
}

void RfbClientManager::onSendUpdate(const UpdateContainer *updateContainer,
                                    const FrameBuffer *frameBuffer,
                                    const CursorShape *cursorShape)
{
  AutoLock al(&m_clientListLocker);
  for (ClientListIter iter = m_clientList.begin();
       iter != m_clientList.end(); iter++) {
    if ((*iter)->getClientState() == IN_NORMAL_PHASE) {
      (*iter)->sendUpdate(updateContainer, frameBuffer, cursorShape);
    }
  }
}

bool RfbClientManager::isReadyToSend()
{
  AutoLock al(&m_clientListLocker);
  bool isReady = false;
  for (ClientListIter iter = m_clientList.begin();
       iter != m_clientList.end(); iter++) {
    if ((*iter)->getClientState() == IN_NORMAL_PHASE) {
      isReady = isReady || (*iter)->clientIsReady();
    }
  }
  return isReady;
}

void RfbClientManager::onAbnormalDesktopTerminate()
{
  Log::error(_T("onAbnormalDesktopTerminate() called"));
  disconnectAllClients();
}

void RfbClientManager::disconnectAllClients()
{
  AutoLock al(&m_clientListLocker);
  disconnectNonAuthClients();
  disconnectAuthClients();
}

void RfbClientManager::disconnectNonAuthClients()
{
  AutoLock al(&m_clientListLocker);
  for (ClientListIter iter = m_nonAuthClientList.begin();
       iter != m_nonAuthClientList.end(); iter++) {
    (*iter)->disconnect();
  }
}

void RfbClientManager::disconnectAuthClients()
{
  AutoLock al(&m_clientListLocker);
  for (ClientListIter iter = m_clientList.begin();
       iter != m_clientList.end(); iter++) {
    (*iter)->disconnect();
  }
}

void RfbClientManager::waitUntilAllClientAreBeenDestroyed()
{
  while (true) {
    {
      AutoLock al(&m_clientListLocker);
      if (m_clientList.empty() && m_nonAuthClientList.empty()) {
        break;
      }
    }
    m_listUnderflowingEvent.waitForEvent();
  }
  ZombieKiller::getInstance()->killAllZombies();
}

void RfbClientManager::validateClientList()
{
  WinDesktop *objectToDestroy = 0;
  {
    AutoLock al(&m_clientListLocker);
    ClientListIter iter = m_nonAuthClientList.begin();
    while (iter != m_nonAuthClientList.end()) {
      RfbClient *client = *iter;
      ClientState state = client->getClientState();
      if (state == IN_READY_TO_REMOVE) {
        iter = m_nonAuthClientList.erase(iter);
        ZombieKiller::getInstance()->addZombie(client);
      } else {
        iter++;
      }
    }
    iter = m_clientList.begin();
    while (iter != m_clientList.end()) {
      RfbClient *client = *iter;
      ClientState state = client->getClientState();
      if (state == IN_READY_TO_REMOVE) {
        iter = m_clientList.erase(iter);
        ZombieKiller::getInstance()->addZombie(client);
      } else {
        iter++;
      }
    }

    if (m_desktop != 0 && m_clientList.empty()) {
      objectToDestroy = m_desktop;
      m_desktop = 0;
    }
  }
  if (objectToDestroy != 0) {
    delete objectToDestroy;
    vector<RfbClientManagerEventListener *>::iterator iter;
    for (iter = m_listeners.begin(); iter != m_listeners.end(); iter++) {
      (*iter)->afterLastClientDisconnect();
    }
  }

  AutoLock al(&m_clientListLocker);
  if (m_clientList.empty() && m_nonAuthClientList.empty()) {
    m_listUnderflowingEvent.notify();
  }
}

bool RfbClientManager::checkForBan(const StringStorage *ip)
{
  AutoLock al(&m_banListMutex);
  refreshBan();

  BanListIter it = m_banList.find(*ip);
  if (it != m_banList.end()) {
    if ((*it).second.count >= MAX_BAN_COUNT) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void RfbClientManager::updateIpInBan(const StringStorage *ip, bool success)
{
  AutoLock al(&m_banListMutex);
  refreshBan();
  BanListIter it = m_banList.find(*ip);
  if (success) {
    if (it != m_banList.end()) {
      m_banList.erase(it);
    }
  } else {
    if (it != m_banList.end()) {
      (*it).second.count += 1;
    } else {
      BanProp banProp;
      banProp.banFirstTime = DateTime::now();
      banProp.count = 0;
      m_banList[*ip] = banProp;
    }
  }
}

void RfbClientManager::refreshBan()
{
  AutoLock al(&m_banListMutex);

  BanListIter it = m_banList.begin();
  while (it != m_banList.end()) {
    DateTime banFirstTime = (*it).second.banFirstTime;
    if ((DateTime::now() - banFirstTime).getTime() >= BAN_TIME) {
      it = m_banList.erase(it);
    } else {
      it++;
    }
  }
}

void RfbClientManager::addNewConnection(SocketIPv4 *socket, const Rect *viewPort,
                                        bool viewOnly, bool isOutgoing)
{
  AutoLock al(&m_clientListLocker);

  m_nonAuthClientList.push_back(new RfbClient(socket, this, this, viewOnly,
                                              isOutgoing,
                                              m_nextClientId, viewPort));
  m_nextClientId++;
}

void RfbClientManager::getClientsInfo(RfbClientInfoList *list)
{
  AutoLock al(&m_clientListLocker);

  for (ClientListIter it = m_clientList.begin(); it != m_clientList.end(); it++) {
    RfbClient *each = *it;
    if (each->getClientState() == IN_NORMAL_PHASE) {
      StringStorage peerHost;

      each->getPeerHost(&peerHost);

      list->push_back(RfbClientInfo(each->getId(), peerHost.getString()));
    }
  }
}
