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

#include "IpAccessControl.h"

void IpAccessControl::serialize(DataOutputStream *output)
{
  output->writeUInt32(size());

  StringStorage string;

  for (size_t i = 0; i < size(); i++) {
    IpAccessRule *rule = at(i);

    rule->toString(&string);

    output->writeUTF8(string.getString());
  }
}

void IpAccessControl::deserialize(DataInputStream *input)
{
  clear();

  size_t count = input->readUInt32();

  StringStorage string;

  for (size_t i = 0; i < count; i++) {
    input->readUTF8(&string);

    IpAccessRule *rule = new IpAccessRule();

    if (!IpAccessRule::parse(string.getString(), rule)) {
      delete rule;
      continue;
    }

    push_back(rule);
  }
}
