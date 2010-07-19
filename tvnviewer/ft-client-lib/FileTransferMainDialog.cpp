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

#include "util/winhdr.h"
#include "FileTransferMainDialog.h"
#include "NewFolderDialog.h"
#include "FileRenameDialog.h"

#include "RemoteFileListOperation.h"
#include "RemoteFilesDeleteOperation.h"
#include "RemoteFolderCreateOperation.h"
#include "RemoteFileRenameOperation.h"
#include "LocalFilesDeleteOperation.h"
#include "UploadOperation.h"
#include "DownloadOperation.h"

#include "file-lib/File.h"
#include "ft-common/FolderListener.h"

#include "util/Log.h"

#include "resource.h"
#include <stdio.h>

FileTransferMainDialog::FileTransferMainDialog(FileTransferRequestSender *sender,
                                               FileTransferReplyBuffer *replyBuffer,
                                               ListenerContainer<FileTransferEventHandler *> *ftListeners)
: m_sender(sender), m_replyBuffer(replyBuffer),
  m_fileTransferListeners(ftListeners),
  m_remoteFilesInfoCount(0), m_remoteFilesInfo(0),
  m_currentOperation(0), m_state(NOTHING_STATE),
  m_localFilesInfo(0), m_localFilesInfoCount(0),
  m_supportedOps(0)
{
  setResourceName(_T("ftclient.mainDialog"));

  m_lastSentFileListPath.setString(_T(""));
  m_lastRecievedFileListPath.setString(_T(""));

  m_fakeMoveUpFolder = new FileInfo(0, 0, FileInfo::DIRECTORY, _T(".."));
}

FileTransferMainDialog::~FileTransferMainDialog()
{
  delete m_fakeMoveUpFolder;

  if (m_remoteFilesInfo != NULL) {
    delete[] m_remoteFilesInfo;
  }
  if (m_currentOperation != NULL) {
    delete m_currentOperation;
  }
  if (m_localFilesInfo != NULL) {
    delete[] m_localFilesInfo;
  }
  if (m_supportedOps != NULL) {
    delete m_supportedOps;
  }
}

void FileTransferMainDialog::updateSupportedOperations(CapsContainer *clientCaps, CapsContainer *serverCaps)
{
  if (m_supportedOps != NULL) {
    delete m_supportedOps;
  }

  m_supportedOps = new OperationSupport(clientCaps, serverCaps);
}

void FileTransferMainDialog::dataChunkCopied(UINT64 totalBytesCopied, UINT64 totalBytesToCopy)
{
  if (m_state == NOTHING_STATE) {
    return ;
  }

  if (totalBytesToCopy != 0) {
    double koef = ((double)totalBytesCopied / totalBytesToCopy * 1.0);
    UINT32 pt = 1000;
    UINT32 pc = (UINT32)(koef * (double)pt);

    m_copyProgressBar.setPos(pc);
  }
}

int FileTransferMainDialog::targetFileExists(FileInfo *sourceFileInfo,
                                             FileInfo *targetFileInfo,
                                             const TCHAR *pathToTargetFile)
{
  m_fileExistDialog.setFilesInfo(targetFileInfo,
                                 sourceFileInfo,
                                 pathToTargetFile);

  switch (m_fileExistDialog.showModal()) {
  case FileExistDialog::SKIP_RESULT:
    return CopyFileEventListener::TFE_SKIP;
  case FileExistDialog::APPEND_RESULT:
    return CopyFileEventListener::TFE_APPEND;
  case FileExistDialog::CANCEL_RESULT:
    onCancelOperationButtonClick();
    return CopyFileEventListener::TFE_CANCEL;
  } 
  return CopyFileEventListener::TFE_OVERWRITE;
} 

void FileTransferMainDialog::ftOpStarted(FileTransferOperation *sender)
{
  enableControls(false);
}

