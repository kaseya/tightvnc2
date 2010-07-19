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

#include "WinDesktop.h"
#include "server-config-lib/Configurator.h"
#include "desktop-ipc/UpdateHandlerClient.h"
#include "LocalUpdateHandler.h"
#include "WindowsInputBlocker.h"
#include "desktop-ipc/UserInputClient.h"
#include "SasUserInput.h"
#include "WindowsUserInput.h"
#include "DesktopConfigLocal.h"
#include "util/Log.h"

WinDesktop::WinDesktop(ClipboardListener *extClipListener,
                       UpdateSendingListener *extUpdSendingListener,
                       AbnormDeskTermListener *extDeskTermListener)
: m_extClipListener(extClipListener),
  m_extUpdSendingListener(extUpdSendingListener),
  m_extDeskTermListener(extDeskTermListener),
  m_clToSrvChan(0),
  m_srvToClChan(0),
  m_clToSrvGate(0),
  m_srvToClGate(0),
  m_deskServWatcher(0),
  m_dispatcher(0),
  m_updateHandler(0),
  m_userInputClient(0),
  m_userInput(0),
  m_deskConf(0),
  m_gateKicker(0),
  m_wallPaper(0)
{
  Log::info(_T("Creating WinDesktop"));

  try {
    if (Configurator::getInstance()->getServiceFlag()) {
      m_deskServWatcher = new DesktopServerWatcher(this);

      m_clToSrvChan = new ReconnectingChannel(60000);
      m_srvToClChan = new ReconnectingChannel(60000);

      m_clToSrvGate = new BlockingGate(m_clToSrvChan);
      m_srvToClGate = new BlockingGate(m_srvToClChan);

      m_dispatcher = new DesktopSrvDispatcher(m_srvToClGate, this);

      m_updateHandler = new UpdateHandlerClient(m_clToSrvGate, m_dispatcher,
                                                this);

      UserInputClient *userInputClient =
        new UserInputClient(m_clToSrvGate, m_dispatcher, this);
      m_userInputClient = userInputClient;
      m_userInput = new SasUserInput(userInputClient);

      m_deskConf = new DesktopConfigClient(m_clToSrvGate);
      m_gateKicker = new GateKicker(m_clToSrvGate);
      m_dispatcher->resume();
      onConfigReload(0);
    } else {
      m_updateHandler = new LocalUpdateHandler(this);
      bool ctrlAltDelEnabled = false;
      m_userInput = new WindowsUserInput(this, ctrlAltDelEnabled);
      m_deskConf = new DesktopConfigLocal();
      applyNewConfiguration();
      m_wallPaper = new WallpaperUtil;
      m_wallPaper->updateWallpaper();
    }
    Configurator::getInstance()->addListener(this);
  } catch (Exception &ex) {
    Log::error(_T("exception during WinDesktop creaion: %s"), ex.getMessage());
    freeResource();
    throw;
  }
  resume();
}

WinDesktop::~WinDesktop()
{
  Log::info(_T("Deleting WinDesktop"));
  terminate();
  wait();
  freeResource();
  Log::info(_T("WinDesktop deleted"));
}

void WinDesktop::freeResource()
{
  Configurator::getInstance()->removeListener(this);

  if (m_wallPaper) delete m_wallPaper;
  if (m_deskServWatcher) delete m_deskServWatcher;

  closeDesktopServerTransport();

  if (m_dispatcher) delete m_dispatcher;

  if (m_gateKicker) delete m_gateKicker;
  if (m_updateHandler) delete m_updateHandler;
  if (m_deskConf) delete m_deskConf;
  if (m_userInput) delete m_userInput;
  if (m_userInputClient) delete m_userInputClient;

  if (m_srvToClGate) delete m_srvToClGate;
  if (m_clToSrvGate) delete m_clToSrvGate;

  if (m_srvToClChan) delete m_srvToClChan;
  if (m_clToSrvChan) delete m_clToSrvChan;
}

void WinDesktop::closeDesktopServerTransport()
{
  try {
    if (m_clToSrvChan) m_clToSrvChan->close();
  } catch (Exception &e) {
    Log::error(_T("Cannot close client->server channel from Windesktop: %s"),
               e.getMessage());
  }
  try {
    if (m_srvToClChan) m_srvToClChan->close();
  } catch (Exception &e) {
    Log::error(_T("Cannot close server->client channel from Windesktop: %s"),
               e.getMessage());
  }
}

void WinDesktop::onAnObjectEvent()
{
  m_extDeskTermListener->onAbnormalDesktopTerminate();
  Log::error(_T("Forced closing of pipe conections"));
  closeDesktopServerTransport();
}

void WinDesktop::onReconnect(Channel *newChannelTo, Channel *newChannelFrom)
{
  BlockingGate gate(newChannelTo);
  if (m_deskConf) {
    Log::info(_T("try update remote configuration from the ")
              _T("WinDesktop::onReconnect() function"));
    m_deskConf->updateByNewSettings(&gate);
  }
  if (m_updateHandler) {
    Log::info(_T("try update remote UpdateHandler from the ")
              _T("WinDesktop::onReconnect() function"));
    m_updateHandler->sendInit(&gate);
  }
  if (m_userInput) {
    Log::info(_T("try update remote UserInput from the ")
              _T("WinDesktop::onReconnect() function"));
    m_userInput->sendInit(&gate);
  }

  m_clToSrvChan->replaceChannel(newChannelTo);
  m_srvToClChan->replaceChannel(newChannelFrom);
}

