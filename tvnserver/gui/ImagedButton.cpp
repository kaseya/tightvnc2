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

#include "ImagedButton.h"
#include <crtdbg.h>

ImagedButton::ImagedButton()
: m_mouseOver(false),
  m_isUsingTheme(false), m_theme(0), m_icon(0),
  m_iconWidth(0), m_iconHeight(0)
{
}

ImagedButton::~ImagedButton()
{
  if (m_theme) {
    ThemeLib::CloseThemeData(m_theme);
    m_theme = NULL;
  }
}

void ImagedButton::drawItem(LPDRAWITEMSTRUCT dis)
{
  HDC dc = dis->hDC;

  BOOL isPressed = (dis->itemState & ODS_SELECTED);
  BOOL isFocused = (dis->itemState & ODS_FOCUS);
  BOOL isDisabled = (dis->itemState & ODS_DISABLED);
  BOOL drawFocusRect = !(dis->itemState & ODS_NOFOCUSRECT);

  RECT itemRect = dis->rcItem;
  SetBkMode(dc, TRANSPARENT);

  if (m_isUsingTheme) {
    DWORD state = (isPressed) ? PBS_PRESSED : PBS_NORMAL;
    if (state == PBS_NORMAL) {
      if (isFocused) {
        state = PBS_DEFAULTED;
      } 
      if (m_mouseOver) {
        state = PBS_HOT;
      } 
      if (isDisabled) {
        state = PBS_DISABLED;
      }
    } 
    ThemeLib::DrawThemeBackground(m_theme, dc, BP_PUSHBUTTON, state, &itemRect, NULL);
  } else {
    if (isFocused) {
      HBRUSH br = CreateSolidBrush(RGB(0,0,0));
      FrameRect(dc, &itemRect, br);
      InflateRect(&itemRect, -1, -1);
      DeleteObject(br);
    } 

    HBRUSH background = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(dc, &itemRect, background);
    DeleteObject(background);

    if (isPressed) {
      HBRUSH shadow = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
      FrameRect(dc, &itemRect, shadow);
      DeleteObject(shadow);
    } else {
      UINT uState = DFCS_BUTTONPUSH |
                    ((m_mouseOver) ? DFCS_HOT : 0) |
                    ((isPressed) ? DFCS_PUSHED : 0);

      DrawFrameControl(dc, &itemRect, DFC_BUTTON, uState);
    } 
  } 

  StringStorage title;
  getText(&title);

  RECT captionRect = dis->rcItem;

  TEXTMETRIC metric;
  GetTextMetrics(dc, &metric);

  RECT imageRect;

  calcRect(&itemRect, isPressed == TRUE, 0, metric.tmHeight,
           m_iconWidth, m_iconHeight, &captionRect, &imageRect);

  if (m_icon != NULL) {
    drawIcon(&dc, &imageRect, isPressed == TRUE, isDisabled == TRUE);
  }

  if (!title.isEmpty()) {
    if (isPressed && !m_isUsingTheme) {
      OffsetRect(&captionRect, 1, 1);
    }

    if (m_isUsingTheme) {
      WCHAR *unicodeString = new WCHAR[title.getLength() + 1];
      size_t len = title.getLength();
      title.toUnicodeString(unicodeString, &len);

      DWORD state = PBS_NORMAL;

      if (isDisabled) {
        state = PBS_DISABLED;
      }

      ThemeLib::DrawThemeText(m_theme, dc, BP_PUSHBUTTON, state,
                              unicodeString, len,
                              DT_CENTER | DT_VCENTER | DT_SINGLELINE,
                              0, &captionRect);
      delete[] unicodeString;
    } else {
      SetBkMode(dc, TRANSPARENT);

      if (isDisabled) {
        OffsetRect(&captionRect, 1, 1);
        SetTextColor(dc, ::GetSysColor(COLOR_3DHILIGHT));
        DrawText(dc, title.getString(), -1, &captionRect, DT_WORDBREAK | DT_CENTER);
        OffsetRect(&captionRect, -1, -1);
        SetTextColor(dc, ::GetSysColor(COLOR_3DSHADOW));
        DrawText(dc, title.getString(), -1, &captionRect, DT_WORDBREAK | DT_CENTER);
      } else {
        SetTextColor(dc, ::GetSysColor(COLOR_BTNTEXT));
        SetBkColor(dc, ::GetSysColor(COLOR_BTNFACE));
        DrawText(dc, title.getString(), -1, &captionRect, DT_WORDBREAK | DT_CENTER);
      } 
    } 
  } 

  if (isFocused && drawFocusRect) {
    RECT focusRect = itemRect;
    InflateRect(&focusRect, -3, -3);
    DrawFocusRect(dc, &focusRect);
  } 
} 

