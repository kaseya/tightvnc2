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

#ifndef _BASE_LOG_H_
#define _BASE_LOG_H_

#include "util/CommonHeader.h"
#include "util/Singleton.h"

class Log : private Singleton<Log>
{
public:
  Log();
  virtual ~Log();

  static void interror(const TCHAR *fmt, ...);
  static void error(const TCHAR *fmt, ...);
  static void warning(const TCHAR *fmt, ...);
  static void message(const TCHAR *fmt, ...);
  static void info(const TCHAR *fmt, ...);
  static void detail(const TCHAR *fmt, ...);
  static void debug(const TCHAR *fmt, ...);

protected:
  static void vprint(int logLevel, const TCHAR *fmt, va_list argList);

  virtual void flushLine(int logLevel, const TCHAR *line) = 0;

  static const int INTERR = 0;
  static const int ERR = 1;
  static const int WARN = 2;
  static const int MSG = 3;
  static const int INFO = 4;
  static const int DETAIL = 5;
  static const int DEBUG = 9;
};

#endif
