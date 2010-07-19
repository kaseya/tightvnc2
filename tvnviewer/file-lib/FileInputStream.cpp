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

#include "FileInputStream.h"
#include "FileSeeker.h"
#include "win-system/Environment.h"

FileInputStream::FileInputStream(const File *file)
: m_file(file)
{
  StringStorage pathname;

  m_file->getPath(&pathname);

  m_fd.m_fd = CreateFile(pathname.getString(),
                         GENERIC_READ,
                         0,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);

  if (!m_fd.valid()) {
    StringStorage errText;
    Environment::getErrStr(&errText);
    throw IOException(errText.getString());
  }
}

FileInputStream::FileInputStream(HANDLE handle)
: m_file(0)
{
  m_fd.m_fd = handle;
}

FileInputStream::~FileInputStream()
{
  try { close(); } catch (IOException &) { }
}

size_t FileInputStream::read(void *buffer, size_t len)
{
  DWORD read;

  UINT8 *byteBuffer = (UINT8 *)buffer;

  if (ReadFile(m_fd.m_fd, byteBuffer, len, &read, NULL) == FALSE) {
    StringStorage errText;
    Environment::getErrStr(&errText);
    throw IOException(errText.getString());
  }

  if (read == 0) {
    throw EOFException();
  }

  return read;
}

void FileInputStream::close()
{
  if (m_file == 0) {
    return;
  }

  if (m_fd.valid()) {
    CloseHandle(m_fd.m_fd);
    m_fd.m_fd = INVALID_HANDLE_VALUE;
  } else {
    throw IOException(_T("Trying to close not valid file descriptor"));
  }
}

const FileDescriptor *FileInputStream::getFD() const
{
  return &m_fd;
}
