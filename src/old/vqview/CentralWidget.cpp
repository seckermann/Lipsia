#include "CentralWidget.h"

#include <qlayout.h>

CentralWidget::CentralWidget( Configurator *conf, QWidget *parent, const char *, WFlags f ) :
	QWidget( parent, "Central Image Widget", f )
{
	m_configurator = conf;
	createWidget();
}

CentralWidget::~CentralWidget()
{
}

void CentralWidget::createWidget()
{
	QHBoxLayout *layout = new QHBoxLayout( this );
	layout->setAutoAdd( true );

	m_coronar = new ImageWidget( m_configurator, this, "coronar image" );
	connect( m_coronar, SIGNAL( positionChanged( ImageWidget *, int, int ) ),
			 this, SLOT( positionChanged( ImageWidget *, int, int ) ) );
	connect( m_coronar, SIGNAL( crossPositionChanged( ImageWidget *, int, int ) ),
			 this, SLOT( crossPositionChanged( ImageWidget *, int, int ) ) );

	m_sagittal = new ImageWidget( m_configurator, this, "sagittal image" );
	connect( m_sagittal, SIGNAL( positionChanged( ImageWidget *, int, int ) ),
			 this, SLOT( positionChanged( ImageWidget *, int, int ) ) );
	connect( m_sagittal, SIGNAL( crossPositionChanged( ImageWidget *, int, int ) ),
			 this, SLOT( crossPositionChanged( ImageWidget *, int, int ) ) );

	m_axial = new ImageWidget( m_configurator, this, "axial image" );
	connect( m_axial, SIGNAL( positionChanged( ImageWidget *, int, int ) ),
			 this, SLOT( positionChanged( ImageWidget *, int, int ) ) );
	connect( m_axial, SIGNAL( crossPositionChanged( ImageWidget *, int, int ) ),
			 this, SLOT( crossPositionChanged( ImageWidget *, int, int ) ) );

	updateImageData();

	connect( m_configurator, SIGNAL( zoomChanged() ),
			 this, SLOT( updateImageData() ) );
	connect( m_configurator, SIGNAL( crossPositionChanged() ),
			 this, SLOT( updateImageData() ) );

	// set backround color to black, so we don't see any white flickering on resize
	setPaletteBackgroundColor( QColor( 0, 0, 0 ) );


}

void CentralWidget::positionChanged( ImageWidget *img, int x, int y )
{
	if ( img == m_coronar ) {
		m_configurator->setMousePosition( y, -1, x );
	} else if ( img == m_sagittal ) {
		m_configurator->setMousePosition( y, x, -1 );
	} else if ( img == m_axial ) {
		m_configurator->setMousePosition( -1, y, x );
	}

}

void CentralWidget::crossPositionChanged( ImageWidget *, int, int )
{
	m_configurator->lockPosition();
	updateImageData();
}

void CentralWidget::leaveEvent( QEvent * )
{
	m_configurator->setMouseValid( false );
}

void CentralWidget::enterEvent( QEvent * )
{
	m_configurator->setMouseValid( true );
}

void CentralWidget::updateImageData()
{
	VImageManager &manager = m_configurator->imageManager();
	unsigned char *data;
	int width;
	int height;

	data = manager.coronarData();
	width = manager.cols();
	height = manager.bands();
	m_coronar->setImageData( data, width, height );
	m_coronar->setCrossPos( m_configurator->col(), m_configurator->band() );
	m_coronar->update();

	data = manager.axialData();
	width = manager.cols();
	height = manager.rows();
	m_axial->setImageData( data, width, height );
	m_axial->setCrossPos( m_configurator->col(), m_configurator->row() );
	m_axial->update();

	data = manager.sagittalData();
	width = manager.rows();
	height = manager.bands();
	m_sagittal->setImageData( data, width, height );
	m_sagittal->setCrossPos( m_configurator->row(), m_configurator->band() );
	m_sagittal->update();
}
