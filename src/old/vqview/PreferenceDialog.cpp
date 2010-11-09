//
// C++ Implementation: PreferenceDialog
//
// Description: 
//
//
// Author: Hannes Niederhausen <niederhausen@cbs.mpg.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "PreferenceDialog.h"


#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>


PreferenceDialog::PreferenceDialog(Configurator *conf)
	: QTabDialog(0, "Preference Dialog", FALSE, WType_TopLevel)
{
	m_configurator = conf;
	resize(320, 300);
	createControl();
}


PreferenceDialog::~PreferenceDialog() {
}

void PreferenceDialog::createLightingPart(QWidget* parent) {
	QGroupBox* widget = new QGroupBox("Lighting", parent);
	widget->setColumns(2);
	widget->setOrientation(Qt::Horizontal);
	
	new QLabel("Brightness:", widget);
	widget->addSpace(1);
		
	m_brightnessValue = new QLabel(widget);
	m_brightnessValue->setText(" 000");
	
	m_brightnessSlider = new QSlider(Qt::Horizontal, widget, "brightness slider");
	m_brightnessSlider->setMinValue(-100);
	m_brightnessSlider->setMaxValue(100);
	m_brightnessSlider->setValue(0);
	
	new QLabel("Contrast:", widget);
	widget->addSpace(1);
		
	m_contrastValue = new QLabel(widget);
	m_contrastValue->setText(" 000");
	
	m_contrastSlider = new QSlider(Qt::Horizontal, widget, "contrast slider");
	m_contrastSlider->setMinValue(-100);
	m_contrastSlider->setMaxValue(100);
	m_contrastSlider->setValue(0);
	
	connect(m_brightnessSlider, SIGNAL( valueChanged ( int )),
			this, SLOT(brightnessChanged( int )));
	connect(m_contrastSlider, SIGNAL(  valueChanged ( int )),
			this, SLOT(contrastChanged( int )));
}

void PreferenceDialog::createTimePart(QWidget* parent) {
	QGroupBox* widget = new QGroupBox("Animation", parent);
	widget->setColumns(2);
	widget->setOrientation(Qt::Horizontal);
	
	new QLabel("Speed:", widget);
	widget->addSpace(1);
	
	m_frameDurationValue = new QLabel(widget);
 	m_frameDurationValue->setText("04");
	
	m_frameDurationSlider = new QSlider(Qt::Horizontal, widget, "frameDurationSlider");
	m_frameDurationSlider->setMinValue(1);
	m_frameDurationSlider->setMaxValue(50);
	m_frameDurationSlider->setValue(m_configurator->fps());
	frameDurationChanged(m_frameDurationSlider->value());
	
	
	connect(m_frameDurationSlider, SIGNAL(  valueChanged ( int )),
			this, SLOT(frameDurationChanged( int )));
	
	
}

void PreferenceDialog::frameDurationChanged(int val) {
	int tmpVal = (val<0) ? -val : val;
	QString valString = QString::number(tmpVal);
	if (valString.length()!=2) {
		QString tmp;
		valString = tmp.fill('0', 2-valString.length()) + valString;
	}
	if (val<0)
		valString = "-" + valString;
	else 
		valString = " " + valString;
	m_frameDurationValue->setText(valString);
	m_configurator->setFps(val);
}

void PreferenceDialog::brightnessChanged(int val) {
	int tmpVal = (val<0) ? -val : val;
	
	QString valString = QString::number(tmpVal);
	if (valString.length()!=3) {
		QString tmp;
		valString = tmp.fill('0', 3-valString.length()) + valString;
	}
	if (val<0)
		valString = "-" + valString;
	else 
		valString = " " + valString;
	
	m_brightnessValue->setText(valString);

	m_configurator->setBrightness(val);

}

void PreferenceDialog::contrastChanged(int val) {
	int tmpVal = (val<0) ? -val : val;
	QString valString = QString::number(tmpVal);
	if (valString.length()!=3) {
		QString tmp;
		valString = tmp.fill('0', 3-valString.length()) + valString;
	}
	if (val<0)
		valString = "-" + valString;
	else 
		valString = " " + valString;
	
	m_contrastValue->setText(valString);
	
	
	m_configurator->setContrast(val);
	
}

void PreferenceDialog::reset() {
	m_brightnessSlider->setValue(0);
	m_contrastSlider->setValue(0);
	m_frameDurationSlider->setValue(25);
}

void PreferenceDialog::createControl() {
	QWidget* widget = new QWidget(this);
	QVBoxLayout* l = new QVBoxLayout(widget);
	l->setAutoAdd(true);
	createTimePart(widget);
	createLightingPart(widget);
	addTab(widget, "Settings");
	
	
	setDefaultButton("&Reset");
	connect (this, SIGNAL(defaultButtonPressed()),
			 this, SLOT(reset()));
}


