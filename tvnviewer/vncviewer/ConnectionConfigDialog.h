//  Copyright (C) 2008 GlavSoft LLC. All Rights Reserved.
//
//  This file is part of the TightVNC software.
//
//  TightVNC is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC homepage on the Web: http://www.tightvnc.com/

#ifndef _CONNECTION_CONFIG_DIALOG_H_
#define _CONNECTION_CONFIG_DIALOG_H_

#include "gui/BaseDialog.h"
#include "gui/ComboBox.h"
#include "gui/CheckBox.h"
#include "gui/Trackbar.h"

#include "client-config-lib/ConnectionConfig.h"

class ConnectionConfigDialog : public BaseDialog
{
public:
  ConnectionConfigDialog(bool connected = false);
  virtual ~ConnectionConfigDialog();

  // Sets connection config.
  void setConnectionConfig(ConnectionConfig *conConfig);

protected:

  //
  // Inherited from BaseDialog.
  //

  virtual BOOL onInitDialog();
  virtual BOOL onNotify(UINT controlID, LPARAM data);
  virtual BOOL onCommand(UINT controlID, UINT notificationID);
  virtual BOOL onDestroy();

  virtual void onMessageRecieved(UINT uMsg, WPARAM wParam, LPARAM lParam);

  //
  // Event handlers
  //

  void onOkButtonClick();

  void onViewOnlyClick();
  void on8BitColorClick();

  void onAllowCustomCompressionClick();
  void onAllowJpegCompressionClick();

  void onCustomCompressionLevelScroll();
  void onJpegCompressionLevelScroll();

  void onPreferredEncodingSelectionChange();

  void onScaleKillFocus();

private:

  void enableCustomCompression(bool enable);
  void enableJpegCompression(bool enable);

  void initControls();
  void updateControls();
  void apply();

  bool isInputValid();

protected:
  ConnectionConfig *m_conConfig;
  bool m_connected;

  // GUI Controls

  Control m_customCompressionLevelLabel;
  Control m_jpegCompressionLevelLabel;

  ComboBox m_preferredEncoding;
  ComboBox m_scale;

  Trackbar m_customCompressionLevel;
  Trackbar m_jpegCompressionLevel;

  CheckBox m_use8BitColor;
  CheckBox m_useCustomCompression;
  CheckBox m_useJpegCompression;
  CheckBox m_allowCopyRect;
  CheckBox m_viewOnly;
  CheckBox m_disableClipboard;
  CheckBox m_fullscreen;
  CheckBox m_deiconifyOnRemoteBell;
  CheckBox m_emulate3Buttons;
  CheckBox m_swapMouse;
  CheckBox m_shared;
  CheckBox m_shapeEnable;
  CheckBox m_shapeDisable;
  CheckBox m_shapeIgnore;
  CheckBox m_localCursorShapeRadio[4];
};

#endif
