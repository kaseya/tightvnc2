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

#include "ConnectionConfig.h"

#include "thread/AutoLock.h"

#include <crtdbg.h>

#define TEST_FAIL(C,R) if (!C) { R = false; }

ConnectionConfig::ConnectionConfig()
: m_restricted(false), m_swapMouse(false), m_viewOnly(false),
  m_useFullscreen(false), m_use8BitColor(false), m_preferredEncoding(7),
  m_requestSharedSession(true), m_deiconifyOnRemoteBell(false),
  m_isClipboardEnabled(true),
  m_customCompressionLevel(-1), m_jpegCompressionLevel(6),
  m_emulate3Buttons(true), m_emulate3ButtonsTimeout(100),
  m_emulate3ButtonsFuzz(4), m_fitWindow(false), m_requestShapeUpdates(true),
  m_ignoreShapeUpdates(false), m_scaleNumerator(1), m_scaleDenominator(1),
  m_localCursor(DOT_CURSOR)
{
  for (int i = FIRST_ENCODING; i<= LAST_ENCODING; i++) {
    m_allowedEncodings[i] = true;
  }
  m_allowedEncodings[3] = false;
}

ConnectionConfig::~ConnectionConfig()
{
}

ConnectionConfig& ConnectionConfig::operator=(ConnectionConfig& other)
{
  AutoLock lockThis(&m_cs);
  AutoLock lockOther(&other.m_cs);

  for (int i = FIRST_ENCODING; i <= LAST_ENCODING; i++) {
    m_allowedEncodings[i] = other.m_allowedEncodings[i];
  }

  m_preferredEncoding = other.m_preferredEncoding;
  m_use8BitColor = other.m_use8BitColor;
  m_customCompressionLevel = other.m_customCompressionLevel;
  m_jpegCompressionLevel = other.m_jpegCompressionLevel;
  m_viewOnly = other.m_viewOnly;
  m_isClipboardEnabled = other.m_isClipboardEnabled;
  m_useFullscreen = other.m_useFullscreen;
  m_deiconifyOnRemoteBell = other.m_deiconifyOnRemoteBell;
  m_scaleNumerator = other.m_scaleNumerator;
  m_scaleDenominator = other.m_scaleDenominator;
  m_swapMouse = other.m_swapMouse;
  m_emulate3Buttons = other.m_emulate3Buttons;
  m_emulate3ButtonsTimeout = other.m_emulate3ButtonsTimeout;
  m_emulate3ButtonsFuzz = other.m_emulate3ButtonsFuzz;
  m_requestSharedSession = other.m_requestSharedSession;
  m_restricted = other.m_restricted;
  m_fitWindow = other.m_fitWindow;
  m_requestShapeUpdates = other.m_requestShapeUpdates;
  m_ignoreShapeUpdates = other.m_ignoreShapeUpdates;
  m_localCursor = other.m_localCursor;

  return *this;
}

void ConnectionConfig::allowEncoding(int enc, bool allow)
{
  if (enc < FIRST_ENCODING || enc > LAST_ENCODING) {
    _ASSERT(FALSE);
    return ;
  }

  {
    AutoLock l(&m_cs);
    m_allowedEncodings[enc] = allow;
  }
}

bool ConnectionConfig::isEncodingAllowed(int enc)
{
  if (enc < FIRST_ENCODING || enc > LAST_ENCODING) {
    return false;
  }

  {
    AutoLock l(&m_cs);
    return m_allowedEncodings[enc];
  }
}

void ConnectionConfig::setPreferredEncoding(int encoding)
{
  AutoLock l(&m_cs);

  switch (encoding) {
  case rfbEncodingRaw:
  case rfbEncodingCopyRect:
  case rfbEncodingRRE:
  case rfbEncodingCoRRE:
  case rfbEncodingHextile:
  case rfbEncodingZlib:
  case rfbEncodingTight:
  case rfbEncodingZlibHex:
  case rfbEncodingZRLE:
    m_preferredEncoding = encoding;
    break;
  default:
    _ASSERT(FALSE);
  } 
}

int ConnectionConfig::getPreferredEncoding()
{
  AutoLock l(&m_cs);
  return m_preferredEncoding;
}

void ConnectionConfig::use8BitColor(bool use)
{
  AutoLock l(&m_cs);
  m_use8BitColor = use;
}

bool ConnectionConfig::isUsing8BitColor()
{
  AutoLock l(&m_cs);
  return m_use8BitColor;
}

