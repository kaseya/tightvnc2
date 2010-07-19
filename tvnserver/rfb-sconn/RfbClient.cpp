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

#include "RfbClient.h"
#include "thread/AutoLock.h"
#include "RfbCodeRegistrator.h"
#include "ft-server-lib/FileTransferRequestHandler.h"
#include "network/socket/SocketStream.h"
#include "RfbInitializer.h"
#include "ClientAuthListener.h"
#include "util/Log.h"

RfbClient::RfbClient(SocketIPv4 *socket,
                     ClientTerminationListener *extTermListener,
                     ClientAuthListener *extAuthListener, bool viewOnly,
                     bool isOutgoing, unsigned int id, const Rect *viewPort)
: m_socket(socket), 
  m_viewOnly(viewOnly),
  m_isOutgoing(isOutgoing),
  m_shared(false),
  m_viewOnlyAuth(true),
  m_clientState(IN_NONAUTH),
  m_isMarkedOk(false),
  m_extTermListener(extTermListener),
  m_extAuthListener(extAuthListener),
  m_updateSender(0),
  m_clipboardExchange(0),
  m_clientInputHandler(0),
  m_id(id),
  m_desktop(0)
{
  if (viewPort != 0) {
    m_viewPort.setArbitraryRect(viewPort);
  } else {
    m_viewPort.setFullDesktop();
  }
  resume();
}

RfbClient::~RfbClient()
{
  terminate();
  wait();
  delete m_socket;
}

void RfbClient::disconnect()
{
  try { m_socket->shutdown(SD_BOTH); } catch (...) { }
  try { m_socket->close(); } catch (...) { }
  Log::message(_T("Connection has been closed"));
}

unsigned int RfbClient::getId() const
{
  return m_id;
}

bool RfbClient::isOutgoing() const
{
  return m_isOutgoing;
}

void RfbClient::getPeerHost(StringStorage *host)
{
  SocketAddressIPv4 addr;

  if (m_socket->getPeerAddr(&addr)) {
    addr.toString(host);
  } else {
    _ASSERT(FALSE);

    host->setString(_T("unknown"));
  }
}

void RfbClient::getSocketAddr(SocketAddressIPv4 *addr) const
{
  m_socket->getPeerAddr(addr);
}

void RfbClient::setClientState(ClientState newState)
{
  AutoLock al(&m_clientStateMut);
  if (newState > m_clientState) {
    m_clientState = newState;
  }
}

ClientState RfbClient::getClientState()
{
  AutoLock al(&m_clientStateMut);
  return m_clientState;
}

void RfbClient::setViewOnlyFlag(bool value)
{
  if (getClientState() < IN_NORMAL_PHASE) {
    throw Exception(_T("Irrelevant call to RfbClient::setViewOnlyFlag()"));
  }
  m_viewOnly = value || m_viewOnlyAuth;
  m_clientInputHandler->setViewOnlyFlag(m_viewOnly);
}

void RfbClient::notifyAbStateChanging(ClientState state)
{
  setClientState(state);
  m_extTermListener->onClientTerminate();
}

void RfbClient::onTerminate()
{
  disconnect();
}

