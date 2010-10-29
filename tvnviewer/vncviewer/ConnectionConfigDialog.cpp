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

#include "ConnectionConfigDialog.h"

#include "util/StringParser.h"

#include "res/resource.h"

ConnectionConfigDialog::ConnectionConfigDialog(bool connected)
: m_conConfig(0),
  m_connected(connected)
{
  setResourceId(IDD_OPTIONDIALOG);
}

ConnectionConfigDialog::~ConnectionConfigDialog()
{
}

void ConnectionConfigDialog::setConnectionConfig(ConnectionConfig *conConfig)
{
  m_conConfig = conConfig;
}

BOOL ConnectionConfigDialog::onInitDialog()
{
  initControls();

  m_preferredEncoding.addItem(_T("Raw"), (void *)rfbEncodingRaw);
  m_preferredEncoding.addItem(_T("Hextile"), (void *)rfbEncodingHextile);
  m_preferredEncoding.addItem(_T("Tight"), (void *)rfbEncodingTight);
  m_preferredEncoding.addItem(_T("RRE"), (void *)rfbEncodingRRE);
  m_preferredEncoding.addItem(_T("CoRRE"), (void *)rfbEncodingCoRRE);
  m_preferredEncoding.addItem(_T("Zlib(pure)"), (void *)rfbEncodingZlib);
  m_preferredEncoding.addItem(_T("ZlibHex(mix)"), (void *)rfbEncodingZlibHex);

  TCHAR scaleComboText[8][20] = {_T("25"), _T("50"), _T("75"), _T("90"),
                                 _T("100"), _T("125"), _T("150"), _T("Auto")};
  for (int i = 0; i <= 7; i++) {
    m_scale.addItem((TCHAR FAR *)scaleComboText[i]);
  }

  m_jpegCompressionLevel.setRange(1, 9);
  m_customCompressionLevel.setRange(1, 9);

  updateControls();

  return TRUE;
}

BOOL ConnectionConfigDialog::onNotify(UINT controlID, LPARAM data)
{
  return TRUE;
}

BOOL ConnectionConfigDialog::onCommand(UINT controlID, UINT notificationID)
{
  switch (controlID) {
  case IDC_SCALE_EDIT:
    if (notificationID == CBN_KILLFOCUS) {
      onScaleKillFocus();
    }
    break;
  case IDCANCEL:
    kill(IDCANCEL);
    break;
  case IDOK:
    onOkButtonClick();
    break;
  case IDC_ENCODING:
    if (notificationID == CBN_SELCHANGE) {
      onPreferredEncodingSelectionChange();
    }
    break;
  case IDC_ALLOW_COMPRESSLEVEL:
    if (notificationID == BN_CLICKED) {
      m_useCustomCompression.check(!m_useCustomCompression.isChecked());
      onAllowCustomCompressionClick();
    }
    break;
  case IDC_ALLOW_JPEG:
    if (notificationID == BN_CLICKED) {
      m_useJpegCompression.check(!m_useJpegCompression.isChecked());
      onAllowJpegCompressionClick();
    }
    break;
  case IDC_8BITCHECK:
    if (notificationID == BN_CLICKED) {
      m_use8BitColor.check(!m_use8BitColor.isChecked());
      on8BitColorClick();
    }
    break;
  case IDC_VIEWONLY:
    if (notificationID == BN_CLICKED) {
      m_viewOnly.check(!m_viewOnly.isChecked());
      onViewOnlyClick();
    }
    break;
  }
  return TRUE;
}

BOOL ConnectionConfigDialog::onDestroy()
{
  return TRUE;
}

void ConnectionConfigDialog::onMessageRecieved(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_HSCROLL:
    if (HWND(lParam) == m_customCompressionLevel.getWindow()) {
      onCustomCompressionLevelScroll();
    }
    if (HWND(lParam) == m_jpegCompressionLevel.getWindow()) {
      onJpegCompressionLevelScroll();
    }
    break;
  }
}

void ConnectionConfigDialog::onOkButtonClick()
{
  if (!isInputValid()) {
    return ;
  }
  apply();
  kill(IDOK);
}

void ConnectionConfigDialog::onViewOnlyClick()
{
  if (m_viewOnly.isChecked()) {
    m_swapMouse.setEnabled(false);
    m_emulate3Buttons.setEnabled(false);
  } else {
    m_swapMouse.setEnabled(true);
    m_emulate3Buttons.setEnabled(true);
  }
}

void ConnectionConfigDialog::on8BitColorClick()
{
  if (!m_use8BitColor.isChecked()) {
    if (m_useJpegCompression.isChecked()) {
      enableJpegCompression(true);
    }
    m_useJpegCompression.setEnabled(true);
  } else {
    m_useJpegCompression.setEnabled(false);
    enableJpegCompression(false);
  }
}

