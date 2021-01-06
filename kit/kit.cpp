// kit.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include "version.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "kitapp.h"

// **************************************************
// * function main... in case I ever want it to run *
// **************************************************
int main(int argc, char **argv)
{
	// about data
	KAboutData aboutData("kit", I18N_NOOP("Kit"), KIT_VER_NUM,
	                     I18N_NOOP("AOL Instant Messenger Client"), KAboutData::License_LGPL, 0,
	                     "http://kitclient.sourceforge.net/", "neil@qualityassistant.com");
	aboutData.addAuthor("Neil Stevens", I18N_NOOP("Original author"), "neil@qualityassistant.com");
	aboutData.addCredit("Benjamin Meyer", I18N_NOOP("Password Changing"));

	// command line
	KCmdLineArgs::init(argc, argv, &aboutData);

	// let's make an application
	if(!KitApp::start()) return 0;
	KitApp app;
	return app.exec();
}
