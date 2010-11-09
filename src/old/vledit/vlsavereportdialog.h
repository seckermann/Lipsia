#ifndef VLSAVEREPORTDIALOG_H
#define VLSAVEREPORTDIALOG_H

#include <qdialog.h>
#include <qobject.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qtextedit.h>


class vlSaveReportDialog : public QDialog 
{

    Q_OBJECT

protected:

        /** The 'Ok' button */
        QPushButton *okButton;
        /** the 'Cancel' button */
        QPushButton *cancelButton;
        /** The formated text area where the save report appears */
        QTextEdit *textArea;

public:
        vlSaveReportDialog(QWidget *parent);
        ~vlSaveReportDialog();

        /**
         * The overloaded exec() function from QDialog. Along the standard
         * functionality, it creates the formated text from the current data
         * which is shown in this dialog window.
         *
         * \param saveAll this flag indicates, that all segments, visible or not,
         * will be saved.
         * \return a DialogCode result.
         * \see QDialog::DialogCode
         */
        int exec(bool saveAll);
};

#endif
