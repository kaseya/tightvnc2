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

#ifndef _FILE_INFO_H_
#define _FILE_INFO_H_

#include "util/StringStorage.h"
#include "util/inttypes.h"
#include "file-lib/File.h"

class FileInfo
{
public:

  const static int DIRECTORY  = 0x1;
  const static int EXECUTABLE = 0x2;

public:

  FileInfo();

  FileInfo(UINT64 size, UINT64 modTime,
           UINT16 flags, const TCHAR *fileName);

  FileInfo(const File *file);

  bool isDirectory() const;

  bool isExecutable() const;

  void setLastModified(UINT64 time);

  void setSize(UINT64 size);

  void setFlags(UINT16 flags);

  void setFileName(const TCHAR *fileName);

  UINT64 lastModified() const;

  UINT64 getSize() const;

  UINT16 getFlags() const;

  const TCHAR *getFileName() const;

protected:
  UINT64 m_sizeInBytes;
  UINT64 m_lastModified;
  UINT16 m_flags;
  StringStorage m_fileName;
};

#endif
