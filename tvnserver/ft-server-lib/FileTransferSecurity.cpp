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

#include "FileTransferSecurity.h"

#include "server-config-lib/Configurator.h"

FileTransferSecurity::FileTransferSecurity(WinDesktop *desktop)
: m_hasAccess(false), m_desktop(desktop)
{
  m_desktop = desktop;
}

FileTransferSecurity::~FileTransferSecurity()
{
}

void FileTransferSecurity::beginMessageProcessing()
{
  bool runAsService = Configurator::getInstance()->getServiceFlag();

  if (!runAsService) {
    m_hasAccess = true;
  } else {
    try {
      StringStorage userName, desktopName;

      if (m_desktop != NULL) {
        m_desktop->getCurrentUserInfo(&desktopName, &userName);
      }

      desktopName.toLowerCase();

      if (!desktopName.isEqualTo(_T("default"))) {
        throw Exception(_T("Desktop is not default desktop."));
      }

      impersonateAsLoggedUser();

      m_hasAccess = true;
    } catch (...) {
      m_hasAccess = false;
    } 
  } 
}

void FileTransferSecurity::throwIfAccessDenied()
{
  if (!m_hasAccess) {
    throw Exception(_T("Access denied."));
  } else if (!Configurator::getInstance()->getServerConfig()->isFileTransfersEnabled()) {
    throw Exception(_T("File transfers are disabled on server side."));
  }
}

void FileTransferSecurity::endMessageProcessing()
{
  if (Configurator::getInstance()->getServiceFlag() && m_hasAccess) {
    try {
      revertToSelf();
    } catch (...) {
    } 
  } 
}
