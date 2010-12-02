/****************************************************************************
** Form interface generated from reading ui file 'aboutdialog.ui'
**
** Created: Wed Sep 12 15:53:57 2007
**      by: The User Interface Compiler ($Id: aboutdialog.h 2400 2007-09-12 13:54:51Z karstenm $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QLabel;
class QTabWidget;
class QWidget;
class QTextEdit;

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	AboutDialog( QWidget *parent = 0, const char *name = 0, bool modal = FALSE, WFlags fl = 0 );
	~AboutDialog();

	QPushButton *closeButton;
	QLabel *titleText;
	QTabWidget *tabWidget;
	QWidget *about;
	QLabel *textLabel1;
	QLabel *aboutText;
	QWidget *author;
	QTextEdit *textEdit1;
	QWidget *license;
	QTextEdit *textEdit2;

public slots:
	virtual void closeButton_clicked();

protected:
	QGridLayout *AboutDialogLayout;
	QGridLayout *authorLayout;
	QGridLayout *licenseLayout;

protected slots:
	virtual void languageChange();

};

#endif // ABOUTDIALOG_H
