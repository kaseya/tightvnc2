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

#ifndef __HOOKSUPDATEDETECTOR_H__
#define __HOOKSUPDATEDETECTOR_H__

#include "UpdateDetector.h"
#include "win-system/WindowsEvent.h"
#include "win-system/DynamicLibrary.h"
#include "gui/Window.h"
#include "HookUpdateTimer.h"

class HooksUpdateDetector : public UpdateDetector
{
public:
  HooksUpdateDetector(UpdateKeeper *updateKeeper,
                      UpdateListener *updateListener);
  virtual ~HooksUpdateDetector(void);

protected:
  virtual void execute();
  virtual void onTerminate();

  FARPROC m_pSetHook;
  FARPROC m_pUnSetHook;
  WindowsEvent m_initWaiter;

  DynamicLibrary *m_lib;
  Window *m_targetWin;
  HookUpdateTimer m_updateTimer;

private:
  bool initHook();
  bool unInitHook();

  static const TCHAR LIBRARY_NAME[];
  static const char SET_HOOK_FUNCTION_NAME[];
  static const char UNSET_HOOK_FUNCTION_NAME[];
};

#endif 
