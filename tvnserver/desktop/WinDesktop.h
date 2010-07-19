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

#ifndef __WINDESKTOP_H__
#define __WINDESKTOP_H__

#include "util/CommonHeader.h"
#include "UpdateHandler.h"
#include "DesktopServerWatcher.h"
#include "UserInput.h"
#include "InputBlocker.h"
#include "desktop-ipc/DesktopConfigClient.h"
#include "desktop/WallpaperUtil.h"
#include "desktop-ipc/ReconnectingChannel.h"
#include "desktop-ipc/BlockingGate.h"
#include "desktop-ipc/GateKicker.h"
#include "desktop-ipc/DesktopSrvDispatcher.h"
#include "util/AnEventListener.h"
#include "desktop-ipc/ReconnectionListener.h"
#include "ClipboardListener.h"
#include "UpdateListener.h"
#include "fb-update-sender/UpdateRequestListener.h"
#include "server-config-lib/ConfigReloadListener.h"
#include "UpdateSendingListener.h"
#include "AbnormDeskTermListener.h"

class WinDesktop : public AnEventListener,
                   public ReconnectionListener,
                   public UpdateListener,
                   public ClipboardListener, public UpdateRequestListener,
                   public ConfigReloadListener,
                   public Thread
{
public:
  WinDesktop(ClipboardListener *extClipListener,
             UpdateSendingListener *extUpdSendingListener,
             AbnormDeskTermListener *extDeskTermListener);
  virtual ~WinDesktop();

  void getCurrentUserInfo(StringStorage *desktopName,
                          StringStorage *userName);
  void getFrameBufferProperties(Dimension *dim, PixelFormat *pf);

  void setKeyboardEvent(UINT32 keySym, bool down);
  void setMouseEvent(UINT16 x, UINT16 y, UINT8 buttonMask);
  void setNewClipText(const StringStorage *newClipboard);

protected:
  virtual void execute();
  virtual void onTerminate();

private:
  virtual void onAnObjectEvent();
  virtual void onReconnect(Channel *newChannelTo, Channel *newChannelFrom);
  virtual void onUpdate();
  virtual void onClipboardUpdate(const StringStorage *newClipboard);
  virtual void onUpdateRequest(const Rect *rectRequested, bool incremental);
  virtual void onConfigReload(ServerConfig *serverConfig);

  void applyNewConfiguration();
  bool isRemoteInputAllowed();

  void sendUpdate();

  void freeResource();
  void closeDesktopServerTransport();

  ReconnectingChannel *m_clToSrvChan;
  ReconnectingChannel *m_srvToClChan;
  BlockingGate *m_clToSrvGate;
  BlockingGate *m_srvToClGate;

  DesktopServerWatcher *m_deskServWatcher;
  DesktopSrvDispatcher *m_dispatcher;

  GateKicker *m_gateKicker;
  UpdateHandler *m_updateHandler;
  UserInput *m_userInputClient; 
  UserInput *m_userInput;
  DesktopConfig *m_deskConf;
  WallpaperUtil *m_wallPaper;

  WindowsEvent m_newUpdateEvent;

  Region m_fullReqRegion;
  LocalMutex m_reqRegMutex;

  StringStorage m_receivedClip;
  StringStorage m_sentClip;
  LocalMutex m_storedClipCritSec;

  ClipboardListener *m_extClipListener;
  UpdateSendingListener *m_extUpdSendingListener;
  AbnormDeskTermListener *m_extDeskTermListener;
};

#endif 
