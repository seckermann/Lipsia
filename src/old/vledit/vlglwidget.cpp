/********************************************************************
 *   Copyright (C) 2005 by Hannes Niederhausen
 *   niederhausen@cbs.mpg.de
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *******************************************************************/
#include "vlglwidget.h"
#include "uiconfig.h"
#include "datamanager.h"

#include <qapplication.h>
#include <qmessagebox.h>
#include <iostream>

#define NEAR_CLIP_PLANE 0.1

#define MOVE_VALUE  2

vlGLWidget::vlGLWidget( QWidget *parent = NULL, int type ) : QGLWidget( parent )
{
	this->type = type;
	setFocusPolicy( WheelFocus );
	setMouseTracking( true );
	m_delete = false;
	UICONFIG->setAimVisible( true );
}


vlGLWidget::~vlGLWidget()
{
	delete plane;
}


void vlGLWidget::initializeGL()
{
	plane = new vlBrainPlane( type );
	glEnable( GL_TEXTURE_2D );
	camera.move( 0, 0, 300 );
	mouseX = mouseY = 0;

	switch ( type ) {
	case CORONAL:
		name = QString( "Coronal" );
		break;
	case SAGITTAL:
		name = QString( "Sagittal" );
		break;
	case AXIAL:
		name = QString( "Axial" );
		break;
	default:
		name = QString( "Unbekannt" );
	}

	glClearColor( 0, 0, 0, 1 );
	glDisable( GL_LIGHTING );
	cursorSize = 2;

}

