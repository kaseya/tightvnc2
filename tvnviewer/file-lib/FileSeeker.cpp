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

#include "FileSeeker.h"

FileSeeker::FileSeeker(const FileDescriptor *fd)
{
  m_fd = fd;
}

FileSeeker::~FileSeeker()
{
}

bool FileSeeker::seek(INT64 n)
{
  HANDLE file = m_fd->m_fd;

  LARGE_INTEGER fileSize;
  GetFileSizeEx(file, &fileSize);
  if (fileSize.QuadPart < n) {
    throw IOException(_T("Specified file pointer position is more than file length"));
  }

  LARGE_INTEGER li;
  li.QuadPart = n;
  li.LowPart = SetFilePointer(file, li.LowPart, &li.HighPart, FILE_CURRENT);
  if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
    throw IOException(_T("Cannot set file pointer to specified position"));
  }

  return true;
}
