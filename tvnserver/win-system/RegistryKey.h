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

#ifndef _REGISTRY_KEY_H_
#define _REGISTRY_KEY_H_

#include "util/CommonHeader.h"

class RegistryKey
{
public:
  RegistryKey(HKEY rootKey, const TCHAR *entry, bool createIfNotExists = true, SECURITY_ATTRIBUTES *sa = 0);
  RegistryKey(RegistryKey *rootKey, const TCHAR *entry, bool createIfNotExists = true, SECURITY_ATTRIBUTES *sa = 0);

protected:
  RegistryKey(HKEY rootKey);

public:

  virtual ~RegistryKey();

  HKEY getHKEY() const;

  bool createSubKey(const TCHAR *subkey);
  bool deleteSubKey(const TCHAR *subkey);
  bool deleteSubKeyTree(const TCHAR *subkey);
  bool deleteValue(const TCHAR *name);

  bool setValueAsInt32(const TCHAR *name, int value);
  bool setValueAsInt64(const TCHAR *name, long value);
  bool setValueAsString(const TCHAR *name, const TCHAR *value);
  bool setValueAsBinary(const TCHAR *name, const void *value,
                        size_t sizeInBytes);

  bool getValueAsInt32(const TCHAR *name, int *out);
  bool getValueAsInt64(const TCHAR *name, long *out);
  bool getValueAsString(const TCHAR *name, StringStorage *out);
  bool getValueAsBinary(const TCHAR *name, void *value, size_t *sizeInBytes);

  bool getSubKeyNames(StringStorage *subKeyNames, size_t *count);

  bool isOpened();
  void close();

private:

  void initialize(HKEY rootKey, const TCHAR *entry, bool createIfNotExists, SECURITY_ATTRIBUTES *sa);

  DWORD enumKey(DWORD i, StringStorage *name);

  static bool tryOpenSubKey(HKEY key, const TCHAR *subkey,
                            HKEY *openedKey, bool createIfNotExists,
                            SECURITY_ATTRIBUTES *sa);

protected:
  HKEY m_key;
  HKEY m_rootKey;
  StringStorage m_entry;

  friend class Registry;
};

#endif
