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

#ifndef _CONTROL_PROTO_H_
#define _CONTROL_PROTO_H_

#include "util/CommonHeader.h"

class ControlProto
{
public:

  static const UINT32 AUTH_MSG_ID = 0x13;
  static const UINT32 GET_CLIENT_LIST_MSG_ID = 0x4;
  static const UINT32 GET_SERVER_INFO_MSG_ID = 0x11;
  static const UINT32 RELOAD_CONFIG_MSG_ID = 0x5;
  static const UINT32 DISCONNECT_ALL_CLIENTS_MSG_ID = 0x6;
  static const UINT32 SHUTDOWN_SERVER_MSG_ID = 0x7;
  static const UINT32 GET_SHOW_TRAY_ICON_FLAG = 0x14;
  static const UINT32 ADD_CLIENT_MSG_ID = 0x8;
  static const UINT32 SET_CONFIG_MSG_ID = 0x10;
  static const UINT32 GET_CONFIG_MSG_ID = 0x12;
  static const UINT32 UPDATE_TVNCONTROL_PROCESS_ID_MSG_ID = 0x15;

  static const UINT32 REPLY_OK = 0x0;
  static const UINT32 REPLY_ERROR = 0x1;
  static const UINT32 REPLY_AUTH_NEEDED = 0x2;
};

#endif
