/****************************************************************
 *
 * vdeform:
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Thomas Arnold, 2001, <lipsia@cbs.mpg.de>
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
 * $Id: vdeform.C 2999 2007-11-30 10:55:56Z karstenm $
 *
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <assert.h>

extern "C"
{
   #include <Vlib.h>
   #include <VImage.h>
   #include <option.h>
   #include <mu.h>
   
   extern void getLipsiaVersion(char*, size_t);
}

#include "util.H"
#include "trilinear.H"


/*------------------------------------------------------------------------------

This program implements a non-linear image deformation by applying a
deformation field created by 'vdemon'.

------------------------------------------------------------------------------*/

void ReadImages (VString Name, VAttrList& Images, VAttrList& history_list)
{
   FILE*         file;    /* input file       */
   VAttrList     list, list1;    /* attribute list   */
   VAttrListPosn pos;     /* position in list */
   VImage        image;   /* image in list    */


   /* initialize results */
   Images  = NULL;

   /* open file */
   file = fopen (Name, "r");
   if (!file)
   {
      VError ("Failed to open input file '%s'", Name);
      return;
   }

   /* read file */
   list = VReadFile (file, NULL);
   if (!list)
   {
      VError ("Failed to read input file '%s'", Name);
      fclose (file);
      return;
   }

   /* Read History */
   list1 = VReadHistory(&list);
   if (list1==NULL) list1=VCreateAttrList();
   history_list = VCopyAttrList(list1);

   /* extract images */
   for (VFirstAttr (list, &pos); VAttrExists (&pos); VNextAttr (&pos))
   {
      if (VGetAttrRepn (&pos) != VImageRepn) continue;
      if (!Images) Images = VCreateAttrList ();
      VGetAttrValue (&pos, NULL, VImageRepn, &image);
      VSetAttrValue (&pos, NULL, VImageRepn, NULL);
      VAppendAttr (Images, "image", NULL, VImageRepn, image);
   }
   if (!Images) VError ("Input file '%s' does not contain an image", Name);

   /* clean-up*/
   VDestroyAttrList (list);
   fclose (file);

} /* ReadImages */

/*----------------------------------------------------------------------------*/

void ReadField (VString Name, VImage& X, VImage& Y, VImage& Z, VAttrList& history_list)
{
   FILE*         file;   /* input file       */
   VAttrList     list;   /* attribute list   */
   VAttrListPosn pos;    /* position in list */


   /* initialize results */
   X = Y = Z = NULL;

   /* open file */
   file = fopen (Name, "r");
   if (!file)
   {
      VError ("Failed to open input file '%s'", Name);
      return;
   }

   /* read file */
   list = VReadFile (file, NULL);
   if (!list)
   {
      VError ("Failed to read input file '%s'", Name);
      fclose (file);
      return;
   }

   /* Read History */
   history_list = VCopyAttrList(VReadHistory(&list));

   /* extract field images */
   if (VLookupAttr (list, "X", &pos))
   {
      VGetAttrValue (&pos, NULL, VImageRepn, &X);
      VSetAttrValue (&pos, NULL, VImageRepn, NULL);
   }
   else VError ("Input file '%s' does not contain an image 'X'", Name);
   if (VLookupAttr (list, "Y", &pos))
   {
      VGetAttrValue (&pos, NULL, VImageRepn, &Y);
      VSetAttrValue (&pos, NULL, VImageRepn, NULL);
   }
   else VError ("Input file '%s' does not contain an image 'Y'", Name);
   if (VLookupAttr (list, "Z", &pos))
   {
      VGetAttrValue (&pos, NULL, VImageRepn, &Z);
      VSetAttrValue (&pos, NULL, VImageRepn, NULL);
   }
   else VError ("Input file '%s' does not contain an image 'Z'", Name);

   /* clean-up*/
   VDestroyAttrList (list);
   fclose (file);

} /* ReadField */

/*----------------------------------------------------------------------------*/

VBoolean Compatible (VImage Image1, VImage Image2)
{
   /* compare image sizes */
   if ((VImageNBands   (Image1) != VImageNBands   (Image2)) ||
       (VImageNRows    (Image1) != VImageNRows    (Image2)) ||
       (VImageNColumns (Image1) != VImageNColumns (Image2)))
   {
      VError ("Images have different sizes");
      return FALSE;
   }

   return TRUE;

} /* Compatible */

