/****************************************************************
 *
 * Program: vsendserv
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Stefan Bohn, <lipsia@cbs.mpg.de>
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
 * $Id: vsendserv.C 3774 2010-09-23 14:45:16Z tuerke $
 *
 *****************************************************************/
 
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <cstdio>

using namespace std;

int fifoIDout, fifoIDin;
char fifoName[256];

extern "C" {
  extern char * getLipsiaVersion();
}

bool startIPC()
{
  char	zeile[20], pipename[20];
  char  vlservName[512];
  char	*zeile1;
  int   x, y, z;
  FILE	*progpipe;

  sprintf(vlservName, "%s", "/usr/bin/vlserv");

  cerr << "Connecting to Lipsia-Server ... ";

  // Pipename (= User-Name)
  progpipe = popen("whoami","r");
  if (fgets(zeile, sizeof(zeile), progpipe) != NULL)
    {
      zeile1 = strtok(zeile,"\n");
      sprintf(pipename,"/tmp/lipsia.%s", zeile1);
    }
  else 
    sprintf(pipename, "/tmp/lipsia.unknown");
  pclose(progpipe);

  fifoIDout = open(pipename, O_RDWR);

  if (fifoIDout == - 1) 
    {
      cerr << "failed." << endl;

      cerr << "Lipsia-Server not running. Trying to start now .." << endl;

      FILE *ftest;
      ftest = fopen(vlservName, "rb");

      if (!ftest)
	{
	  cerr << "vlserv daemon not installed. Giving up" << endl;
	  return false;
	} 
      else 
	{
	  fclose(ftest);
	  strcat(vlservName, "&");
	  system(vlservName);
	  sleep(1);

	  cerr << " ok" << endl;
	  fifoIDout = open(pipename, O_RDWR);

	  if (fifoIDout == - 1) 
	    {
	      cerr << "No contact with Lipsia Server. Giving up" << endl;
	      return false;
	    };
	};
    }
  else
    cerr << "ok." << endl;
    
    sprintf(fifoName,"/tmp/lipsia.%d\0", getpid());

    if(mknod(fifoName, S_IFIFO | S_IRUSR | S_IWUSR, 0) == - 1) 
      {
	cerr <<  "Can't create a fifo." << endl;
	return false;
      } 

  return true;
}



int main(int argc, char *argv[])
{
  int	xc, yc, zc, xr, yr, zr;
  int   type = 0, modus = 1;
  float zp, zn;
  char	buffer[2000];  

  char prg_name[50];	
  sprintf(prg_name,"vsendserv V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg_name);


  if (argc != 9)
    {
      cerr << "Usage: vsendserv xrot yrot zrot xcoord ycoord zcoord zpos zneg" << endl;
      cerr << "Note: multiply zneg by -1." << endl;
      exit(1);
    }

  xc = atoi(argv[1]);
  yc = atoi(argv[2]);
  zc = atoi(argv[3]);
  xr = atoi(argv[4]);
  yr = atoi(argv[5]);
  zr = atoi(argv[6]);
  zp = (float) atof(argv[7]);
  zn = (float) atof(argv[8]);

  startIPC();

  cerr << "Sending ... ";

  sprintf(buffer, "%d %d %d %d %d %d %d %d %d %f %f \0", getpid(), xc, yc, zc, modus, xr, yr, zr, type, zp, zn);

  if(write(fifoIDout, buffer, strlen(buffer))  == -1)
    {
      cerr << "fail: Can't write into Lipsia FIFO" << endl;
      return false;
    }
  else
    cerr << "ok." << endl;

  unlink(fifoName);

  return 0;
}
