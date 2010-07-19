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

#include "BaseDialog.h"

#include "util/CommonHeader.h"

#include <commctrl.h>
#include <crtdbg.h>

BaseDialog::BaseDialog()
: m_ctrlParent(NULL), m_resourceName(0), m_resourceId(0)
{
}

BaseDialog::BaseDialog(DWORD resourceId)
: m_ctrlParent(NULL), m_resourceName(0), m_resourceId(resourceId)
{
}

BaseDialog::BaseDialog(const TCHAR *resourceName)
: m_ctrlParent(NULL), m_resourceName(0), m_resourceId(0)
{
  setResourceName(resourceName);
}

BaseDialog::~BaseDialog()
{
  if (m_resourceName != 0) {
    free(m_resourceName);
  }
}

void BaseDialog::setResourceName(const TCHAR *resourceName)
{
  if (m_resourceName != 0) {
    free(m_resourceName);
  }

  m_resourceName = _tcsdup(resourceName);
}

void BaseDialog::setResourceId(DWORD id)
{
  m_resourceId = id;
}

void BaseDialog::setDefaultPushButton(UINT buttonId)
{
  SendMessage(m_ctrlThis.getWindow(), DM_SETDEFID, buttonId, 0);
}

void BaseDialog::setParent(Control *ctrlParent)
{
  m_ctrlParent = ctrlParent;
}

int BaseDialog::show()
{
  if (m_ctrlThis.getWindow() == NULL) {
    create();
  } else {
    m_ctrlThis.setForeground();
  }
  return 0;
}

void BaseDialog::hide()
{
  m_ctrlThis.setVisible(false);
}

void BaseDialog::kill(int code)
{
  if (!m_isModal) {
    DestroyWindow(m_ctrlThis.getWindow());
  } else {
    EndDialog(m_ctrlThis.getWindow(), code);
  }
  m_ctrlThis.setWindow(NULL);
}

void BaseDialog::create()
{
  HWND window, parentWindow = NULL;

  if (m_ctrlParent != NULL) {
    parentWindow = m_ctrlParent->getWindow();
  }

  window = CreateDialogParam(GetModuleHandle(NULL), getResouceName(),
                             parentWindow, dialogProc, (LPARAM)this);

  m_isModal = false;

  _ASSERT(window != NULL);
}

int BaseDialog::showModal()
{
  int result = 0;
  if (m_ctrlThis.getWindow() == NULL) {
    m_isModal = true;
    HWND parentWindow = (m_ctrlParent != NULL) ? m_ctrlParent->getWindow() : NULL;
    result = DialogBoxParam(GetModuleHandle(NULL),
                            getResouceName(),
                            parentWindow, dialogProc, (LPARAM)this);
  } else {
    m_ctrlThis.setVisible(true);
    m_ctrlThis.setForeground();
  }

  if (result == -1) {
  }

  return result;
}

bool BaseDialog::isCreated()
{
  return m_ctrlThis.getWindow() != 0;
}

BOOL BaseDialog::onDrawItem(UINT controlID, LPDRAWITEMSTRUCT dis)
{
  return TRUE;
}

void BaseDialog::onMessageRecieved(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
}

BOOL CALLBACK BaseDialog::dialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{  
  BaseDialog *_this;
  if (uMsg == WM_INITDIALOG) {
    _this = (BaseDialog *)lParam;
    SetWindowLong(hwnd, GWL_USERDATA, (LONG)_this);
    _this->m_ctrlThis.setWindow(hwnd);
  } else {
    _this = (BaseDialog *)GetWindowLong(hwnd, GWL_USERDATA);
    if (_this == 0) {
      return FALSE;
    }
  }

  _this->onMessageRecieved(uMsg, wParam, lParam);

  switch (uMsg) {
  case WM_INITDIALOG:
    return _this->onInitDialog();
  case WM_NOTIFY:
    return _this->onNotify(LOWORD(wParam), lParam);
  case WM_COMMAND:
    return _this->onCommand(LOWORD(wParam), HIWORD(wParam));
  case WM_DESTROY:
    return _this->onDestroy();
  case WM_DRAWITEM:
    return _this->onDrawItem(wParam, (LPDRAWITEMSTRUCT)lParam);
  }

  return FALSE;
}

TCHAR *BaseDialog::getResouceName()
{
  if (m_resourceId != 0) {
    return MAKEINTRESOURCE(m_resourceId);
  }
  return m_resourceName;
}
