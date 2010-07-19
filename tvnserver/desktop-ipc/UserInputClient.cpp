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

#include "UserInputClient.h"
#include "thread/AutoLock.h"
#include "ReconnectException.h"

UserInputClient::UserInputClient(BlockingGate *forwGate,
                                 DesktopSrvDispatcher *dispatcher,
                                 ClipboardListener *clipboardListener)
: DesktopServerProto(forwGate),
  m_clipboardListener(clipboardListener),
  m_sendMouseFlags(0)
{
  dispatcher->registerNewHandle(CLIPBOARD_CHANGED, this);
}

UserInputClient::~UserInputClient()
{
}

void UserInputClient::onRequest(UINT8 reqCode, BlockingGate *backGate)
{
  switch (reqCode) {
  case CLIPBOARD_CHANGED:
    {
      StringStorage newClipboard;
      readNewClipboard(&newClipboard, backGate);
      m_clipboardListener->onClipboardUpdate(&newClipboard);
    }
    break;
  default:
    StringStorage errMess;
    errMess.format(_T("Unknown %d protocol code received from a pipe ")
                   _T("UserInputServer"), (int)reqCode);
    throw Exception(errMess.getString());
    break;
  }
}

void UserInputClient::sendInit(BlockingGate *gate)
{
  AutoLock al(gate);
  gate->writeUInt8(USER_INPUT_INIT);
  gate->writeUInt8(m_sendMouseFlags);
}

void UserInputClient::setMouseEvent(const Point *newPos, UINT8 keyFlag)
{
  AutoLock al(m_forwGate);
  try {
    m_forwGate->writeUInt8(POINTER_POS_CHANGED);
    sendNewPointerPos(newPos, keyFlag, m_forwGate);
    m_sendMouseFlags = keyFlag;
  } catch (ReconnectException &) {
  }
}

void UserInputClient::setNewClipboard(const StringStorage *newClipboard)
{
  AutoLock al(m_forwGate);
  try {
    m_forwGate->writeUInt8(CLIPBOARD_CHANGED);
    sendNewClipboard(newClipboard, m_forwGate);
  } catch (ReconnectException &) {
  }
}

void UserInputClient::setKeyboardEvent(UINT32 keySym, bool down)
{
  AutoLock al(m_forwGate);
  try {
    m_forwGate->writeUInt8(KEYBOARD_EVENT);
    sendKeyEvent(keySym, down, m_forwGate);
  } catch (ReconnectException &) {
  }
}

void UserInputClient::getCurrentUserInfo(StringStorage *desktopName,
                                         StringStorage *userName)
{
  AutoLock al(m_forwGate);
  try {
    m_forwGate->writeUInt8(USER_INFO_REQ);
    readUserInfo(desktopName, userName, m_forwGate);
  } catch (ReconnectException &) {
  }
}
