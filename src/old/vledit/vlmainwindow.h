/***************************************************************************
 *   Copyright (C) 2005 by Hannes Niederhausen                             *
 *   niederhausen@cbs.mpg.de			                                   *
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
#ifndef VLMAINWINDOW_H
#define VLMAINWINDOW_H

#include "mainwindow.h"
#include "vlglwidget.h"
#include "vlserverconnection.h"
#include "vlsegmentwindow.h"

#include <qlcdnumber.h>

/**
 * Diese Klasse repraesentiert das Hauptfenster der Anwendung.
 * Es erbt die Eigenschaften von MainWindow, welches mit dem QTDesigner
 * erstellt wurde und f??gt eine OpenGL Ansicht hinzu.
 * @author Hannes Niederhausen
 */
class vlMainWindow : public MainWindow
{
Q_OBJECT
protected:

	/**GL Widget f??r die Coronale Ansicht (Seitenansicht, vorderer Hirnteil
     * ist auf der linken Seite)*/
	vlGLWidget*	coronal;

	/**GL Widget f??r die Axiale Ansicht (Horizontaler Schnitt, vorderer
     * Hirnteil links)*/
	vlGLWidget*	axial;

	/**GL Widget f??r die Sagitale Ansicht (Senkrechter Schnitt)*/
	vlGLWidget*	sagittal;

	/**Der Segmentdialog (deprecated)*/
	//vlSegmentDialog*	segDialog;

	/** Das neue Segmentfenster */
	vlSegmentWindow* segWindow;

	/** Der ConnectionManager f??r den lipsia server*/
	vlServerConnection* m_serverConnection;

	//some widgets for the status bar
	QLCDNumber*	valMouse;
	QLCDNumber* valAim;
	QLabel* mouseCoords;
	QLabel* aimCoords;
	QLabel* scale;
	QLabel* modeLabel;

    // the pixmaps for sync oder nosync state
    QPixmap connect0_img;
    QPixmap connect1_img;

	int m_MousePos[3];

	/**
	 *    Erstellt den zentralen Teil des Fensters, den der QTDesigner nicht
     *    erstellen kann.Im Zentrum werden die OpenGL Ansichten gesetzt
     *    um die 3D Ansichten der MRT Daten darzustellen.
	 */
	void createCenterWidget();

	/**
	 *    Erstellt die Statusbar
	 */
	void createStatusBar();


public:
    vlMainWindow( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );

    ~vlMainWindow();

	/**
	 *    ueberlagert den Showoperator des QMainWindows um vor dessen
     *    aufruf das zentrale Widget zu erstellen.
	 */
	void show();
	
	

public slots:
	/**
	* Diese Methode wird duch die FileExitAction aufgerufen und schließt
    * das Hauptfenster um das Programm zu beenden.
	*/
	virtual void fileExit();

	/**
	 *    Slot zum Setzen des Editieremodus (l??schen an/aus)
	 * @param val
	 */
	virtual void editDelete(bool val);

	/**
	 *    Slot um auf die ??nderungen in der SpinBox zu reagieren.
	 * @param val
	 */
	virtual void radiusChanged(int val);


	/**
	 *    Der Speichern Slot, f??r die entsprechende Action
	 */
	virtual void fileSave();

    /**
     * opens the about dialog.
     */
     virtual void helpAbout();

	/**
	 *    Ereignis Handling f??r den Fall das die Anwendung geschlossen wird.
	 * @param e
	 */
	virtual void closeEvent(QCloseEvent* e);

	/**
	 *    Behandelt die Wert??nderung f??r den Grauwert.
	 * @param
	 */
	virtual void greyValChanged(int);

	/**
	 *    Ereignisbehandlung f&uuml;r die Checkbox zur Segmentl&ouml;schauswahl
	 * @param
	 */
	virtual void delSegCheckedChanged(int);

	/**
	 *    Updated die Mauskoordinaten in der Statusleiste
	 * @param band
	 * @param row
	 * @param column
	 */
	virtual void mouseMoved(int band, int row, int column);

	/**
	 *    Updated Zielkreuzkoordinaten in der Statusleiste
	 */
	virtual void aimMoved();

	/**
	 *    Updaten des Zoomfaktor in der Satusleiste
	 */
	virtual void zoomChanged();

	virtual void reset();

	void resetView();

    virtual void markSegment(bool);

	virtual void zoomIn();
    virtual void zoomOut();

	virtual void viewActionToggled(bool);

	virtual void viewSegmentWindow(bool);

	void filterComboBoxChanged(const QString &);

	void filterChanged(const QString &);

    void coordComboBox_activated( const QString & );

    void toggleSyncAction_activated();

    // Kontakt zum Server verloren -> Synchronisation
    void disableSync();
    void leaveEvent ( QEvent *);

signals:

	/**
	* Dieses Signal wird gesendet wenn erfolgreich ein Segment in den
    * Speicher geladen wurde.
	*/
	void segmentLoaded(int);
 
    

    /**
     * Dieses Signal wird gesendet wenn das Hauptfenster geschlossen wurde.
     */
    void vlMainWindowClosed();

};

#endif
