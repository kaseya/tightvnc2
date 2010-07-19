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

#include "UserInputServer.h"
#include "thread/AutoLock.h"

UserInputServer::UserInputServer(BlockingGate *forwGate,
                                 DesktopSrvDispatcher *dispatcher,
                                 AnEventListener *extTerminationListener)
: DesktopServerProto(forwGate),
  IpcServer(dispatcher),
  m_extTerminationListener(extTerminationListener)
{
  bool ctrlAltDelEnabled = true;
  m_userInput = new WindowsUserInput(this, ctrlAltDelEnabled);

  m_dispatcher->registerNewHandle(POINTER_POS_CHANGED, this);
  m_dispatcher->registerNewHandle(CLIPBOARD_CHANGED, this);
  m_dispatcher->registerNewHandle(KEYBOARD_EVENT, this);
  m_dispatcher->registerNewHandle(USER_INFO_REQ, this);
  m_dispatcher->registerNewHandle(USER_INPUT_INIT, this);
}

UserInputServer::~UserInputServer()
{
  m_dispatcher->unregisterHandle(POINTER_POS_CHANGED);
  m_dispatcher->unregisterHandle(CLIPBOARD_CHANGED);
  m_dispatcher->unregisterHandle(KEYBOARD_EVENT);
  m_dispatcher->unregisterHandle(USER_INFO_REQ);
  m_dispatcher->unregisterHandle(USER_INPUT_INIT);

  delete m_userInput;
}

void UserInputServer::onClipboardUpdate(const StringStorage *newClipboard)
{
  AutoLock al(m_forwGate);
  try {
    m_forwGate->writeUInt8(CLIPBOARD_CHANGED);
    sendNewClipboard(newClipboard, m_forwGate);
  } catch (...) {
    m_extTerminationListener->onAnObjectEvent();
  }
}

void UserInputServer::onRequest(UINT8 reqCode, BlockingGate *backGate)
{
  switch (reqCode) {
  case POINTER_POS_CHANGED:
    applyNewPointerPos(backGate);
    break;
  case CLIPBOARD_CHANGED:
    applyNewClipboard(backGate);
    break;
  case KEYBOARD_EVENT:
    applyKeyEvent(backGate);
    break;
  case USER_INFO_REQ:
    ansUserInfo(backGate);
    break;
  case USER_INPUT_INIT:
    serverInit(backGate);
    break;
  default:
    StringStorage errMess;
    errMess.format(_T("Unknown %d protocol code received")
                   _T(" from a UserInputClient"), reqCode);
    throw Exception(errMess.getString());
    break;
  }
}

void UserInputServer::serverInit(BlockingGate *backGate)
{
  UINT8 keyFlags = backGate->readUInt8();
  m_userInput->initKeyFlag(keyFlags);
}

void UserInputServer::applyNewPointerPos(BlockingGate *backGate)
{
  Point newPointerPos;
  UINT8 keyFlags;
  readNewPointerPos(&newPointerPos, &keyFlags, backGate);
  m_userInput->setMouseEvent(&newPointerPos, keyFlags);
}

void UserInputServer::applyNewClipboard(BlockingGate *backGate)
{
  StringStorage newClipboard;
  readNewClipboard(&newClipboard, backGate);
  m_userInput->setNewClipboard(&newClipboard);
}

void UserInputServer::applyKeyEvent(BlockingGate *backGate)
{
  UINT32 keySym;
  bool down;
  readKeyEvent(&keySym, &down, backGate);
  m_userInput->setKeyboardEvent(keySym, down);
}

void UserInputServer::ansUserInfo(BlockingGate *backGate)
{
  StringStorage desktopName, userName;

  m_userInput->getCurrentUserInfo(&desktopName, &userName);
  sendUserInfo(&desktopName, &userName, backGate);
}
