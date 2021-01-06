// kitprofileinfo.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include <qhbox.h>
#include <qmessagebox.h>
#include <kapp.h>
#include <kglobal.h>
#include <ksimpleconfig.h>
#include <kbuttonbox.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kitprofileinfo.h"
#include "kituserprofile.h"
#include "kitapp.h"

// * public *
KitProfileInfo::KitProfileInfo(QWidget *parent, const char *name)
: KDialogBase(parent, name, true, i18n("User Profiles"), KDialogBase::Ok | KDialogBase::Cancel)
{
	QHBox *mainBox = new QHBox(this);
	setMainWidget(mainBox);

	profileList = new QListBox(mainBox);
	profileList->setMinimumWidth(fontMetrics().maxWidth() * 10);
	profileList->setMinimumHeight(fontMetrics().height() * 5);

	KButtonBox *buttonBox = new KButtonBox(mainBox, Qt::Vertical);
	
	useButton = buttonBox->addButton(i18n("Use"));
	setButton = buttonBox->addButton(i18n("Set Default"));
	addButton = buttonBox->addButton(i18n("Add"));
	deleteButton = buttonBox->addButton(i18n("Delete"));

	buttonBox->layout();

	connect(useButton, SIGNAL(clicked()), this, SLOT(useClicked()) );
	connect(setButton, SIGNAL(clicked()), this, SLOT(setClicked()) );
	connect(addButton, SIGNAL(clicked()), this, SLOT(addClicked()) );
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteClicked()) );
	
	retval = QString::null;

	updateList();
}
KitProfileInfo::~KitProfileInfo()
{
}
const QString &KitProfileInfo::profileSelected(void)
{
	return retval;
}

// * protected slots *
#define KPI_SANITY if(profileList->currentItem() == -1) {\
                     KMessageBox::error(0, i18n("Select a profile first."));\
                     return; }

void KitProfileInfo::useClicked(void)
{
	slotOk();
}
void KitProfileInfo::setClicked(void)
{
	KPI_SANITY
	KConfig &globalConfig = *KGlobal::config();
	globalConfig.setGroup("Profiles");
	globalConfig.writeEntry("Default Profile", profileList->currentText().latin1());
	globalConfig.sync();
}
void KitProfileInfo::addClicked(void)
{
	// offer to make a new account
	if(KMessageBox::warningYesNo(0, i18n("Would you like to go to AOL and create a new account?")) == KMessageBox::Yes)
		KApplication::kApplication()->invokeBrowser("http://aim.aol.com/aimnew/Aim/register.adp");
	// now go
	KLineEditDlg lineDialog(i18n("Enter a name for the profile."), QString::null, this);
	if(lineDialog.exec() == KLineEditDlg::Accepted)
	{
		KSimpleConfig config("kitprofilerc");
		const QStringList &groupList = config.groupList();
		QString name = lineDialog.text();
		if( 0 < groupList.contains(name) )
			KMessageBox::error(0, i18n("You cannot add two profiles with the same name."));
		else
		{
			config.setGroup(name);
			config.writeEntry("Locked", "Unlocked");
			config.sync();
			updateList();
			// edit the new profile
			KitUserProfile profile(name);
			profile.execEditor();
		}
	}
}
void KitProfileInfo::deleteClicked(void)
{
	KPI_SANITY
	QString name = profileList->currentText();
	QString message;
	message = i18n("Are you sure you want to remove profile \"%1\"?")
		.arg(name);
	if(KMessageBox::warningContinueCancel(0, message, QString::null, i18n("&Remove"))
		== KMessageBox::Continue)
	{
		KSimpleConfig config("kitprofilerc", false);
		config.deleteGroup(name);
		// unset as default if it's the default
		KConfig &globalConfig = *KGlobal::config();
		globalConfig.setGroup("Profiles");
		if(globalConfig.readEntry("Default Profile") == name)
		{
			globalConfig.writeEntry("Default Profile", "(null)");
			globalConfig.sync();
		}
	}
	updateList();
}
void KitProfileInfo::slotOk(void)
{
	// if there is just one profile set it as default
	if(profileList->count() == 1)
	{
		profileList->setCurrentItem(0);
		setClicked();
	}
	else
		KPI_SANITY
	retval = profileList->currentText();
	accept();
}
void KitProfileInfo::slotDefault(void)
{
	slotOk();
}
void KitProfileInfo::slotCancel(void)
{
	reject();
}
// * protected *
void KitProfileInfo::updateList(void)
{
	profileList->clear();

	KSimpleConfig config("kitprofilerc", true);
	const QStringList &groupList = config.groupList();

	for(unsigned i = 0; i < groupList.count(); i++)
	{
		if(groupList[i] != "<default>" && groupList[i].right(17) != " -- Away Messages")
			profileList->insertItem(groupList[i]);
	}

	bool enableButtons = (profileList->count() > 0);
	useButton->setEnabled(enableButtons);
	setButton->setEnabled(enableButtons); 
	deleteButton->setEnabled(enableButtons);
	actionButton( Ok )->setEnabled(enableButtons);

	if(!enableButtons)
	if(KMessageBox::warningYesNo(0, i18n("Would you like to create a profile?")) == KMessageBox::Yes)
		addClicked();
}

#include "kitprofileinfo.moc"