void FileTransferMainDialog::ftOpFinished(FileTransferOperation *sender)
{

  if (m_state == FILE_LIST_STATE) {
    RemoteFileListOperation *fileListOp = (RemoteFileListOperation *)sender;

    if (fileListOp->isOk()) {
      if (m_remoteFilesInfo != NULL) {
        delete[] m_remoteFilesInfo;
      }

      m_remoteFilesInfoCount = m_replyBuffer->getFilesInfoCount();
      m_remoteFilesInfo = new FileInfo[m_remoteFilesInfoCount];
      for (UINT32 i = 0; i < m_remoteFilesInfoCount; i++) {
        m_remoteFilesInfo[i] = m_replyBuffer->getFilesInfo()[i];
      } 
    } 

    PostMessage(m_ctrlThis.getWindow(), WM_OPERATION_FINISHED, m_state, fileListOp->isOk() ? 1 : 0);
    return ;
  } 

  PostMessage(m_ctrlThis.getWindow(), WM_OPERATION_FINISHED, m_state, 0);
} 

void FileTransferMainDialog::ftOpErrorMessage(FileTransferOperation *sender,
                                              const TCHAR *message)
{
  insertMessageIntoComboBox(message);
}

void FileTransferMainDialog::ftOpInfoMessage(FileTransferOperation *sender,
                                             const TCHAR *message)
{
  insertMessageIntoComboBox(message);
}

BOOL FileTransferMainDialog::onInitDialog()
{
  m_isClosing = false;

  initControls();

  tryListRemoteFolder(_T("/"));
  tryListLocalFolder(_T(""));

  return TRUE;
}

BOOL FileTransferMainDialog::onNotify(UINT controlID, LPARAM data)
{
  LPNMHDR nmhdr = (LPNMHDR)data;
  switch (controlID) {
  case IDC_REMOTE_FILE_LIST:
    switch (nmhdr->code) {
    case NM_DBLCLK:
      onRemoteListViewDoubleClick();
      break;
    case LVN_KEYDOWN:
      {
        LPNMLVKEYDOWN nmlvkd = (LPNMLVKEYDOWN)data;
        onRemoteListViewKeyDown(nmlvkd->wVKey);
      }
      break;
    } 

    checkRemoteListViewSelection();
    break;
  case IDC_LOCAL_FILE_LIST:
    switch (nmhdr->code) {
    case NM_DBLCLK:
      onLocalListViewDoubleClick();
      break;
    case LVN_KEYDOWN:
      {
        LPNMLVKEYDOWN nmlvkd = (LPNMLVKEYDOWN)data;
        onLocalListViewKeyDown(nmlvkd->wVKey);
      }
      break;
    } 

    checkLocalListViewSelection();
    break;
  } 
  return TRUE;
}

BOOL FileTransferMainDialog::onCommand(UINT controlID, UINT notificationID)
{
  switch (controlID) {
  case IDCANCEL:
    onCancelButtonClick();
    break;
  case IDC_CANCEL_BUTTON:
    onCancelOperationButtonClick();
    break;
  case IDC_RENAME_REMOTE_BUTTON:
    onRenameRemoteButtonClick();
    break;
  case IDC_MKDIR_REMOTE_BUTTON:
    onMkDirRemoteButtonClick();
    break;
  case IDC_REMOVE_REMOTE_BUTTON:
    onRemoveRemoteButtonClick();
    break;
  case IDC_REFRESH_REMOTE_BUTTON:
    onRefreshRemoteButtonClick();
    break;
  case IDC_RENAME_LOCAL_BUTTON:
    onRenameLocalButtonClick();
    break;
  case IDC_MKDIR_LOCAL_BUTTON:
    onMkDirLocalButtonClick();
    break;
  case IDC_REMOVE_LOCAL_BUTTON:
    onRemoveLocalButtonClick();
    break;
  case IDC_REFRESH_LOCAL_BUTTON:
    onRefreshLocalButtonClick();
    break;
  case IDC_UPLOAD_BUTTON:
    onUploadButtonClick();
    break;
  case IDC_DOWNLOAD_BUTTON:
    onDownloadButtonClick();
    break;
  }
  return TRUE;
}

BOOL FileTransferMainDialog::onDestroy()
{
  return TRUE;
}

