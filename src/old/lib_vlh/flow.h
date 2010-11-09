/****************************************************************************
** $Id: flow.h 522 2003-06-26 09:31:01Z karstenm $
**
** Definition of simple flow layout for custom layout example
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef FLOW_H
#define FLOW_H

#include <qlayout.h>
#if (QT_VERSION >= 0x030000)
#include <qptrlist.h>
#else
#include <qlist.h>
#define QPtrList QList
#endif

class SimpleFlow : public QLayout
{
public:
    SimpleFlow( QWidget *parent, int border=0, int space=-1,
		const char *name=0 )
	: QLayout( parent, border, space, name ),
	cached_width(0) {}
    SimpleFlow( QLayout* parent, int space=-1, const char *name=0 )
	: QLayout( parent, space, name ),
	cached_width(0) {}
    SimpleFlow( int space=-1, const char *name=0 )
	: QLayout( space, name ),
	cached_width(0) {}

    ~SimpleFlow();

    void addItem( QLayoutItem *item);
    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutIterator iterator();
    QSizePolicy::ExpandData expanding() const;

protected:
    void setGeometry( const QRect& );

private:
    int doLayout( const QRect&, bool testonly = FALSE );
    QPtrList<QLayoutItem> list;
    int cached_width;
    int cached_hfw;

};

#endif
