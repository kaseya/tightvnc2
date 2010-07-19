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

#include "StringStorage.h"
#include "CommonHeader.h"
#include <stdio.h>

#include <crtdbg.h>

StringStorage::StringStorage()
: m_buffer(0), m_length(0)
{
}

StringStorage::StringStorage(const TCHAR *string)
: m_buffer(0), m_length(0)
{
  setString(string);
}

StringStorage::StringStorage(const StringStorage &stringBuffer)
: m_buffer(0), m_length(0)
{
  *this = stringBuffer;
}

StringStorage::~StringStorage()
{
  release();
}

bool StringStorage::isEmpty() const
{
  return isNull() || m_length == 0;
}

bool StringStorage::isNull() const
{
  return m_buffer == 0;
}

bool StringStorage::endsWith(TCHAR postfix) const
{
  if (isEmpty()) {
    return false;
  }
  TCHAR lastCharacter = m_buffer[m_length - 1];
  return (lastCharacter == postfix);
}

bool StringStorage::beginsWith(TCHAR prefix) const
{
  if (isEmpty()) {
    return false;
  }
  TCHAR firstCharacter = m_buffer[0];
  return (firstCharacter == prefix);
}

void StringStorage::getSubstring(StringStorage *substr, int startIndex, int endIndex) const
{
  if (isNull()) {
    substr->setString(0);
    return;
  }
  startIndex = max(startIndex, 0);
  endIndex = min(endIndex, max(((int)m_length - 1), 0));

  int length = max(endIndex - startIndex + 1, 1);

  _ASSERT(length > 0);

  TCHAR *buffer = new TCHAR[length + 1];

  size_t j = 0;

  for (int i = startIndex; i <= endIndex; i++) {
    buffer[j] = m_buffer[i];
    j++;
  }

  buffer[j] = '\0';

  substr->setString(buffer);

  delete[] buffer;
}

void StringStorage::trim()
{
  TCHAR *buffer = new TCHAR[m_length + 1];

  size_t j = 0;

  for (size_t i = 0; i < m_length; i++) {
    if (!((buffer[i] == ' ') && (buffer[i + 1]))) {
      buffer[j++] = m_buffer[i];
    }
  }

  buffer[j] = _T('\0');

  setString(buffer);

  delete[] buffer;
}

bool StringStorage::toAnsiString(char *buffer, size_t size) const
{
  size_t neededSize = getSize();

#ifdef _UNICODE
  neededSize = WideCharToMultiByte(CP_ACP, 0, m_buffer, m_length, NULL, 0, NULL, NULL);
#endif

  if (size < neededSize) {
    return false;
  }

#ifdef _UNICODE
  WideCharToMultiByte(CP_ACP, 0, m_buffer, m_length, (LPSTR)buffer, size, NULL, NULL);
  buffer[m_length] = '\0';
#else
  memcpy(buffer, m_buffer, neededSize);
#endif
  return true;
}

bool StringStorage::toUnicodeString(WCHAR *unicodeBuffer, size_t *length) const
{
  const TCHAR *input = getString();
  if (*length < m_length) {
    return false;
  }
#ifndef _UNICODE
  MultiByteToWideChar(CP_ACP, 0, input, m_length , unicodeBuffer, m_length );
#else
  memcpy(unicodeBuffer, input, m_length * sizeof(TCHAR));
#endif
  unicodeBuffer[m_length] = _T('\0');
  *length = m_length;
  return true;
}

void StringStorage::toUTF8String(char *buffer, int *sizeInBytes)
{
#ifndef _UNICODE
  WCHAR *unicodeBuffer = new WCHAR[m_length + 1];

  MultiByteToWideChar(CP_ACP, 0, getString(), m_length + 1 , unicodeBuffer, m_length + 1);

  int bufferSizeInBytes = WideCharToMultiByte(CP_UTF8, 0, unicodeBuffer, m_length, NULL, 0, NULL, FALSE);
  *sizeInBytes = bufferSizeInBytes;

  if (buffer != NULL) {
    WideCharToMultiByte(CP_UTF8, 0, unicodeBuffer, m_length + 1, buffer, bufferSizeInBytes, NULL, FALSE);
  }

  delete[] unicodeBuffer;
#else
  int bufferSizeInBytes = WideCharToMultiByte(CP_UTF8, 0, m_buffer, m_length, NULL, 0, NULL, FALSE);
  *sizeInBytes = bufferSizeInBytes;

  if (buffer != NULL) {
    WideCharToMultiByte(CP_UTF8, 0, m_buffer, m_length + 1, buffer, bufferSizeInBytes, NULL, FALSE);
  }
#endif
}

