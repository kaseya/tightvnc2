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

#ifndef _DATE_TIME_H_
#define _DATE_TIME_H_

#include "inttypes.h"
#include "winhdr.h"
#include "StringStorage.h"

class DateTime
{
public:
  DateTime();

  DateTime(UINT64 timeValue);

  DateTime(FILETIME ft);

  UINT64 getTime() const;

  DateTime operator- (const DateTime &d2);
  DateTime operator+ (const DateTime &d2);

  void toFileTime(LPFILETIME ft) const;

  void toUtcSystemTime(LPSYSTEMTIME st) const;

  void toLocalSystemTime(LPSYSTEMTIME st) const;
  void toString(StringStorage *out) const;

  static DateTime now();

protected:
  UINT64 m_timeValue;
};

#endif
