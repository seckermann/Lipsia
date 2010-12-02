/****************************************************************************
** Form implementation generated from reading ui file 'aboutdialog.ui'
**
** Created: Wed Sep 12 15:54:19 2007
**      by: The User Interface Compiler ($Id: aboutdialog.cpp 2400 2007-09-12 13:54:51Z karstenm $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "aboutdialog.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qtextedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "aboutdialog.ui.h"

/*
 *  Constructs a AboutDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AboutDialog::AboutDialog( QWidget *parent, const char *name, bool modal, WFlags fl )
	: QDialog( parent, name, modal, fl )
{
	if ( !name )
		setName( "AboutDialog" );

	setSizeGripEnabled( FALSE );
	setModal( TRUE );
	AboutDialogLayout = new QGridLayout( this, 1, 1, 11, 6, "AboutDialogLayout" );

	closeButton = new QPushButton( this, "closeButton" );
	closeButton->setSizePolicy( QSizePolicy( ( QSizePolicy::SizeType )0, ( QSizePolicy::SizeType )0, 0, 0, closeButton->sizePolicy().hasHeightForWidth() ) );

	AboutDialogLayout->addWidget( closeButton, 2, 1 );

	titleText = new QLabel( this, "titleText" );

	AboutDialogLayout->addWidget( titleText, 0, 0 );

	tabWidget = new QTabWidget( this, "tabWidget" );

	about = new QWidget( tabWidget, "about" );

	textLabel1 = new QLabel( about, "textLabel1" );
	textLabel1->setGeometry( QRect( 138, -22, 230, 30 ) );

	aboutText = new QLabel( about, "aboutText" );
	aboutText->setGeometry( QRect( 20, 100, 370, 53 ) );
	tabWidget->insertTab( about, QString::fromLatin1( "" ) );

	author = new QWidget( tabWidget, "author" );
	authorLayout = new QGridLayout( author, 1, 1, 11, 6, "authorLayout" );

	textEdit1 = new QTextEdit( author, "textEdit1" );
	textEdit1->setEnabled( TRUE );
	textEdit1->setTextFormat( QTextEdit::RichText );
	textEdit1->setWordWrap( QTextEdit::WidgetWidth );
	textEdit1->setReadOnly( TRUE );
	textEdit1->setUndoRedoEnabled( FALSE );

	authorLayout->addWidget( textEdit1, 0, 0 );
	tabWidget->insertTab( author, QString::fromLatin1( "" ) );

	license = new QWidget( tabWidget, "license" );
	licenseLayout = new QGridLayout( license, 1, 1, 11, 6, "licenseLayout" );

	textEdit2 = new QTextEdit( license, "textEdit2" );
	textEdit2->setWordWrap( QTextEdit::WidgetWidth );

	licenseLayout->addWidget( textEdit2, 0, 0 );
	tabWidget->insertTab( license, QString::fromLatin1( "" ) );

	AboutDialogLayout->addMultiCellWidget( tabWidget, 1, 1, 0, 1 );
	languageChange();
	resize( QSize( 431, 453 ).expandedTo( minimumSizeHint() ) );
	clearWState( WState_Polished );

	// signals and slots connections
	connect( closeButton, SIGNAL( clicked() ), this, SLOT( close() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AboutDialog::~AboutDialog()
{
	// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AboutDialog::languageChange()
{
	setCaption( tr( "About vlEdit" ) );
	closeButton->setText( tr( "Close" ) );
	titleText->setText( tr( "<b>vlEdit (Version 0.1.1)</b>" ) );
	textLabel1->setText( tr( "textLabel1" ) );
	aboutText->setText( tr( "An editing tool for fMRI data.\n"
							"\n"
							"(c) 2005-2007 MPI of Cognitive Neuroscience, Leipzig" ) );
	tabWidget->changeTab( about, tr( "About" ) );
	textEdit1->setText( tr( "This software is developed at the working group for <i>Mathematical Methods in fMRI</i>.<br>\n"
							"If you find a bug please send a report to:\n"
							"<a href=lipsia@cbs.mpg.de>lipsia@cbs.mpg.de</a>" ) );
	tabWidget->changeTab( author, tr( "Author" ) );
	textEdit2->setText( tr( "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by  the Free Software Foundation; either version 2 of the License, or (at your option) any later version.                                   \n"
							"                                                                      \n"
							"This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.                          \n"
							"                                                                         \n"
							"You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA." ) );
	tabWidget->changeTab( license, tr( "License agreement" ) );
}

