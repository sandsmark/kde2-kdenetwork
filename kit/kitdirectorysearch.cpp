// kitdirectorysearch.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include "config.h"

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>

#include <kbuttonbox.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kitdirectorysearch.h"
#include "kitapp.h"

// * class KitDirectorySearch -- public *

KitDirectorySearch::KitDirectorySearch(const char *name)
	: KDialogBase(KDialogBase::Tabbed, name, Ok|Cancel|Help, Ok, 0, name)
{
#define PE_SETUPBOX(a) a->setMargin( marginHint() ); a->setSpacing( spacingHint() );

	QVBox *uDir = addVBoxPage(i18n("Directory Search")); PE_SETUPBOX(uDir);

	QHBox *uDirName = new QHBox(uDir); PE_SETUPBOX(uDirName);
	new QLabel(i18n("First Name"), uDirName);
	uDirFirstName = new KLineEdit(uDirName);
	new QLabel(i18n("Middle Name"), uDirName);
	uDirMiddleName = new KLineEdit(uDirName);
	new QLabel(i18n("Last Name"), uDirName);
	uDirLastName = new KLineEdit(uDirName);

	QHBox *uDirMName = new QHBox(uDir); PE_SETUPBOX(uDirMName);
	new QLabel(i18n("Maiden Name"), uDirMName);
	uDirMaidenName = new KLineEdit(uDirMName);

	QHBox *uDirLocation = new QHBox(uDir); PE_SETUPBOX(uDirLocation);
	new QLabel(i18n("City"), uDirLocation);
	uDirCity = new KLineEdit(uDirLocation);
	new QLabel(i18n("State"), uDirLocation);
	uDirState = new KLineEdit(uDirLocation);
	new QLabel(i18n("Country"), uDirLocation);
	uDirCountry = new KLineEdit(uDirLocation);

	QHBox *uDirBEmail = new QHBox(uDir); PE_SETUPBOX(uDirBEmail);
	new QLabel(i18n("Email Address"), uDirBEmail);
	uDirEmail = new KLineEdit(uDirBEmail);

#undef PE_SETUPBOX
}
KitDirectorySearch::~KitDirectorySearch()
{
}
// * protected slots *
void KitDirectorySearch::accept(void)
{
	dir.firstName = uDirFirstName->text();
	dir.middleName = uDirMiddleName->text();
	dir.lastName = uDirLastName->text();
	dir.maidenName = uDirMaidenName->text();
	dir.city = uDirCity->text();
	dir.state = uDirState->text();
	dir.country = uDirCountry->text();
	dir.email = uDirEmail->text();

	// close dialog, return accepted
	KDialogBase::accept();
}
void KitDirectorySearch::reject(void)
{
	// Nothing needs to be done
	KDialogBase::reject();
}

#include "kitdirectorysearch.moc"
