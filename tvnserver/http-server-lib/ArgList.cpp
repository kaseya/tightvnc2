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

#include "ArgList.h"

#include "util/CommonHeader.h"

ArgList::ArgList(const char *argString)
{
  size_t len = strlen(argString);

  const char *src = argString;

  char *pair = new char[len + 1];
  char *left = new char[len + 1];

  while (true) {
    splitPair(src, '&', pair, left);

    if ((strlen(pair) == 0) && (strlen(left) == 0)) {
      break;
    }

    size_t pairLen = strlen(pair);

    char *key = new char[pairLen + 1];
    char *val = new char[pairLen + 1];

    splitPair(pair, '=', key, val);

    htmlDecode(key);
    htmlDecode(val);

    m_args[key] = val;

    delete key;

    src = left;
  }

  delete left;
  delete pair;
}

ArgList::~ArgList()
{
  for (map<string, char *>::iterator it = m_args.begin(); it != m_args.end(); it++) {
    delete it->second;
  }
  m_args.clear();
}

size_t ArgList::getCount() const
{
  return m_args.size();
}

const char *ArgList::getKey(size_t index)
{
  if (index >= getCount()) {
    return NULL;
  }

  map<string, char *>::iterator it = m_args.begin();

  for (size_t i = 0; i < index; i++) {
    it++;
  }

  return it->first.c_str();
}

const char *ArgList::getValue(const char *key)
{
  string stdStr = key;

  if (m_args.find(stdStr) == m_args.end()) {
    return NULL;
  }

  return m_args[stdStr];
}

void ArgList::splitPair(const char *par, char delimitter, char *key, char *value)
{
  bool delimiterFound = false;

  size_t vi = 0;
  size_t ki = 0;

  for (size_t i = 0; i < strlen(par); i++) {
    if (par[i] == delimitter) {
      if (!delimiterFound) {
        delimiterFound = true;
        continue;
      }
    }

    if (delimiterFound) {
      value[vi++] = par[i];
    } else {
      key[ki++] = par[i];
    }
  }

  key[ki] = '\0';
  value[vi] = '\0';
}

void ArgList::htmlDecode(char *value) const
{
  for (size_t i = 0; i < strlen(value); i++) {
    if (value[i] == '+') {
      value[i] = ' ';
    }
  }
}
