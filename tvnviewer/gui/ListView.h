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

#ifndef __LISTVIEW_H_
#define __LISTVIEW_H_

#include "util/CommonHeader.h"
#include <commctrl.h>
#include "Control.h"

typedef struct
{
  TCHAR * caption;
  int index;
  LPARAM tag;
} ListViewItem;

class ListView : public Control
{
public:

  void addColumn(int index, const TCHAR *caption, int width, int fmt);
  void addColumn(int index, const TCHAR *caption, int width);

  ListViewItem getItem(int index);

  int getCount() { return ListView_GetItemCount(m_hwnd); }

  void addItem(int index, const TCHAR *caption);

  void addItem(int index, const TCHAR *caption, LPARAM tag);

  void addItem(int index, const TCHAR *caption, LPARAM tag, int imageIndex);

  void removeItem(int i);

  void clear();

  void setSubItemText(int index, int subIndex, const TCHAR *caption);

  void setItemData(int index, LPARAM tag);

  LPARAM getItemData(int index);

  ListViewItem getSelectedItem();

  int getSelectedIndex();

  void selectItem(int index);

  void setFullRowSelectStyle(bool fullRowSelect);

  void allowMultiSelection(bool allow);

  unsigned int getSelectedItemsCount();

  void getSelectedItemsIndexes(int *indexes);

protected:

  void setExStyle(DWORD style);
  DWORD getExStyle();

  void addExStyle(DWORD style);
  void removeExStyle(DWORD style);
};

#endif