void ConnectionConfig::setCustomCompressionLevel(int level)
{
  if (level < 1) {
    if (level != -1) {
      level = 1;
    }
  } else if (level > 9) {
    level = 9;
  }

  {
    AutoLock l(&m_cs);
    m_customCompressionLevel = level;
  }
}

void ConnectionConfig::setJpegCompressionLevel(int level)
{
  if (level < 1) {
    if (level != -1) {
      level = 1;
    }
  } else if (level > 9) {
    level = 9;
  }

  {
    AutoLock l(&m_cs);
    m_jpegCompressionLevel = level;
  }
}

int ConnectionConfig::getCustomCompressionLevel()
{
  AutoLock l(&m_cs);
  return m_customCompressionLevel;
}

int ConnectionConfig::getJpegCompressionLevel()
{
  AutoLock l(&m_cs);
  return m_jpegCompressionLevel;
}

bool ConnectionConfig::isCustomCompressionEnabled()
{
  return getCustomCompressionLevel() != -1;
}

bool ConnectionConfig::isJpegCompressionEnabled()
{
  return getJpegCompressionLevel() != -1;
}

void ConnectionConfig::disableCustomCompression()
{
  setCustomCompressionLevel(-1);
}

void ConnectionConfig::disableJpegCompression()
{
  setJpegCompressionLevel(-1);
}

void ConnectionConfig::setViewOnly(bool viewOnly)
{
  AutoLock l(&m_cs);
  m_viewOnly = viewOnly;
}

bool ConnectionConfig::isViewOnly()
{
  AutoLock l(&m_cs);
  return m_viewOnly;
}

void ConnectionConfig::enableClipboard(bool enabled)
{
  AutoLock l(&m_cs);
  m_isClipboardEnabled = enabled;
}

bool ConnectionConfig::isClipboardEnabled()
{
  AutoLock l(&m_cs);
  return m_isClipboardEnabled;
}

void ConnectionConfig::enableFullscreen(bool useFullscreen)
{
  AutoLock l(&m_cs);
  m_useFullscreen = useFullscreen;
}

bool ConnectionConfig::isFullscreenEnabled()
{
  AutoLock l(&m_cs);
  return m_useFullscreen;
}

void ConnectionConfig::deiconifyOnRemoteBell(bool deiconifyFlag)
{
  AutoLock l(&m_cs);
  m_deiconifyOnRemoteBell = deiconifyFlag;
}

bool ConnectionConfig::isDeiconifyOnRemoteBellEnabled()
{
  AutoLock l(&m_cs);
  return m_deiconifyOnRemoteBell;
}

bool ConnectionConfig::scalingActive()
{
  AutoLock l(&m_cs);
  return (m_scaleNumerator != m_scaleDenominator);
}

int ConnectionConfig::getScaleNumerator()
{
  AutoLock l(&m_cs);
  return m_scaleNumerator;
}

int ConnectionConfig::getScaleDenominator()
{
  AutoLock l(&m_cs);
  return m_scaleDenominator;
}

void ConnectionConfig::setScale(int n, int d)
{
  if ((n < 1) || (d < 1)) {
    n = d = 1;
  }

  int g = gcd(n, d);

  n /= g;
  d /= g;

  if ((n * 100) / d > 150) {
    n = 150;
    d = 100;
  }

  {
    AutoLock l(&m_cs);
    m_scaleNumerator = n;
    m_scaleDenominator = d;
  }
}

void ConnectionConfig::swapMouse(bool enabled)
{
  AutoLock l(&m_cs);
  m_swapMouse = enabled;
}

bool ConnectionConfig::isMouseSwapEnabled()
{
  AutoLock l(&m_cs);
  return m_swapMouse;
}

void ConnectionConfig::emulate3Buttons(bool enabled)
{
  AutoLock l(&m_cs);
  m_emulate3Buttons = enabled;
}

bool ConnectionConfig::isEmulate3ButtonsEnabled()
{
  AutoLock l(&m_cs);
  return m_emulate3Buttons;
}

void ConnectionConfig::setEmulate3ButtonsTimeout(int millis)
{
  if (millis < 0) {
    millis = 0;
  }

  {
    AutoLock l(&m_cs);
    m_emulate3ButtonsTimeout = millis;
  }
}

int ConnectionConfig::getEmulate3ButtonsTimeout()
{
  AutoLock l(&m_cs);
  return m_emulate3ButtonsTimeout;
}

