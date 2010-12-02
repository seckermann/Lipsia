#include <qpixmap.h>
#include <qcolor.h>
#include <qstringlist.h>
#include <qapplication.h>
#include <qcolordialog.h>
#include "vlcolorboxtableitem.h"
#include "uiconfig.h"

vlColorBoxTableItem::vlColorBoxTableItem( QTable *t, QColor &c )
	: QTableItem( t, QTableItem::Always ), m_pbutton( )
{
	m_color = c;
	setReplaceable( false );
}

QWidget *vlColorBoxTableItem::createEditor() const
{

	// neuen Knopf erstellen
	( ( vlColorBoxTableItem * )this )->m_pbutton = new QPushButton( table()->viewport() );
	// generate a color pixmap with configured color.
	QPixmap pm ( PIXMAP_WIDTH, PIXMAP_HEIGHT );
	pm.fill( m_color );
	m_pbutton->setPixmap( pm );

	connect( m_pbutton, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );

	return m_pbutton;

}

void vlColorBoxTableItem::setContentFromEditor( QWidget *w )
{
	// Pushbutton -> setze Zelltext auf Name des Pixmaps(RGB-String, siehe
	// createEditor()).
	if ( w->inherits( "QPushButton" ) ) {
		QString s = QString::number( m_color.red() ) + " " +
					QString::number( m_color.green() ) + " " +
					QString::number( m_color.blue() );
		QTableItem::setText( s );
	} else
		QTableItem::setContentFromEditor( w );
}

void vlColorBoxTableItem::setText( const QString &s )
{

	// Button definiert, sollte immer so sein
	if( m_pbutton ) {
		// Farbwerte ermitteln
		QStringList colors = QStringList::split( " ", s );

		if( colors.count() < 3 )
			qWarning( "wrong color string (%s)", s.latin1() );

		// Farbe aktualisieren
		m_color.setRgb( colors[0].toInt(), colors[1].toInt(), colors[2].toInt() );

		// Button neu zeichnen
		refreshButton();

	}

	QTableItem::setText( s );
}

void vlColorBoxTableItem::buttonClicked()
{

	QColor color = QColorDialog::getColor( m_color, table(), 0 );

	// a color was chosen and the dialog was closed with "ok".
	if( color.isValid() ) {
		m_color = color;
		refreshButton();
		QString s = QString::number( m_color.red() ) + " " +
					QString::number( m_color.green() ) + " " +
					QString::number( m_color.blue() );

		QTableItem::setText( s );

		// tells the table that the color was changed in the table items row.
		emit colorChanged( row() );

	}

}

void vlColorBoxTableItem::refreshButton()
{

	if( m_pbutton ) {
		// neues Pixmap anlegen
		QPixmap pm = QPixmap( PIXMAP_WIDTH, PIXMAP_HEIGHT );

		pm.fill( QColor( m_color ) );
		m_pbutton->setPixmap( pm );
	} else
		qFatal( "color button is NULL. Such things should never happen!" );

}