void FileTransferMainDialog::onMessageRecieved(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_OPERATION_FINISHED:
    m_fileTransferListeners->removeListener(m_currentOperation);

    if (m_isClosing) {
      kill(0);
      return ;
    }

    int state = (int)wParam;
    switch (state) {
      case FILE_LIST_STATE:
        m_state = NOTHING_STATE;

        if (lParam == 0) {
          break;
        }

        m_lastRecievedFileListPath = m_lastSentFileListPath;
        m_remoteCurFolderTextBox.setText(m_lastRecievedFileListPath.getString());

        m_remoteFileListView.clear();
        m_remoteFileListView.addRange(&m_remoteFilesInfo, m_remoteFilesInfoCount);

        {
          bool isRoot = m_lastSentFileListPath.isEqualTo(_T("/"));

          if (!isRoot) {
            m_remoteFileListView.addItem(0, m_fakeMoveUpFolder);
          }
        }
        break;
      case REMOVE_STATE:
        refreshRemoteFileList();
        break;
      case MKDIR_STATE:
        refreshRemoteFileList();
        break;
      case RENAME_STATE:
        refreshRemoteFileList();
        break;
      case LOCAL_REMOVE_STATE:
        refreshLocalFileList();
        m_state = NOTHING_STATE;
        break;
      case UPLOAD_STATE:
        refreshRemoteFileList();
        break;
      case DOWNLOAD_STATE:
        refreshLocalFileList();
        m_state = NOTHING_STATE;
        break;
    } 

    m_copyProgressBar.setPos(0);
    enableControls(true);
    break;
  } 
} 

BOOL FileTransferMainDialog::onDrawItem(UINT controlID, LPDRAWITEMSTRUCT dis)
{
  if (controlID == IDC_UPLOAD_BUTTON) {
    m_uploadButton.drawItem(dis);
  }
  if (controlID == IDC_DOWNLOAD_BUTTON) {
    m_downloadButton.drawItem(dis);
  }
  return TRUE;
}

void FileTransferMainDialog::onCancelButtonClick()
{
  if (m_state != NOTHING_STATE) {
    if (MessageBox(m_ctrlThis.getWindow(),
                   _T("Do you want to close file transfer and terminate current operation?"),
                   _T("File transfer"),
                   MB_YESNO | MB_ICONQUESTION) != IDYES) {
      return ;
    } 
    m_isClosing = true;
    m_currentOperation->terminate();
  } else {
    kill(IDCANCEL);
  }
}

void FileTransferMainDialog::onCancelOperationButtonClick()
{
  if (m_state != NOTHING_STATE) {
    StringStorage message(_T("Operation have been canceled by user"));

    insertMessageIntoComboBox(message.getString());

    m_currentOperation->terminate();

    m_cancelButton.setEnabled(false);
  }
}

void FileTransferMainDialog::onRenameRemoteButtonClick()
{
  FileInfo *fileInfo = m_remoteFileListView.getSelectedFileInfo();

  if (fileInfo == NULL) {
    MessageBox(m_ctrlThis.getWindow(),
               _T("No file selected"),
               _T("Rename"), MB_OK | MB_ICONWARNING);
    return ;
  }

  FileRenameDialog renameDialog(&m_ctrlThis);
  renameDialog.setFileName(fileInfo->getFileName());

  if (renameDialog.showModal() == IDOK) {
    StringStorage remoteFolder;
    getPathToCurrentRemoteFolder(&remoteFolder);

    StringStorage oldName(fileInfo->getFileName());

    StringStorage newName;
    renameDialog.getFileName(&newName);

    m_state = RENAME_STATE;

    executeOperation(new RemoteFileRenameOperation(FileInfo(0, 0, FileInfo::DIRECTORY, oldName.getString()),
                                                   FileInfo(0, 0, FileInfo::DIRECTORY, newName.getString()),
                                                   remoteFolder.getString()));
  }
} 

void FileTransferMainDialog::onMkDirRemoteButtonClick()
{
  NewFolderDialog folderDialog(&m_ctrlThis);
  if (folderDialog.showModal() == IDOK) {
    StringStorage remoteFolder;
    m_remoteCurFolderTextBox.getText(&remoteFolder);

    StringStorage fileName;
    folderDialog.getFileName(&fileName);

    m_state = MKDIR_STATE;

    executeOperation(new RemoteFolderCreateOperation(FileInfo(0, 0,
                                                     FileInfo::DIRECTORY,
                                                     fileName.getString()),
                                                     remoteFolder.getString()));
  }
}

