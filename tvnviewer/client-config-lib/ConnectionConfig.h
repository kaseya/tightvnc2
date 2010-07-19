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

#ifndef _CONNECTION_CONFIG_H_
#define _CONNECTION_CONFIG_H_

#include "config-lib/SettingsManager.h"

#include "thread/LocalMutex.h"

#include "vncviewer/rfb.h"

class ConnectionConfig
{
public:
  static const int NO_CURSOR = 0;
  static const int DOT_CURSOR = 1;
  static const int NORMAL_CURSOR = 2;
  static const int SMALL_CURSOR = 3;

  static const int FIRST_ENCODING = rfbEncodingRaw;
  static const int LAST_ENCODING = rfbEncodingZlibHex;

public:
  ConnectionConfig();
  ~ConnectionConfig();

  ConnectionConfig& operator=(ConnectionConfig& other);

  void allowEncoding(int enc, bool allow);
  bool isEncodingAllowed(int enc);

  void setPreferredEncoding(int encoding);

  int getPreferredEncoding();

  void use8BitColor(bool use);

  bool isUsing8BitColor();

  void setCustomCompressionLevel(int level);

  void setJpegCompressionLevel(int level);

  int getCustomCompressionLevel();
  int getJpegCompressionLevel();

  bool isCustomCompressionEnabled();
  bool isJpegCompressionEnabled();

  void disableCustomCompression();
  void disableJpegCompression();

  void setViewOnly(bool viewOnly);
  bool isViewOnly();

  void enableClipboard(bool enabled);
  bool isClipboardEnabled();

  void enableFullscreen(bool useFullscreen);
  bool isFullscreenEnabled();

  void deiconifyOnRemoteBell(bool deiconifyFlag);
  bool isDeiconifyOnRemoteBellEnabled();

  bool scalingActive();
  int getScaleNumerator();
  int getScaleDenominator();

  void setScale(int n, int d);

  void swapMouse(bool enabled);
  bool isMouseSwapEnabled();

  void emulate3Buttons(bool enabled);
  bool isEmulate3ButtonsEnabled();

  void setEmulate3ButtonsTimeout(int millis);
  int getEmulate3ButtonsTimeout();

  void setEmulate3ButtonsFuzz(int sec);
  int getEmulate3ButtonsFuzz();

  void setSharedFlag(bool shared);
  bool getSharedFlag();

  void setRestricted(bool restricted);
  bool isRestricted();

  void fitWindow(bool enabled);
  bool isFitWindowEnabled();

  void requestShapeUpdates(bool requestFlag);
  void ignoreShapeUpdates(bool ignoreFlag);

  bool isRequestingShapeUpdates();
  bool isIgnoringShapeUpdates();

  void setLocalCursorShape(int cursorShape);
  int getLocalCursorShape();

  bool saveToStorage(SettingsManager *sm);
  bool loadFromStorage(SettingsManager *sm);
private:
  int gcd(int a, int b);

protected:

  bool m_allowedEncodings[LAST_ENCODING + 1];
  unsigned char m_preferredEncoding;
  bool m_use8BitColor;
  int m_customCompressionLevel;
  int m_jpegCompressionLevel;

  bool m_viewOnly;
  bool m_isClipboardEnabled;

  bool m_useFullscreen;
  bool m_deiconifyOnRemoteBell;
  bool m_useAutoScale;
  int m_scaleNumerator;
  int m_scaleDenominator;

  bool m_swapMouse;
  bool m_emulate3Buttons;
  int m_emulate3ButtonsTimeout;
  int m_emulate3ButtonsFuzz;

  bool m_requestSharedSession;
  bool m_restricted;

  bool m_fitWindow;
  bool m_requestShapeUpdates;
  bool m_ignoreShapeUpdates;

  int m_localCursor;

  LocalMutex m_cs;
};

#endif
