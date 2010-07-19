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

#ifndef _FILE_INFO_LIST_VIEW_H_
#define _FILE_INFO_LIST_VIEW_H_

#include "gui/ListView.h"
#include "ft-common/FileInfo.h"

class FileInfoListView : public ListView
{
public:
  FileInfoListView();
  virtual ~FileInfoListView();

  virtual void setWindow(HWND hwnd);

  void addItem(int index, FileInfo *fileInfo);

  void addRange(FileInfo **filesInfo, size_t count);

  FileInfo *getSelectedFileInfo();

protected:

  void loadImages();

protected:
  HIMAGELIST m_smallImageList;
private:
  static LRESULT CALLBACK s_newWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  static const int IMAGE_FOLDER_UP_INDEX = 0;
  static const int IMAGE_FOLDER_INDEX = 1;
  static const int IMAGE_FILE_INDEX = 2;
};

#endif