void FileTransferMainDialog::onRemoveRemoteButtonClick()
{
  unsigned int siCount = m_remoteFileListView.getSelectedItemsCount();

  if (siCount == 0) {
    MessageBox(m_ctrlThis.getWindow(),
               _T("No files selected"),
               _T("Delete"), MB_OK | MB_ICONWARNING);
    return ;
  }

  int *indexes = new int[siCount];
  FileInfo *filesInfo = new FileInfo[siCount];

  m_remoteFileListView.getSelectedItemsIndexes(indexes);
  for (unsigned int i = 0; i < siCount; i++) {
    FileInfo *fileInfo = (FileInfo *)m_remoteFileListView.getItemData(indexes[i]);
    filesInfo[i] = *fileInfo;
  }

  if (MessageBox(m_ctrlThis.getWindow(),
                 _T("Do you wish to delete selected files?"),
                 _T("Delete"),
                 MB_YESNO | MB_ICONQUESTION) != IDYES) {
    return ;
  }

  StringStorage remoteFolder;
  m_remoteCurFolderTextBox.getText(&remoteFolder);

  m_state = REMOVE_STATE;
  executeOperation(new RemoteFilesDeleteOperation(filesInfo, siCount,
                                                  remoteFolder.getString()));

  delete[] indexes;
  delete[] filesInfo;
}

void FileTransferMainDialog::onRefreshRemoteButtonClick()
{
  refreshRemoteFileList();
}

void FileTransferMainDialog::onRenameLocalButtonClick()
{
  FileInfo *fileInfo = m_localFileListView.getSelectedFileInfo();

  if (fileInfo == NULL) {
    MessageBox(m_ctrlThis.getWindow(),
               _T("No file selected"),
               _T("Rename"), MB_OK | MB_ICONWARNING);
    return ;
  }

  FileRenameDialog renameDialog(&m_ctrlThis);
  renameDialog.setFileName(fileInfo->getFileName());

  if (renameDialog.showModal() == IDOK) {
    StringStorage localFolder;
    getPathToCurrentLocalFolder(&localFolder);

    StringStorage oldName;
    StringStorage newName;

    oldName.setString(fileInfo->getFileName());
    renameDialog.getFileName(&newName);

    StringStorage pathToOldFile(localFolder.getString());
    StringStorage pathToNewFile(localFolder.getString());

    if (!localFolder.endsWith('\\')) {
      pathToOldFile.appendString(_T("\\"));
      pathToNewFile.appendString(_T("\\"));
    }

    pathToOldFile.appendString(oldName.getString());
    pathToNewFile.appendString(newName.getString());

    StringStorage message;

    message.format(_T("Renaming local file '%s' to '%s'"),
                   pathToOldFile.getString(), pathToNewFile.getString());

    insertMessageIntoComboBox(message.getString());

    File oldFile(pathToOldFile.getString());

    if (!oldFile.renameTo(pathToNewFile.getString())) {
      message.format(_T("Error: failed to rename local '%s' file"),
                     pathToOldFile.getString());

      insertMessageIntoComboBox(message.getString());
    }

    refreshLocalFileList();
  } 
} 

void FileTransferMainDialog::onMkDirLocalButtonClick()
{
  StringStorage pathToFile;

  getPathToCurrentLocalFolder(&pathToFile);

  if (pathToFile.isEmpty()) {
    MessageBox(m_ctrlThis.getWindow(),
               _T("It's not allowed to create folder in this catalog"),
               _T("New folder"), MB_OK | MB_ICONWARNING);
  }

  NewFolderDialog folderDialog(&m_ctrlThis);

  if (folderDialog.showModal() == IDOK) {
    StringStorage fileName;
    folderDialog.getFileName(&fileName);

    if (!pathToFile.endsWith(_T('\\'))) {
      pathToFile.appendString(_T("\\"));
    }
    pathToFile.appendString(fileName.getString());

    StringStorage message;

    message.format(_T("Creating local '%s' folder"), pathToFile.getString());

    insertMessageIntoComboBox(message.getString());

    File file(pathToFile.getString());

    if (pathToFile.isEmpty() || !file.mkdir()) {
      message.format(_T("Error: failed to create local '%s' folder'"),
                     pathToFile.getString());

      insertMessageIntoComboBox(message.getString());
    }

    refreshLocalFileList();
  } 
} 

