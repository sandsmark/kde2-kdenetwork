/**********************************************************************

	--- Qt Architect generated file ---

	File: welcomeWinData.cpp
	Last generated: Sun Oct 25 00:01:05 1998

	DO NOT EDIT!!!  This file will be automatically
	regenerated by qtarch.  All changes will be lost.

 *********************************************************************/

#include "welcomeWinData.h"
#include <klocale.h>
#include "config.h"

#undef Inherited
#define Inherited QDialog


welcomeWinData::welcomeWinData
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name, TRUE, 643072 )
{
	DisplayDialog = new KSircListBox( this, "Dispay dialog" );
	DisplayDialog->setGeometry( 10, 10, 580, 490 );
	DisplayDialog->setMinimumSize( 10, 10 );
	DisplayDialog->setMaximumSize( 32767, 32767 );

	ShowAgain = new QCheckBox( this, "Show again checkbox" );
	ShowAgain->setGeometry( 10, 510, 100, 30 );
	ShowAgain->setMinimumSize( 10, 10 );
	ShowAgain->setMaximumSize( 32767, 32767 );
	ShowAgain->setText( i18n("Show Again") );
	ShowAgain->setAutoRepeat( FALSE );
	ShowAgain->setAutoResize( FALSE );

	But_dismiss = new QPushButton( this, "Dismiss Button" );
	But_dismiss->setGeometry( 480, 510, 110, 30 );
	But_dismiss->setMinimumSize( 10, 10 );
	But_dismiss->setMaximumSize( 32767, 32767 );
	connect( But_dismiss, SIGNAL(clicked()), SLOT(dismiss()) );
	But_dismiss->setText( i18n("Dismiss") );
	But_dismiss->setAutoRepeat( FALSE );
	But_dismiss->setAutoResize( FALSE );

	resize( 600,550 );
	setMinimumSize( 600, 550 );
	setMaximumSize( 600, 550 );
}


welcomeWinData::~welcomeWinData()
{
}
void welcomeWinData::dismiss()
{
}
#include "welcomeWinData.moc"

