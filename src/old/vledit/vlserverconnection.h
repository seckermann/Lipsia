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
#ifndef VLSERVERCONNECTION_H
#define VLSERVERCONNECTION_H

#include <qobject.h>
#include <qsocketnotifier.h>
#include <stdio.h>


/**
 *	Diese Klasse bearbeitet eingehende Serveranfragen und schickt veränderte Zielkoordinaten
 *	an den Server.
 * @author Hannes Niederhausen
*/
class vlServerConnection : public QObject{
Q_OBJECT
private:
	/** Die Pipe die zum Server fuehrt*/
	int	serverPipe;
	
	/** Die Pipe zum Programm*/
	int clientPipe;

	QSocketNotifier* m_notif;

	/**Pfad und Dateiname fuer die Clientpipe*/
	char m_fifoname[200];

    /** Dieses Flag signalisiert ob ein Server gefunden werden und eine 
     * Pipe geoffnet wurde. */
	bool m_useServerInput;

    // oeffne eine Pipe zu einem lokalen Server Prozess.
	bool createServerPipe();	

public:
    vlServerConnection();

    ~vlServerConnection();

	/**
	 * connects to the server and create the pipe for response
	 */
	void connectToServer();
	
	/**
	 * closes the connection and deletes the pipe of the client
	 */
	void disconnectFromServer();

   /**
    * Returns true if a server process was found. 
    */
    bool serverFound() { return m_useServerInput; }

public slots:

	void sendCoordinates(int);
	void syncronize(int);

signals:

    // Dieses Signal wird gesendet wenn die Kommunikation mit dem Server fehlgeschlagen ist.
    // Das ist eine Reaktion auf einen gekillten Serverprozess.
    void serverDown();
};

#endif
