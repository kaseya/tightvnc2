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

#include "FileInfoListView.h"
#include "util/DateTime.h"
#include "vncviewer/res/resource.h"
#include "util/ResourceLoader.h"
#include <crtdbg.h>
#include <stdio.h>

FileInfoListView::FileInfoListView()
: m_smallImageList(0)
{
}

FileInfoListView::~FileInfoListView()
{
  if (m_smallImageList != 0) {
    ImageList_Destroy(m_smallImageList);
  }
}

void FileInfoListView::setWindow(HWND hwnd)
{
  ListView::setWindow(hwnd);

  ListView::addColumn(0, _T("Filename"), 135);
  ListView::addColumn(1, _T("Size"), 80, LVCFMT_RIGHT);
  ListView::addColumn(2, _T("Modification time"), 115);

  setFullRowSelectStyle(true);

  loadImages();

  ListView_SetImageList(m_hwnd, m_smallImageList, LVSIL_SMALL);

  Control::replaceWindowProc(FileInfoListView::s_newWndProc);
}

void FileInfoListView::addItem(int index, FileInfo *fileInfo)
{
  const TCHAR *filename = fileInfo->getFileName();

  int imageIndex = IMAGE_FILE_INDEX;

  if (_tcscmp(fileInfo->getFileName(), _T("..")) == 0) {
    imageIndex = IMAGE_FOLDER_UP_INDEX;
  } else if (fileInfo->isDirectory()) {
    imageIndex = IMAGE_FOLDER_INDEX;
  }

  ListView::addItem(index, filename, (LPARAM)fileInfo, imageIndex);

  StringStorage sizeString(_T("<Folder>"));
  StringStorage modTimeString(_T(""));

  if (!fileInfo->isDirectory()) {

    UINT64 fileSize = fileInfo->getSize();

    if (fileSize <= 1024) {
      sizeString.format(_T("%ld B"), fileSize);
    } else if ((fileSize > 1024) && (fileSize <= 1024 * 1024)) {
      sizeString.format(_T("%4.2f KB"), (double)fileSize / 1024.0);
    } else if (fileSize > 1024 * 1024) {
      sizeString.format(_T("%4.2f MB"), (double)fileSize / (1024.0 * 1024));
    }

    DateTime dateTime(fileInfo->lastModified());

    dateTime.toString(&modTimeString);
  }

  ListView::setSubItemText(index, 1, sizeString.getString());
  ListView::setSubItemText(index, 2, modTimeString.getString());
}

void FileInfoListView::addRange(FileInfo **filesInfo, size_t count)
{
  int index = max(0, (getCount() - 1));
  size_t i = 0;
  FileInfo *arr = *filesInfo;

  for (i = 0; i < count; i++) {
    FileInfo *fi = &arr[i];
    if (fi->isDirectory()) {
      addItem(index++, fi);
    } 
  } 

  for (i = 0; i < count; i++) {
    FileInfo *fi = &arr[i];
    if (!fi->isDirectory()) {
      addItem(index++, fi);
    } 
  } 
} 

FileInfo *FileInfoListView::getSelectedFileInfo()
{
  int si = getSelectedIndex();
  if (si == -1) {
    return NULL;
  }
  return (FileInfo *)getSelectedItem().tag;
}

void FileInfoListView::loadImages()
{
  if (m_smallImageList != NULL) {
    ImageList_Destroy(m_smallImageList);
  }

  m_smallImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
                                      GetSystemMetrics(SM_CYSMICON),
                                      ILC_MASK, 1, 1);

  HICON icon;

  ResourceLoader *rLoader = ResourceLoader::getInstance();

  icon = rLoader->loadIcon(MAKEINTRESOURCE(IDI_FILEUP));
  _ASSERT(icon != NULL);
  ImageList_AddIcon(m_smallImageList, icon);
  DestroyIcon(icon);

  icon = rLoader->loadIcon(MAKEINTRESOURCE(IDI_FOLDER_ICON));
  _ASSERT(icon != NULL);
  ImageList_AddIcon(m_smallImageList, icon);
  DestroyIcon(icon);

  icon = rLoader->loadIcon(MAKEINTRESOURCE(IDI_FILE_ICON));
  _ASSERT(icon != NULL);
  ImageList_AddIcon(m_smallImageList, icon);
  DestroyIcon(icon);
}

LRESULT CALLBACK FileInfoListView::s_newWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  FileInfoListView *_this = (FileInfoListView *)GetWindowLong(hwnd, GWL_USERDATA);

  switch (uMsg) {
  case WM_GETDLGCODE:
    LRESULT lres = CallWindowProc(_this->m_defWindowProc, hwnd, uMsg, wParam, lParam);
    if (lParam &&
        ((MSG *)lParam)->message == WM_KEYDOWN &&
        ((MSG *)lParam)->wParam == VK_RETURN) {
      lres = DLGC_WANTMESSAGE;
    }
    return lres;
  } 

  return CallWindowProc(_this->m_defWindowProc, hwnd, uMsg, wParam, lParam);
}
