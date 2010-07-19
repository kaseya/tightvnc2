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

#include "RegistrySettingsManager.h"
#include "util/StringParser.h"

RegistrySettingsManager::RegistrySettingsManager()
: m_key(0)
{
}

RegistrySettingsManager::RegistrySettingsManager(HKEY rootKey, const TCHAR *entry, SECURITY_ATTRIBUTES *sa)
: m_key(0)
{
  setRegistryKey(rootKey, entry, sa);
}

RegistrySettingsManager::~RegistrySettingsManager()
{
  if (m_key != NULL) {
    delete m_key;
  }
}

void RegistrySettingsManager::setRegistryKey(HKEY rootKey, const TCHAR *entry, SECURITY_ATTRIBUTES *sa)
{
  if (m_key != 0) {
    delete m_key;
  }

  m_key = new RegistryKey(rootKey, entry, true, sa);
}

bool RegistrySettingsManager::isOk()
{
  return (m_key != NULL) && (m_key->isOpened());
}

void RegistrySettingsManager::extractKeyName(const TCHAR *key, StringStorage *folder)
{
  TCHAR *folderString = new TCHAR[_tcslen(key) + 1];
  _tcscpy(folderString, key);
  TCHAR *token = _tcsrchr(folderString, _T('\\'));
  if (token != NULL) {
    *token = _T('\0');
    folder->setString(folderString);
  } else {
    folder->setString(_T(""));
  }
  delete[] folderString;
}

void RegistrySettingsManager::extractValueName(const TCHAR *key, StringStorage *keyName)
{
  TCHAR *nameString = new TCHAR[_tcslen(key) + 1];
  _tcscpy(nameString, key);
  TCHAR *token = _tcsrchr(nameString, _T('\\'));
  if (token != NULL) {
    keyName->setString(++token);
  } else {
    keyName->setString((TCHAR *)key);
  }
  delete[] nameString;
}

bool RegistrySettingsManager::keyExist(const TCHAR *name)
{
  if (!isOk()) return false;
  RegistryKey subKey(m_key, name, false);
  return subKey.isOpened();
}

bool RegistrySettingsManager::deleteKey(const TCHAR *name)
{
  StringStorage keyName;
  StringStorage valueName;

  extractKeyName(name, &keyName);
  extractValueName(name, &valueName);

  RegistryKey subKey(m_key, keyName.getString(), false);

  bool deleteAsSubKey = m_key->deleteSubKeyTree(valueName.getString());
  bool deleteAsValue = subKey.deleteValue(valueName.getString());

  return deleteAsSubKey || deleteAsValue;
}

bool RegistrySettingsManager::getString(const TCHAR *name, StringStorage *value)
{
  StringStorage keyName;
  StringStorage valueName;

  extractKeyName(name, &keyName);
  extractValueName(name, &valueName);

  RegistryKey subKey(m_key, keyName.getString(), false);

  return subKey.getValueAsString(valueName.getString(), value);
}

bool RegistrySettingsManager::setString(const TCHAR *name, const TCHAR *value)
{
  StringStorage keyName;
  StringStorage valueName;

  extractKeyName(name, &keyName);
  extractValueName(name, &valueName);

  RegistryKey subKey(m_key, keyName.getString(), false);

  return subKey.setValueAsString(name, value);
}

bool RegistrySettingsManager::getLong(const TCHAR *name, long *value)
{
  StringStorage keyName;
  StringStorage valueName;

  extractKeyName(name, &keyName);
  extractValueName(name, &valueName);

  RegistryKey subKey(m_key, keyName.getString(), false);

  return subKey.getValueAsInt64(valueName.getString(), value);
}

bool RegistrySettingsManager::setLong(const TCHAR *name, long value)
{
  StringStorage keyName;
  StringStorage valueName;

  extractKeyName(name, &keyName);
  extractValueName(name, &valueName);

  RegistryKey subKey(m_key, keyName.getString(), false);

  return subKey.setValueAsInt64(name, value);
}

bool RegistrySettingsManager::getBoolean(const TCHAR *name, bool *value)
{
  int intVal = 0;
  if (!getInt(name, &intVal)) {
    return false;
  }
  *value = intVal == 1;
  return true;
}

bool RegistrySettingsManager::setBoolean(const TCHAR *name, bool value)
{
  return setInt(name, value ? 1 : 0);
}

bool RegistrySettingsManager::getUINT(const TCHAR *name, UINT *value)
{
  return getInt(name, (int *)value);
}

bool RegistrySettingsManager::setUINT(const TCHAR *name, UINT value)
{
  return setInt(name, (int)value);
}

bool RegistrySettingsManager::getInt(const TCHAR *name, int *value)
{
  StringStorage keyName;
  StringStorage valueName;

  extractKeyName(name, &keyName);
  extractValueName(name, &valueName);

  RegistryKey subKey(m_key, keyName.getString(), false);

  return subKey.getValueAsInt32(valueName.getString(), value);
}

bool RegistrySettingsManager::setInt(const TCHAR *name, int value)
{
  StringStorage keyName;
  StringStorage valueName;

  extractKeyName(name, &keyName);
  extractValueName(name, &valueName);

  RegistryKey subKey(m_key, keyName.getString(), false);

  return subKey.setValueAsInt32(name, value);
}

bool RegistrySettingsManager::getByte(const TCHAR *name, char *value)
{
  int intVal = 0;
  if (!getInt(name, &intVal)) {
    return false;
  }
  *value = (char)intVal;
  return true;
}

bool RegistrySettingsManager::setByte(const TCHAR *name, char value)
{
  return setInt(name, (int)value);
}

bool RegistrySettingsManager::getBinaryData(const TCHAR *name, void *value, size_t *size)
{
  StringStorage keyName;
  StringStorage valueName;

  extractKeyName(name, &keyName);
  extractValueName(name, &valueName);

  RegistryKey subKey(m_key, keyName.getString(), false);

  return subKey.getValueAsBinary(name, value, size);
}

bool RegistrySettingsManager::setBinaryData(const TCHAR *name, const void *value, size_t size)
{
  StringStorage keyName;
  StringStorage valueName;

  extractKeyName(name, &keyName);
  extractValueName(name, &valueName);

  RegistryKey subKey(m_key, keyName.getString(), false);

  return subKey.setValueAsBinary(name, value, size);
}
