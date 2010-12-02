#ifndef RAWPAINT_H
#define RAWPAINT_H

#include <viaio/VImage.h>

#include <qwidget.h>
#include <qpainter.h>
#include <qimage.h>
#include <qpixmap.h>
#include "prefs.h"

class RawPaint : public QWidget
{
	Q_OBJECT
public:
	RawPaint( QWidget *parent = 0, const char *name = 0, prefs *pr_ = 0, int i = 0, float *wert = 0, int pi = 0, float *powerwert = 0, float *fitres = 0, float *se = 0, double trpre = 0, int *length = 0, float *model = 0, QStringList fname = 0, float *conditions = 0, float *Sess = 0, int bline = 0, float **evres = 0, float **evresSE = 0, int length_ER = 0, int width_ER = 0, int cnd = 0 );
	QStringList fname_m;
	int posit, *cline, obabst, bline_m, raster;
	int persip, bolresp, firespp, tilinep, fitevoboolp;
	float *seplusneu, *seminusneu, *fitresneu;
	double *mittelfitt, *mittelseminus, *mittelseplus;
	QPixmap cpm;
	QImage pic;
	QColor *bgfarbemidd;

public slots:
	void position( int pos = 0 );
	void saveTrialAverageTC();
	void saveTrialAverageFR();
	void saveTrialAverage( float **evres = NULL );

signals:
	void neuZoom( int zoom );

protected:
	void paintEvent( QPaintEvent * );
	void mousePressEvent( QMouseEvent *e );

private:
	prefs *pr;
	int i_m;
	float *wert_m;
	int pi_m, zoom, *maxorder_m, maxfarben, session, *onslengths;
	float *powerwert_m, *fitres_m, *se_m;
	double trpre_m;
	int *length_m;
	float mean, mean2, *model_m;
	float *conditions1m, *AAA, *TYPE, *N1, *N2;
	float *wertneu, *Sess_m;
	float **evres_m, * *evresSE_m;
	int lER, wER, cnd_m;
	double *condptr, *condi, *durat, *onsptr, *durptr;
	double *mittelwert, *qmittelwert, min_m, max_m, eplength;
	QColor *bgfarbe, *bgfarbedark;
};


#endif // RAWPAINT_H
