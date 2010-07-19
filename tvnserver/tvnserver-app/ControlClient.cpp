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

#include "ControlClient.h"
#include "TvnServer.h"
#include "OutgoingRfbConnectionThread.h"

#include "tvncontrol-app/ControlProto.h"

#include "network/socket/SocketStream.h"

#include "server-config-lib/Configurator.h"

#include "util/VncPassCrypt.h"
#include "util/Log.h"

#include "rfb/HostPath.h"

#include "win-system/WTS.h"

#include "tvnserver/resource.h"

#include <time.h>

const UINT32 ControlClient::REQUIRES_AUTH[] = { ControlProto::ADD_CLIENT_MSG_ID,
                                                ControlProto::DISCONNECT_ALL_CLIENTS_MSG_ID,
                                                ControlProto::GET_CONFIG_MSG_ID,
                                                ControlProto::RELOAD_CONFIG_MSG_ID,
                                                ControlProto::SET_CONFIG_MSG_ID,
                                                ControlProto::SHUTDOWN_SERVER_MSG_ID };

ControlClient::ControlClient(Transport *transport,
                             RfbClientManager *rfbClientManager,
                             ControlAppAuthenticator *authenticator)
: m_transport(transport), m_rfbClientManager(rfbClientManager),
  m_authenticator(authenticator)
{
  m_stream = m_transport->getIOStream();

  m_gate = new ControlGate(m_stream);

  m_authPassed = false;
}

ControlClient::~ControlClient()
{
  terminate();
  wait();

  delete m_gate;
  delete m_transport;
}

void ControlClient::execute()
{
  if (!Configurator::getInstance()->getServerConfig()->isControlAuthEnabled()) {
    m_authPassed = true;
  }

  try {
    while (!isTerminating()) {
      UINT32 messageId = m_gate->readUInt32();
      UINT32 messageSize = m_gate->readUInt32();

      Log::info(_T("Recieved %d control message with %d size"), messageId, messageSize);

      bool requiresControlAuth = false;

      if (Configurator::getInstance()->getServerConfig()->isControlAuthEnabled()) {
        for (size_t i = 0; i < sizeof(REQUIRES_AUTH) / sizeof(UINT32); i++) {
          if (messageId == REQUIRES_AUTH[i]) {
            requiresControlAuth = true;
            break;
          }
        }
      }

      try {
        if (requiresControlAuth && !m_authPassed) {
          Log::info(_T("Message requires control authentication"));

          m_gate->skipBytes(messageSize);
          m_gate->writeUInt32(ControlProto::REPLY_AUTH_NEEDED);

          continue;
        }

        switch (messageId) {
        case ControlProto::AUTH_MSG_ID:
          Log::message(_T("Control authentication requested"));
          authMsgRcdv();
          break;
        case ControlProto::RELOAD_CONFIG_MSG_ID:
          Log::message(_T("Config reload command requested"));
          reloadConfigMsgRcvd();
          break;
        case ControlProto::DISCONNECT_ALL_CLIENTS_MSG_ID:
          Log::message(_T("Disconnect all clients command requested"));
          disconnectAllMsgRcvd();
          break;
        case ControlProto::SHUTDOWN_SERVER_MSG_ID:
          Log::message(_T("Shutdown TightVNC command requested"));
          shutdownMsgRcvd();
          break;
        case ControlProto::ADD_CLIENT_MSG_ID:
          Log::message(_T("Outgoing connection command requested"));
          addClientMsgRcvd();
          break;
        case ControlProto::GET_SERVER_INFO_MSG_ID:
          Log::info(_T("Get server info command requested"));
          getServerInfoMsgRcvd();
          break;
        case ControlProto::GET_CLIENT_LIST_MSG_ID:
          Log::info(_T("Get client list command requested"));
          getClientsListMsgRcvd();
          break;
        case ControlProto::SET_CONFIG_MSG_ID:
          Log::message(_T("Set server config message requested"));
          setServerConfigMsgRcvd();
          break;
        case ControlProto::GET_CONFIG_MSG_ID:
          Log::message(_T("Get server config message requested"));
          getServerConfigMsgRcvd();
          break;
        case ControlProto::GET_SHOW_TRAY_ICON_FLAG:
          Log::message(_T("Get run tvncontrol flag message requested"));
          getShowTrayIconFlagMsgRcvd();
          break;
        case ControlProto::UPDATE_TVNCONTROL_PROCESS_ID_MSG_ID:
          Log::message(_T("Update tvncontrol process id message recieved"));
          updateTvnControlProcessIdMsgRcvd();
          break;
        default:
          m_gate->skipBytes(messageSize);
          Log::warning(_T("Control message is not supported."));
        } 
      } catch (ControlException &controlEx) {
        Log::error(_T("Exception during processing control request: \"%s\""), controlEx.getMessage());

        sendError(controlEx.getMessage());
      }
    } 
  } catch (Exception &ex) {
    Log::error(_T("Exception in control client thread: \"%s\""), ex.getMessage());
  }
}