void FileTransferMainDialog::onRemoveLocalButtonClick()
{
  unsigned int siCount = m_localFileListView.getSelectedItemsCount();

  if (siCount == 0) {
    MessageBox(m_ctrlThis.getWindow(),
               _T("No files selected"),
               _T("Delete"), MB_OK | MB_ICONWARNING);
    return ;
  }

  int *indexes = new int[siCount];
  FileInfo *filesInfo = new FileInfo[siCount];

  m_localFileListView.getSelectedItemsIndexes(indexes);
  for (unsigned int i = 0; i < siCount; i++) {
    FileInfo *fileInfo = (FileInfo *)m_localFileListView.getItemData(indexes[i]);
    filesInfo[i] = *fileInfo;
  }

  if (MessageBox(m_ctrlThis.getWindow(),
                 _T("Do you wish to delete selected files?"),
                 _T("Delete"),
                 MB_YESNO | MB_ICONQUESTION) != IDYES) {
    return ;
  }

  StringStorage localFolder;
  getPathToCurrentLocalFolder(&localFolder);

  m_state = LOCAL_REMOVE_STATE;
  executeOperation(new LocalFilesDeleteOperation(filesInfo, siCount,
                                                 localFolder.getString()));

  delete[] indexes;
  delete[] filesInfo;
}

void FileTransferMainDialog::onRefreshLocalButtonClick()
{
  refreshLocalFileList();
}

void FileTransferMainDialog::onUploadButtonClick()
{
  unsigned int siCount = m_localFileListView.getSelectedItemsCount();

  if (siCount == 0) {
    MessageBox(m_ctrlThis.getWindow(),
               _T("No files selected"),
               _T("Upload"), MB_OK | MB_ICONWARNING);
    return ;
  }

  int *indexes = new int[siCount];
  FileInfo *filesInfo = new FileInfo[siCount];

  m_localFileListView.getSelectedItemsIndexes(indexes);
  for (unsigned int i = 0; i < siCount; i++) {
    FileInfo *fileInfo = (FileInfo *)m_localFileListView.getItemData(indexes[i]);
    filesInfo[i] = *fileInfo;
  }

  if (MessageBox(m_ctrlThis.getWindow(),
                 _T("Do you wish to upload selected files?"),
                 _T("Upload"),
                 MB_YESNO | MB_ICONQUESTION) != IDYES) {
    return ;
  }

  StringStorage localFolder;
  getPathToCurrentLocalFolder(&localFolder);

  StringStorage remoteFolder;
  getPathToCurrentRemoteFolder(&remoteFolder);

  m_state = UPLOAD_STATE;

  m_fileExistDialog.resetDialogResultValue();

  UploadOperation *uOp = new UploadOperation(filesInfo, siCount,
                                             localFolder.getString(),
                                             remoteFolder.getString());
  uOp->setCopyProcessListener(this);

  executeOperation(uOp);

  delete[] indexes;
  delete[] filesInfo;
}

void FileTransferMainDialog::onDownloadButtonClick()
{
  unsigned int siCount = m_remoteFileListView.getSelectedItemsCount();

  if (siCount == 0) {
    MessageBox(m_ctrlThis.getWindow(),
               _T("No files selected"),
               _T("Download"), MB_OK | MB_ICONWARNING);
    return ;
  }

  int *indexes = new int[siCount];
  FileInfo *filesInfo = new FileInfo[siCount];

  m_remoteFileListView.getSelectedItemsIndexes(indexes);
  for (unsigned int i = 0; i < siCount; i++) {
    FileInfo *fileInfo = (FileInfo *)m_remoteFileListView.getItemData(indexes[i]);
    filesInfo[i] = *fileInfo;
  }

  if (MessageBox(m_ctrlThis.getWindow(),
                 _T("Do you wish to download selected files?"),
                 _T("Download"),
                 MB_YESNO | MB_ICONQUESTION) != IDYES) {
    return ;
  }

  StringStorage remoteFolder;
  getPathToCurrentRemoteFolder(&remoteFolder);

  StringStorage localFolder;
  getPathToCurrentLocalFolder(&localFolder);

  m_state = DOWNLOAD_STATE;

  m_fileExistDialog.resetDialogResultValue();

  DownloadOperation *dOp = new DownloadOperation(filesInfo, siCount,
                                                 localFolder.getString(),
                                                 remoteFolder.getString());
  dOp->setCopyProcessListener(this);

  executeOperation(dOp);

  delete[] indexes;
  delete[] filesInfo;
}

