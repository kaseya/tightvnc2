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

#include "CommandLine.h"

CommandLine::CommandLine()
{
}

CommandLine::~CommandLine()
{
  std::vector<KeyContainer *>::iterator iter;
  for (iter = m_foundKeys.begin(); iter != m_foundKeys.end(); iter++) {
    if (*iter) {
      delete *iter;
    }
  }
}

bool CommandLine::parse(const CommandLineFormat *format,
                        int formatSize,
                        const TCHAR *commandLine)
{
  std::vector<TCHAR *> argContainer;
  std::vector<TCHAR *>::iterator argIter;

  const TCHAR *startPos = commandLine, *endPos = commandLine;

  while (*endPos != 0 && *startPos != 0) {
    for (; *startPos == _T(' ') && *startPos != 0; startPos++);

    endPos = startPos;

    bool isEmptyArg = false;
    if (*endPos == _T('\"')) {
      endPos++; startPos++;
      if (*endPos == _T('\"')) {
        isEmptyArg = true;
      }
      for (; *endPos != _T('\"') && *endPos != 0; endPos++);
    } else {
      for (; *endPos != _T(' ') && *endPos != 0; endPos++);
    }

    if (endPos > startPos || isEmptyArg) {
      size_t argLen = endPos - startPos;
      TCHAR *arg = new TCHAR[argLen + 1];
      memcpy(arg, startPos, argLen * sizeof(TCHAR));
      arg[argLen] = 0;
      argContainer.push_back(arg);
    }

    startPos = endPos + 1;
  }

  bool result = true;
  for (argIter = argContainer.begin(); argIter != argContainer.end() && result;
       argIter++) {
    TCHAR *key = *argIter;
    if (!removeKeyPrefix(key)) {
      result = false;
      break;
    }

    result = false;
    for (int i = 0; i < formatSize; i++) {
      if (matchKey(format[i].keyName, key)) {
        KeyContainer *keyContainer = new KeyContainer;
        result = true;

        if (format[i].useArg == NEEDS_ARG) {
          argIter++;
          if (argIter == argContainer.end()) {
            delete keyContainer;
            return false;
          }
          keyContainer->isArgument = true;
          keyContainer->argument.setString(*argIter);
        }
        keyContainer->key.setString(format[i].keyName);
        m_foundKeys.push_back(keyContainer);
      }
    }
  }

  for (argIter = argContainer.begin(); argIter != argContainer.end(); argIter++) {
    delete[] *argIter;
  }

  return result;
}

bool CommandLine::optionSpecified(const TCHAR *key, StringStorage *arg) const
{
  bool found = false;
  std::vector<KeyContainer *>::const_iterator iter;
  for (iter = m_foundKeys.begin(); iter != m_foundKeys.end(); iter++) {
    KeyContainer *foundKey = (*iter);
    if (_tcscmp(foundKey->key.getString(), key) == 0) {
      found = true;
      if (foundKey->isArgument && arg != 0) {
        arg->setString(foundKey->argument.getString());
      }
    }
  }
  return found;
}

bool CommandLine::getOption(int index, StringStorage *key, StringStorage *arg) const
{
  if (index < 0 || (size_t)index >= m_foundKeys.size()) {
    return false;
  }

  KeyContainer *foundKey = m_foundKeys.at((size_t)index);

  key->setString(foundKey->key.getString());

  if (foundKey->isArgument && arg != 0) {
    arg->setString(foundKey->argument.getString());
  }

  return true;
}

bool CommandLine::matchKey(const TCHAR *keyTemplate, TCHAR *key)
{
  keyTemplate++;
  TCHAR *k = key;
  for (; *k != 0; k++) {
    *k = _totlower(*k);
  }
  return _tcscmp(keyTemplate, key) == 0;
}

bool CommandLine::removeKeyPrefix(TCHAR *key)
{
  if (*key != '/' && *key != '-') {
    return false;
  }
  for (; *key != 0; key++) {
    *key = *(key + 1);
  }
  return true;
}
