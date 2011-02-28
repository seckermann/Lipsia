/* From the Vista library: */

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <nifti1.h>
#include <nifti1_io.h>
#include <znzlib.h>


VImage
VNiftSort(VImage src, int icod, int jcod, int kcod, VBoolean flipx, VBoolean flipy, VBoolean flipz)
{
  int i, b, r, c, bb;
  VImage dest=NULL;

  /* Create image */
  dest = VCopyImage(src, NULL, VAllBands);

  /* Superior to Inferior ??? */
  if ( (kcod==5 && flipz==FALSE) || (kcod==6 && flipz==TRUE) ) {
    VFillImage(dest,VAllBands,0);
    bb = VImageNBands(dest)-1;
    for (b=0; b<VImageNBands(dest); b++) {
      for (r=0; r<VImageNRows(dest); r++) {
	for (c=0; c<VImageNColumns(dest); c++) 
	  VSetPixel(dest, bb, r, c, VGetPixel(src, b, r, c)); 
      }
      bb--;
    }
    VCopyImage(dest, src, VAllBands);
  } 
  if (kcod !=5 && kcod != 6)  VWarning("z orientation might be incorrect");
  
  /* Anterior to Posterior ??? */
  if ( (jcod==3 && flipy==FALSE) || (jcod==4 && flipy==TRUE) ) {
    VFillImage(dest,VAllBands,0);   
    VFlipImage(src, dest,VAllBands,TRUE);
    VCopyImage(dest, src, VAllBands);
  } 
  if (jcod != 3 && jcod != 4) VWarning("y orientation might be incorrect");
  
  /* Right to Left ??? */
  if ( (icod==2 && flipx==TRUE) || (icod==1 && flipx==FALSE) ) {
    VFillImage(dest,VAllBands,0);   
    VFlipImage(src,dest,VAllBands,FALSE); 
    VCopyImage(dest, src, VAllBands);
  } 
  if (icod != 1 && icod != 2) VWarning("x orientation might be incorrect"); 
  
  

  /* Free src Image */
  VDestroyImage(dest);

  return src;

}
