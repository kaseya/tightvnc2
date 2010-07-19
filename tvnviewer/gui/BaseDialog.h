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

#ifndef _BASE_DIALOG_H_
#define _BASE_DIALOG_H_

#include "Control.h"
#include "util/StringStorage.h"

class BaseDialog
{
public:
  BaseDialog();
  BaseDialog(DWORD resourceId);
  BaseDialog(const TCHAR *resourceName);
  virtual ~BaseDialog();
public:

  void create();

  int show();
  int showModal();

  bool isCreated();

  void hide();
  virtual void kill(int code);
  void setParent(Control *ctrlParent);
  void setResourceName(const TCHAR *resourceName);
  void setResourceId(DWORD id);
  Control *getControl() { return &m_ctrlThis; }
protected:
  void setDefaultPushButton(UINT buttonId);

protected:

  virtual BOOL onInitDialog() = 0;
  virtual BOOL onNotify(UINT controlID, LPARAM data) = 0;
  virtual BOOL onCommand(UINT controlID, UINT notificationID) = 0;
  virtual BOOL onDestroy() = 0;

  virtual BOOL onDrawItem(UINT controlID, LPDRAWITEMSTRUCT dis);
  virtual void onMessageRecieved(UINT uMsg, WPARAM wParam, LPARAM lParam);

  static BOOL CALLBACK dialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
  TCHAR *getResouceName();

protected:

  TCHAR *m_resourceName;        
  DWORD m_resourceId;            
  Control m_ctrlThis;           
  Control *m_ctrlParent;        

  bool m_isModal;
  bool m_isCreated;
};

#endif
