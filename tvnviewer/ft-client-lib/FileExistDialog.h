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

#ifndef _FILE_EXIST_DIALOG_H_
#define _FILE_EXIST_DIALOG_H_

#include "gui/BaseDialog.h"
#include "gui/Control.h"
#include "ft-common/FileInfo.h"

class FileExistDialog : public BaseDialog
{
public:

  static const int OVERWRITE_RESULT = 0x0;
  static const int SKIP_RESULT = 0x1;
  static const int APPEND_RESULT = 0x2;
  static const int CANCEL_RESULT = 0x3;

public:
  FileExistDialog();
  ~FileExistDialog();

  virtual int showModal();

  void setFilesInfo(FileInfo *existingFileInfo, FileInfo *newFileInfo,
                    const TCHAR *pathToFileCaption);

  void resetDialogResultValue();

protected:

  virtual BOOL onInitDialog();
  virtual BOOL onNotify(UINT controlID, LPARAM data);
  virtual BOOL onCommand(UINT controlID, UINT notificationID);
  virtual BOOL onDestroy();

  void onOverwriteButtonClick();
  void onOverwriteAllButtonClick();
  void onSkipButtonClick();
  void onSkipAllButtonClick();
  void onAppendButtonClick();
  void onCancelButtonClick();

private:

  void updateGui(FileInfo *fileInfo, Control *sizeLabel, Control *modTimeLabel);
  void initControls();

protected:
  FileInfo *m_newFileInfo;
  FileInfo *m_existingFileInfo;
  StringStorage m_pathToFileCaption;

  bool m_controlsInitialized;
  bool m_overwriteAll;
  bool m_skipAll;

  Control m_fileNameLabel;
  Control m_newSizeLabel;
  Control m_newModTimeLabel;
  Control m_existingSizeLabel;
  Control m_existingModTimeLabel;

  Control m_appendButton;

  bool m_canAppend;
};

#endif