void FileTransferMainDialog::moveUpLocalFolder()
{
  StringStorage pathToFile;
  getPathToParentLocalFolder(&pathToFile);
  tryListLocalFolder(pathToFile.getString());
}

void FileTransferMainDialog::moveUpRemoteFolder()
{
  StringStorage parent;
  getPathToParentRemoteFolder(&parent);
  tryListRemoteFolder(parent.getString());
}

void FileTransferMainDialog::onRemoteListViewDoubleClick()
{
  FileInfo *selFileInfo = m_remoteFileListView.getSelectedFileInfo();

  int si = m_remoteFileListView.getSelectedIndex();

  if ((si == 0) && (selFileInfo != 0) && (_tcscmp(selFileInfo->getFileName(), _T(".."))) == 0) {
    moveUpRemoteFolder();
    return ;
  }
  if (si == -1) {
    return ;
  }
  StringStorage pathToFile;
  getPathToSelectedRemoteFile(&pathToFile);
  tryListRemoteFolder(pathToFile.getString());
}

void FileTransferMainDialog::onLocalListViewDoubleClick()
{
  int si = m_localFileListView.getSelectedIndex();

  FileInfo *selFileInfo = m_localFileListView.getSelectedFileInfo();

  if ((si == 0) && (selFileInfo != 0) && (_tcscmp(selFileInfo->getFileName(), _T(".."))) == 0) {
    moveUpLocalFolder();
    return ;
  }
  if (si == -1) {
    return ;
  }

  StringStorage pathToFile;
  getPathToSelectedLocalFile(&pathToFile);
  tryListLocalFolder(pathToFile.getString());
}

void FileTransferMainDialog::onRemoteListViewKeyDown(UINT key)
{
  switch (key) {
  case VK_RETURN:
    onRemoteListViewDoubleClick();
    break;
  case VK_DELETE:
    onRemoveRemoteButtonClick();
    break;
  case VK_BACK:
    moveUpRemoteFolder();
    break;
  }
}

void FileTransferMainDialog::onLocalListViewKeyDown(UINT key)
{
  switch (key) {
  case VK_RETURN:
    onLocalListViewDoubleClick();
    break;
  case VK_DELETE:
    onRemoveLocalButtonClick();
    break;
  case VK_BACK:
    moveUpLocalFolder();
    break;
  }
}

void FileTransferMainDialog::checkRemoteListViewSelection()
{
  if (m_state != NOTHING_STATE) {
    return ;
  }

  bool enabled = m_remoteFileListView.getSelectedItemsCount() > 0;

  m_renameRemoteButton.setEnabled(enabled && m_supportedOps->isRenameSupported());
  m_removeRemoteButton.setEnabled(enabled && m_supportedOps->isRemoveSupported());
}

void FileTransferMainDialog::checkLocalListViewSelection()
{
  if (m_state != NOTHING_STATE) {
    return ;
  }

  bool enabled = m_localFileListView.getSelectedItemsCount() > 0;

  m_renameLocalButton.setEnabled(enabled);
  m_removeLocalButton.setEnabled(enabled);
}

void FileTransferMainDialog::insertMessageIntoComboBox(const TCHAR *message)
{
  m_logComboBox.insertItem(0, message);
  m_logComboBox.setSelectedItem(0);
}

