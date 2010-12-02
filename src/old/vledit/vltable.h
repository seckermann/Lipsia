#ifndef VLTABLE_H_
#define VLTABLE_H_

#include <qtable.h>
#include <qevent.h>

class vlTable : public QTable
{
	Q_OBJECT
protected:
	virtual void mouseReleaseEvent( QMouseEvent *e );

public:
	vlTable( QWidget *parent = 0, const char *name = 0 );

signals:
	// Dieses Signal wird gesendet wenn eine neue
	// Reihe selektiert wurde.
	void rowSelectionChanged( int );

	void clicked();

};

#endif // VLTABLE_H_