void ConnectionConfig::setEmulate3ButtonsFuzz(int sec)
{
  if (sec < 0) {
    sec = 0;
  }

  {
    AutoLock l(&m_cs);
    m_emulate3ButtonsFuzz = sec;
  }
}

int ConnectionConfig::getEmulate3ButtonsFuzz()
{
  AutoLock l(&m_cs);
  return m_emulate3ButtonsFuzz;
}

void ConnectionConfig::setSharedFlag(bool shared)
{
  AutoLock l(&m_cs);
  m_requestSharedSession = shared;
}

bool ConnectionConfig::getSharedFlag()
{
  AutoLock l(&m_cs);
  return m_requestSharedSession;
}

void ConnectionConfig::setRestricted(bool restricted)
{
  AutoLock l(&m_cs);
  m_restricted = restricted;
}

bool ConnectionConfig::isRestricted()
{
  AutoLock l(&m_cs);
  return m_restricted;
}

void ConnectionConfig::fitWindow(bool enabled)
{
  AutoLock l(&m_cs);
  m_fitWindow = enabled;
}

bool ConnectionConfig::isFitWindowEnabled()
{
  AutoLock l(&m_cs);
  return m_fitWindow;
}

void ConnectionConfig::requestShapeUpdates(bool requestFlag)
{
  AutoLock l(&m_cs);
  m_requestShapeUpdates = requestFlag;
}

void ConnectionConfig::ignoreShapeUpdates(bool ignoreFlag)
{
  AutoLock l(&m_cs);
  m_ignoreShapeUpdates = ignoreFlag;
}

bool ConnectionConfig::isRequestingShapeUpdates()
{
  AutoLock l(&m_cs);
  return m_requestShapeUpdates;
}

bool ConnectionConfig::isIgnoringShapeUpdates()
{
  AutoLock l(&m_cs);
  return m_ignoreShapeUpdates;
}

void ConnectionConfig::setLocalCursorShape(int cursorShape)
{
  switch (cursorShape) {
  case NO_CURSOR:
  case DOT_CURSOR:
  case NORMAL_CURSOR:
  case SMALL_CURSOR:
    break;
  default:
    cursorShape = DOT_CURSOR;
    _ASSERT(FALSE);
  } 

  {
    AutoLock l(&m_cs);
    m_localCursor = cursorShape;
  }
}

int ConnectionConfig::getLocalCursorShape()
{
  AutoLock l(&m_cs);
  return m_localCursor;
}

bool ConnectionConfig::saveToStorage(SettingsManager *sm)
{
  AutoLock l(&m_cs);

  bool saveAllOk = true;

  StringStorage useEncEntryName;
  for (int i = FIRST_ENCODING; i <= LAST_ENCODING; i++) {
    useEncEntryName.format(_T("use_encoding_%d"), i);
    TEST_FAIL(sm->setBoolean(useEncEntryName.getString(), m_allowedEncodings[i]), saveAllOk);
  }

  TEST_FAIL(sm->setBoolean(_T("viewonly"),         m_viewOnly), saveAllOk);
  TEST_FAIL(sm->setBoolean(_T("fullscreen"),       m_useFullscreen), saveAllOk);
  TEST_FAIL(sm->setBoolean(_T("8bit"),             m_use8BitColor), saveAllOk);
  TEST_FAIL(sm->setBoolean(_T("shared"),           m_requestSharedSession), saveAllOk);
  TEST_FAIL(sm->setBoolean(_T("belldeiconify"),    m_deiconifyOnRemoteBell), saveAllOk);
  TEST_FAIL(sm->setBoolean(_T("disableclipboard"), !m_isClipboardEnabled), saveAllOk);
  TEST_FAIL(sm->setBoolean(_T("restricted"),       m_restricted), saveAllOk);
  TEST_FAIL(sm->setBoolean(_T("swapmouse"),        m_swapMouse), saveAllOk);
  TEST_FAIL(sm->setBoolean(_T("emulate3"),         m_emulate3Buttons), saveAllOk);
  TEST_FAIL(sm->setBoolean(_T("fitwindow"),        m_fitWindow), saveAllOk);
  TEST_FAIL(sm->setBoolean(_T("cursorshape"),      m_requestShapeUpdates), saveAllOk);
  TEST_FAIL(sm->setBoolean(_T("noremotecursor"),   m_ignoreShapeUpdates), saveAllOk);

  TEST_FAIL(sm->setByte(_T("preferred_encoding"),  m_preferredEncoding), saveAllOk);
  TEST_FAIL(sm->setInt(_T("compresslevel"),        m_customCompressionLevel), saveAllOk);
  TEST_FAIL(sm->setInt(_T("quality"),              m_jpegCompressionLevel), saveAllOk);
  TEST_FAIL(sm->setInt(_T("emulate3timeout"),      m_emulate3ButtonsTimeout), saveAllOk);
  TEST_FAIL(sm->setInt(_T("emulate3fuzz"),         m_emulate3ButtonsFuzz), saveAllOk);
  TEST_FAIL(sm->setInt(_T("localcursor"),          m_localCursor), saveAllOk);
  TEST_FAIL(sm->setInt(_T("scale_den"),            m_scaleDenominator), saveAllOk);
  TEST_FAIL(sm->setInt(_T("scale_num"),            m_scaleNumerator), saveAllOk);

  TEST_FAIL(sm->setInt(_T("local_cursor_shape"),   m_localCursor), saveAllOk);

  return saveAllOk;
}