void ImagedButton::setWindow(HWND hwnd)
{
  Control::setWindow(hwnd);
  replaceWindowProc(ImagedButton::wndProc);
  Control::addStyle(BS_OWNERDRAW);

  if (ThemeLib::isLoaded())  {
    if (m_theme) {
      ThemeLib::CloseThemeData(m_theme);
    }
    m_theme = ThemeLib::OpenThemeData(hwnd, L"Button");
    if (m_theme) {
      m_isUsingTheme = true;
    }
  }
}

void ImagedButton::setIcon(HICON *icon, int width, int height)
{
  m_icon = icon;
  m_iconWidth = width;
  m_iconHeight = height;
}

void ImagedButton::calcRect(RECT* buttonRect, bool isButtonPressed,
                            DWORD textWidth, DWORD textHeight,
                            DWORD imageWidth, DWORD imageHeight,
                            RECT *textRect, RECT* imageRect)
{
  CopyRect(imageRect, buttonRect);
  CopyRect(textRect, buttonRect);

  if (m_icon != NULL) {
    long buttonWidth = buttonRect->right - buttonRect->left;
    long buttonHeight = -buttonRect->top + buttonRect->bottom;

    imageRect->left += ((buttonWidth - (long)imageWidth) / 2);
    imageRect->top += (((buttonHeight) - (long)imageHeight) / 2) - textHeight;

    DWORD margin = 10;
    textRect->top += (textHeight + margin) * 2;
  }

  if (isButtonPressed && !m_isUsingTheme) {
    OffsetRect(imageRect, 1, 1);
  }
}

void ImagedButton::drawIcon(HDC* dc, RECT* imageRect, bool isPressed, bool isDisabled)
{
  DrawState(*dc, NULL, NULL, (LPARAM)*m_icon, 0,
            imageRect->left, imageRect->top,
            (imageRect->right - imageRect->left),
            (imageRect->bottom - imageRect->top), 
            (isDisabled ? DSS_DISABLED : DSS_NORMAL) | DST_ICON);
} 

LRESULT CALLBACK ImagedButton::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  ImagedButton *_this = (ImagedButton *)GetWindowLong(hWnd, GWL_USERDATA);

  switch (message) {
  case WM_LBUTTONDBLCLK:
    PostMessage(hWnd, WM_LBUTTONDOWN, wParam, lParam);
  break;
  case WM_MOUSEMOVE:
    if (!_this->m_mouseOver) {
      _this->m_mouseOver = true;

      TRACKMOUSEEVENT evt;

      evt.cbSize = sizeof(evt);
      evt.dwFlags = TME_LEAVE;
      evt.dwHoverTime = HOVER_DEFAULT;
      evt.hwndTrack = hWnd;

      _this->invalidate();

      TrackMouseEvent(&evt);
    }
    break;
  case WM_MOUSELEAVE:
    _this->m_mouseOver = false;
    _this->invalidate();
    break;
  } 
  return CallWindowProc((WNDPROC)_this->m_defWindowProc, hWnd, message, wParam, lParam);
}
