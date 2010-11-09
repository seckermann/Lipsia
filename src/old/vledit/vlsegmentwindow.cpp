/***************************************************************************
 *   Copyright (C) 2005 by Thomas Proeger
 *   proeger@cbs.mpg.de                                               *
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

#include "vlsegmentwindow.h"
#include "datamanager.h"
#include "uiconfig.h"
#include "vlcolorboxtableitem.h"
#include "vlvisibleboxtableitem.h"
#include "vlsavereportdialog.h"

#include <vector>

#include <qfiledialog.h>
#include <qtable.h>
#include <qmessagebox.h>
#include <qaction.h>
#include <qevent.h>
#include <qcolor.h>
#include <qpixmap.h>

 /* Constructor */
 vlSegmentWindow::vlSegmentWindow(QWidget* parent, const char* name)
 : SegmentWindow(parent, name){

     volumeTable = new vlTable(volume, "volumeTable");
     volumeTable->setNumCols( volumeTable->numCols() + 1 );
     volumeTable->horizontalHeader()->setLabel( volumeTable->numCols() - 1, tr( "Name" ) );
     volumeTable->setNumCols( volumeTable->numCols() + 1 );
     volumeTable->horizontalHeader()->setLabel( volumeTable->numCols() - 1, tr( "Resolution" ) );
     volumeTable->setNumCols( volumeTable->numCols() + 1 );
     volumeTable->horizontalHeader()->setLabel( volumeTable->numCols() - 1, tr( "Volume" ) );
     volumeTable->setFocusPolicy( QTable::WheelFocus );
     volumeTable->setNumRows( 0 );
     volumeTable->setNumCols( 4 );
     volumeTable->setReadOnly( TRUE );
     volumeTable->setSelectionMode( QTable::SingleRow );
     volumeTable->setFocusStyle( QTable::FollowStyle );
     volumeTable->setLeftMargin(0);
     // configure column 4
     volumeTable->setColumnWidth(3, 32);
     volumeTable->horizontalHeader()->setLabel(3,tr(" "));

     colorTable = new vlTable(color, "colorTable");
     colorTable->setNumCols( colorTable->numCols() + 1 );
     colorTable->horizontalHeader()->setLabel( colorTable->numCols() - 1, tr( "Name" ) );
     colorTable->setNumCols( colorTable->numCols() + 1 );
     colorTable->horizontalHeader()->setLabel( colorTable->numCols() - 1, tr( "Color" ) );
     colorTable->setNumRows( 0 );
     colorTable->setNumCols( 2 );
     colorTable->setReadOnly( TRUE );
     colorTable->setSelectionMode( QTable::SingleRow );
     colorTable->setLeftMargin(0);


     // Connections
     connect(volumeTable, SIGNAL(currentChanged(int,int)), this,
             SLOT(volumeTable_currentChanged(int,int)));
     connect(volumeTable, SIGNAL(pressed(int,int,int,const QPoint&)), this,
             SLOT(volumeTable_pressed(int,int,int,const QPoint&)));

     connect(colorTable, SIGNAL(currentChanged(int,int)), this,
             SLOT(colorTable_currentChanged(int,int)));
     connect(colorTable, SIGNAL(pressed(int,int,int,const QPoint&)), this,
             SLOT(colorTable_pressed(int,int,int,const QPoint&)));

     // no event occured, yet
     event = EVENT_NONE;
}

 /* Destructor */
 vlSegmentWindow::~vlSegmentWindow(){
 }

void vlSegmentWindow::closeEvent(QCloseEvent* e) {
 	emit segmentWindowClosed();
 	e->accept();
 }