void StringStorage::fromUTF8String(char *buffer, int sizeInBytes)
{
#ifndef _UNICODE
  int length = MultiByteToWideChar(CP_UTF8, 0, buffer, sizeInBytes, NULL, 0);

  WCHAR *unicodeBuffer = new WCHAR[length + 1];
  char *ansiBuffer = new char[length + 1];

  MultiByteToWideChar(CP_UTF8, 0, buffer, sizeInBytes, unicodeBuffer, length + 1);
  unicodeBuffer[length] = '\0';

  WideCharToMultiByte(CP_ACP, 0, unicodeBuffer, length + 1, ansiBuffer, length + 1, NULL, FALSE);

  setString(ansiBuffer);

  delete[] ansiBuffer;
  delete[] unicodeBuffer;
#else
  int length = MultiByteToWideChar(CP_UTF8, 0, buffer, sizeInBytes, NULL, 0);

  WCHAR *unicodeBuffer = new WCHAR[length + 1];

  MultiByteToWideChar(CP_UTF8, 0, buffer, sizeInBytes, unicodeBuffer, length + 1);
  unicodeBuffer[length] = _T('\0');

  setString(unicodeBuffer);

  delete[] unicodeBuffer;
#endif
}

void StringStorage::fromAnsiString(const char *string)
{
#ifndef _UNICODE
  setString(string);
#else
  int length = MultiByteToWideChar(CP_ACP, 0, string, strlen(string) + 1, NULL, 0);

  WCHAR *unicodeBuffer = new WCHAR[length + 1];

  MultiByteToWideChar(CP_ACP, 0, string, strlen(string) + 1, unicodeBuffer, length + 1);
  unicodeBuffer[length] = _T('\0');

  setString(unicodeBuffer);

  delete[] unicodeBuffer;
#endif
}

void StringStorage::fromUnicodeString(const WCHAR *string)
{
#ifndef _UNICODE
  size_t length = wcslen(string);
  size_t numBytes = WideCharToMultiByte(CP_ACP, 0, string, length + 1,
                                        NULL, 0, NULL, NULL);

  char *buffer = new char[numBytes];
  WideCharToMultiByte(CP_ACP, 0, string, length + 1,
                      (LPSTR)buffer, numBytes, NULL, NULL);
  setString(buffer);
  delete[] buffer;
#else
  setString(string);
#endif
}

void StringStorage::appendString(const TCHAR *string)
{
  if (string == 0) {
    return;
  }
  if (isNull()) {
    setString(string);
  } else {
    size_t stringLength = _tcslen(string);
    size_t oldLength = m_length;
    TCHAR *buffer = new TCHAR[oldLength + stringLength + 1];
    memcpy((char *)buffer, m_buffer, oldLength * sizeof(TCHAR));
    memcpy((char *)buffer + oldLength * sizeof(TCHAR), string, (stringLength + 1) * sizeof(TCHAR));
    release();
    m_length = oldLength + stringLength;
    m_buffer = buffer;
  }
}

void StringStorage::appendChar(TCHAR c)
{
  TCHAR string[2] = {c, _T('\0') };

  appendString(string);
}

void StringStorage::setString(const TCHAR *string)
{
  release();

  if (string != 0) {
    m_length = _tcslen(string);
    m_buffer = new TCHAR[m_length + 1];
    memcpy(m_buffer, string, (m_length + 1) * sizeof(TCHAR));
  }
}

const TCHAR *StringStorage::getString() const
{
  return m_buffer;
}

size_t StringStorage::getLength() const
{
  return m_length;
}

size_t StringStorage::getSize() const
{
  return isNull() ? 0 : (getLength() + 1) * sizeof(TCHAR);
}

bool StringStorage::isEqualTo(const StringStorage *other) const
{
  return isEqualTo(other->getString());
}

