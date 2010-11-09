/****************************************************************
 *
 * FuncCompress.c
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 1999, <lipsia@cbs.mpg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * $Id: FuncCompress.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


#include <stdlib.h>
#include <stdio.h>

#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>
#include <viaio/file.h>


VImage
VFuncCompress(VImage src,VShort minval)
{
  VImage dest=NULL;
  VShort *short_pp;
  int i;


  short_pp = VImageData(src);
  for (i=0; i<VImageNPixels(src); i++) {
    if (*short_pp > minval) return src;  /* a non-zero voxel is found */
    short_pp++;
  }

  dest = VCreateImage(1,1,1,VShortRepn);
  VPixel(dest,0,0,0,VShort) = 0;
  VCopyImageAttrs (src, dest);
  VAppendAttr(VImageAttrList(dest),"ori_nrows",NULL,VShortRepn,VImageNRows(src)); 
  VAppendAttr(VImageAttrList(dest),"ori_ncolumns",NULL,VShortRepn,VImageNColumns(src)); 
  VDestroyImage(src);
 
  return dest;
}