void ControlClient::onTerminate()
{
  try { m_transport->close(); } catch (...) { }
}

void ControlClient::sendError(const TCHAR *message)
{
  m_gate->writeUInt32(ControlProto::REPLY_ERROR);
  m_gate->writeUTF8(message);
}

void ControlClient::authMsgRcdv()
{
  UINT8 challenge[16];
  UINT8 response[16];

  srand((unsigned)time(0));
  for (int i = 0; i < sizeof(challenge); i++) {
    challenge[i] = rand() & 0xff;
  }

  m_gate->writeFully(challenge, sizeof(challenge));
  m_gate->readFully(response, sizeof(response));

  ServerConfig *config = Configurator::getInstance()->getServerConfig();
  UINT8 cryptPassword[8];
  config->getControlPassword(cryptPassword);

  bool isAuthSucceed = m_authenticator->authenticate(cryptPassword,
                                                     challenge,
                                                     response);
  if (!isAuthSucceed) {
    sendError(StringTable::getString(IDS_INVALID_CONTROL_PASSWORD));
  } else {
    m_gate->writeUInt32(ControlProto::REPLY_OK);
    m_authPassed = true;
  }
}

void ControlClient::getClientsListMsgRcvd()
{
  UINT32 clientCount = 0;

  RfbClientInfoList clients;

  m_rfbClientManager->getClientsInfo(&clients);

  m_gate->writeUInt32(ControlProto::REPLY_OK);
  m_gate->writeUInt32(clients.size());

  for (RfbClientInfoList::iterator it = clients.begin(); it != clients.end(); it++) {
    m_gate->writeUInt32((*it).m_id);
    m_gate->writeUTF8((*it).m_peerAddr.getString());
  }
}

void ControlClient::getServerInfoMsgRcvd()
{
  bool acceptFlag = false;
  bool serviceFlag = false;

  StringStorage logPath;
  StringStorage statusText;

  TvnServerInfo info;

  TvnServer::getInstance()->getServerInfo(&info);

  m_gate->writeUInt32(ControlProto::REPLY_OK);

  m_gate->writeUInt8(info.m_acceptFlag ? 1 : 0);
  m_gate->writeUInt8(info.m_serviceFlag ? 1 : 0);
  m_gate->writeUTF8(info.m_statusText.getString());
}

void ControlClient::reloadConfigMsgRcvd()
{
  m_gate->writeUInt32(ControlProto::REPLY_OK);

  Configurator::getInstance()->load();
}

void ControlClient::disconnectAllMsgRcvd()
{
  m_gate->writeUInt32(ControlProto::REPLY_OK);

  m_rfbClientManager->disconnectAllClients();
}

void ControlClient::shutdownMsgRcvd()
{
  m_gate->writeUInt32(ControlProto::REPLY_OK);

  TvnServer::getInstance()->generateExternalShutdownSignal();
}

void ControlClient::addClientMsgRcvd()
{
  m_gate->writeUInt32(ControlProto::REPLY_OK);

  StringStorage connectString;

  m_gate->readUTF8(&connectString);

  bool viewOnly = m_gate->readUInt8() == 1;

  char connectStringAnsi[1024];

  connectString.toAnsiString(connectStringAnsi, 1024);

  HostPath hp(connectStringAnsi, 5500);

  if (!hp.isValid()) {
    return;
  }

  StringStorage host; host.fromAnsiString(hp.getVncHost());

  OutgoingRfbConnectionThread *newConnectionThread =
                               new OutgoingRfbConnectionThread(host.getString(),
                                                               hp.getVncPort(), viewOnly,
                                                               m_rfbClientManager);

  newConnectionThread->resume();

  ZombieKiller::getInstance()->addZombie(newConnectionThread);
}

void ControlClient::setServerConfigMsgRcvd()
{
  m_gate->writeUInt32(ControlProto::REPLY_OK);

  Configurator::getInstance()->getServerConfig()->deserialize(m_gate);
  Configurator::getInstance()->save();
  Configurator::getInstance()->load();
}

void ControlClient::getShowTrayIconFlagMsgRcvd()
{
  bool showIcon = Configurator::getInstance()->getServerConfig()->getShowTrayIconFlag();

  m_gate->writeUInt32(ControlProto::REPLY_OK);

  m_gate->writeUInt8(showIcon ? 1 : 0);
}

void ControlClient::updateTvnControlProcessIdMsgRcvd()
{
  WTS::defineConsoleUserProcessId(m_gate->readUInt32());

  m_gate->writeUInt32(ControlProto::REPLY_OK);
}

void ControlClient::getServerConfigMsgRcvd()
{
  m_gate->writeUInt32(ControlProto::REPLY_OK);

  Configurator::getInstance()->getServerConfig()->serialize(m_gate);
}