bool ConnectionConfig::loadFromStorage(SettingsManager *sm)
{
  AutoLock l(&m_cs);

  bool loadAllOk = true;

  StringStorage useEncEntryName;
  for (int i = FIRST_ENCODING; i <= LAST_ENCODING; i++) {
    useEncEntryName.format(_T("use_encoding_%d"), i);
    TEST_FAIL(sm->getBoolean(useEncEntryName.getString(), &m_allowedEncodings[i]), loadAllOk);
  }

  TEST_FAIL(sm->getBoolean(_T("viewonly"),         &m_viewOnly), loadAllOk);
  TEST_FAIL(sm->getBoolean(_T("fullscreen"),       &m_useFullscreen), loadAllOk);
  TEST_FAIL(sm->getBoolean(_T("8bit"),             &m_use8BitColor), loadAllOk);
  TEST_FAIL(sm->getBoolean(_T("shared"),           &m_requestSharedSession), loadAllOk);
  TEST_FAIL(sm->getBoolean(_T("belldeiconify"),    &m_deiconifyOnRemoteBell), loadAllOk);

  if (sm->getBoolean(_T("disableclipboard"), &m_isClipboardEnabled)) {
    m_isClipboardEnabled = !m_isClipboardEnabled;
  } else {
    loadAllOk = false;
  }

  TEST_FAIL(sm->getBoolean(_T("restricted"),       &m_restricted), loadAllOk);
  TEST_FAIL(sm->getBoolean(_T("swapmouse"),        &m_swapMouse), loadAllOk);
  TEST_FAIL(sm->getBoolean(_T("emulate3"),         &m_emulate3Buttons), loadAllOk);
  TEST_FAIL(sm->getBoolean(_T("fitwindow"),        &m_fitWindow), loadAllOk);
  TEST_FAIL(sm->getBoolean(_T("cursorshape"),      &m_requestShapeUpdates), loadAllOk);
  TEST_FAIL(sm->getBoolean(_T("noremotecursor"),   &m_ignoreShapeUpdates), loadAllOk);

  TEST_FAIL(sm->getByte(_T("preferred_encoding"),  (char *)&m_preferredEncoding), loadAllOk);

  TEST_FAIL(sm->getInt(_T("compresslevel"),        &m_customCompressionLevel), loadAllOk);
  TEST_FAIL(sm->getInt(_T("quality"),              &m_jpegCompressionLevel), loadAllOk);
  TEST_FAIL(sm->getInt(_T("emulate3timeout"),      &m_emulate3ButtonsTimeout), loadAllOk);
  TEST_FAIL(sm->getInt(_T("emulate3fuzz"),         &m_emulate3ButtonsFuzz), loadAllOk);
  TEST_FAIL(sm->getInt(_T("localcursor"),          &m_localCursor), loadAllOk);
  TEST_FAIL(sm->getInt(_T("scale_den"),            &m_scaleDenominator), loadAllOk);
  TEST_FAIL(sm->getInt(_T("scale_num"),            &m_scaleNumerator), loadAllOk);

  TEST_FAIL(sm->getInt(_T("local_cursor_shape"),   &m_localCursor), loadAllOk);

  return loadAllOk;
}

int ConnectionConfig::gcd(int a, int b)
{
  if (a < b) {
    return gcd(b, a);
  }
  if (b == 0) {
    return a;
  }
  return gcd(b, a % b);
}
