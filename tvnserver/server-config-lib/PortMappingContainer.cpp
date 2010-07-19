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

#include "PortMappingContainer.h"

PortMappingContainer::PortMappingContainer()
{
}

PortMappingContainer::PortMappingContainer(const PortMappingContainer &other)
  : m_vector(other.m_vector)
{
}

PortMappingContainer::~PortMappingContainer()
{
}

PortMappingContainer &
PortMappingContainer::operator=(const PortMappingContainer &other)
{
  m_vector = other.m_vector;
  return *this;
}

void PortMappingContainer::pushBack(PortMapping element)
{
  m_vector.push_back(element);
}

int PortMappingContainer::find(PortMapping searchElement) const
{
  for (size_t i = 0; i < count(); i++) {
    PortMapping each = m_vector.at(i);
    PortMappingRect rect = each.getRect();
    if (each.getPort() == searchElement.getPort() &&
        each.getRect().isEqualTo(&rect)) {
      return i;
    }
  }
  return -1;
}

int PortMappingContainer::findByPort(int port) const
{
  for (size_t i = 0; i < count(); i++) {
    if (m_vector.at(i).getPort() == port) {
      return i;
    }
  }
  return -1;
}

void PortMappingContainer::remove(int index)
{
  int i = 0;
  for (std::vector<PortMapping>::iterator it = m_vector.begin();
       it != m_vector.end(); it++) {
    if (i == index) {
      m_vector.erase(it);
      break;
    }
    i++;
  }
}

void PortMappingContainer::remove(PortMapping removeMapping)
{
  int index = find(removeMapping);

  if (index != -1) {
    remove(index);
  }
}

void PortMappingContainer::removeAll()
{
  m_vector.clear();
}

size_t PortMappingContainer::count() const
{
  return m_vector.size();
}

bool PortMappingContainer::equals(const PortMappingContainer *other) const
{
  if (count() != other->count()) {
    return false;
  }
  for (size_t i = 0; i < count(); i++) {
    if (!at(i)->isEqualTo(other->at(i))) {
      return false;
    }
  }
  return true;
}

void PortMappingContainer::serialize(DataOutputStream *output) const
{
  output->writeUInt32(count());

  StringStorage string;

  for (size_t i = 0; i < count(); i++) {
    at(i)->toString(&string);

    output->writeUTF8(string.getString());
  }
}

void PortMappingContainer::deserialize(DataInputStream *input)
{
  removeAll();

  size_t cnt = input->readUInt32();

  StringStorage string;

  PortMapping record;

  for (size_t i = 0; i < cnt; i++) {

    input->readUTF8(&string);

    if (!PortMapping::parse(string.getString(), &record)) {
      throw Exception(_T("Invalid port mapping string"));
    }

    pushBack(record);
  }
}

const PortMapping *PortMappingContainer::at(size_t index) const
{
  bool outOfRange = index >= count();

  return outOfRange ? 0 : &m_vector.at(index);
}

PortMapping *PortMappingContainer::at(size_t index)
{
  bool outOfRange = index >= count();

  return outOfRange ? 0 : &m_vector.at(index);
}