void vlSegmentWindow::fileNew() {

    // neues Segment erzeugen und id holen.
    int	segNumber = DATAMANAGER->createSegment();

    // Einträge in den Tabellen des Segmentfensters erzeugen.
    newEntry(segNumber);

}

 void vlSegmentWindow::fileOpen() {

	QString tmp;
	QString filename = QFileDialog::getOpenFileName(
		QString::null,
		"Vista Images (*.v)",

		this,
		"open segment dialog",
		"Choose a file to open");

	if ( filename != "" )
	{
        std::vector<int> indexList = DATAMANAGER->loadSegments(filename);

		if(indexList.size() == 0){
			QString error_str(
				"Error loading segment!\nPlease, check the log and\
 standard output for details!");
			QMessageBox::warning(
				(QWidget*)this,
				tr("Error!"),
				tr(error_str),
				QMessageBox::Ok,
				QMessageBox::NoButton,
				QMessageBox::NoButton);
			qWarning("fileOpen error: error reading segment file!");
			return;
		}
        // Fuege die neuen Segmente der Liste hinzu
        std::vector<int>::const_iterator iter = indexList.begin();
        while(iter != indexList.end()) {
            newEntry(*(iter++));
        }
	}

 }

 void vlSegmentWindow::fileSaveVisibleAction_activated() {

    // check if there is something to save

    if(vlSaveReportDialog(this).exec(false) == QDialog::Rejected)
        return;

 	// nur gueltige Segmente speichern... !
 	if(DATAMANAGER->isValidSegment()) {
 			QString filename = QFileDialog::getSaveFileName(
			QString::null,
			"Vista Images (*.v)",
			this,
			"save file dialog",
			"Save selected segment to .. ");

		if ( filename != "" )
		{
			if (QFile::exists(filename))
			{
				QString text=tr("The file ") + filename +
					tr(" already exists.\nDo you want to overwrite it?");
				if(QMessageBox::question(this, "overwrite file??", text, "Yes",
					"No") == 1 )
					return;
			}
			DATAMANAGER->saveSelection(filename);
		}
	 }
 }

 /** This slot handles events which should delete segments */
void vlSegmentWindow::editCut() {

    // sage allen currentChanged() events, dass ein Segment
    // aus der Liste entfernt wurde.
    event = EVENT_DELETE;

    // id string der aktuellen Auswahl holen
    int row = volumeTable->currentRow();
    int id = UICONFIG->idByRow(row);

    // Selection mit gewaehlter id aus der Liste der Segmente loeschen.
    DATAMANAGER->removeSegment(id);
    vector<int>* plist = UICONFIG->segList();
    vector<int>::iterator iter = plist->begin();
    while(iter != plist->end()) {
        if(*iter == id) {
            plist->erase(iter);
            iter = plist->end();
        }else {
            iter++;
        }
    }

    // aktuellen Listeneintrag entfernen
    volumeTable->removeRow(row);
    colorTable->removeRow(row);

    // neue ID fuer neues Segment ermitteln
    int new_id = 0;
    int new_row = 0;
    // Anzahl der verbleibenden Tabellenelemente
    int table_size = volumeTable->numRows();
    // letztes Element gelöscht, Tabelle leer
    if ( table_size == 0 ) {
        new_id = 0;
        new_row = -1;
    }
    else {
        // es ist noch mind. ein Element nach dem geloeschten vorhanden
        if( table_size > row ) {
            new_row = row;
        }
        else {
            // es wurde das Element am Ende der Liste geloescht
            if(table_size == row) {
                new_row = row - 1;
            }
        }
    }

    // Es kann ein neuer Listeneintrag ausgewaehlt werden.
    // Zuvor wird die alte Selektion zurueckgesetzt.
    if(new_row >= 0) {
        volumeTable->clearSelection(true);
        volumeTable->selectRow(new_row);
        colorTable->clearSelection(true);
        colorTable->selectRow(new_row);
        // neue id bestimmen
        new_id = UICONFIG->idByRow(new_row);
    }

    DATAMANAGER->selCurrentSegment(new_id);
    UICONFIG->setCurrSegmentNumber(new_id);

    updateGUI();

}



