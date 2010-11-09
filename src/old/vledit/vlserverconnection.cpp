/***************************************************************************
 *   Copyright (C) 2005 by Hannes Niederhausen                             *
 *   niederhausen@cbs.mpg.de                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "vlserverconnection.h"
#include "uiconfig.h"
#include "datamanager.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

int serverini=0;

extern "C" {
    extern void VTal2Pixel(float [3],float [3],float [3],float,float,float,
           int*, int*, int*); 
    extern void VPixel2Tal(float [3],float [3],float [3],int,int,int,float
            *,float *,float *);
}

vlServerConnection::vlServerConnection()
{
	m_useServerInput=true;
}


vlServerConnection::~vlServerConnection()
{
	
}

void vlServerConnection::connectToServer( )
{
	char zeile[100];
	
    /* make own pipe for IPC if the server exists and worls*/
	if (createServerPipe()) {
    	sprintf(m_fifoname,"/tmp/lipsia.%d",getpid());
    	if(mknod(m_fifoname, S_IFIFO | S_IRUSR | S_IWUSR, 0) == - 1) {
      		qWarning ("Can't create a fifo.........\n");
    	} else {
      		if((clientPipe=open(m_fifoname, O_RDWR)) == - 1) {
      			qWarning("Can't open the fifo socket %s.\n",m_fifoname);
      		} else {
				m_notif = new QSocketNotifier (clientPipe, QSocketNotifier::Read);
				connect (m_notif, SIGNAL(activated(int)), this, SLOT(syncronize(int)));
				connect (UICONFIG, SIGNAL(sendCoordinates(int)), this,SLOT(sendCoordinates(int)) );
				sendCoordinates(serverini);
      		}
    	}
  	}
}

void vlServerConnection::sendCoordinates(int status )
{

	char buffer[512];

	//no suitable vlserv process found
	if (!m_useServerInput)
		return;

    // Server found, but user has disabled synchronization
    if(! UICONFIG->sync())
        return;

	// test if vlserv is available
    bool sw_vlserv=FALSE; char zeile[100], zeile2[100]; FILE *progpipe; char *zeile1;
    int vlservpid=0;
    progpipe = popen("whoami","r");
    if (fgets(zeile,100,progpipe) != NULL) zeile1 = strtok(zeile,"\n");
    pclose(progpipe);
    sprintf(zeile2,"ps -u %s | grep vlserv",zeile1);
    progpipe = popen(zeile2,"r");
    if (fgets(zeile,100,progpipe) != NULL) {
      zeile1 = strtok(strtok(zeile,"\n")," ");
      vlservpid=atoi(zeile1);
      while (zeile1) {
	zeile1 = strtok(NULL," ");
	if (zeile1) {
	  if (!strcmp(zeile1,"vlserv")) sw_vlserv=TRUE;
	}
      }
    }
    pclose(progpipe);

	if (sw_vlserv){	

        int type;
        float x,y,z;

        // im Talairachmode werden die Koordinaten auch als Talairachkoordinaten 
        // uebermittelt.

	if (status==0) {
	  if(UICONFIG->coordMode() == TAL_MODE) {
            type = 1;
            float* ca = new float[3];
            ca[0] =rintf(DATAMANAGER->ca()[0]);
            ca[1] =rintf(DATAMANAGER->ca()[1]);
            ca[2] =rintf(DATAMANAGER->ca()[2]);
            float* res = DATAMANAGER->image()->getResolution();
            float* extent = new float[3];
            extent[0] = DATAMANAGER->extent()[0] * res[0];
            extent[1] = DATAMANAGER->extent()[1] * res[1];
            extent[2] = DATAMANAGER->extent()[2] * res[2];
	    
            VPixel2Tal(ca,res,extent,(int)rintf(UICONFIG->band()),
		       (int)rintf(UICONFIG->row()), (int)rintf(UICONFIG->column()),&x,&y,&z);
	    
            x = rintf(x);
            y = rintf(y);
            z = rintf(z);
	    
            delete extent;
	    
	  }
	  // in einem der anderen Modi werden die Koordinaten als Pixelkoordinaten
	  // uebermittelt.
	  else {
            type = 0;
            x = UICONFIG->column();
            y = UICONFIG->row();
            z = UICONFIG->band();
	  }

	  sprintf (buffer, "%d %d %d %d %d %d %d %d %d %f %f", getpid(), 0,0,0, 
		   0, (int)x, (int)y, (int)z, type, 0.f, 0.f);

	} else {
	  sprintf (buffer, "%d %d %d %d %d %d %d %d %d %f %f", getpid(), (int)0, (int)0, (int)0, 
		   (int)0, (int)32000, (int)32000, (int)32000, 0, 0.0, 0.0);
	}

	write(serverPipe,buffer,strlen(buffer));


	} else {
		qWarning("Server down. Disable server use");
		m_useServerInput=FALSE;
        emit serverDown();
	}

}