void FileTransferMainDialog::enableControls(bool enabled)
{
  m_mkDirRemoteButton.setEnabled(enabled && m_supportedOps->isMkDirSupported());

  if (m_remoteFileListView.getSelectedItemsCount() > 0 && enabled) {
    m_renameRemoteButton.setEnabled(true && m_supportedOps->isRenameSupported());
    m_removeRemoteButton.setEnabled(true && m_supportedOps->isRemoveSupported());
  } else {
    m_renameRemoteButton.setEnabled(enabled && m_supportedOps->isRenameSupported());
    m_removeRemoteButton.setEnabled(enabled && m_supportedOps->isRemoveSupported());
  }

  m_refreshRemoteButton.setEnabled(enabled);

  if (enabled) {
    StringStorage curLocalPath;

    getPathToCurrentLocalFolder(&curLocalPath);

    if (!curLocalPath.isEmpty()) {
      m_mkDirLocalButton.setEnabled(true);
    }
  } else {
    m_mkDirLocalButton.setEnabled(enabled);
  }

  if (m_localFileListView.getSelectedItemsCount() > 0 && enabled) {
    m_renameLocalButton.setEnabled(true);
    m_removeLocalButton.setEnabled(true);
  } else {
    m_renameLocalButton.setEnabled(enabled);
    m_removeLocalButton.setEnabled(enabled);
  }

  m_refreshLocalButton.setEnabled(enabled);

  m_uploadButton.setEnabled(enabled && m_supportedOps->isUploadSupported());
  m_downloadButton.setEnabled(enabled && m_supportedOps->isDownloadSupported());

  m_localFileListView.setEnabled(enabled);
  m_remoteFileListView.setEnabled(enabled);

  m_cancelButton.setEnabled(!enabled);
}

void FileTransferMainDialog::initControls()
{
  HWND hwnd = m_ctrlThis.getWindow();

  m_renameRemoteButton.setWindow(GetDlgItem(hwnd, IDC_RENAME_REMOTE_BUTTON));
  m_mkDirRemoteButton.setWindow(GetDlgItem(hwnd, IDC_MKDIR_REMOTE_BUTTON));
  m_removeRemoteButton.setWindow(GetDlgItem(hwnd, IDC_REMOVE_REMOTE_BUTTON));
  m_refreshRemoteButton.setWindow(GetDlgItem(hwnd, IDC_REFRESH_REMOTE_BUTTON));

  m_renameLocalButton.setWindow(GetDlgItem(hwnd, IDC_RENAME_LOCAL_BUTTON));
  m_mkDirLocalButton.setWindow(GetDlgItem(hwnd, IDC_MKDIR_LOCAL_BUTTON));
  m_removeLocalButton.setWindow(GetDlgItem(hwnd, IDC_REMOVE_LOCAL_BUTTON));
  m_refreshLocalButton.setWindow(GetDlgItem(hwnd, IDC_REFRESH_LOCAL_BUTTON));

  m_uploadButton.setWindow(GetDlgItem(hwnd, IDC_UPLOAD_BUTTON));
  m_downloadButton.setWindow(GetDlgItem(hwnd, IDC_DOWNLOAD_BUTTON));

  m_cancelButton.setWindow(GetDlgItem(hwnd, IDC_CANCEL_BUTTON));

  m_copyProgressBar.setWindow(GetDlgItem(hwnd, IDC_TOTAL_PROGRESS));
  m_copyProgressBar.setRange(0, 1000);

  m_logComboBox.setWindow(GetDlgItem(hwnd, IDC_LOG_COMBO));

  m_localCurFolderTextBox.setWindow(GetDlgItem(hwnd, IDC_LOCAL_CURRENT_FOLDER_EDIT));
  m_remoteCurFolderTextBox.setWindow(GetDlgItem(hwnd, IDC_REMOTE_CURRENT_FOLDER_EDIT));

  m_localFileListView.setWindow(GetDlgItem(hwnd, IDC_LOCAL_FILE_LIST));
  m_remoteFileListView.setWindow(GetDlgItem(hwnd, IDC_REMOTE_FILE_LIST));

  m_fileExistDialog.setParent(&m_ctrlThis);
}

void FileTransferMainDialog::raise(Exception &ex)
{
  MessageBox(m_ctrlThis.getWindow(), ex.getMessage(),
             _T("Exception"), MB_OK | MB_ICONERROR);
  throw ex;
}

void FileTransferMainDialog::refreshLocalFileList()
{
  StringStorage pathToFile;
  getPathToCurrentLocalFolder(&pathToFile);
  tryListLocalFolder(pathToFile.getString());
}