void ConnectionConfigDialog::onAllowCustomCompressionClick()
{
  enableCustomCompression(m_useCustomCompression.isChecked());
}

void ConnectionConfigDialog::onAllowJpegCompressionClick()
{
  enableJpegCompression(m_useJpegCompression.isChecked());
}

void ConnectionConfigDialog::onCustomCompressionLevelScroll()
{
  StringStorage labelText;
  labelText.format(_T("%ld"), m_customCompressionLevel.getPos());
  m_customCompressionLevelLabel.setText(labelText.getString());
}

void ConnectionConfigDialog::onJpegCompressionLevelScroll()
{
  StringStorage labelText;
  labelText.format(_T("%ld"), m_jpegCompressionLevel.getPos());
  m_jpegCompressionLevelLabel.setText(labelText.getString());
}

void ConnectionConfigDialog::onPreferredEncodingSelectionChange()
{
  int index = m_preferredEncoding.getSelectedItemIndex();
  if (index < 0) {
    return ;
  }
  int encoding = (int)m_preferredEncoding.getItemData(index);
  switch (encoding) {
  case rfbEncodingTight:
  case rfbEncodingZlib:
  case rfbEncodingZlibHex:
    enableCustomCompression(m_useCustomCompression.isChecked());
    m_useCustomCompression.setEnabled(true);
    break;
  case rfbEncodingRRE:
  case rfbEncodingCoRRE:
  case rfbEncodingRaw:
  case rfbEncodingHextile:
    enableCustomCompression(false);
    m_useCustomCompression.setEnabled(false);
    break;
  } // switch
} // void

void ConnectionConfigDialog::onScaleKillFocus()
{
  StringStorage scaleText;
  m_scale.getText(&scaleText);

  int scale;

  if (!StringParser::parseInt(scaleText.getString(), &scale)) {
    if (scaleText.isEqualTo(_T("Auto"))) {
      return ;
    }
    scale = 100;
  }

  if (scale < 1) {
    scale = 1;
  } else if (scale > 150) {
    scale = 150;
  }

  scaleText.format(_T("%d"), scale);
  m_scale.setText(scaleText.getString());
}

// FIXME: Does not use WinApi here.
// FIXME: Debug.
void ConnectionConfigDialog::enableCustomCompression(bool enable)
{
  HWND hwnd = m_ctrlThis.getWindow();

  HWND hfast = GetDlgItem(hwnd, IDC_STATIC_FAST);
  HWND hlevel = GetDlgItem(hwnd, IDC_STATIC_LEVEL);
  HWND htextlevel = GetDlgItem(hwnd, IDC_STATIC_TEXT_LEVEL);
  HWND hbest = GetDlgItem(hwnd, IDC_STATIC_BEST);
  HWND hCompressLevel = GetDlgItem(hwnd, IDC_COMPRESSLEVEL);

  EnableWindow(hCompressLevel,enable);
  EnableWindow(hfast, enable);
  EnableWindow(hlevel,enable);
  EnableWindow(htextlevel, enable);
  EnableWindow(hbest, enable);
}

// FIXME: Does not use WinApi here.
// FIXME: Debug.
void ConnectionConfigDialog::enableJpegCompression(bool enable)
{
  HWND hwnd = m_ctrlThis.getWindow();

  HWND hpoor = GetDlgItem(hwnd, IDC_STATIC_POOR);
  HWND hqbest = GetDlgItem(hwnd, IDC_STATIC_QBEST);
  HWND hqualitytext = GetDlgItem(hwnd, IDC_STATIC_TEXT_QUALITY);
  HWND hquality = GetDlgItem(hwnd, IDC_STATIC_QUALITY);
  HWND hJpeg = GetDlgItem(hwnd, IDC_QUALITYLEVEL);

  EnableWindow( hJpeg, enable);
  EnableWindow(hpoor, enable);
  EnableWindow(hqbest, enable);
  EnableWindow(hqualitytext, enable);
  EnableWindow(hquality, enable);
}