void vlGLWidget::resizeGL( int w, int h )
{
	glViewport( 0, 0, w, h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	//gluPerspective(45.f, (GLfloat) w/h, NEAR_CLIP_PLANE, 2000.f);
	glOrtho( -w / 2, w / 2, -h / 2, h / 2, NEAR_CLIP_PLANE, 2000 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

}

void vlGLWidget::paintGL()
{
	/* relative Position (x ODER y) des Fadenkreuzes auf der Textur (inkl. Scale!!). Befindet sich z.B. das FK bei (0,0), also in der Mitte
	 * der Textur, ist die relat. Position (egal ob x oder y) 0,5.*/
	float rel_aim = 0;
	/* Zielposition der Kamera innerhalb des Widgets. */
	float cam_goal = 0;
	/* Breite der Textur inkl. Skalierungsfaktor */
	float plane_w = plane->width() * UICONFIG->scale();
	/* momentane Breite des Widgets */
	float widget_w = ( float ) width();
	/* Hoehe der Textur inklusive Skalierungsfaktor*/
	float plane_h = plane->height() * UICONFIG->scale();
	/* momentane Hoehe des Widgets */
	float widget_h = ( float ) height();

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();
	//hier kann man die verschiebung ändern (Maus nach links->objekte nach links)

	//pruefe die Weite
	//widged.with < plane.width
	if( UICONFIG->translate() ) {
		if( widget_w < plane_w || widget_h < plane_h ) {

			if ( widget_w < plane_w ) {
				/* Hinweis: Der Koordinatenursprung von aim und camera befindet sich bei (plane_w/2, plane_h/2)*/
				//Bestimme relat. Pos. des Fadenkreuzes auf der Brainplane
				rel_aim = ( plane_w / 2 + aim.X() ) / plane_w;

				/*Ziel: Kamera so einstellen, dass die relat. Position des Fadenkreuzes innerhalb des Widgets
				  auch der relativen Position des Fadenkreuzes innheralb der Brainplane entspricht.*/

				//Hier wird ausgnutzt, dass die X-Position der Kamera stets bei widget_w/2 liegt.
				cam_goal = -( rel_aim * widget_w ) + widget_w / 2 + aim.X();
				camera.moveX( cam_goal - camera.x() );
			}

			if ( widget_h < plane_h ) {
				rel_aim = ( plane_h / 2 + aim.Y() ) / plane_h;
				cam_goal = -( rel_aim * widget_h ) + widget_h / 2 + aim.Y();
				camera.moveY( cam_goal - camera.y() );
			}

		} else {
			camera.setPos( 0, 0, camera.z() );
		}
	}

	/* update new camera positions */
	camera.setGlTranslation();

	plane->draw();
	aim.draw();
	glFlush();

	// if debug flag was set --> no more debugging
	UICONFIG->debug = 0;

}

void vlGLWidget::wheelEvent( QWheelEvent *e )
{
	if ( e->delta() < 0 )
		UICONFIG->addScale( -SCALE_STEP );
	else
		UICONFIG->addScale( SCALE_STEP );

	e->accept();
	emit recreateViews( false );
	reaim();
	repaint();
}

void vlGLWidget::mouseMoveEvent( QMouseEvent *e )
{
	commonMouseClick( e );
}

void vlGLWidget::keyPressEvent ( QKeyEvent *e )
{
	if ( e->key() == Key_Up ) {
		UICONFIG->setBand( UICONFIG->band() - 1 );
		emit recreateViews( false );
		e->accept();
		repaint();
		return;
	}

	if ( e->key() == Key_Down ) {
		UICONFIG->setBand( UICONFIG->band() + 1 );
		emit recreateViews( false );
		e->accept();
		repaint();
		return;
	}

	if ( e->key() == Key_Left ) {
		UICONFIG->setColumn( UICONFIG->column() - 1 );
		emit recreateViews( false );
		e->accept();
		repaint();
		return;
	}

	if ( e->key() == Key_Right ) {
		UICONFIG->setColumn( UICONFIG->column() + 1 );
		emit recreateViews( false );
		e->accept();
		repaint();
		return;
	}

	if ( e->key() == Key_PageUp ) {
		UICONFIG->setRow( UICONFIG->row() - 1 );
		emit recreateViews( false );
		e->accept();
		repaint();
		return;
	}

	if ( e->key() == Key_PageDown ) {
		UICONFIG->setRow( UICONFIG->row() + 1 );
		emit recreateViews( false );
		e->accept();
		repaint();
		return;
	}

	if( e->key() == Key_C ) {
		UICONFIG->setAimVisible( !UICONFIG->isAimVisible() );
		emit recreateViews( false );
		e->accept();
		repaint();
		return;
	}

	// the QT-Framework suggests to leave unknown key events to the parent
	// widget.
	e->ignore();
}

void vlGLWidget::mousePressEvent( QMouseEvent *e )
{

	if( e->button() & LeftButton ) {

		// Die Mausposition in Weltkoordinaten
		float x, y;
		calculateWorldCoord( e->x(), e->y(), &x, &y );

		// Die Mausposition in Texturkoordinaten auf der Anatomietextur
		int anaPx, anaPy;
		calculateTexCoord( x, y, &anaPx, &anaPy );

		// Berechnungskorrekturen an den Rändern.
		anaPx = anaPx < 0 ? 0 : anaPx;
		anaPy = anaPy < 0 ? 0 : anaPy;
		anaPy = anaPy > plane->height() ? plane->height() : anaPy;
		anaPx = anaPx > plane->width() ? plane->width() : anaPx;

		switch ( type ) {
		case CORONAL:
			//emit mouseMoved(anaPy, plane->getPlane(), anaPx); break;
			UICONFIG->setDeltaTr( DATAMANAGER->image()->get( anaPy, plane->getPlane(), anaPx ) );
			break;
		case SAGITTAL:
			//emit mouseMoved(anaPy, anaPx, plane->getPlane()); break;
			UICONFIG->setDeltaTr( DATAMANAGER->image()->get( anaPy, anaPx, plane->getPlane() ) );
			break;
		case AXIAL:
			//emit mouseMoved(plane->getPlane(), anaPy, anaPx); break;
			UICONFIG->setDeltaTr( DATAMANAGER->image()->get( plane->getPlane(), anaPy, anaPx ) );
			break;
		}

		UICONFIG->setTranslate( false );
		moveView( anaPx, anaPy );
	}

	e->accept();

}


void vlGLWidget::commonMouseClick( QMouseEvent *e )
{

	// Die Mausposition in Weltkoordinaten
	float x, y;
	calculateWorldCoord( e->x(), e->y(), &x, &y );

	// Die Mausposition in Texturkoordinaten auf der Anatomietextur
	int anaPx, anaPy;
	calculateTexCoord( x, y, &anaPx, &anaPy );

	// Berechnungskorrekturen an den Rändern.
	anaPx = anaPx < 0 ? 0 : anaPx;
	anaPy = anaPy < 0 ? 0 : anaPy;
	anaPy = anaPy > plane->height() ? plane->height() : anaPy;
	anaPx = anaPx > plane->width() ? plane->width() : anaPx;

	switch ( type ) {
	case CORONAL:
		emit mouseMoved( anaPy, plane->getPlane(), anaPx );
		break;
	case SAGITTAL:
		emit mouseMoved( anaPy, anaPx, plane->getPlane() );
		break;
	case AXIAL:
		emit mouseMoved( plane->getPlane(), anaPy, anaPx );
		break;
	}

	if ( e->state()&RightButton ) {
		UICONFIG->setTranslate( true );
		moveView( anaPx, anaPy );
	}

	if ( e->state()&LeftButton ) {
		if ( m_delete & VIEW ) {
			UICONFIG->setTranslate( true );
			moveView( anaPx, anaPy );
		}

		if ( m_delete & MARK )
			markPart( anaPx, anaPy );

		if ( m_delete & DELETE )
			removePart( anaPx, anaPy );

	}

	e->accept();
}


void vlGLWidget::calculateWorldCoord( int mouseX, int mouseY, float *x, float *y )
{
	*x = mouseX + camera.x() - width() / 2;
	*y = mouseY - camera.y() - height() / 2;

}

void vlGLWidget::calculateTexCoord( float worldX, float worldY, int *x, int *y )
{
	//berechnen der Texturkoordinaten
	*x = ( int ) ( worldX / UICONFIG->scale() + plane->width() / 2 );
	*y = ( int ) ( worldY / UICONFIG->scale() + plane->height() / 2 );
}

void vlGLWidget::moveView( int px, int py )
{
	switch ( type ) {
	case CORONAL:
		UICONFIG->setAim( py, -1, px );
		break;
	case SAGITTAL:
		UICONFIG->setAim( py, px, -1 );
		break;
	case AXIAL:
		UICONFIG->setAim( -1, py, px );
		break;
	}
}

void vlGLWidget::removePart( int px, int py )
{
	//segment oder voxel l??schen?

	if ( m_delete & USE_DELTA )
		plane->deleteSegment( ( int ) px, ( int ) py, cursorSize, m_deltaGrey );
	else
		plane->deleteVoxel( ( int ) px, ( int ) py, cursorSize );

	// Selektionsmodus ?? Dann sollte auch das Selektionsfenster davon erfahren.
	if ( UICONFIG->getMode() == 1 ) {
		emit volumeChanged();
	}

	//ein kleiner Trick um die anderen Ebenen neu zu zeichnen
	emit recreateViews( true );
	reaim();
	repaint();
}

void vlGLWidget::recreateView( bool rebuildTex )
{
	makeCurrent();
	plane->createTexture( rebuildTex );
	reaim();
	repaint();
}

void vlGLWidget::mouseReleaseEvent( QMouseEvent *e )
{
	UICONFIG->mouseReleased();
	commonMouseClick( e );
}

void vlGLWidget::reset( )
{
	camera.setPos( 0.f, 0.f, 300.f );
	repaint();
}

void vlGLWidget::reaim( )
{
	makeCurrent();

	switch ( type ) {
	case CORONAL:
		aim.setX( ( int ) ( ( UICONFIG->column() - plane->width() / 2 )*UICONFIG->scale() ) );
		aim.setY( ( int ) ( ( UICONFIG->band() - plane->height() / 2 )*UICONFIG->scale() ) );
		plane->setPlane( ( int )UICONFIG->row() );
		break;
	case SAGITTAL:
		aim.setX( ( int ) ( ( UICONFIG->row() - plane->width() / 2 )*UICONFIG->scale() ) );
		aim.setY( ( int ) ( ( UICONFIG->band() - plane->height() / 2 )*UICONFIG->scale() ) );
		plane->setPlane( ( int )UICONFIG->column() );
		break;
	case AXIAL:
		aim.setX( ( int ) ( ( UICONFIG->column() - plane->width() / 2 )*UICONFIG->scale() ) );
		aim.setY( ( int ) ( ( UICONFIG->row() - plane->height() / 2 )*UICONFIG->scale() ) );
		plane->setPlane( ( int )UICONFIG->band() );
		break;
	}

	repaint();
}

void vlGLWidget::markPart( int px, int py )
{

	if( !DATAMANAGER->isValidSegment() ) {
		QMessageBox::warning( ( QWidget * )this, tr( "Error!" ), tr( "You didn't choose a valid segment. Segmentation will not be available" ), QMessageBox::Ok,  QMessageBox::NoButton,  QMessageBox::NoButton );
		qWarning( "markPart error: no valid segment selected!" );
		return;
	}

	if ( m_delete & USE_DELTA )
		plane->markSegment( px, py );
	else
		plane->markVoxel( px, py );

	// Selektionsmodus ?? Dann sollte auch das Selektionsfenster davon erfahren.
	if ( UICONFIG->getMode() == 1 ) {
		emit volumeChanged();
	}

	emit recreateViews( false );
	reaim();
	repaint();
}

