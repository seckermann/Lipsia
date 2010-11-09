#ifndef VLCOLORBOXTABLEITEM_H_
#define VLCOLORBOXTABLEITEM_H_

#include <qtable.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <qcolor.h>
#include <qobject.h>

#define PIXMAP_WIDTH 80
#define PIXMAP_HEIGHT 18

class vlColorBoxTableItem : public QObject, public QTableItem 
{
	Q_OBJECT

private:
	// The push button to open the QColorChooserDialog
	QPushButton* m_pbutton;
	// the currently selected color
	QColor m_color;
	
	// refresh the button and fill it with the current color value.
	void refreshButton();
	
public:
	vlColorBoxTableItem(QTable*, QColor&);
	QWidget* createEditor() const;
	void setContentFromEditor(QWidget*);
	void setText(const QString &);
	
	// access methods
	QPushButton* colorButton(){ return m_pbutton; }

public slots:

	// this method is called when the color button was clicked	
	void buttonClicked();
	
signals:

	// this signal is emitted when the color value was has been changed.
	void colorChanged(int);	
	
};

#endif /*VLCOLORBOXTABLEITEM_H_*/
