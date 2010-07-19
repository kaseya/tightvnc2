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

#ifndef _FILE_TRANSFER_MAIN_DIALOG_H_
#define _FILE_TRANSFER_MAIN_DIALOG_H_

#include "gui/BaseDialog.h"
#include "gui/Control.h"
#include "gui/TextBox.h"
#include "gui/ComboBox.h"
#include "gui/ImagedButton.h"
#include "gui/ProgressBar.h"

#include "FileTransferEventHandler.h"
#include "FileTransferRequestSender.h"
#include "FileTransferReplyBuffer.h"
#include "FileInfoListView.h"
#include "FileTransferOperation.h"
#include "OperationEventListener.h"
#include "CopyFileEventListener.h"
#include "FileExistDialog.h"
#include "OperationSupport.h"

#include "ft-common/FileInfo.h"

#include <vector>

using namespace std;

class FileTransferMainDialog : public BaseDialog, OperationEventListener,
                               public CopyFileEventListener
{
public:
  friend class DownloadOperation;
  friend class UploadOperation;

  FileTransferMainDialog(FileTransferRequestSender *sender,
                         FileTransferReplyBuffer *replyBuffer,
                         ListenerContainer<FileTransferEventHandler *> *ftListeners);
  virtual ~FileTransferMainDialog();

  void updateSupportedOperations(CapsContainer *clientCaps, CapsContainer *serverCaps);

protected:

  virtual void dataChunkCopied(UINT64 totalBytesCopied, UINT64 totalBytesToCopy);

  virtual int targetFileExists(FileInfo *sourceFileInfo,
                               FileInfo *targetFileInfo,
                               const TCHAR *pathToTargetFile);

  virtual void ftOpStarted(FileTransferOperation *sender);
  virtual void ftOpFinished(FileTransferOperation *sender) throw(IOException);
  virtual void ftOpErrorMessage(FileTransferOperation *sender, const TCHAR *message);
  virtual void ftOpInfoMessage(FileTransferOperation *sender,
                               const TCHAR *message);

  virtual BOOL onInitDialog();
  virtual BOOL onNotify(UINT controlID, LPARAM data) throw(IOException);
  virtual BOOL onCommand(UINT controlID, UINT notificationID) throw(IOException);
  virtual BOOL onDestroy();

  virtual void onMessageRecieved(UINT uMsg, WPARAM wParam, LPARAM lParam);
  virtual BOOL onDrawItem(UINT controlID, LPDRAWITEMSTRUCT dis);

  void onCancelButtonClick();
  void onCancelOperationButtonClick();

  void onRenameRemoteButtonClick() throw(IOException);
  void onMkDirRemoteButtonClick() throw(IOException);
  void onRemoveRemoteButtonClick() throw(IOException);
  void onRefreshRemoteButtonClick() throw(IOException);

  void onRenameLocalButtonClick();
  void onMkDirLocalButtonClick();
  void onRemoveLocalButtonClick();
  void onRefreshLocalButtonClick();

  void onUploadButtonClick();
  void onDownloadButtonClick();

  void moveUpLocalFolder();
  void moveUpRemoteFolder() throw(IOException);

  void onRemoteListViewDoubleClick() throw(IOException);
  void onLocalListViewDoubleClick();

  void onRemoteListViewKeyDown(UINT key);
  void onLocalListViewKeyDown(UINT key);

  void checkRemoteListViewSelection();
  void checkLocalListViewSelection();

  void insertMessageIntoComboBox(const TCHAR *message);

private:

  void enableControls(bool enabled);

  void initControls();

  void raise(Exception &ex);

  void refreshLocalFileList();

  void refreshRemoteFileList() throw(IOException);

  void tryListLocalFolder(const TCHAR *pathToFile);

  void tryListRemoteFolder(const TCHAR *pathToFile) throw(IOException);

  void executeOperation(FileTransferOperation *newOperation) throw(IOException);

  void getPathToCurrentLocalFolder(StringStorage *out);
  void getPathToParentLocalFolder(StringStorage *out);
  void getPathToSelectedLocalFile(StringStorage *out);

  void getPathToCurrentRemoteFolder(StringStorage *out);
  void getPathToParentRemoteFolder(StringStorage *out);
  void getPathToSelectedRemoteFile(StringStorage *out);

protected:

  bool m_isClosing;

  int m_state;

  FileTransferOperation *m_currentOperation;

  ListenerContainer<FileTransferEventHandler *> *m_fileTransferListeners;

  UINT32 m_remoteFilesInfoCount;
  FileInfo *m_remoteFilesInfo;

  StringStorage m_lastSentFileListPath;
  StringStorage m_lastRecievedFileListPath;

  UINT32 m_localFilesInfoCount;
  FileInfo *m_localFilesInfo;

  FileTransferRequestSender *m_sender;
  FileTransferReplyBuffer *m_replyBuffer;

  Control m_renameRemoteButton;
  Control m_mkDirRemoteButton;
  Control m_removeRemoteButton;
  Control m_refreshRemoteButton;

  Control m_renameLocalButton;
  Control m_mkDirLocalButton;
  Control m_removeLocalButton;
  Control m_refreshLocalButton;

  ImagedButton m_uploadButton;
  ImagedButton m_downloadButton;

  Control m_cancelButton;

  ProgressBar m_copyProgressBar;

  ComboBox m_logComboBox;

  TextBox m_localCurFolderTextBox;
  TextBox m_remoteCurFolderTextBox;

  FileInfoListView m_localFileListView;
  FileInfoListView m_remoteFileListView;

  FileExistDialog m_fileExistDialog;

  FileInfo *m_fakeMoveUpFolder;

  OperationSupport *m_supportedOps;

private:

  static const int NOTHING_STATE  = 0x0;
  static const int FILE_LIST_STATE = 0x1;
  static const int REMOVE_STATE   = 0x2;
  static const int MKDIR_STATE    = 0x3;
  static const int RENAME_STATE   = 0x4;

  static const int LOCAL_REMOVE_STATE = 0x5;

  static const int UPLOAD_STATE = 0x6;
  static const int DOWNLOAD_STATE = 0x7;

  static const UINT WM_OPERATION_FINISHED = WM_USER + 2;
};

#endif
