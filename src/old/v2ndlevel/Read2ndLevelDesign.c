
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

extern int VStringToken(char *, char *, int, int);

#define LEN  10000


/*
** read a 2nd level design file
*/
VImage
VRead2ndLevel(FILE *fp) {
    VImage dest = NULL;
    int i, j, nrows, ncols;
    float val;
    char buf[LEN], token[32];
    nrows = ncols = 0;
    while(!feof(fp)) {
        memset(buf, 0, LEN);
        if(!fgets(buf, LEN, fp))
            break;
        if(strlen(buf) < 1)
            continue;
        if(buf[0] == '%')
            continue;
        j = 0;
        while(VStringToken(buf, token, j, 30)) {
            if(!sscanf(token, "%f", &val))
                VError("illegal input string: %s", token);
            j++;
        }
        if(ncols == 0)
            ncols = j;
        if(j < 1)
            continue;
        else if(ncols != j)
            VError(" inconsistent number of columns in row %d", nrows + 1);
        nrows++;
    }
    rewind(fp);
    dest = VCreateImage(1, nrows, ncols, VFloatRepn);
    VFillImage(dest, VAllBands, 0);
    VSetAttr(VImageAttrList(dest), "modality", NULL, VStringRepn, "X");
    VSetAttr(VImageAttrList(dest), "name", NULL, VStringRepn, "X");
    VSetAttr(VImageAttrList(dest), "ntimesteps", NULL, VLongRepn, (VLong)VImageNRows(dest));
    VSetAttr(VImageAttrList(dest), "nsessions", NULL, VShortRepn, (VShort)1);
    VSetAttr(VImageAttrList(dest), "designtype", NULL, VShortRepn, (VShort)2);
    i = 0;
    while(!feof(fp)) {
        memset(buf, 0, LEN);
        if(!fgets(buf, LEN, fp))
            break;
        if(strlen(buf) < 1)
            continue;
        if(buf[0] == '%')
            continue;
        j = 0;
        while(VStringToken(buf, token, j, 30)) {
            sscanf(token, "%f", &val);
            VPixel(dest, 0, i, j, VFloat) = val;
            j++;
        }
        if(j < 1)
            continue;
        i++;
    }
    return dest;
}



/*
int
main (int argc,char *argv[])
{
  FILE *in_file,*out_file;
  VAttrList out_list=NULL;
  VImage dest=NULL;

  VParseFilterCmd(0,NULL,argc,argv,&in_file,&out_file);

  dest = VRead2ndLevel (in_file);
  fclose(in_file);
  if (!dest) VError(" error creating dest image");

  out_list = VCreateAttrList();
  VAppendAttr (out_list, "image", NULL, VImageRepn,dest);

  if (! VWriteFile (out_file, out_list)) exit (1);
  fprintf(stderr," %s: done.\n",argv[0]);
  return 0;
}
*/