bool StringStorage::isEqualTo(const TCHAR *other) const
{
  if (other == 0 && getString() == 0) {
    return true;
  }
  if (other == 0 && getString() != 0) {
    return false;
  }
  if (getString() == 0 && other != 0) {
    return false;
  }
  return _tcscmp(getString(), other) == 0;
}

bool StringStorage::split(const TCHAR *delimiters, StringStorage *stringArray, size_t *arrayLength) const
{
  if (this->getLength() == 0) {
    *arrayLength = 0;
    return true;
  }

  StringStorage copy(this->getString());

  size_t chunksCount = 0;

  int index = 0;

  do {
    StringStorage chunk(_T(""));

    index = copy.findOneOf(delimiters);

    if ((index == -1) && (chunksCount == 0)) {
      if (arrayLength != 0) {
        if (stringArray != 0) {
          if (*arrayLength < 1) {
            return false;
          }
        }
        *arrayLength = 1;
      } else {
        return false;
      }
      if (stringArray != 0) {
        stringArray->setString(getString());
      }
      return true;
    }

    if (index == -1) {
      chunk = copy;
    } else {
      copy.getSubstring(&chunk, 0, index - 1);
    }

    copy.getSubstring(&copy, index + 1, copy.getLength() - 1);

    if ((stringArray != NULL) && (chunksCount >= *arrayLength)) {
      return false;
    } else if (stringArray != NULL) {
      stringArray[chunksCount] = chunk;
    }

    chunksCount++;
  } while (index != -1);

  *arrayLength = chunksCount;

  return true;
}

int StringStorage::findChar(const TCHAR c)
{
  for (size_t i = 0; i < m_length; i++) {
    if (m_buffer[i] == c) {
      return i;
    }
  }
  return -1;
}

int StringStorage::findLast(const TCHAR c)
{
  for (int i = m_length - 1; i >= 0; i--) {
    if (m_buffer[i] == c) {
      return i;
    }
  }
  return -1;
}

void StringStorage::removeChars(const TCHAR badCharacters[], size_t count)
{
  TCHAR *newBuffer = new TCHAR[getLength() + 1];

  size_t j = 0;

  for (size_t i = 0; i < getLength(); i++) {
    TCHAR each = m_buffer[i];
    bool badCharacter = false;
    for (size_t k = 0; k < count; k++) {
      if (each == badCharacters[k]) {
        badCharacter = true;
        break;
      }
    }
    if (!badCharacter) {
      newBuffer[j++] = each;
    }
  }

  newBuffer[j] = _T('\0');

  release();

  m_buffer = newBuffer;
  m_length = j;
}

void StringStorage::truncate(size_t count)
{
  count = min(m_length, count);

  getSubstring(this, 0, m_length - count - 1);
}

int StringStorage::findOneOf(const TCHAR *string)
{
  for (size_t i = 0; i < m_length; i++) {
    for (size_t j = 0; j < _tcslen(string); j++) {
      if (m_buffer[i] == string[j]) {
        return i;
      }
    }
  }
  return -1;
}

void StringStorage::toLowerCase()
{
  for (size_t i = 0; i < m_length; i++) {
    if (_istalpha(m_buffer[i]) != 0) {
      m_buffer[i] = _totlower(m_buffer[i]);
    }
  }
}

void StringStorage::format(const TCHAR *format, ...)
{
  va_list vl;

  va_start(vl, format);
  int count = _vsctprintf(format, vl);
  va_end(vl);

  if (m_length < (size_t)count) {
    release();
    m_buffer = new TCHAR[count + 1];
  }

  m_length = (size_t)count;

  va_start(vl, format);
  _vstprintf_s(m_buffer, count + 1, format, vl);
  va_end(vl);
}

void StringStorage::operator= (const StringStorage &other)
{
  setString(other.getString());
}

bool StringStorage::operator == (const StringStorage &str) const
{
  return isEqualTo(&str);
}

bool StringStorage::operator < (const StringStorage &str) const
{
  return _tcscmp(getString(), str.getString()) < 0;
}

void StringStorage::replaceChar(TCHAR oldChar, TCHAR newChar)
{
  for (size_t i = 0; i < m_length; i++) {
    if (m_buffer[i] == oldChar) {
      m_buffer[i] = newChar;
    }
  }
}

void StringStorage::release()
{
  if (m_buffer != 0) {
    delete[] m_buffer;
    m_buffer = 0;
  }
  m_length = 0;
}