void vlSegmentWindow::newEntry(int id) {

 	/*****************************************************************
 	 * Aenderungen an volumeTable
 	 ****************************************************************/

    // signalisiere allen currentChanged() events, dass ein neues Segment
    // in die Liste aufgenommen wurde.
    event = EVENT_NEW;

    //bisherige Selektionen loeschen
    volumeTable->clearSelection(true);
    //colorTable->clearSelection(true);

 	// neuer Index am Anfang der Tabelle, erster Aufruf von
 	// tableCurrentChanged()
 	int newRow = 0;
 	// neuen Eintrag an's Ende der Tabelle
 	volumeTable->insertRows(newRow);

 	DATAMANAGER->selCurrentSegment(id);

 	// Farbe ermitteln
    int* color = DATAMANAGER->selection()->color;
    QColor c (color[0], color[1], color[2]);

 	// Pixmap generieren, wird verwendet fuer die id-Felder aller Tabellen.
 	QPixmap pixmap(PIXMAP_LENGTH,PIXMAP_LENGTH);
 	pixmap.fill(c);

 	// Name setzen
 	QTableItem *item = new QTableItem(
 		volumeTable,
 		QTableItem::Never,
 		QString::number(DATAMANAGER->segment(id)->name),
 		pixmap);
    volumeTable->setItem(newRow, 0, item);

 	// Auflösung setzen
 	// Auflösung noch einmal holen (zur Kontrolle)
 	float* res = DATAMANAGER->selection()->getResolution();
 	QString strResolution = QString::number(res[0]) + " x "
 		+ QString::number(res[1]) + " x "
 		+ QString::number(res[2]);
    volumeTable->setText(newRow, 1, strResolution);

 	// Volumen setzen
 	volumeTable->setText(newRow, 2,
 		QString::number(DATAMANAGER->selection()->getVolume()));

    // Visible Box setzen
    vlVisibleBoxTableItem* vlvbti = new vlVisibleBoxTableItem(volumeTable);
    volumeTable->setItem(newRow, 3, vlvbti);

    connect(vlvbti, SIGNAL(visibilityChanged(bool, int)),
        this, SLOT(visibilityChanged(bool, int)));

    // die erste Zeile auswaehlen.
 	volumeTable->selectRow(newRow);

 	/*********************************************************************
 	 * Aenderungen an colorTable
 	 ********************************************************************/

 	 colorTable->insertRows(newRow);

 	 //Name setzen
 	 item = new QTableItem(
 		volumeTable,
 		QTableItem::Never,
 		QString::number(DATAMANAGER->segment(id)->name),
 		pixmap);

 	 colorTable->setItem(newRow, 0, item);
 	 //Farbe setzen
 	 vlColorBoxTableItem* vlcbti = new vlColorBoxTableItem(colorTable, c);
 	 colorTable->setItem(newRow,1,vlcbti);

 	 // wir wollen ueber Aenderungen an der Farbe informiert werden.
 	 connect(vlcbti, SIGNAL(colorChanged(int)), this, SLOT(colorChanged(int)));

 	 //neuen Eintrag auswaehlen
 	 colorTable->selectRow(newRow);

     // UI aktualisiseren
     // neuer Eintrag kommt immer an's Ende der Liste.
     UICONFIG->segList()->push_back(id);
     UICONFIG->setCurrSegmentNumber(id);

     updateGUI();
}

void vlSegmentWindow::volumeChanged() {

    // nur mit gueltigen Segmenten arbeiten
    if(DATAMANAGER->isValidSegment()){
        // wir nehmen an, dass das gewaehlte Segment das aktuelle Segment ist
        int currentRow = volumeTable->currentRow();
        volumeTable->setText(currentRow, 2,
                QString::number(DATAMANAGER->selection()->getVolume()));
    }
}

void vlSegmentWindow::volumeTable_currentChanged(int, int) {

    // Aenderungen wurden schon beruecksichtigt.
    if((event == EVENT_NEW) || (event == EVENT_DELETE))
        return;

    int row = volumeTable->currentRow();

    // Aktualisiere colorTable
    colorTable->selectRow(row);

    int id = UICONFIG->idByRow(row);
    DATAMANAGER->selCurrentSegment(id);
    UICONFIG->setCurrSegmentNumber(id);

    updateGUI();


}