void RfbClient::execute()
{
  ServerConfig *config = Configurator::getInstance()->getServerConfig();

  WindowsEvent connClosingEvent;

  SocketStream sockStream(m_socket);

  RfbOutputGate output(&sockStream);
  RfbInputGate input(&sockStream);

  FileTransferRequestHandler *fileTransfer = 0;

  RfbInitializer rfbInitializer(&sockStream, m_extAuthListener, this,
                                !m_isOutgoing);

  try {
    try {
      Log::info(_T("Entering RFB initialization phase 1"));
      rfbInitializer.authPhase();
      setClientState(IN_AUTH);
      Log::debug(_T("RFB initialization phase 1 completed"));

      m_shared = rfbInitializer.getSharedFlag();
      Log::debug(_T("Shared flag = %d"), (int)m_shared);
      m_viewOnlyAuth = rfbInitializer.getViewOnlyAuth();
      Log::debug(_T("Initial view-only state = %d"), (int)m_viewOnly);
      Log::debug(_T("Authenticated with view-only password = %d"), (int)m_viewOnlyAuth);
      m_viewOnly = m_viewOnly || m_viewOnlyAuth;

      m_desktop = m_extAuthListener->onClientAuth(this);

      Log::info(_T("View only = %d"), (int)m_viewOnly);
    } catch (Exception &e) {
      Log::error(_T("Error during RFB initialization: %s"), e.getMessage());
    }
    if (m_desktop == 0) {
      throw Exception(_T("Connection unsuccessful"));
    }

    RfbDispatcher dispatcher(&input, &connClosingEvent);
    Log::debug(_T("Dispatcher has been created"));
    CapContainer srvToClCaps, clToSrvCaps, encCaps;
    RfbCodeRegistrator codeRegtor(&dispatcher, &srvToClCaps, &clToSrvCaps,
                                  &encCaps);
    m_updateSender = new UpdateSender(&codeRegtor, m_desktop,
                                      &output, m_id);
    Log::debug(_T("UpdateSender has been created"));
    PixelFormat pf;
    Dimension fbDim;
    m_desktop->getFrameBufferProperties(&fbDim, &pf);
    m_updateSender->init(&fbDim, &pf);
    Log::debug(_T("UpdateSender has been initialized"));
    m_clientInputHandler = new ClientInputHandler(&codeRegtor, this,
                                                  m_viewOnly);
    Log::debug(_T("ClientInputHandler has been created"));
    m_clipboardExchange = new ClipboardExchange(&codeRegtor, m_desktop, &output,
                                                m_viewOnly);
    Log::debug(_T("ClipboardExchange has been created"));

    if (config->isFileTransfersEnabled() &&
        rfbInitializer.getTightEnabledFlag()) {
      fileTransfer = new FileTransferRequestHandler(&codeRegtor, &output, m_desktop, !m_viewOnly);
      Log::debug(_T("File transfer has been created"));
    } else {
      Log::info(_T("File transfer is not allowed"));
    }

    m_viewPort.update(&fbDim);
    Rect viewPort = m_viewPort.getViewPortRect();

    Log::debug(_T("View port: (%d,%d) (%dx%d)"), viewPort.left,
                                                 viewPort.top,
                                                 viewPort.getWidth(),
                                                 viewPort.getHeight());
    Log::info(_T("Entering RFB initialization phase 2"));
    rfbInitializer.afterAuthPhase(&srvToClCaps, &clToSrvCaps,
                                  &encCaps, &Dimension(&viewPort), &pf);
    Log::debug(_T("RFB initialization phase 2 completed"));

    setClientState(IN_NORMAL_PHASE);

    Log::info(_T("Entering normal phase of the RFB protocol"));
    dispatcher.resume();

    connClosingEvent.waitForEvent();
  } catch (Exception &e) {
    Log::error(_T("Connection will be closed: %s"), e.getMessage());
  }

  disconnect();

  notifyAbStateChanging(IN_PENDING_TO_REMOVE);

  if (fileTransfer)         delete fileTransfer;
  if (m_clipboardExchange)  delete m_clipboardExchange;
  if (m_clientInputHandler) delete m_clientInputHandler;
  if (m_updateSender)       delete m_updateSender;

  notifyAbStateChanging(IN_READY_TO_REMOVE);
}

void RfbClient::sendUpdate(const UpdateContainer *updateContainer,
                           const FrameBuffer *frameBuffer,
                           const CursorShape *cursorShape)
{
  m_viewPort.update(&frameBuffer->getDimension());
  m_updateSender->newUpdates(updateContainer, frameBuffer, cursorShape,
                             &m_viewPort.getViewPortRect());
}

void RfbClient::sendClipboard(const StringStorage *newClipboard)
{
  m_clipboardExchange->sendClipboard(newClipboard);
}

void RfbClient::onKeyboardEvent(UINT32 keySym, bool down)
{
  m_desktop->setKeyboardEvent(keySym, down);
}

void RfbClient::onMouseEvent(UINT16 x, UINT16 y, UINT8 buttonMask)
{
  m_updateSender->blockCursorPosSending();
  m_desktop->setMouseEvent(x, y, buttonMask);
}
