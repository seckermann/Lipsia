#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <qtabdialog.h>
#include <qslider.h>
#include <qlabel.h>


#include "Configurator.h"

/**
	@author Hannes Niederhausen <niederhausen@cbs.mpg.de>
*/
class PreferenceDialog : public QTabDialog
{
	Q_OBJECT
public:
	PreferenceDialog(Configurator *conf);

    ~PreferenceDialog();

public slots:
	void brightnessChanged(int);
	void contrastChanged(int);
	void frameDurationChanged(int);
	void reset();
	
	
private:
	
	void createLightingPart(QWidget*);
	void createTimePart(QWidget*);
	void createControl();
	
	QSlider *m_frameDurationSlider;
	QLabel *m_frameDurationValue;
	
	QSlider *m_brightnessSlider;
	QLabel *m_brightnessValue;
	
	QSlider *m_contrastSlider;
	QLabel *m_contrastValue;
	
	Configurator *m_configurator;	
};

#endif
