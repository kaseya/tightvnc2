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

#include "FileOutputStream.h"

FileOutputStream::FileOutputStream(const File *file)
: m_file(file)
{
  StringStorage pathname;
  m_file->getPath(&pathname);

  DWORD shareMode = FILE_SHARE_READ;
  m_fd.m_fd = CreateFile(pathname.getString(),
                         GENERIC_WRITE,
                         shareMode,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
  if (!m_fd.valid()) {
    throw IOException(_T("Cannot open file for writting"));
  }
}

FileOutputStream::~FileOutputStream()
{
  try { close(); } catch (...) { }
}

size_t FileOutputStream::write(const void *buffer, size_t len)
{
  DWORD written = 0;
  if (WriteFile(m_fd.m_fd, buffer, len, &written, NULL) == FALSE) {
    throw IOException(_T("Cannot write data to file"));
  }
  return (size_t)written;
}

void FileOutputStream::close()
{
  if (m_fd.valid()) {
    CloseHandle(m_fd.m_fd);
    m_fd.m_fd = INVALID_HANDLE_VALUE;
  } else {
    throw IOException(_T("Trying to close not valid file descriptor"));
  }
}

const FileDescriptor *FileOutputStream::getFD() const
{
  return &m_fd;
}
