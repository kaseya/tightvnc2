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

#include "IniFileSettingsManager.h"

#include <crtdbg.h>

IniFileSettingsManager::IniFileSettingsManager(const TCHAR *pathToFile, const TCHAR *appName)
{
  m_pathToFile.setString(pathToFile);
  m_appName.setString(appName);
}

IniFileSettingsManager::IniFileSettingsManager(const TCHAR *pathToFile)
{
  m_pathToFile.setString(pathToFile);
}

IniFileSettingsManager::IniFileSettingsManager()
{
}

IniFileSettingsManager::~IniFileSettingsManager()
{
}

void IniFileSettingsManager::setApplicationName(const TCHAR *appName)
{
  m_appName.setString(appName);
}

void IniFileSettingsManager::setPathToFile(const TCHAR *pathToFile)
{
  m_pathToFile.setString(pathToFile);
}

bool IniFileSettingsManager::isOk()
{
  return !m_pathToFile.isEmpty();
}

bool IniFileSettingsManager::keyExist(const TCHAR *name)
{

  StringStorage value;

  const TCHAR * defaultValue = _T("TightVNC_Ini_File_Key_Does_Not_Exist_Test");

  getPrivateProfileString(name, &value, defaultValue);

  if (value.isEqualTo(defaultValue)) {
    return false;
  }
  return true;
}

bool IniFileSettingsManager::deleteKey(const TCHAR *name)
{
  return WritePrivateProfileString(m_appName.getString(), name,
                                   NULL, m_pathToFile.getString()) == TRUE;
}

bool IniFileSettingsManager::getString(const TCHAR *name, StringStorage *storage)
{
  if (!keyExist(name)) {
    return false;
  }
  getPrivateProfileString(name, storage, NULL);
  return true;
}

bool IniFileSettingsManager::setString(const TCHAR *name, const TCHAR *value)
{
  if (value == NULL) {
    return false;
  }
  return WritePrivateProfileString(m_appName.getString(), name,
                                   value, m_pathToFile.getString()) == TRUE;
}

bool IniFileSettingsManager::getLong(const TCHAR *name, long *value)
{
  _ASSERT(FALSE);
  return false;
}

bool IniFileSettingsManager::setLong(const TCHAR *name, long value)
{
  _ASSERT(FALSE);
  return false;
}

bool IniFileSettingsManager::getBoolean(const TCHAR *name, bool *value)
{
  int intVal;
  if (!getInt(name, &intVal)) {
    return false;
  }
  *value = intVal == 1;
  return true;
}

bool IniFileSettingsManager::setBoolean(const TCHAR *name, bool value)
{
  return setInt(name, value ? 1 : 0);
}

bool IniFileSettingsManager::getUINT(const TCHAR *name, UINT *value)
{
  return getIntAndCastTo<UINT>(name, value);
}

bool IniFileSettingsManager::setUINT(const TCHAR *name, UINT value)
{
  StringStorage stringVal;
  stringVal.format(_T("%u"), value);

  return setString(name, stringVal.getString());
}

bool IniFileSettingsManager::getInt(const TCHAR *name, int *value)
{
  UINT defVal = 0xABCDEF;
  UINT ret = GetPrivateProfileInt(m_appName.getString(), name, defVal,
                                  m_pathToFile.getString());
  if (ret == defVal) {
    return false;
  }

  *value = (int)ret;

  return true;
}

bool IniFileSettingsManager::setInt(const TCHAR *name, int value)
{
  StringStorage stringVal;
  stringVal.format(_T("%d"), value);

  return setString(name, stringVal.getString());
}

bool IniFileSettingsManager::getByte(const TCHAR *name, char *value)
{
  return getIntAndCastTo<char>(name, value);
}

bool IniFileSettingsManager::setByte(const TCHAR *name, char value)
{
  return setInt(name, value);
}

bool IniFileSettingsManager::getBinaryData(const TCHAR *name, void *value, size_t *size)
{
  _ASSERT(FALSE);
  return false;
}

bool IniFileSettingsManager::setBinaryData(const TCHAR *name, const void *value, size_t size)
{
  _ASSERT(FALSE);
  return false;
}

void
IniFileSettingsManager::getPrivateProfileString(const TCHAR *name,
                                                StringStorage *value,
                                                const TCHAR *defaultValue)
{
  TCHAR *buffer = 0;
  size_t bufferSize = 0;
  size_t increaseStep = 1024;

  bool tooSmall = false;

  do {
    bufferSize += increaseStep;
    buffer = new TCHAR[bufferSize];

    DWORD ret = GetPrivateProfileString(m_appName.getString(), name,
                                        defaultValue, buffer, bufferSize,
                                        m_pathToFile.getString());

    tooSmall = (ret == bufferSize - 1);

    if (!tooSmall) {
      value->setString(buffer);
    }

    delete[] buffer;

    if (!tooSmall) {
      break;
    }

  } while (tooSmall);

}
