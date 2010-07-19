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

#ifndef _FILE_H_
#define _FILE_H_

#include "util/CommonHeader.h"
#include "util/inttypes.h"

class File
{
public:

  const static int MODE_READ  = 1;
  const static int MODE_WRITE = 2;

  File(const TCHAR *pathname);
  File(const TCHAR *parent, const TCHAR *child);
  ~File();

  bool canRead() const;

  bool canWrite() const;

  bool createNewFile() const;

  bool remove() const;

  bool exists() const;

  void getName(StringStorage *name) const;

  void getFileExtension(StringStorage *ext) const;

  bool isFile() const;

  bool isDirectory() const;

  void getPath(StringStorage *pathname) const;

  UINT64 lastModified() const;

  UINT64 length() const;

  bool list(StringStorage *fileList, UINT32 *filesCount) const;

  static bool listRoots(StringStorage *rootList, UINT32 *rootsCount);

  bool mkdir() const;

  bool renameTo(const TCHAR *destPathName);
  bool renameTo(File *dest);

  bool setLastModified(INT64 time);

  bool truncate();

private:

  bool getFileInfo(WIN32_FIND_DATA *fileInfo) const;
  bool tryCreateFile(DWORD desiredAccess, DWORD creationDisposition) const;

public:
  static TCHAR s_separatorChar;

protected:
  StringStorage m_pathName;
};

#endif
