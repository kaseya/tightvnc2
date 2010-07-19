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

#include "ControlAuthDialog.h"

#include "tvnserver/resource.h"

#include "gui/TextBox.h"

ControlAuthDialog::ControlAuthDialog()
: BaseDialog(IDD_CONTROL_AUTH)
{
}

ControlAuthDialog::~ControlAuthDialog()
{
}

const TCHAR *ControlAuthDialog::getPassword() const
{
  return m_password.getString();
}

BOOL ControlAuthDialog::onInitDialog()
{
  m_password.setString(0);

  SetForegroundWindow(m_ctrlThis.getWindow());
  SetFocus(GetDlgItem(m_ctrlThis.getWindow(), IDC_PASSWORD_EDIT));

  return TRUE;
}

BOOL ControlAuthDialog::onNotify(UINT controlID, LPARAM data)
{
  return TRUE;
}

BOOL ControlAuthDialog::onCommand(UINT controlID, UINT notificationID)
{
  switch (controlID) {
  case IDOK:
  case IDCANCEL:
    {
      TextBox passwordTB;
      passwordTB.setWindow(GetDlgItem(m_ctrlThis.getWindow(), IDC_PASSWORD_EDIT));
      passwordTB.getText(&m_password);
    }
    kill(controlID);
    break;
  }
  return TRUE;
}

BOOL ControlAuthDialog::onDestroy()
{
  return TRUE;
}