void WinDesktop::onUpdate()
{
  Log::info(_T("update detected"));
  m_newUpdateEvent.notify();
}

void WinDesktop::onClipboardUpdate(const StringStorage *newClipboard)
{
  Log::info(_T("clipboard update detected"));

  bool isEqual;
  {
    AutoLock al(&m_storedClipCritSec);
    isEqual = m_receivedClip.isEqualTo(newClipboard);
    isEqual = isEqual || m_sentClip.isEqualTo(newClipboard);
  }
  if (!isEqual) {
    {
      AutoLock al(&m_storedClipCritSec);
      m_sentClip = *newClipboard;
    }
    m_extClipListener->onClipboardUpdate(newClipboard);
  }
}

void WinDesktop::onUpdateRequest(const Rect *rectRequested, bool incremental)
{
  Log::info(_T("update requested"));

  AutoLock al(&m_reqRegMutex);
  if (!incremental) {
    m_fullReqRegion.addRect(rectRequested);
  }
  m_newUpdateEvent.notify();
}

void WinDesktop::onConfigReload(ServerConfig *serverConfig)
{
  applyNewConfiguration();
}

void WinDesktop::onTerminate()
{
  m_newUpdateEvent.notify();
}

void WinDesktop::execute()
{
  Log::info(_T("WinDesktop thread started"));

  while (!isTerminating()) {
    m_newUpdateEvent.waitForEvent();
    if (!isTerminating()) {
      sendUpdate();
    }
  }

  Log::info(_T("WinDesktop thread stopped"));
}

void WinDesktop::sendUpdate()
{
  if (!m_extUpdSendingListener->isReadyToSend()) {
    Log::info(_T("nobody is ready for updates"));
    return;
  }
  UpdateContainer updCont;
  try {
    if (!m_fullReqRegion.isEmpty()) {
      Log::info(_T("set full update request to UpdateHandler"));
      m_updateHandler->setFullUpdateRequested(&m_fullReqRegion);
    }

    Log::info(_T("extracting updates from UpdateHandler"));
    m_updateHandler->extract(&updCont);
  } catch (Exception &e) {
    Log::info(_T("WinDesktop::sendUpdate() failed with error:%s"),
               e.getMessage());
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }

  if (!updCont.isEmpty() || !m_fullReqRegion.isEmpty()) {
    Log::info(_T("UpdateContainer is not empty.")
              _T(" Updates will be given to all."));
    m_extUpdSendingListener->onSendUpdate(&updCont,
                                          m_updateHandler->getFrameBuffer(),
                                          m_updateHandler->getCursorShape());
    Log::info(_T("Updates have been given to all."));
    AutoLock al(&m_reqRegMutex);
    m_fullReqRegion.clear();
  } else {
    Log::info(_T("UpdateContainer is empty"));
  }
}

void WinDesktop::getCurrentUserInfo(StringStorage *desktopName,
                                    StringStorage *userName)
{
  Log::info(_T("get current user information"));
  try {
    m_userInput->getCurrentUserInfo(desktopName, userName);
  } catch (...) {
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }
}

void WinDesktop::getFrameBufferProperties(Dimension *dim, PixelFormat *pf)
{
  Log::info(_T("get frame buffer properties"));
  try {
    m_updateHandler->getFrameBufferProp(dim, pf);
  } catch (...) {
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }
}

void WinDesktop::setKeyboardEvent(UINT32 keySym, bool down)
{
  Log::info(_T("set keyboard event (keySym = %u, down = %d)"), keySym, (int)down);
  try {
    if (isRemoteInputAllowed()) {
      m_userInput->setKeyboardEvent(keySym, down);
    }
  } catch (Exception &e) {
    Log::error(_T("setKeyboardEvent() crashed: %s"), e.getMessage());
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }
}

void WinDesktop::setMouseEvent(UINT16 x, UINT16 y, UINT8 buttonMask)
{
  Log::info(_T("set mouse event (x = %u, y = %u)"), (UINT32)x, (UINT32)y);
  Point point(x, y);
  try {
    if (isRemoteInputAllowed()) {
      m_userInput->setMouseEvent(&point, buttonMask);
    }
  } catch (...) {
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }
}

void WinDesktop::applyNewConfiguration()
{
  Log::info(_T("reload WinDesktop configuration"));
  m_deskConf->updateByNewSettings(m_clToSrvGate);
}

bool WinDesktop::isRemoteInputAllowed()
{
  Log::info(_T("checking remote input allowing"));

  bool enabled = !Configurator::getInstance()->getServerConfig()->isBlockingRemoteInput();
  enabled = enabled && m_deskConf->isRemoteInputAllowed();
  return enabled;
}

void WinDesktop::setNewClipText(const StringStorage *newClipboard)
{
  Log::info(_T("set new clipboard text"));

  bool isEqual;
  {
    AutoLock al(&m_storedClipCritSec);
    isEqual = m_sentClip.isEqualTo(newClipboard);
    isEqual = isEqual || m_receivedClip.isEqualTo(newClipboard);
  }
  if (!isEqual) {
    {
      AutoLock al(&m_storedClipCritSec);
      m_receivedClip = *newClipboard;
    }
    try {
      m_userInput->setNewClipboard(newClipboard);
    } catch (...) {
      m_extDeskTermListener->onAbnormalDesktopTerminate();
    }
  }
}
