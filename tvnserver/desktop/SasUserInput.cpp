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

#include "SasUserInput.h"
#include "win-system/Environment.h"
#include "util/Log.h"

#define XK_MISCELLANY
#include "rfb/keysymdef.h"

SasUserInput::SasUserInput(UserInputClient *client)
: m_client(client),
  m_ctrlPressed(false),
  m_altPressed(false),
  m_underVista(false)
{
  m_underVista = Environment::isVistaOrLater();
}

SasUserInput::~SasUserInput()
{
}

void SasUserInput::sendInit(BlockingGate *gate)
{
  m_client->sendInit(gate);
}

void SasUserInput::setMouseEvent(const Point *newPos, UINT8 keyFlag)
{
  m_client->setMouseEvent(newPos, keyFlag);
}

void SasUserInput::setNewClipboard(const StringStorage *newClipboard)
{
  m_client->setNewClipboard(newClipboard);
}

void SasUserInput::setKeyboardEvent(UINT32 keySym, bool down)
{
  bool delPressed = false;

  if (m_underVista) {
    switch (keySym) {
    case XK_Alt_L:
    case XK_Alt_R:
      m_altPressed = down;
      break;
    case XK_Control_L:
    case XK_Control_R:
      m_ctrlPressed = down;
      break;
    case XK_Delete:
      delPressed = down;
    }
  }

  if (m_ctrlPressed && m_altPressed && delPressed && m_underVista) {
    Environment::simulateCtrlAltDelUnderVista();
  } else {
    m_client->setKeyboardEvent(keySym, down);
  }
}

void SasUserInput::getCurrentUserInfo(StringStorage *desktopName,
                                         StringStorage *userName)
{
  m_client->getCurrentUserInfo(desktopName, userName);
}