void ConnectionConfigDialog::initControls()
{
  HWND hWnd = m_ctrlThis.getWindow();

  m_preferredEncoding.setWindow(GetDlgItem(hWnd, IDC_ENCODING));
  m_use8BitColor.setWindow(GetDlgItem(hWnd, IDC_8BITCHECK));
  m_useCustomCompression.setWindow(GetDlgItem(hWnd, IDC_ALLOW_COMPRESSLEVEL));
  m_useJpegCompression.setWindow(GetDlgItem(hWnd, IDC_ALLOW_JPEG));
  m_allowCopyRect.setWindow(GetDlgItem(hWnd, ID_SESSION_SET_CRECT));
  m_viewOnly.setWindow(GetDlgItem(hWnd, IDC_VIEWONLY));
  m_disableClipboard.setWindow(GetDlgItem(hWnd, IDC_DISABLECLIPBOARD));
  m_fullscreen.setWindow(GetDlgItem(hWnd, IDC_FULLSCREEN));
  m_deiconifyOnRemoteBell.setWindow(GetDlgItem(hWnd, IDC_BELLDEICONIFY));
  m_emulate3Buttons.setWindow(GetDlgItem(hWnd, IDC_EMULATECHECK));
  m_swapMouse.setWindow(GetDlgItem(hWnd, ID_SESSION_SWAPMOUSE));
  m_shared.setWindow(GetDlgItem(hWnd, IDC_SHARED));
  m_scale.setWindow(GetDlgItem(hWnd, IDC_SCALE_EDIT));
  m_shapeEnable.setWindow(GetDlgItem(hWnd, IDC_CSHAPE_ENABLE_RADIO));
  m_shapeDisable.setWindow(GetDlgItem(hWnd, IDC_CSHAPE_DISABLE_RADIO));
  m_shapeIgnore.setWindow(GetDlgItem(hWnd, IDC_CSHAPE_IGNORE_RADIO));
  m_customCompressionLevel.setWindow(GetDlgItem(hWnd, IDC_COMPRESSLEVEL));
  m_jpegCompressionLevel.setWindow(GetDlgItem(hWnd, IDC_QUALITYLEVEL));
  m_customCompressionLevelLabel.setWindow(GetDlgItem(hWnd, IDC_STATIC_LEVEL));
  m_jpegCompressionLevelLabel.setWindow(GetDlgItem(hWnd, IDC_STATIC_QUALITY));

  m_localCursorShapeRadio[0].setWindow(GetDlgItem(hWnd, IDC_DOT_CURSOR_RADIO));
  m_localCursorShapeRadio[1].setWindow(GetDlgItem(hWnd, IDC_SMALL_DOT_CURSOR_RADIO));
  m_localCursorShapeRadio[2].setWindow(GetDlgItem(hWnd, IDC_NORMAL_ARROW_CURSOR_RADIO));
  m_localCursorShapeRadio[3].setWindow(GetDlgItem(hWnd, IDC_NO_LOCAL_CURSOR_RADIO));
}

void ConnectionConfigDialog::updateControls()
{
  m_preferredEncoding.setSelectedItem(2);

  // Preferred encoding
  for (int i = 0; i < m_preferredEncoding.getItemsCount(); i++) {
    int enc = (int)m_preferredEncoding.getItemData(i);
    if (enc == m_conConfig->getPreferredEncoding()) {
      m_preferredEncoding.setSelectedItem(i);
      break;
    } // if found
  } // for i

  m_use8BitColor.check(m_conConfig->isUsing8BitColor());

  m_useCustomCompression.check(m_conConfig->isCustomCompressionEnabled());
  m_useJpegCompression.check(m_conConfig->isJpegCompressionEnabled());

  m_allowCopyRect.check(m_conConfig->isEncodingAllowed(rfbEncodingCopyRect));
  m_viewOnly.check(m_conConfig->isViewOnly());
  m_disableClipboard.check(!m_conConfig->isClipboardEnabled());
  m_fullscreen.check(m_conConfig->isFullscreenEnabled());
  m_deiconifyOnRemoteBell.check(m_conConfig->isDeiconifyOnRemoteBellEnabled());
  m_emulate3Buttons.check(m_conConfig->isEmulate3ButtonsEnabled());
  m_swapMouse.check(m_conConfig->isMouseSwapEnabled());

  m_shared.check(m_conConfig->getSharedFlag());
  m_shared.setEnabled(!m_connected);

  if (m_conConfig->isFitWindowEnabled()) {
    m_scale.setSelectedItem(7);
  } else {
    int n = m_conConfig->getScaleNumerator();
    int d = m_conConfig->getScaleDenominator();

    int percent = (n * 100) / d;

    StringStorage text;
    text.format(_T("%d"), percent);

    m_scale.setText(text.getString());
  }

  m_shapeEnable.check(false);
  m_shapeDisable.check(false);
  m_shapeIgnore.check(false);

  if (m_conConfig->isRequestingShapeUpdates() &&
     !m_conConfig->isIgnoringShapeUpdates()) {
    m_shapeEnable.check(true);
  } else if (m_conConfig->isRequestingShapeUpdates()) {
    m_shapeIgnore.check(true);
  } else {
    m_shapeDisable.check(true);
  }

  StringStorage labelText;
  if (m_conConfig->isCustomCompressionEnabled()) {
    int level = m_conConfig->getCustomCompressionLevel();
    m_customCompressionLevel.setPos(level);
    labelText.format(_T("%d"), level);
  } else {
    labelText.setString(_T("1"));
    m_customCompressionLevel.setPos(1);
  }
  m_customCompressionLevelLabel.setText(labelText.getString());

  if (m_conConfig->isJpegCompressionEnabled()) {
    int level = m_conConfig->getJpegCompressionLevel();
    m_jpegCompressionLevel.setPos(level);
    labelText.format(_T("%d"), level);
  } else {
    labelText.setString(_T("1"));
    m_customCompressionLevel.setPos(1);
  }
  m_jpegCompressionLevelLabel.setText(labelText.getString());

  int cursorRadioIndex = 0;

  switch (m_conConfig->getLocalCursorShape()) {
  case ConnectionConfig::SMALL_CURSOR:
    cursorRadioIndex = 1;
    break;
  case ConnectionConfig::NORMAL_CURSOR:
    cursorRadioIndex = 2;
    break;
  case ConnectionConfig::NO_CURSOR:
    cursorRadioIndex = 3;
    break;
  }

  m_localCursorShapeRadio[cursorRadioIndex].check(true);

  onViewOnlyClick();

  onAllowCustomCompressionClick();
  onAllowJpegCompressionClick();

  onCustomCompressionLevelScroll();
  onJpegCompressionLevelScroll();

  onPreferredEncodingSelectionChange();
  on8BitColorClick();
}

