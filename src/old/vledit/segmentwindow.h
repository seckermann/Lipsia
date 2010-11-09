/****************************************************************************
** Form interface generated from reading ui file 'segmentwindow.ui'
**
** Created: Wed Sep 12 15:53:57 2007
**      by: The User Interface Compiler ($Id: segmentwindow.h 2400 2007-09-12 13:54:51Z karstenm $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef SEGMENTWINDOW_H
#define SEGMENTWINDOW_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qmainwindow.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QTabWidget;
class QWidget;

class SegmentWindow : public QMainWindow
{
    Q_OBJECT

public:
    SegmentWindow( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~SegmentWindow();

    QTabWidget* segmentTabs;
    QWidget* volume;
    QWidget* color;
    QToolBar *toolBar;
    QAction* fileNewAction;
    QAction* fileOpenAction;
    QAction* fileSaveVisibleAction;
    QAction* editCutAction;
    QAction* editCopyAction;
    QAction* fileSaveAllAction;
    QAction* segmentUpAction;
    QAction* segmentDownAction;

public slots:
    virtual void fileOpen();
    virtual void fileNew();
    virtual void editCut();
    virtual void editCopy();
    virtual void volumeTable_clicked( int, int, int, const QPoint & );
    virtual void colorTable_clicked( int, int, int, const QPoint & );
    virtual void colorTable_valueChanged( int, int );
    virtual void fileSaveAllAction_activated();
    virtual void segmentDownAction_activated();
    virtual void segmentUpAction_activated();
    virtual void fileSaveVisibleAction_activated();

protected:
    QGridLayout* SegmentWindowLayout;
    QGridLayout* volumeLayout;
    QGridLayout* colorLayout;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;
    QPixmap image2;
    QPixmap image3;
    QPixmap image4;
    QPixmap image5;
    QPixmap image6;
    QPixmap image7;

};

#endif // SEGMENTWINDOW_H