void vlSegmentWindow::colorTable_currentChanged(int, int) {


    // Aenderungen wurden schon beruecksichtigt.
    if((event == EVENT_NEW) || (event == EVENT_DELETE))
        return;

    int row = colorTable->currentRow();

    // Aktualisiere colorTable
    volumeTable->selectRow(row);

    int id = UICONFIG->idByRow(row);
    DATAMANAGER->selCurrentSegment(id);
    UICONFIG->setCurrSegmentNumber(id);

    updateGUI();

}

void vlSegmentWindow::volumeTable_pressed(int row,int,int,const QPoint&) {

    // sage allen folgenden currentChanged events, dass ein Klick
    // voraus ging
    event = EVENT_CLICK;

 	if(row >= 0) {

 		// Aktualisiere colorTable
 		colorTable->selectRow(row);

 		int id = UICONFIG->idByRow(row);
 		DATAMANAGER->selCurrentSegment(id);
 		UICONFIG->setCurrSegmentNumber(id);

        updateGUI();
 	}

}

void vlSegmentWindow::colorTable_pressed(int row,int,int,const QPoint&) {

    // sage allen folgenden currentChanged events, dass ein Klick
    // voraus ging
    event = EVENT_CLICK;

 	if(row >= 0) {

 		// Aktualisiere colorTable
 		volumeTable->selectRow(row);

 		int id = UICONFIG->idByRow(row);
 		DATAMANAGER->selCurrentSegment(id);
 		UICONFIG->setCurrSegmentNumber(id);

        updateGUI();
 	}

}

void vlSegmentWindow::colorChanged(int row) {

	/**
	 * Die Farbe des Segments in "row" wurde geaendert
	 */

	int id = UICONFIG->idByRow(row);

	// Farbwert anpassen
	VistaSegment* seg = DATAMANAGER->segment(id);
	if(seg) {
		QString s = colorTable->text(row,1);
		QStringList strlist = QStringList::split(" ",s);
		if(strlist.count() >= 3) {
			//Segment
			seg->setColor(
				strlist[0].toInt(),
				strlist[1].toInt(),
				strlist[2].toInt()
			);
			// id-Felder der Tabellen
			QColor c(
				strlist[0].toInt(),
				strlist[1].toInt(),
				strlist[2].toInt()
			);
			QPixmap pixmap(PIXMAP_LENGTH,PIXMAP_LENGTH);
			pixmap.fill(c);
			volumeTable->item(row, 0)->setPixmap(pixmap);
			colorTable->item(row, 0)->setPixmap(pixmap);

		}
		else
			qWarning("%s: no valid color value.", s.latin1());
	}
	else
		qWarning("segment id %d not found.", id);

    UICONFIG->recreateViews();

}

void vlSegmentWindow::fileSaveAllAction_activated(){

    // check if there is something to save
    if(vlSaveReportDialog(this).exec(true) == QDialog::Rejected)
        return;

    QString filename = QFileDialog::getSaveFileName(
            QString::null,
            "Vista Images (*.v)",
            this,
            "save file dialog",
            "Save all segments to ..");

    if ( filename != "" )
    {
        if (QFile::exists(filename))
        {
            QString text=tr("The file ") + filename +
                tr(" already exists.\nDo you want to overwrite it?");
            if(QMessageBox::question(this, "overwrite file??", text, "Yes",
                        "No") == 1 )
                return;
        }
        DATAMANAGER->saveSelection(filename, true);
    }
}


void vlSegmentWindow::visibilityChanged(bool visible, int row) {

    int id = UICONFIG->idByRow(row);

    // set new visibility val
    DATAMANAGER->segment(id)->visible = visible;
    // update views
    UICONFIG->recreateViews();

    updateGUI();
}

