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

#include "FileInfoList.h"

FileInfoList::FileInfoList(FileInfo fileInfo)
: m_parent(0), m_child(0), m_next(0), m_prev(0)
{
  setFileInfo(fileInfo);
}

FileInfoList::FileInfoList(const FileInfo *filesInfo, size_t count)
: m_parent(0), m_child(0), m_next(0), m_prev(0)
{
  setFileInfo(filesInfo[0]);

  FileInfoList *list = fromArray(filesInfo, count);
  FileInfoList *second = list->getNext();

  list->m_next = NULL;

  if (second != NULL) {
    second->m_prev = this;
  }

  m_next = second;

  delete list;
}

FileInfoList::~FileInfoList()
{

  if (m_child != NULL) {
    delete m_child;
  }
  if (m_next != NULL) {
    delete m_next;
  }
}

void FileInfoList::setNext(FileInfoList *next)
{
  if (m_next != NULL) {
    delete m_next;
  }
  m_next = next;
}

void FileInfoList::setPrev(FileInfoList *prev)
{
  if (m_prev != NULL) {
    delete m_prev;
  }
  m_prev = prev;
}

void FileInfoList::setChild(const FileInfo *filesInfo, UINT32 count)
{
  if (m_child != NULL) {
    delete m_child;
  }
  m_child = FileInfoList::fromArray(filesInfo, count);
  if (m_child != NULL) {
    m_child->m_parent = this;
  }
}

FileInfoList *FileInfoList::getChild()
{
  return m_child;
}

FileInfoList *FileInfoList::getParent()
{
  return m_parent;
}

FileInfoList *FileInfoList::getRoot()
{
  FileInfoList *first = getFirst();
  if (first->getParent() == NULL) {
    return first;
  }
  return first->getParent()->getRoot();
}

FileInfoList *FileInfoList::getFirst()
{
  if (getPrev() == NULL) {
    return this;
  }
  return getPrev()->getFirst();
}

FileInfoList *FileInfoList::getNext()
{
  return m_next;
}

FileInfoList *FileInfoList::getPrev()
{
  return m_prev;
}

void FileInfoList::setFileInfo(FileInfo fileInfo)
{
  m_fileInfo = fileInfo;
}

FileInfo *FileInfoList::getFileInfo()
{
  return &m_fileInfo;
}

void FileInfoList::getAbsolutePath(StringStorage *storage, TCHAR directorySeparator)
{
  FileInfoList *first = getFirst();

  StringStorage parentAbsolutePath(_T(""));

  bool firstHasParent = (first != NULL) && (first->getParent() != NULL);

  if (firstHasParent) {
    FileInfoList *firstParent = first->getParent();
    firstParent->getAbsolutePath(&parentAbsolutePath, directorySeparator);
    if (!parentAbsolutePath.endsWith(directorySeparator)) {
      parentAbsolutePath.appendChar(directorySeparator);
    } 
  } 

  const TCHAR *fileName = m_fileInfo.getFileName();

  storage->setString(parentAbsolutePath.getString());
  storage->appendString(fileName);
}

FileInfoList *FileInfoList::fromArray(const FileInfo *filesInfo, UINT32 count)
{
  if (count == 0) {
    return NULL;
  }

  FileInfoList *current = NULL;
  FileInfoList *prev = NULL;

  for (UINT32 i = 0; i < count; i++) {
    current = new FileInfoList(filesInfo[i]);
    current->setPrev(prev);
    if (prev != NULL) {
      prev->setNext(current);
    }
    prev = current;
  }

  return current->getFirst();
}