void vlServerConnection::syncronize( int )
{
    // kein benutzbarer Server gefunden
	if (!m_useServerInput)
		return;

    // Server da, aber UI verbietet Synchronisation
    if (!UICONFIG->sync())
        return;

	int pid;
	int a,b,c;//Rotation -> wird hier nicht benutzt
	
	int rotMode; //Rotationsmodus
	
	int x,y,z;	//Koordinaten fuer das Zielkreuz
	
	int coordType;	//Koordinatenmodus
	
	float pos, neg; //Sliderdaten, werden ebenfalls nicht benoetigt
	char buf[2000]; //buffer zum Einlesen der Daten
	
	//%todo check connection
	if (read(clientPipe, &buf, sizeof(buf))==-1){
		qWarning("Can't read from client pipe");
	} else {
		sscanf(buf, "%d %d %d %d %d %d %d %d %d %f %f\n", &pid, &a, &b, &c, &rotMode, &x, &y, &z, &coordType, &pos, &neg);
		
		//sscanf(buf, "%d %d %d %d %d %d %d %d %d %f %f\n", &pid, &a, &b, &c, &x, &y, &z, &coordType, &pos, &neg);
	}
    

    // coordType = [0|1]
	//Pixelkoordinaten verarbeiten
	if(coordType==0 || (x>30000 && y>30000 && z>30000)) {
		UICONFIG->setAim(z,y, x);
	}
    // Talairachkoordinaten verarbeiten.
    else {
        if(DATAMANAGER->ca() == NULL) {
            qWarning("Anatomie image doesn't provide a ca-Value. No Talairach mode available");
            return;
        }

        int b,r,c;
        // der Extent wird in mm angegeben (?)
        float* res = DATAMANAGER->image()->getResolution();
        float* extent = new float[3];
        extent[0] = DATAMANAGER->extent()[0] * res[0];
        extent[1] = DATAMANAGER->extent()[1] * res[1];
        extent[2] = DATAMANAGER->extent()[2] * res[2];

        // wir brauchen die Koordinaten als Pixelwerte.
        VTal2Pixel(DATAMANAGER->ca(),res,extent,x,y,z,&b,&r,&c);

        UICONFIG->setAim(b,r,c);

        delete extent;
    }
}

void vlServerConnection::disconnectFromServer( )
{
	unlink(m_fifoname);
	close(clientPipe);
}

bool vlServerConnection::createServerPipe( )
{
/* Erstelle Name fuer Pipe */
  char zeile[100], 
  	   zeile2[100]; 
  FILE *progpipe; 
  char pipename[100]; 
  char *zeile1;


  progpipe = popen("whoami","r");
  if (fgets(zeile,100,progpipe) != NULL) {
    zeile1 = strtok(zeile,"\n");
    sprintf(pipename,"/tmp/lipsia.%s",zeile1);
  } else 
    sprintf(pipename,"/tmp/lipsia.unknown");
  pclose(progpipe);
 
//   check if vlserv is running
  bool sw_vlserv=FALSE;
  int vlservpid=0;
  sprintf(zeile2,"ps -u %s | grep vlserv",zeile1);
  progpipe = popen(zeile2,"r");
  if (fgets(zeile,100,progpipe) != NULL) {
    zeile1 = strtok(strtok(zeile,"\n")," ");
    vlservpid=atoi(zeile1);
    while (zeile1) {
      zeile1 = strtok(NULL," ");
      if (zeile1) {
	if (!strcmp(zeile1,"vlserv")) sw_vlserv=TRUE;
      }
    }
  }
  pclose(progpipe);
  //free(zeile); free(zeile2);
  
  /* open own pipe for obtaining from server */
  if (sw_vlserv==TRUE) {
    serverini=1;
    if ((serverPipe=open(pipename, O_WRONLY)) == - 1) {
      sprintf(zeile2,"kill -9 %d\0",vlservpid);
      system(zeile2);
      sw_vlserv=FALSE;
    }
  }
  if (sw_vlserv==FALSE) {
    serverini=0;
    fprintf(stderr, "Do not find Lipsia Server. Restarting.\n");
    FILE *ftest;
    ftest = fopen ("/usr/bin/vlserv", "r");
    if (! ftest) {
      fprintf(stderr,"vlserv daemon not installed. Giving up.\n");
    } else {
      fclose(ftest);
      system("/usr/bin/vlserv &");
      
      sleep(1);
      if((serverPipe=open(pipename, O_WRONLY)) == - 1) {
	fprintf(stderr, "No contact with Lipsia Server. Giving up.\n");
      } else {
	sw_vlserv=TRUE;
      }
    }
  }

  // found server at all?
  m_useServerInput = sw_vlserv;

  return sw_vlserv;
}

