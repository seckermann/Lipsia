#ifndef ATTRWIN_H
#define ATTRWIN_H

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qdialog.h>
#include <qwidget.h>
#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qlayout.h>

class BildWin : public QMainWindow
{
    Q_OBJECT
public:
    BildWin( QWidget *parent=0, const char *name=0, QString cp=0 );
    
};

#endif // ATTRWIN_H
