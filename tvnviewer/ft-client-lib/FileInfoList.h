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

#ifndef _FILE_INFO_LIST_H_
#define _FILE_INFO_LIST_H_

#include "ft-common/FileInfo.h"

class FileInfoList
{
public:

  FileInfoList(FileInfo fileInfo);

  FileInfoList(const FileInfo *filesInfo, size_t count);

  ~FileInfoList();

  void setChild(const FileInfo *filesInfo, UINT32 count);

  FileInfoList *getChild();

  FileInfoList *getParent();

  FileInfoList *getRoot();

  FileInfoList *getFirst();

  FileInfoList *getNext();

  FileInfoList *getPrev();

  void setFileInfo(FileInfo fileInfo);

  FileInfo *getFileInfo();

  void getAbsolutePath(StringStorage *storage, TCHAR directorySeparator);

protected:

  void setNext(FileInfoList *next);
  void setPrev(FileInfoList *prev);

  static FileInfoList *fromArray(const FileInfo *filesInfo, UINT32 count);

protected:

  FileInfoList *m_child;
  FileInfoList *m_parent;

  FileInfoList *m_next;
  FileInfoList *m_prev;

  FileInfo m_fileInfo;
};

#endif