void FileTransferMainDialog::tryListLocalFolder(const TCHAR *pathToFile)
{
  FolderListener fl(pathToFile);

  if (!fl.list()) {
    StringStorage message;

    message.format(_T("Error: failed to get file list from local '%s' folder"),
                   pathToFile);

    insertMessageIntoComboBox(message.getString());
    return ;
  }

  if (m_localFilesInfo != NULL) {
    delete[] m_localFilesInfo;
  }

  m_localFileListView.clear();
  m_localFilesInfoCount = fl.getFilesCount();
  m_localFilesInfo = new FileInfo[m_localFilesInfoCount];
  for (UINT32 i = 0; i < m_localFilesInfoCount; i++) {
    m_localFilesInfo[i] = fl.getFilesInfo()[i];
  }

  m_localFileListView.addRange(&m_localFilesInfo,
                               m_localFilesInfoCount);

  bool isRoot = (_tcscmp(pathToFile, _T("")) == 0);

  if (!isRoot) {
    m_localFileListView.addItem(0, m_fakeMoveUpFolder);
  }
  m_localCurFolderTextBox.setText(pathToFile);
  m_mkDirLocalButton.setEnabled(!isRoot);
}

void FileTransferMainDialog::refreshRemoteFileList()
{
  StringStorage currentFolder;
  m_remoteCurFolderTextBox.getText(&currentFolder);
  tryListRemoteFolder(currentFolder.getString());
}

void FileTransferMainDialog::tryListRemoteFolder(const TCHAR *pathToFile)
{
  m_lastSentFileListPath.setString(pathToFile);
  m_state = FILE_LIST_STATE;
  executeOperation(new RemoteFileListOperation(pathToFile));
}

void FileTransferMainDialog::executeOperation(FileTransferOperation *newOperation)
    
{

  if (m_currentOperation != NULL) {
    m_currentOperation->removeListener(this);
    m_fileTransferListeners->removeListener(m_currentOperation);
    delete m_currentOperation;
  }

  m_currentOperation = newOperation;

  m_currentOperation->setRequestSender(m_sender);
  m_currentOperation->setReplyBuffer(m_replyBuffer);
  m_currentOperation->addListener(this);

  m_fileTransferListeners->addListener(m_currentOperation);

  try {
    m_currentOperation->start();
  } catch (IOException &ioEx) {
    raise(ioEx);
  } 
}

void FileTransferMainDialog::getPathToCurrentLocalFolder(StringStorage *out)
{
  m_localCurFolderTextBox.getText(out);
}

void FileTransferMainDialog::getPathToParentLocalFolder(StringStorage *out)
{
  getPathToCurrentLocalFolder(out);
  int ld = out->findLast(_T('\\'));
  if (ld >= 0) {
    out->getSubstring(out, 0, ld);  
  } else {
    out->setString(_T(""));
    return ;
  }
  if (out->endsWith('\\') && (out->getLength() > 2)) {
    out->getSubstring(out, 0, out->getLength() - 2);
  }
}

void FileTransferMainDialog::getPathToSelectedLocalFile(StringStorage *out)
{
  StringStorage *pathToFile = out;
  getPathToCurrentLocalFolder(pathToFile);

  if (!pathToFile->isEmpty() && !pathToFile->endsWith(_T('\\'))) {
    pathToFile->appendString(_T("\\"));
  }

  const TCHAR *filename = m_localFileListView.getSelectedFileInfo()->getFileName();
  pathToFile->appendString(filename);
}

void FileTransferMainDialog::getPathToCurrentRemoteFolder(StringStorage *out)
{
  m_remoteCurFolderTextBox.getText(out);
}

void FileTransferMainDialog::getPathToParentRemoteFolder(StringStorage *out)
{
  getPathToCurrentRemoteFolder(out);
  int ld = out->findLast(_T('/'));
  if (ld >= 0) {
    out->getSubstring(out, 0, ld);  
  } else {
    out->setString(_T("/"));
    return ;
  }
  if (out->endsWith('/') && (out->getLength() > 2)) {
    out->getSubstring(out, 0, out->getLength() - 2);
  }
}

void FileTransferMainDialog::getPathToSelectedRemoteFile(StringStorage *out)
{
  StringStorage *pathToFile = out;
  getPathToCurrentRemoteFolder(pathToFile);

  if (!pathToFile->endsWith(_T('/'))) {
    pathToFile->appendString(_T("/"));
  }

  const TCHAR *filename = m_remoteFileListView.getSelectedFileInfo()->getFileName();
  pathToFile->appendString(filename);
}
