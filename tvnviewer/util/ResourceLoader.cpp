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

#include "ResourceLoader.h"

#include <crtdbg.h>

ResourceLoader::ResourceLoader(HINSTANCE appInst)
: m_appInstance(appInst)
{
}

ResourceLoader::~ResourceLoader()
{
}

HICON ResourceLoader::loadStandartIcon(const TCHAR *iconName)
{
  return LoadIcon(NULL, iconName);
}

HICON ResourceLoader::loadIcon(const TCHAR *iconName)
{
  return LoadIcon(m_appInstance, iconName);
}

bool ResourceLoader::loadString(UINT id, StringStorage *string)
{
  _ASSERT(string != 0);

  TCHAR buffer[1024 * 10];

  if (LoadString(m_appInstance, id, buffer, 1024 * 10) == 0) {
    _ASSERT(FALSE);

    string->setString(_T("(Undef)"));

    return false;
  }

  string->setString(buffer);

  return true;
}
