#ifndef PLOTDIALOG_H
#define PLOTDIALOG_H

#include <qmainwindow.h>
#include <qaction.h>

#include "Configurator.h"
#include "plotwidget.h"

/**
	@author Hannes Niederhausen <niederhausen@cbs.mpg.de>
*/
class PlotDialog : public QMainWindow {
	Q_OBJECT
public:
	PlotDialog(Configurator* conf, QWidget* parent = 0, const char *name="Help Window");

    ~PlotDialog();

public slots:
	void drawGrid(bool);
	void setSync(bool);	
	
private:
	
	void createWidget();
	void createStatusBar();
	void createToolBar();
	void createActions();
	
	Configurator* m_conf;
	
	PlotWidget* m_plotWidget;
	QAction *m_drawGridAction;
	QAction *m_syncAction;
};

#endif
