#include <vlsavereportdialog.h>
#include <qlayout.h>
#include <vector>
#include "datamanager.h"
#include "uiconfig.h"

using namespace std;

vlSaveReportDialog::vlSaveReportDialog( QWidget *parent )
	: QDialog( parent, "Summary", true )
{
	// create widgets

	// 2 rows, 2 cols, Border = 10
	QGridLayout *glayout = new QGridLayout( this, 2, 2, 10 );
	textArea = new QTextEdit( this );
	textArea->setTextFormat( RichText );
	okButton = new QPushButton( this, "OkButton" );
	okButton->setSizePolicy( QSizePolicy(
								 ( QSizePolicy::SizeType )0,
								 ( QSizePolicy::SizeType )0,
								 0, 0,
								 okButton->sizePolicy().hasHeightForWidth() ) );
	okButton->setText( "&Ok" );
	cancelButton = new QPushButton( this, "CancelButton" );
	cancelButton->setSizePolicy( QSizePolicy(
									 ( QSizePolicy::SizeType )0,
									 ( QSizePolicy::SizeType )0,
									 0, 0,
									 cancelButton->sizePolicy().hasHeightForWidth() ) );
	cancelButton->setText( "&Cancel" );

	glayout->addMultiCellWidget( textArea, 0, 0, 0, 1 );
	glayout->addWidget( okButton, 1, 0, Qt::AlignRight );
	glayout->addWidget( cancelButton, 1, 1, Qt::AlignLeft );

	resize( 300, 400 );
}

vlSaveReportDialog::~vlSaveReportDialog() {}

int vlSaveReportDialog::exec( bool saveAll )
{

	// list of integer names from all empty segments.
	vector<int> emptySegs;
	// list of integer names from all invisible segments.
	vector<int> invisibleSegs;
	// create copy of segment id list
	vector<int> segList = *UICONFIG->segList();
	vector<int>::iterator iter;

	// To stabilise any iterator after a delete operation the
	// STL-documentation suggests to create a vector of static size.
	segList.reserve( segList.capacity() + 1 );

	iter = segList.begin();

	// iterate over id list and remove all ids from empty segments.
	while( iter != segList.end() ) {
		if( DATAMANAGER->segment( *iter )->getVolume() == 0.0 ) {
			emptySegs.push_back( DATAMANAGER->segment( *iter )->name );
			segList.erase( iter );
		} else {
			iter++;
		}
	}

	// iterate over id list and remove invisible segments, if necessary
	if( !saveAll ) {
		iter = segList.begin();

		while( iter != segList.end() ) {
			if( !DATAMANAGER->segment( *iter )->visible ) {
				invisibleSegs.push_back(
					DATAMANAGER->segment( *iter )->name );
				segList.erase( iter );
			} else {
				iter++;
			}
		}
	}

	// create message string
	QString text;
	// some flags
	bool emptySegments = false;
	bool invSegments = false;
	bool saveSegments = false;
	bool duplicate = false;

	text +=  "<p><b>empty segments:</b>\n<ul>";
	iter = emptySegs.begin();

	while( iter != emptySegs.end() ) {
		text += "\n<li>Segment " +
				QString::number( DATAMANAGER->segment( *iter )->name ) + "</li>";
		iter++;
		emptySegments = true;
	}

	if( !emptySegments )
		text += "\n<li><font color=\"green\">None</font></li>";

	text += "\n</ul>\n</p>";

	if( !saveAll ) {
		text += "<p><b>Iinvisible segments:</b>\n<ul>";
		iter = invisibleSegs.begin();

		while( iter != invisibleSegs.end() ) {
			text += "\n<li> Segment" +
					QString::number( DATAMANAGER->segment( *iter )->name ) + "</li>";
			iter++;
			invSegments = true;
		}

		if( !invSegments )
			text += "\n<li><font color=\"green\">None</font></li>";

		text  += "\n</ul>\n</p>";
	}

	text += "<p><b>segments that will be saved:</b>\n<ul>";
	iter = segList.begin();

	while( iter != segList.end() ) {
		if ( DATAMANAGER->findByName(
				 DATAMANAGER->segment( *iter )->name ) > 1 )  {
			text += "<\n><li> <font color = \"red\">Segment" +
					QString::number( DATAMANAGER->segment( *iter )->name )  +
					"</font></li>";
			duplicate = true;
		} else {
			text += "<\n><li> Segment" +
					QString::number( DATAMANAGER->segment( *iter )->name )  +
					"</li>";
		}

		saveSegments = true;
		iter++;
	}

	if( !saveSegments )
		text += "\n<li><font color=\"red\">None</font></li>";

	text += "\n</ul>\n</p>";

	// duplicate Entries warning
	if ( duplicate )
		text = "<p><b><font color=\"orange\">Warning:</font></b> \
        You are going to save segments with duplicate names. If your \
        target type is <b>UByte</b> (default!) then only the segment with the \
        highest priority will be saved. All other segments with the same \
        name will get lost.<br>To avoid namespace collisions you should \
        consider to save your segments to target type <b>Bit</b>. To change \
        the output type at the commandline use the parameter \
        -t [type] .</p>\n\n" + text;

	// connect buttons to correct slot.
	// if no segments to save -> reject
	if( segList.size() == 0 )
		connect( okButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
	// if segments to save -> accept
	else
		connect( okButton, SIGNAL( clicked() ), this, SLOT( accept() ) );

	// cancel -> reject
	connect( cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );

	textArea->setText( text );

	// call parent implementation to enter event loop
	return QDialog::exec();
}
