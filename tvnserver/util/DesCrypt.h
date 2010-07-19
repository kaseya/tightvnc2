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

#ifndef __DESCRYPT_H__
#define __DESCRYPT_H__

#include "util/inttypes.h"

enum CodeMode
{
  ENCRYPT,
  DECRYPT
};

class DesCrypt
{
public:
  DesCrypt();
  virtual ~DesCrypt();

  void clearKey();

  void encrypt(UINT8 *dst, const UINT8 *src, size_t dataLen,
               const UINT8 *key);
  void decrypt(UINT8 *dst, const UINT8 *src, size_t dataLen,
               const UINT8 *key);

private:
  void deskey(const UINT8 hexKey[8], CodeMode mode);

  void des(const unsigned char from[8], unsigned char to[8]);

  static void scrunch(const unsigned char *outof, unsigned long *into);
  static void unscrun(const unsigned long *outof, unsigned char *into);
  static void desfunc(unsigned long *block, unsigned long *keys);
  void cookey(register unsigned long *raw1);
  void usekey(register unsigned long *from);


  unsigned long m_knL[32];
};

#endif 
