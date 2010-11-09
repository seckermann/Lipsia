#ifndef VLVISIBLEBOXTABLEITEM_H_
#define VLVISIBLEBOXTABLEITEM_H_

#include <qobject.h>
#include <qtable.h>
#include <qwidget.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qpixmap.h>

class vlVisibleBoxTableItem : public QObject, public QTableItem {
    
    Q_OBJECT
    
private:

    QPushButton* m_pbutton;
    bool m_visible;    
    QPixmap m_eye_image;

public:

    vlVisibleBoxTableItem(QTable *);
    ~vlVisibleBoxTableItem();
    QWidget* createEditor() const;
    void setContentFromEditor(QWidget*);
    void setText(const QString &);
    
public slots:

    void buttonClicked();
    
signals:
    
    void visibilityChanged(bool, int);
    
};

#endif /*VLVISIBLEBOXTABLEITEM_H_*/
