/****************************************************************
 *
 * lView.C
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Heiko Mentzel, 1999, <lipsia@cbs.mpg.de>
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
 * $Id: dock.C 2401 2007-09-12 14:29:20Z karstenm $
 *
 *****************************************************************/

#include "dock.h"

#define MAXSLOTS 10

FILE *progpipe; 
char zeile[100], zeile2[100];
char buf[1000], buftmp[1000], str[30], puffer[1000];
char       *pipename;

/* the first widget */
lDock::lDock( QWidget *parent, const char *name ) {

  /* Initialize vlslots */
  pipename = (char *) malloc(sizeof(char) * 40);
  fifoo    = (int *)  malloc(sizeof(int)  * MAXSLOTS);
  vlslot   = (int *)  malloc(sizeof(int)  * MAXSLOTS);
  for (int i=0; i<MAXSLOTS; i++) vlslot[i]=0;
  pidlabel = new QLabel( this, "pids", 0 );
  pidlabel->resize(150,50);
  pidlabel->setText(tr("X:0 Y:0 Z:0 \nConnected: 0"));
  resize(150,50);

  /* Erstelle Name für Pipe */
  char zeile[20]; FILE *progpipe; char *zeile1;
  progpipe = popen("whoami","r");
  if (fgets(zeile,20,progpipe) != NULL) {
    zeile1 = strtok(zeile,"\n");
    sprintf(pipename,"/tmp/lipsia.%s",zeile1);
  } else 
    sprintf(pipename,"/tmp/lipsia.unknown");
  pclose(progpipe);
  unlink(pipename);

  /* Lege neue pipe an */
  if(mknod(pipename, S_IFIFO | S_IRUSR | S_IWUSR, 0) == - 1)
    fprintf(stderr, "Can't creat a fifo.........\n");
  else {
    if((fd_fifo=open(pipename, O_RDWR )) == - 1)
      fprintf(stderr, "Can't open the fifo.....\n");
    else {
      notif_m = new QSocketNotifier (fd_fifo, QSocketNotifier::Read);
      connect (notif_m, SIGNAL(activated(int)), SLOT(syncronize(int)));
    }
  }
}

/* cleanup memory */
lDock::~lDock() {
  disconnect(notif_m, 0, 0, 0);
  unlink(pipename);
} 


void lDock::syncronize(int i) {
  int pid_nr=0, sw=0, count=0;
  int x, y, z, type, a, b, c, modus;
  int nix, ini=0;
  float tpos, tneg;

  if(read(fd_fifo, &buf, sizeof(buf)) == -1)
    fprintf(stderr, "Error! can't read from FIFO.......\n");
  else {
    sscanf (buf, "%d %d %d %d %d %d %d %d %d %f %f\n", &pid_nr, &a, &b, &c, &modus, &x, &y, &z, &type, &tpos, &tneg);
  }

  /* x,y,z have value 32766 */
  ini=0;
  if (x>30000 || y>30000 || z>30000) {
    ini=1;
    strcpy(buf,buftmp);
    sscanf (buf, "%d %d %d %d %d %d %d %d %d %f %f\n", &nix, &a, &b, &c, &modus, &x, &y, &z, &type, &tpos, &tneg);
  }
   


  /* return if fifo could not read successfully */
  if (pid_nr==0) return;
  
  /* look if the PID already exists */
  for (int i=0; i<MAXSLOTS; i++) {
    if (vlslot[i]==pid_nr) sw=1;
  }

  /* add pid to list if not yet included */
  if (sw==0) {
    for (int i=0; i<MAXSLOTS; i++) {
      if (sw==0 && vlslot[i]==0) {
	// fprintf(stderr,"Adding pid %d to control list %d\n",pid_nr,i);
	vlslot[i]=pid_nr; sw=1;
      }
    }
  }

  /* check if pid_nr is available */
  int vlservpid=0;
  char *zeile1;
  for (int i=0; i<MAXSLOTS; i++) {
    if (vlslot[i]>0) {
      sprintf(zeile2,"ps %d | grep %d\0",vlslot[i],vlslot[i]);
      progpipe = popen(zeile2,"r");
      if (fgets(zeile,100,progpipe) != NULL) {
	zeile1 = strtok(strtok(zeile,"\n")," ");	
	vlservpid=atoi(zeile1);
      }
      pclose(progpipe);
      if (vlservpid != vlslot[i]) vlslot[i]=0;
    }
  }

  /* open all fifos and send koords EXCEPT for sender */
  sprintf(puffer, "%d %d %d %d %d %d %d %d %d %f %f \0",(int)16,a,b,c,modus,x,y,z,type,tpos,tneg);
  for (int i=0; i<MAXSLOTS; i++) {
    if (vlslot[i]>0) {
      if (vlslot[i]!=pid_nr || ini==1) {
     
	sprintf(str,"/tmp/lipsia.%d\0",vlslot[i]);
	if((fifoo[i]=open(str, O_WRONLY)) == - 1) {
	  // fprintf(stderr,"Terminating link to PID: %d\n",pid_nr);
	  vlslot[i]=0; sw=0; // Can not open fifo. Terminating Slot.
	} else {
	  if(write(fifoo[i],puffer,strlen(puffer))  == -1) 
	    fprintf(stderr, "Error! can't write into Lipsia FIFO for PID %d\n",vlslot[i]);
	}
      }
    }
  }


  /* counting no of active vlviews */
  count=0;
  for (int i=0; i<MAXSLOTS; i++) {
    if (vlslot[i]>0) count++;
  }
  
  QString pidtext=tr("X:%1 Y:%2 Z:%3\nConnected: %4").arg(x).arg(y).arg(z).arg(count);
  pidlabel->setText( tr(pidtext));

  if (ini==0) strcpy(buftmp,buf);
}
