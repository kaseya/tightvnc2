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

#include "AnonymousPipe.h"
#include "win-system/Environment.h"
#include "util/Log.h"

AnonymousPipe::AnonymousPipe(HANDLE hWrite, HANDLE hRead)
: m_hWrite(hWrite),
  m_hRead(hRead),
  m_neededToClose(true)
{
}

AnonymousPipe::~AnonymousPipe()
{
  try {
    close();
  } catch (Exception &e) {
    Log::error(_T("The close() function failed at AnonymousPipe destructor: %s"),
               e.getMessage());
  }
}

void AnonymousPipe::close()
{
  bool wrSuc = true;
  bool rdSuc = true;
  StringStorage wrErrText, rdErrText;
  if (m_hWrite != 0 && m_neededToClose) {
    if (CloseHandle(m_hWrite) == 0) {
      Environment::getErrStr(_T("Cannot close anonymous pipe write handle."),
                             &wrErrText);
      wrSuc = false;
    }
  }
  m_hWrite = 0;
  if (m_hRead != 0 && m_neededToClose) {
    if (CloseHandle(m_hRead) == 0) {
      Environment::getErrStr(_T("Cannot close anonymous pipe read handle."),
                             &wrErrText);
      rdSuc = false;
    }
  }
  m_hRead = 0;
  if (!wrSuc || !rdSuc) {
    StringStorage errMess;
    errMess.format(_T("AnonymousPipe::close() funciton has failed (%s %s)"),
                   wrErrText.getString(), rdErrText.getString());
    throw Exception(errMess.getString());
  }
}

size_t AnonymousPipe::read(void *buffer, size_t len) throw(IOException)
{
  DWORD bytesRead;
  if (ReadFile(m_hRead, buffer, len, &bytesRead, 0) == 0) {
    StringStorage errText;
    Environment::getErrStr(_T("Anonymous pipe read() function failed"),
                           &errText);
    throw IOException(errText.getString());
  }
  return bytesRead;
}

size_t AnonymousPipe::write(const void *buffer, size_t len) throw(IOException)
{
  DWORD bytesWritten;
  if (WriteFile(m_hWrite, buffer, len, &bytesWritten, 0) == 0) {
    StringStorage errText;
    Environment::getErrStr(_T("Anonymous pipe write() function failed"),
                           &errText);
    throw IOException(errText.getString());
  }
  return bytesWritten;
}

HANDLE AnonymousPipe::getWriteHandle() const
{
  return m_hWrite;
}

HANDLE AnonymousPipe::getReadHandle() const
{
  return m_hRead;
}

void AnonymousPipe::assignHandlesFor(HANDLE hTargetProc, bool neededToClose)
{
  HANDLE hSrcProc = GetCurrentProcess();
  HANDLE hWrite = 0, hRead = 0;
  if (DuplicateHandle(hSrcProc, m_hWrite, hTargetProc, &hWrite, 0, FALSE,
                      DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS) == 0) {
    StringStorage errText;
    Environment::getErrStr(_T("Cannot dupplicate write")
                           _T(" handle for the anonymous pipe"),
                           &errText);
    throw Exception(errText.getString());
  }
  m_hWrite = hWrite;
  if (DuplicateHandle(hSrcProc, m_hRead, hTargetProc, &hRead, 0, FALSE,
                      DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS) == 0) {
    StringStorage errText;
    Environment::getErrStr(_T("Cannot dupplicate read")
                           _T(" handle for the anonymous pipe"),
                           &errText);
    throw Exception(errText.getString());
  }
  m_hRead = hRead;
  m_neededToClose = neededToClose;
}
