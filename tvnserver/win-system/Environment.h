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

#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include "util/StringStorage.h"

#include <winnt.h>

class Environment
{
public:
  static const int APPLICATION_DATA_SPECIAL_FOLDER = 0x0; 
  static const int COMMON_APPLICATION_DATA_SPECIAL_FOLDER = 0x1;
public:
  Environment();
  ~Environment();

  static void getErrStr(StringStorage *out);

  static void getErrStr(const TCHAR *specification, StringStorage *out);

  static bool getSpecialFolderPath(int specialFolderId, StringStorage *out);

  static bool getCurrentModulePath(StringStorage *out);

  static bool getCurrentModuleFolderPath(StringStorage *out);

  static bool getCurrentUserName(StringStorage *out);

  static bool getComputerName(StringStorage *out);

  static void restoreWallpaper();
  static void disableWallpaper();

  static bool isWinNTFamily();
  static bool isWinXP();
  static bool isWin2003Server();
  static bool isVistaOrLater();

  static void simulateCtrlAltDel();

  static void simulateCtrlAltDelUnderVista();

private:
  static void init();
  static OSVERSIONINFO m_osVerInfo;
};

#endif
