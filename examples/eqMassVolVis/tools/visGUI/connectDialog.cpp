
/* Copyright (c) 2011, Fatih Erol
 *
 */


#include "connectDialog.h"

#include <volVis/EQ/guiConnectionDefs.h> // VOL_VIS_GUI_DEFAULT_HOST, DEFAULT_PORT

#include <QtGui/QFormLayout>
#include <QtGui/QDialogButtonBox>


namespace massVolGUI
{

ConnectDialog::ConnectDialog( QWidget *parent_ )    // to compensate for a bug in g++, 'parent' is renamed
    : QDialog( parent_ )
{
    QFormLayout *_formLayout = new QFormLayout( this );
    setModal( true );

    _hostLineEdit = new QLineEdit( this );
    _hostLineEdit->setText( QString::fromUtf8( VOL_VIS_GUI_DEFAULT_HOST ) );
    _formLayout->addRow( QString::fromUtf8( "Application Host:" ), _hostLineEdit );

    _portLineEdit = new QLineEdit( this );
    _portLineEdit->setText( QString( "%1" ).arg( VOL_VIS_GUI_DEFAULT_PORT ) );
    _formLayout->addRow( QString::fromUtf8( "Port:" ), _portLineEdit );


    QDialogButtonBox *buttons = new QDialogButtonBox( this );
    buttons->addButton( "Connect", QDialogButtonBox::AcceptRole );
    buttons->addButton( "Cancel", QDialogButtonBox::RejectRole );   
    QObject::connect( buttons, SIGNAL( accepted() ), this, SLOT( accept() ) );
    QObject::connect( buttons, SIGNAL( rejected() ), this, SLOT( reject() ) );
    _formLayout->addRow( buttons );


    setLayout( _formLayout );

    setWindowTitle( tr( "Connection to volVis" ));
}

} // namespace massVolGUI