void ConnectionConfigDialog::apply()
{
  // Preferred encoding
  int pesii = m_preferredEncoding.getSelectedItemIndex();
  int preferredEncoding = (int)m_preferredEncoding.getItemData(pesii);
  m_conConfig->setPreferredEncoding(preferredEncoding);

  if (m_useCustomCompression.isChecked()) {
    int level = (int)m_customCompressionLevel.getPos();
    m_conConfig->setCustomCompressionLevel(level);
  } else {
    m_conConfig->disableCustomCompression();
  }

  if (m_useJpegCompression.isChecked()) {
    int level = (int)m_jpegCompressionLevel.getPos();
    m_conConfig->setJpegCompressionLevel(level);
  } else {
    m_conConfig->disableJpegCompression();
  }

  m_conConfig->use8BitColor(m_use8BitColor.isChecked());
  m_conConfig->allowEncoding(rfbEncodingCopyRect, m_allowCopyRect.isChecked());
  m_conConfig->setViewOnly(m_viewOnly.isChecked());
  m_conConfig->enableClipboard(!m_disableClipboard.isChecked());
  m_conConfig->enableFullscreen(m_fullscreen.isChecked());
  m_conConfig->deiconifyOnRemoteBell(m_deiconifyOnRemoteBell.isChecked());
  m_conConfig->emulate3Buttons(m_emulate3Buttons.isChecked());
  m_conConfig->swapMouse(m_swapMouse.isChecked());
  m_conConfig->setSharedFlag(m_shared.isChecked());

  StringStorage scaleText;

  m_scale.getText(&scaleText);

  int scaleInt = 0;

  if (StringParser::parseInt(scaleText.getString(), &scaleInt)) {
    m_conConfig->setScale(scaleInt, 100);
    m_conConfig->fitWindow(false);
  } else {
    m_conConfig->fitWindow(true);
  }

  m_conConfig->requestShapeUpdates(false);
  m_conConfig->ignoreShapeUpdates(false);

  if (m_shapeEnable.isChecked()) {
    m_conConfig->requestShapeUpdates(true);
  } else if (m_shapeIgnore.isChecked()) {
    m_conConfig->requestShapeUpdates(true);
    m_conConfig->ignoreShapeUpdates(true);
  }

  int localCursorShape = ConnectionConfig::DOT_CURSOR;
  if (m_localCursorShapeRadio[1].isChecked()) {
    localCursorShape = ConnectionConfig::SMALL_CURSOR;
  } else if (m_localCursorShapeRadio[2].isChecked()) {
    localCursorShape = ConnectionConfig::NORMAL_CURSOR;
  } else if (m_localCursorShapeRadio[3].isChecked()) {
    localCursorShape = ConnectionConfig::NO_CURSOR;
  }

  m_conConfig->setLocalCursorShape(localCursorShape);
}

bool ConnectionConfigDialog::isInputValid()
{
  int scaleInt;
  StringStorage scaleText;

  m_scale.getText(&scaleText);

  if (scaleText.isEqualTo(_T("Auto"))) {
    return true;
  }

  if (!StringParser::parseInt(scaleText.getString(), &scaleInt)) {
    MessageBox(m_ctrlThis.getWindow(), _T("Scale paratemer must be numeric."),
               _T("Input data"), MB_OK | MB_ICONWARNING);
    return false;
  }

  if (scaleInt < 0) {
    MessageBox(m_ctrlThis.getWindow(), _T("Scale paratemer must be positive number."),
               _T("Input data"), MB_OK | MB_ICONWARNING);
    return false;
  }

  return true;
}