/*----------------------------------------------------------------------------*/

VBoolean WriteImages (VString Name, VAttrList Images, VAttrList& history_list)
{
   FILE*         file;      /* output file      */
   VAttrList     list;      /* attribute list   */
   VAttrListPosn pos;       /* position in list */
   VImage        image;     /* image in list    */
   VBoolean      success;   /* success flag     */
   char history[]="history";

   /* open file */
   file = fopen (Name, "w");
   if (!file)
   {
      VError ("Failed to open output file '%s'", Name);
      return FALSE;
   }

   /* create list */
   list = VCreateAttrList();
 
   /* insert images */
   for (VFirstAttr (Images, &pos); VAttrExists (&pos); VNextAttr (&pos))
   {
      VGetAttrValue (&pos, NULL, VImageRepn, &image);
      VAppendAttr (list, VGetAttrName (&pos), NULL, VImageRepn, image);
   }

   /* Prepend history */
   VPrependAttr(list, history ,NULL,VAttrListRepn,history_list);
   
   /* write file */
   success = VWriteFile (file, list);
   if (!success) VError ("Failed to write output file '%s'", Name);

   /* remove images */
   for (VFirstAttr (list, &pos); VAttrExists (&pos); VNextAttr (&pos))
      if (VGetAttrRepn (&pos) == VImageRepn)
         VSetAttrValue (&pos, NULL, VImageRepn, NULL);

   /* clean-up*/
   VDestroyAttrList (list);
   fclose (file);

   return success;

} /* WriteImages */

/*----------------------------------------------------------------------------*/

VBoolean FunctionalZero (VImage Image)
{
   int     Pixels;   /* number of pixels   */
   VShort* image;    /* image data pointer */
   int     n;        /* index              */


   /* check image size */
   if ((VImageNBands (Image) == 1) && (VImageNRows (Image) == 1) && (VImageNColumns (Image) == 1))
      return TRUE;

   /* check image content */
   Pixels = VImageNBands (Image) * VImageNRows (Image) * VImageNColumns (Image);
   image  = (VShort*) VPixelPtr (Image, 0, 0, 0);
   for (n = 0; n < Pixels; ++n)
      if (*(image++) != 0)
         return FALSE;

   return TRUE;

} /* FunctionalZero */

/*----------------------------------------------------------------------------*/

void FunctionalResize (VImage& Image, int Bands, int Rows, int Columns)
{
   VImage zero;   /* empty image */


   /* resize image */
   zero = VCreateImage (Bands, Rows, Columns, VShortRepn);
   VImageAttrList (zero) = VCopyAttrList (VImageAttrList (Image));
   VFillImage (zero, VAllBands, 0);
   VDestroyImage (Image);
   Image = zero;

} /* FunctionalResize */

/*----------------------------------------------------------------------------*/

