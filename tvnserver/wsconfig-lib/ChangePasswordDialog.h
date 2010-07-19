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

#ifndef _CHANGE_PASSWORD_DIALOG_H_
#define _CHANGE_PASSWORD_DIALOG_H_

#include "gui/BaseDialog.h"
#include "gui/TextBox.h"
#include "gui/BalloonTip.h"
#include "util/StringStorage.h"

class ChangePasswordDialog : public BaseDialog
{
public:
  ChangePasswordDialog(Control *parent, bool isNewPassword);
  virtual ~ChangePasswordDialog();

  const TCHAR *getPasswordInPlainText() const;
protected:

  virtual BOOL onInitDialog();
  virtual BOOL onCommand(UINT cID, UINT nID);
  virtual BOOL onNotify(UINT controlID, LPARAM data) { return TRUE; }
  virtual BOOL onDestroy() { return TRUE; }

  void onOkButtonClick();
  void onCancelButtonClick();

private:
  void initControls();

protected:
  TextBox m_password1;
  TextBox m_password2;
  StringStorage m_passwordText;
  BalloonTip m_passwordEmptyTooltip;
  BalloonTip m_passwordsNotMatchTooltip;
  bool m_newPassword;
  bool m_allowEmptyPassword;
};

#endif
