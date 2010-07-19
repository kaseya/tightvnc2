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

#include "UipiControl.h"
#include "Environment.h"
#include "DynamicLibrary.h"
#include "SystemException.h"
#include "util/Log.h"

#ifndef MSGFLT_ADD
#define MSGFLT_ADD 1
#endif

typedef BOOL (WINAPI *SetFilterEx)(HWND hWnd,
                                   UINT message,
                                   DWORD action,
                                   UINT32 *reserved
                                   );
typedef BOOL (WINAPI *SetFilter)(UINT message,
                                 DWORD action);

UipiControl::UipiControl()
{
}

UipiControl::~UipiControl()
{
}

void UipiControl::allowMessage(UINT message, HWND hwnd)
{
  Log::info(_T("Try allow to receive the %u windows message"));
  if (Environment::isVistaOrLater()) {
    DynamicLibrary user32lib(_T("user32.dll"));
    Log::info(_T("user32.dll successfully loaded."));
    SetFilterEx setFilterEx;
    setFilterEx = (SetFilterEx)user32lib.getProcAddress("ChangeWindowMessageFilterEx");
    if (setFilterEx == 0) {
      SetFilter setFilter;
      setFilter = (SetFilter)user32lib.getProcAddress("ChangeWindowMessageFilter");
      if (setFilter == 0) {
        throw Exception(_T("Can't load the ChangeWindowMessageFilterEx() or ")
                        _T("ChangeWindowMessageFilter() functions."));
      }
      Log::info(_T("The ChangeWindowMessageFilter() function ")
                _T("successfully found."));
      if (setFilter(message, MSGFLT_ADD) != TRUE) {
        DWORD errCode = GetLastError();
        StringStorage errMess;
        errMess.format(_T("Can't allow to receive the %d windows message by ")
                       _T("the ChangeWindowMessageFilter() function."));
        throw SystemException(errMess.getString(), errCode);
      }
      Log::info(_T("The ChangeWindowMessageFilter() function ")
                _T("successfully executed."));
    } else {
      Log::info(_T("The ChangeWindowMessageFilterEx() function ")
                _T("successfully found."));
      if (setFilterEx(hwnd, message, MSGFLT_ADD, 0) != TRUE) {
        DWORD errCode = GetLastError();
        StringStorage errMess;
        errMess.format(_T("Can't allow to receive the %d windows message by ")
                       _T("the ChangeWindowMessageFilterEx() function."));
        throw SystemException(errMess.getString(), errCode);
      }
      Log::info(_T("The ChangeWindowMessageFilterEx() function ")
                _T("successfully executed."));
    }
  } else {
    Log::info(_T("The allowMessage() function call is ignored."));
  }
}
