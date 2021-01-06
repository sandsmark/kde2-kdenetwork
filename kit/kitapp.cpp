// kitapp.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include "config.h"

#include <qstring.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kitapp.h"
#include "kitprofileinfo.h"
#include "kituserprofile.h"
#include "twindow.h"

// * static members *
int KitApp::numInstances = 0;

// * public *
KitApp::KitApp(bool a, bool b)
	: KUniqueApplication(a, b)
{
}
KitApp::~KitApp()
{
}
// IMPORTANT: this must be kept in line with TWindow::{read,save}Properties
int KitApp::newInstance(void)
{
	bool opened = false;
	if(isRestored())
		for(int n = 1; TWindow::canBeRestored(n); n++)
			if(TWindow::classNameOfToplevel(n) == "TWindow")
			{
				numInstances++;
				opened = true;
				TWindow *bangle = new TWindow;
				connect(bangle, SIGNAL(closing(void)), this, SLOT(windowClosing(void)) );
				bangle->restore(n);
			}
	if(!opened) popProfileInfo(true);
	return 0;
}
// IMPORTANT: this must be kept in line with TWindow::{read,save}Properties
void KitApp::popProfileInfo(bool useDefault)
{
	QString user = QString::null;
	KitUserProfile *profile = 0;

	numInstances++;

	if(useDefault)
	{
		KConfig &config = *KGlobal::config();
		config.setGroup("Profiles");
		user = config.readEntry(i18n("Default Profile"));
		if(user == "(null)") user = QString::null;
		profile = new KitUserProfile(user);
		if(!profile->valid())
		{
			if(!numInstances && KMessageBox::warningYesNo(0, i18n("%1 is locked.  Ignore the lock?").arg(user)) == KMessageBox::Yes)
			{
				profile->unlock();
				delete profile;
				profile = new KitUserProfile(user);
			}
			else
			{
				delete profile;
				user = QString::null;
			}
		}
	}
	if(!profile || user == QString::null)
	{
		KitProfileInfo picker;
		if(!picker.exec())
		{
			windowClosing();
			return;
		}
		user = picker.profileSelected();
		profile = new KitUserProfile(user);
	}
	if(!profile->valid())
	{
		if(KMessageBox::warningYesNo(0, i18n("%1 is locked.  Ignore the lock?").arg(user)) == KMessageBox::Yes)
		{
			profile->unlock();
			delete profile;
			profile = new KitUserProfile(user);
		}
		else
		{
			windowClosing();
			return;
		}
	}
	TWindow *window = new TWindow(profile);
	connect(window, SIGNAL(closing(void)), this, SLOT(windowClosing(void)) );
	window->show();
}
// * protected slots *
void KitApp::windowClosing(void)
{
	numInstances--;
	if(!numInstances) exit(0);
}

#include "kitapp.moc"
