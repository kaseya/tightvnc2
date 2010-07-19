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

#include "ListBox.h"

ListBox::ListBox()
{
}

ListBox::~ListBox()
{
}

void ListBox::addString(const TCHAR *str)
{
  addString(str, 0);
}

void ListBox::addString(const TCHAR *str, void *tag)
{
  int i = SendMessage(m_hwnd, LB_ADDSTRING, 0, (LPARAM)str);
  setItemData(i, (LPARAM)tag);
}

void ListBox::removeString(int index)
{
  int top = getTopIndex();
  SendMessage(m_hwnd, LB_DELETESTRING, index, NULL);
  setTopIndex(top);
}

int ListBox::getTopIndex()
{
  return (int)SendMessage(m_hwnd, LB_GETTOPINDEX, NULL, NULL);
}

void ListBox::setTopIndex(int index)
{
  SendMessage(m_hwnd, LB_SETTOPINDEX, index, NULL);
}

void ListBox::getItemText(int index, StringStorage *storage)
{
  int length = SendMessage(m_hwnd, LB_GETTEXTLEN, index, NULL);
  TCHAR *buffer = new TCHAR[length + 1];
  SendMessage(m_hwnd, LB_GETTEXT, index, (LPARAM)buffer);
  storage->setString(buffer);
  delete[] buffer;
}

void ListBox::setItemText(int index, const TCHAR *str)
{
  int si = getSelectedIndex();
  int top = getTopIndex();
  UINT topIndex = SendMessage(m_hwnd, LB_GETTOPINDEX, NULL, NULL);
  LPARAM data = getItemData(index);
  removeString(index);
  insertString(index, str, data);
  if (si == index) {
    setSelectedIndex(si);
  }
  setTopIndex(top);
}

void ListBox::appendString(const TCHAR *str, LPARAM data)
{
  long index = SendMessage(m_hwnd, LB_ADDSTRING, 0, (LPARAM)str);
  setItemData(index, data);
}

void ListBox::insertString(int index, const TCHAR *str)
{
  SendMessage(m_hwnd, LB_INSERTSTRING, index, (LPARAM)str);
}

void ListBox::insertString(int index, const TCHAR *str, LPARAM data)
{
  long i = SendMessage(m_hwnd, LB_INSERTSTRING, index, (LPARAM)str);
  setItemData(i, data);
}

void ListBox::setItemData(int index, LPARAM data)
{
  SendMessage(m_hwnd, LB_SETITEMDATA, index, data);
}

LPARAM ListBox::getItemData(int index)
{
  return SendMessage(m_hwnd, LB_GETITEMDATA, index, NULL);
}

int ListBox::getCount()
{
  return SendMessage(m_hwnd, LB_GETCOUNT, NULL, NULL);
}

void ListBox::clear()
{
  SendMessage(m_hwnd, LB_RESETCONTENT, NULL, NULL);
}

int ListBox::getSelectedIndex()
{
  int index = SendMessage(m_hwnd, LB_GETCURSEL, NULL, NULL);
  return (index == LB_ERR) ? -1 : index;
}

void ListBox::setSelectedIndex(int index)
{
  SendMessage(m_hwnd, LB_SETCURSEL, index, NULL);
}
