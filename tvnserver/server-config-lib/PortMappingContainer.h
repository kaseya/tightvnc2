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

#ifndef _PORT_MAPPING_CONTAINER_H_
#define _PORT_MAPPING_CONTAINER_H_

#include "PortMapping.h"

#include "util/Exception.h"

#include "io-lib/DataInputStream.h"
#include "io-lib/DataOutputStream.h"

#include <vector>

class PortMappingContainer
{
public:
  PortMappingContainer();
  PortMappingContainer(const PortMappingContainer &other);
  virtual ~PortMappingContainer();

  PortMappingContainer &operator=(const PortMappingContainer &other);

  void pushBack(PortMapping element);

  int find(PortMapping searchElement) const;

  int findByPort(int port) const;

  void remove(int index);
  void remove(PortMapping removeMapping);

  void removeAll();

  size_t count() const;

  bool equals(const PortMappingContainer *other) const;

  void serialize(DataOutputStream *output) const throw(Exception);

  void deserialize(DataInputStream *input) throw(Exception);

  const PortMapping *at(size_t index) const;

  PortMapping *at(size_t index);

private:
  std::vector<PortMapping> m_vector;
};

#endif