int main (int argc, char* argv[])
{
   VString        inname;            /* name of input images      */
   VString        outname;           /* name of output images     */
   VString        fieldname;         /* name of deformation field */
   VBoolean       verbose = TRUE;    /* verbose flag              */
   VOptionDescRec options[] =        /* options of program        */
   {
      {"in",      VStringRepn,  1, &inname,    VRequiredOpt, NULL, "Input image"},
      {"out",     VStringRepn,  1, &outname,   VRequiredOpt, NULL, "Deformed output image"},
      {"field",   VStringRepn,  1, &fieldname, VRequiredOpt, NULL, "3D deformation field"},
      {"verbose", VBooleanRepn, 1, &verbose,   VOptionalOpt, NULL, "Show status messages. Optional"}
   };

   VAttrList in_history=NULL;      /* history of input images      */
   VAttrList field_history=NULL;   /* history of deformation field */

   VAttrList In;           /* input images  */
   VImage    Dx, Dy, Dz;   /* field images  */

   VAttrListPosn pos;   /* position in list */
   VImage        in;    /* image in list    */

   float  fx, fy, fz;   /* scaling factors          */
   VImage dx, dy, dz;   /* scaled deformation field */

   VAttrListPosn rider;                         /* functional data rider   */
   int           bands, rows, columns, steps;   /* size of functional data */
   VImage        data;                          /* functional data         */
   VShort        *src, *dest;                   /* functional data pointer */

   VBoolean success;   /* success flag */

   int n, t, z;   /* indices */


   /* print information */
   char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vdeform V%s", ver);
  
   fprintf (stderr, "%s\n", prg_name); fflush (stderr);

   /* parse command line */
   if (!VParseCommand (VNumber (options), options, &argc, argv))
   {
      if (argc > 1) VReportBadArgs (argc, argv);
      VReportUsage (argv[0], VNumber (options), options, NULL);
      exit (1);
   }

   /* read input images */
   if (verbose) {fprintf (stderr, "Reading input image '%s' ...\n", inname); fflush (stderr);}
   ReadImages (inname, In, in_history);
   if (!In) exit (2);

   /* read deformation field */
   if (verbose) {fprintf (stderr, "Reading 3D deformation field '%s' ...\n", fieldname); fflush (stderr);}
   ReadField (fieldname, Dx, Dy, Dz, field_history);
   if (!Dx || !Dy || !Dz) exit (3);


   /* deform anatomical images */
   for (VFirstAttr (In, &pos); VAttrExists (&pos); VNextAttr (&pos))
   {
      /* get image */
      VGetAttrValue (&pos, NULL, VImageRepn, &in);
      if (VPixelRepn (in) != VUByteRepn) break;

      /* compare image and field */
      if (verbose) {fprintf (stderr, "Comparing anatomical image and deformation field ...\n"); fflush (stderr);}
      if (!Compatible (in, Dx)) exit (4);
      if (!Compatible (in, Dy)) exit (4);
      if (!Compatible (in, Dz)) exit (4);

      /* deform image */
      if (verbose) {fprintf (stderr, "Deforming anatomical image ...\n"); fflush (stderr);}
      RTTI (in, TrilinearInverseDeform, (in, Dx, Dy, Dz));
      VSetAttrValue (&pos, NULL, VImageRepn, in);
   }


   /* deform map images */
   for (; VAttrExists (&pos); VNextAttr (&pos))
   {
      /* get image */
      VGetAttrValue (&pos, NULL, VImageRepn, &in);
      if (VPixelRepn (in) != VFloatRepn) break;

      /* scale field */
      if (verbose) {fprintf (stderr, "Scaling deformation field ...\n"); fflush (stderr);}
      fx = (float) VImageNColumns (in) / (float) VImageNColumns (Dx);
      fy = (float) VImageNRows    (in) / (float) VImageNRows    (Dy);
      fz = (float) VImageNBands   (in) / (float) VImageNBands   (Dz);
      TrilinearScale<VFloat> (Dx, fx, fy, fz, dx); Multiply<VFloat> (dx, fx);
      TrilinearScale<VFloat> (Dy, fx, fy, fz, dy); Multiply<VFloat> (dy, fy);
      TrilinearScale<VFloat> (Dz, fx, fy, fz, dz); Multiply<VFloat> (dz, fz);

      /* compare image and field */
      if (verbose) {fprintf (stderr, "Comparing map image and deformation field ...\n"); fflush (stderr);}
      if (!Compatible (in, dx)) exit (5);
      if (!Compatible (in, dy)) exit (5);
      if (!Compatible (in, dz)) exit (5);

      /* deform image */
      if (verbose) {fprintf (stderr, "Deforming map image ...\n"); fflush (stderr);}
      RTTI (in, TrilinearInverseDeform, (in, dx, dy, dz));
      VSetAttrValue (&pos, NULL, VImageRepn, in);

      /* clean-up */
      VDestroyImage (dx);
      VDestroyImage (dy);
      VDestroyImage (dz);
   }


   /* deform functional images */
   if (VAttrExists (&pos))
   {
      /* get data size */
      bands = rows = columns = steps = 0;
      for (rider = pos; VAttrExists (&rider); VNextAttr (&rider))
      {
         /* get image */
         VGetAttrValue (&rider, NULL, VImageRepn, &data);
         if (VPixelRepn (data) != VShortRepn) break;

         /* store image size */
         if (VImageNBands   (data) > steps)   steps   = VImageNBands   (data);
         if (VImageNRows    (data) > rows)    rows    = VImageNRows    (data);
         if (VImageNColumns (data) > columns) columns = VImageNColumns (data);
         bands++;
      }
      in = VCreateImage (bands, rows, columns, VShortRepn);

      /* scale field */
      if (verbose) {fprintf (stderr, "Scaling deformation field ...\n"); fflush (stderr);}
      fx = (float) VImageNColumns (in) / (float) VImageNColumns (Dx);
      fy = (float) VImageNRows    (in) / (float) VImageNRows    (Dy);
      fz = (float) VImageNBands   (in) / (float) VImageNBands   (Dz);
      TrilinearScale<VFloat> (Dx, fx, fy, fz, dx); Multiply<VFloat> (dx, fx);
      TrilinearScale<VFloat> (Dy, fx, fy, fz, dy); Multiply<VFloat> (dy, fy);
      TrilinearScale<VFloat> (Dz, fx, fy, fz, dz); Multiply<VFloat> (dz, fz);

      /* compare image and field */
      if (verbose) {fprintf (stderr, "Comparing functional images and deformation field ...\n"); fflush (stderr);}
      if (!Compatible (in, dx)) exit (6);
      if (!Compatible (in, dy)) exit (6);
      if (!Compatible (in, dz)) exit (6);


      /* expand zero images */
      for (rider = pos, z = 0; z < bands; z++, VNextAttr (&rider))
      {
         VGetAttrValue (&rider, NULL, VImageRepn, &data);
         if (FunctionalZero (data))
         {
            FunctionalResize (data, steps, rows, columns);
            VSetAttrValue (&rider, NULL, VImageRepn, data);
         }
      }

      /* deform images */
      if (verbose) {fprintf (stderr, "Deforming functional images ...\n"); fflush (stderr);}
      for (t = 0; t < steps; t++)
      {
         /* collect data */
         dest = (VShort*) VPixelPtr (in, 0, 0, 0);
         for (rider = pos, z = 0; z < bands; z++, VNextAttr (&rider))
         {
            VGetAttrValue (&rider, NULL, VImageRepn, &data);
            src = (VShort*) VPixelPtr (data, t, 0, 0);
            for (n = 0; n < rows * columns; ++n)
               *(dest++) = *(src++);
         }

         /* deform image */
         if (verbose) {fprintf (stderr, "Timestep %d of %d ...\r", t + 1, steps); fflush (stderr);}
         RTTI (in, TrilinearInverseDeform, (in, dx, dy, dz));

         /* spread data */
         src = (VShort*) VPixelPtr (in, 0, 0, 0);
         for (rider = pos, z = 0; z < bands; z++, VNextAttr (&rider))
         {
            VGetAttrValue (&rider, NULL, VImageRepn, &data);
            dest = (VShort*) VPixelPtr (data, t, 0, 0);
            for (n = 0; n < rows * columns; ++n)
               *(dest++) = *(src++);
         }
      }

      /* collapse zero images */
      for (rider = pos, z = 0; z < bands; z++, VNextAttr (&rider))
      {
         VGetAttrValue (&rider, NULL, VImageRepn, &data);
         if (FunctionalZero (data))
         {
            FunctionalResize (data, 1, 1, 1);
            VSetAttrValue (&rider, NULL, VImageRepn, data);
         }
      }


      /* clean-up */
      VDestroyImage (in);
      VDestroyImage (dx);
      VDestroyImage (dy);
      VDestroyImage (dz);

      /* proceed */
      pos = rider;
   }


   /* check list */
   if (VAttrExists (&pos))
   {
      VError ("Remaining image does not contain valid data");
      exit (7);
   }

   /* Prepend History */
   VPrependHistory(VNumber(options),options,prg_name,&in_history);

   /* write output images */
   if (verbose) {fprintf (stderr, "Writing output image '%s' ...\n", outname); fflush (stderr);}
   success = WriteImages (outname, In, in_history);
   if (!success) exit (8);


   /* clean-up 
      VDestroyAttrList (inhistory);
      VDestroyAttrList (fieldhistory);
      VDestroyAttrList (In);
      VDestroyImage (Dx);
      VDestroyImage (Dy);
      VDestroyImage (Dz); */

   /* exit */
   if (verbose) {fprintf (stderr, "Finished.\n"); fflush (stderr);}
   return 0;

} /* main */

/*----------------------------------------------------------------------------*/
