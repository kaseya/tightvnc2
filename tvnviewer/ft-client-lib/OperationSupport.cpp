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

#include "OperationSupport.h"

#include "ft-common/FTMessage.h"

#include "vncviewer/CapsContainer.h"

OperationSupport::OperationSupport(CapsContainer *clientCaps, CapsContainer *serverCaps)
{
  m_isFileListSupported = ((clientCaps->IsEnabled(FTMessage::FILE_LIST_REQUEST)) &&
                           (serverCaps->IsEnabled(FTMessage::FILE_LIST_REPLY)));

  m_isRenameSupported = ((clientCaps->IsEnabled(FTMessage::RENAME_REQUEST)) &&
                         (serverCaps->IsEnabled(FTMessage::RENAME_REPLY)));

  m_isRemoveSupported = ((clientCaps->IsEnabled(FTMessage::REMOVE_REQUEST)) &&
                         (serverCaps->IsEnabled(FTMessage::REMOVE_REPLY)) &&
                         (m_isFileListSupported));

  m_isMkDirSupported = ((clientCaps->IsEnabled(FTMessage::MKDIR_REQUEST)) &&
                        (serverCaps->IsEnabled(FTMessage::MKDIR_REPLY)));

  m_isCompressionSupported = ((clientCaps->IsEnabled(FTMessage::COMPRESSION_SUPPORT_REQUEST)) &&
                              (serverCaps->IsEnabled(FTMessage::COMPRESSION_SUPPORT_REPLY)));

  m_isMD5Supported = ((clientCaps->IsEnabled(FTMessage::MD5_REQUEST)) &&
                      (serverCaps->IsEnabled(FTMessage::MD5_REPLY)));

  m_isDirSizeSupported = ((clientCaps->IsEnabled(FTMessage::DIRSIZE_REQUEST)) &&
                          (serverCaps->IsEnabled(FTMessage::DIRSIZE_REPLY)));

  m_isUploadSupported = ((clientCaps->IsEnabled(FTMessage::UPLOAD_START_REQUEST)) &&
                         (clientCaps->IsEnabled(FTMessage::UPLOAD_DATA_REQUEST)) &&
                         (clientCaps->IsEnabled(FTMessage::UPLOAD_END_REQUEST)) &&
                         (serverCaps->IsEnabled(FTMessage::UPLOAD_START_REPLY)) &&
                         (serverCaps->IsEnabled(FTMessage::UPLOAD_DATA_REPLY)) &&
                         (serverCaps->IsEnabled(FTMessage::UPLOAD_END_REPLY)) &&
                         m_isMkDirSupported && m_isFileListSupported);

  m_isDownloadSupported = ((clientCaps->IsEnabled(FTMessage::DOWNLOAD_START_REQUEST)) &&
                           (clientCaps->IsEnabled(FTMessage::DOWNLOAD_DATA_REQUEST)) &&
                           (serverCaps->IsEnabled(FTMessage::DOWNLOAD_START_REPLY)) &&
                           (serverCaps->IsEnabled(FTMessage::DOWNLOAD_DATA_REPLY)) &&
                           (serverCaps->IsEnabled(FTMessage::DOWNLOAD_END_REPLY)) &&
                           m_isFileListSupported && m_isDirSizeSupported);
}

OperationSupport::~OperationSupport()
{
}

bool OperationSupport::isFileListSupported() const
{
  return m_isFileListSupported;
}

bool OperationSupport::isUploadSupported() const
{
  return m_isUploadSupported;
}

bool OperationSupport::isDownloadSupported() const
{
  return m_isDownloadSupported;
}

bool OperationSupport::isRenameSupported() const
{
  return m_isRenameSupported;
}

bool OperationSupport::isRemoveSupported() const
{
  return m_isRemoveSupported;
}

bool OperationSupport::isMkDirSupported() const
{
  return m_isMkDirSupported;
}

bool OperationSupport::isCompressionSupported() const
{
  return m_isCompressionSupported;
}

bool OperationSupport::isMD5Supported() const
{
  return m_isMD5Supported;
}

bool OperationSupport::isDirSizeSupported() const
{
  return m_isDirSizeSupported;
}
