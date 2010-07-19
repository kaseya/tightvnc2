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

#include "AppletParameter.h"

#include "util/CommonHeader.h"

#include <crtdbg.h>

AppletParameter::AppletParameter(const char *name, const char *value)
: m_isValid(false), m_formattedString(NULL)
{
  _ASSERT(name != NULL);
  _ASSERT(value != NULL);

  m_isValid = isStringValid(name) && isStringValid(value);

  if (isValid()) {
    const char *format = "<PARAM NAME=\"%s\" VALUE=\"%s\" >\n";

    int len = _scprintf(format, name, value) + 1;

    m_formattedString = new char[len];

    sprintf_s(m_formattedString, (size_t)(len), format, name, value);
  }
}

AppletParameter::~AppletParameter()
{
  if (m_formattedString != NULL) {
    delete[] m_formattedString;
  }
}

const char *AppletParameter::getFormattedString() const
{
  return m_formattedString;
}

bool AppletParameter::isValid() const
{
  return m_isValid;
}

bool AppletParameter::isStringValid(const char *str) const
{
  for (size_t i = 0; i < strlen(str); i++) {
    if (!(isalnum(str[i]) || str[i] == '_' || str[i] == '.' || str[i] == ' ')) {
      return false;
    } 
  } 

  return true;
}
