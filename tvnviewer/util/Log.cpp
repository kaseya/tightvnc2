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

#include "Log.h"

Log::Log()
{
}

Log::~Log()
{
}

void Log::interror(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(INTERR, fmt, vl);
  va_end(vl);
}

void Log::error(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(ERROR, fmt, vl);
  va_end(vl);
}

void Log::warning(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(WARN, fmt, vl);
  va_end(vl);
}

void Log::message(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(MSG, fmt, vl);
  va_end(vl);
}

void Log::info(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(INFO, fmt, vl);
  va_end(vl);
}

void Log::detail(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(DETAIL, fmt, vl);
  va_end(vl);
}

void Log::debug(const TCHAR *fmt, ...)
{
  va_list vl;
  va_start(vl, fmt);
  Log::vprint(DEBUG, fmt, vl);
  va_end(vl);
}

#pragma warning(push)
#pragma warning(disable:4996)

void Log::vprint(int logLevel, const TCHAR *fmt, va_list argList)
{
  Log *instance = getInstance();
  if (instance == 0) {
    return;
  }

  StringStorage timeString(_T("[Temporary unavaliable]"));
  SYSTEMTIME st;
  GetLocalTime(&st);
  timeString.format(_T("%.4d-%.2d-%.2d %.2d:%.2d:%.2d"),
                    st.wYear, st.wMonth, st.wDay,
                    st.wHour, st.wMinute, st.wSecond);

  const TCHAR logLevelSignature[] = _T("@!*+-:    xxxxxx");
  TCHAR sig = logLevelSignature[logLevel & 0x0F];

  int count = _vsctprintf(fmt, argList);
  TCHAR *formattedString = new TCHAR[count + 1];
  _vstprintf(formattedString, fmt, argList);

  StringStorage message;
  message.format(_T("[%4d] %s %c %s"),
                 GetCurrentThreadId(),
                 timeString.getString(),
                 sig,
                 formattedString);
  const TCHAR badCharacters[] = {13, 10, 0};
  message.removeChars(badCharacters, sizeof(badCharacters) / sizeof(TCHAR));
  delete[] formattedString;

  const TCHAR endLine[3] = {13, 10, 0};
  message.appendString(endLine);

  instance->flushLine(logLevel, message.getString());
}

#pragma warning(pop)
