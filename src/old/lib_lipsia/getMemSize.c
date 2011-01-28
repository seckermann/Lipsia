/*
 * =====================================================================================
 *
 *       Filename:  getMemSize.c
 *
 *    Description:  Reads the size of available physical memory from /proc/meminfo.
 *
 *        Version:  1.0
 *        Created:  01/27/2011 05:41:05 PM CET
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   (Thomas Proeger), 
 *        Company:  
 *
 * =====================================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* maximum number of digits for memory size */
#define MAXLEN 100

/* Helper fcuntion.
 *
 * This function reads the first line from the file /proc/meminfo and returns the
 * memory size value. The value represents the overall size of RAM memory available
 * given in kB. It returns 0 if an error occures.
 */
unsigned long getMemSize() {

  FILE *fp;
  char line[MAXLEN], numstr[MAXLEN], *cp,*cp2;
  char *funcname = "getMemSize";
  unsigned long size;
  unsigned int i;

  if (!(fp=fopen("/proc/meminfo","r"))){
      fprintf(stderr,"%s: Error reading /proc/meminfo. Abort\n",funcname);
      return 0;
  }

  /* get first line */
  if (!(fgets(line, MAXLEN, fp))) {
    fprintf(stderr,"%s: Error reading first line. Abort\n",funcname);
    fclose(fp);
    return 0;
  }
  fclose(fp);

  /* traverse line and collect numbers */
  cp2 = numstr;
  unsigned char scan = 0;
  for(cp=line;*cp != '\n';cp++){
    if(scan && ((*cp < '0') || (*cp > '9'))) {
      *cp2 = '\0';
      break;
    }

    if(((*cp >= '0') && (*cp <= '9'))){
      scan=1;
      *(cp2++)=*cp;
    }
  }

  /* convert string to long if string length > 0 */
  if(strlen(numstr) > 0)
    size = atol(numstr);
  else
    size = 0;

  return size;

}
