
/* Copyright (c) 2011, Fatih Erol
 *
 */


#ifndef MASS_VOL__GUI_CONNECT_DIALOG_H
#define MASS_VOL__GUI_CONNECT_DIALOG_H


#include <QtGui/QDialog>
#include <QtGui/QLineEdit>


namespace massVolGUI
{

/** Dialog to connect to an application */
class ConnectDialog : public QDialog
{
public:

    ConnectDialog( QWidget *parent = NULL );

    QString getHost() const { return _hostLineEdit->text(); }
    QString getPort() const { return _portLineEdit->text(); }

private:

    QLineEdit *_hostLineEdit;
    QLineEdit *_portLineEdit;
};


}// namespace massVolGUI

#endif  //  MASS_VOL__GUI_CONNECT_DIALOG_H






