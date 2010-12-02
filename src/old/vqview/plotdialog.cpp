#include "plotdialog.h"

#include "grid.xpm"
#include "connected.xpm"
#include "disconnected.xpm"


#include <qpixmap.h>
#include <qtoolbar.h>


PlotDialog::PlotDialog( Configurator *conf, QWidget *parent, const char *name )
	: QMainWindow( parent, name, WType_TopLevel | WDestructiveClose )
{
	m_conf = conf;
	setBaseSize( 800, 400 );
	createWidget();
}


PlotDialog::~PlotDialog()
{
}

void PlotDialog::createActions()
{
	m_drawGridAction = new QAction( "&Draw Grid", CTRL + Key_G , this, "grid" );
	m_drawGridAction->setToggleAction( true );
	m_drawGridAction->setIconSet( QPixmap( grid_xpm ) );
	m_drawGridAction->setOn( true );
	connect( m_drawGridAction, SIGNAL( toggled( bool ) ), this, SLOT( drawGrid( bool ) ) );

	m_syncAction = new QAction( "&Syncronize wiith Selection", CTRL + Key_S , this, "sync" );
	m_syncAction->setToggleAction( true );
	m_syncAction->setOn( true );
	QIconSet is;
	is.setPixmap( QPixmap( connected_xpm ), QIconSet::Automatic, QIconSet::Normal, QIconSet::On );
	is.setPixmap( QPixmap( disconnected_xpm ), QIconSet::Automatic, QIconSet::Normal, QIconSet::Off );
	m_syncAction->setIconSet( is );
	connect( m_syncAction, SIGNAL( toggled( bool ) ), this, SLOT( setSync( bool ) ) );

}

void PlotDialog::createStatusBar()
{

}

void PlotDialog::createToolBar()
{
	QToolBar *toolbar = new QToolBar( this );

	m_drawGridAction->addTo( toolbar );
	m_syncAction->addTo( toolbar );

}

void PlotDialog::createWidget()
{
	createActions();
	createToolBar();
	createStatusBar();
	m_plotWidget = new PlotWidget( m_conf, this, "Plotter Central Widget" );
	setCentralWidget( m_plotWidget );
}

void PlotDialog::drawGrid( bool on )
{
	m_plotWidget->setDrawGrid( on );
}

void PlotDialog::setSync( bool on )
{
	m_plotWidget->setSync( on );
}
