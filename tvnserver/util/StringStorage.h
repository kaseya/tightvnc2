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

#ifndef _STRING_BUFFER_H_
#define _STRING_BUFFER_H_

#include "winhdr.h"
#include <wchar.h>

class StringStorage
{
public:
  StringStorage();

  StringStorage(const TCHAR *string);

  StringStorage(const StringStorage &stringBuffer);

  virtual ~StringStorage();

  bool isEmpty() const;
  bool isNull() const;

  bool endsWith(TCHAR postfix) const;

  bool beginsWith(TCHAR prefix) const;

  void getSubstring(StringStorage *substr, int startIndex, int endIndex) const;

  void trim();

  bool toAnsiString(char *buffer, size_t size) const;

  bool toUnicodeString(WCHAR *unicodeBuffer, size_t *length) const;

  void toUTF8String(char *buffer, int *sizeInBytes);

  void fromUTF8String(char *buffer, int sizeInBytes);

  void fromAnsiString(const char *string);
  void fromUnicodeString(const WCHAR *string);

  void appendString(const TCHAR *string);

  void appendChar(TCHAR c);

  virtual void setString(const TCHAR *string);

  const TCHAR *getString() const;

  size_t getLength() const;

  size_t getSize() const;

  bool isEqualTo(const TCHAR *other) const;

  bool isEqualTo(const StringStorage *other) const;

  bool split(const TCHAR *delimiters, StringStorage *stringArray, size_t *arrayLength) const;

  int findChar(const TCHAR c);

  int findOneOf(const TCHAR *string);

  int findLast(const TCHAR c);

  void removeChars(const TCHAR badCharacters[], size_t count);

  void truncate(size_t count);

  void toLowerCase();

  void format(const TCHAR *format, ...);

  void operator = (const StringStorage &other);

  bool operator == (const StringStorage &str) const;
  bool operator < (const StringStorage &str) const;

  void replaceChar(TCHAR oldChar, TCHAR newChar);

protected:

  void release();

protected:

  TCHAR *m_buffer;
  size_t m_length;
};

#endif
