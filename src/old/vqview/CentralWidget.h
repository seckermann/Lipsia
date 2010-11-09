#ifndef CENTRALWIDGET_H_
#define CENTRALWIDGET_H_

#include <qwidget.h>

#include "ImageWidget.h"
#include "Configurator.h"

class CentralWidget : public QWidget
{
	Q_OBJECT
public:
	CentralWidget (Configurator* conf, QWidget * parent = 0, const char * name = 0, WFlags f = 0 ) ;
	virtual ~CentralWidget();

protected:
	void leaveEvent(QEvent *);
	void enterEvent(QEvent *);
	
public slots:
	void positionChanged(ImageWidget* img, int x, int y);
	void crossPositionChanged(ImageWidget* img, int x, int y);
	void updateImageData();
	
private:
	void createWidget();
	
	
	ImageWidget *m_coronar;
	ImageWidget *m_sagittal;
	ImageWidget *m_axial;
	
	Configurator* m_configurator;
};

#endif /*CENTRALWIDGET_H_*/