void vlSegmentWindow::segmentUpAction_activated() {

    int row = volumeTable->currentRow();

    // das erste Element kann Tabelle nicht verlassen.
    if(row <= 0)
        return;

    /******************
     * volumeTable
     *****************/
    volumeTable->swapRows(row,row-1,TRUE);
    volumeTable->updateContents();

    /******************
     * colorTable
     ******************/
    colorTable->swapRows(row,row-1,TRUE);
    colorTable->updateContents();

    /* swap internal order */
    vector<int>::iterator iter = UICONFIG->segList()->begin();
    while(iter != UICONFIG->segList()->end()) {
        if(*iter == UICONFIG->idByRow(row)) {
            int tmp = *iter;
            *iter = *(++iter);
            *iter = tmp;
            break;
        }
        iter++;
    }

    row--;

    // Zeile auswaehlen
    volumeTable->clearSelection(true);
    volumeTable->selectRow(row);
    colorTable->clearSelection(true);
    colorTable->selectRow(row);

    // Grafiken aktualisieren
    UICONFIG->recreateViews();

    updateGUI();
}

void vlSegmentWindow::segmentDownAction_activated() {

    int row = volumeTable->currentRow();

    // das letzte Element kann Tabelle nicht verlassen.
    if(row >= volumeTable->numRows()-1)
        return;

    /******************
     * volumeTable
     *****************/
    volumeTable->swapRows(row,row+1,TRUE);
    volumeTable->updateContents();

    /******************
     * colorTable
     ******************/
    colorTable->swapRows(row,row+1,TRUE);
    colorTable->updateContents();

    /* swap internal order */
    vector<int>::iterator iter = UICONFIG->segList()->begin();
    while(iter != UICONFIG->segList()->end()) {
        if(*iter == UICONFIG->idByRow(row)) {
            int tmp = *iter;
            *iter = *(--iter);
            *iter = tmp;
            break;
        }
        iter++;
    }

    row++;

    // Zeile auswaehlen
    volumeTable->clearSelection(true);
    volumeTable->selectRow(row);
    colorTable->clearSelection(true);
    colorTable->selectRow(row);

    // Grafiken aktualisieren
    UICONFIG->recreateViews();

    updateGUI();

}

void vlSegmentWindow::updateGUI() {

    /**
     * Save Buttons
     */
    // Liste ist leer
    if(UICONFIG->segList()->size() == 0) {
        fileSaveAllAction->setEnabled(false);
        fileSaveVisibleAction->setEnabled(false);
        editCutAction->setEnabled(false);
    }
    // mindestens 1 Segment
    else {
        //Grundeinstellung
        fileSaveAllAction->setEnabled(true);
        fileSaveVisibleAction->setEnabled(false);
        editCutAction->setEnabled(true);
        //Nachjustieren: "saveVisible" nur anschalten wenn sichtbares Segment
        //vorhanden.
        vector<int>::iterator iter = UICONFIG->segList()->begin();
        while(iter != UICONFIG->segList()->end()) {
            if(DATAMANAGER->segment(*iter)->visible) {
                fileSaveVisibleAction->setEnabled(true);
                break;
            }
            iter++;
        }

    }
    /**
     * Pfeile
     */
    int row = volumeTable->currentRow();

    // einzeilige Tabelle oder keine ausgewaehlte Zeile
    if((row < 0) || (volumeTable->numRows() < 2)) {
        segmentUpAction->setEnabled(false);
        segmentDownAction->setEnabled(false);

        return;
    }

    /** ab hier: mindestens zwei Eintraege in der Tabelle */

    // erste Zeile
    if(row == 0)  {
        segmentUpAction->setEnabled(false);
        segmentDownAction->setEnabled(true);

        return;
    }

    // letzte Zeile
    if(row == volumeTable->numRows()-1)  {
        segmentUpAction->setEnabled(true);
        segmentDownAction->setEnabled(false);

        return;
    }

    // irgendeine Zeile zwischendrin
    segmentUpAction->setEnabled(true);
    segmentDownAction->setEnabled(true);

}
