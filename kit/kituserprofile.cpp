// kituserprofile.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include "config.h"

#include <kglobal.h>
#include <ksimpleconfig.h>

#include "aim.h"
#include "kituserprofile.h"
#include "kitprofileeditor.h"
#include "kitawaypicker.h"

// **********************************
// * class KitUserProfile -- public *
// **********************************
KitUserProfile::KitUserProfile(const QString &user, QObject *parent, const char *name)
	: QObject(parent, name)
{
	userSection = user;
	KSimpleConfig config("kitprofilerc");

	config.setGroup(userSection);
	if( !(QString("Unlocked") == config.readEntry("Locked")) )
	{
		isValid = false;
		return;
	}
	config.setGroup(userSection);
	config.writeEntry("Locked", "Locked");

	isValid = true;
	load();
}
KitUserProfile::~KitUserProfile()
{
	if(isValid)
	{
		save();
		KSimpleConfig config("kitprofilerc");
		config.setGroup(userSection);
		config.writeEntry("Locked", "Unlocked");
	}
}
void KitUserProfile::unlock(void)
{
	KSimpleConfig config("kitprofilerc");
	config.setGroup(userSection);
	config.writeEntry("Locked", "Unlocked");
}
bool KitUserProfile::execEditor(void)
{
	ProfileEditor dlg(this);
	if(dlg.exec() == ProfileEditor::Accepted)
	{
		emit updated();
		return ProfileEditor::Accepted;
	}
	return ProfileEditor::Rejected;
}
QString KitUserProfile::execAwayPicker(void)
{
	AwayPicker dlg(this);
	if(dlg.exec() == AwayPicker::Accepted)
		return dlg.message();
	else
		return QString::null;
}

// ****************************************
// * class KitUserProfile -- public slots *
// ****************************************
void KitUserProfile::save(void)
{
	if(!isValid) return;
	KSimpleConfig config("kitprofilerc");

	config.setGroup(userSection);
	if( !myData.name.isNull())
		config.writeEntry("Screen Name", myData.name);
	if( !myData.password.isNull() && myData.savePassword)
		config.writeEntry("Password", myData.password);
	else
		config.writeEntry("Password", "");
	config.writeEntry("Save Password", myData.savePassword);

	config.writeEntry("Logging", myData.logging);
	config.writeEntry("Timestamping", myData.timestamping);
	config.writeEntry("List Columns", myData.listColumns);
	config.writeEntry("Window X", myData.x);
	config.writeEntry("Window Y", myData.y);
	config.writeEntry("Window Width", myData.width);
	config.writeEntry("Window Height", myData.height);

	config.writeEntry("Foreground Color", myData.foregroundColor);
	config.writeEntry("Background Color", myData.backgroundColor);
	config.writeEntry("Bold", myData.bold);
	config.writeEntry("Font Size", myData.fontSize);
	config.writeEntry("ICQ Mode", myData.icqMode);

	config.writeEntry("Keep Connection Alive", myData.keepAlive);
	config.writeEntry("Automatically Connect", myData.autoConnect);
	config.writeEntry("Use Custom Server", myData.useCustomServer);
	config.writeEntry("Custom Server", myData.server);
	config.writeEntry("Custom Server Port", myData.serverPort);
	config.writeEntry("Custom Authorizer", myData.authorizer);
	config.writeEntry("Custom Authorizer Port", myData.authorizerPort);

	config.writeEntry("Personal Information", myData.personalInformation);
	config.writeEntry("Directory First Name", myData.dir.firstName);
	config.writeEntry("Directory Middle Name", myData.dir.middleName);
	config.writeEntry("Directory Last Name", myData.dir.lastName);
	config.writeEntry("Directory Maiden Name", myData.dir.maidenName);
	config.writeEntry("Directory City", myData.dir.city);
	config.writeEntry("Directory State", myData.dir.state);
	config.writeEntry("Directory Country", myData.dir.country);
	config.writeEntry("Directory Email", myData.dir.email);
	config.writeEntry("Directory Allow Web Searches", myData.dir.allowWebSearches);

	QString conf;
	conf = tocWriteConfig( &(myData.names), &(myData.permit), &(myData.deny), myData.permitStatus );
	config.writeEntry("Configuration", "CONFIG:" + conf);

	config.setGroup(userSection + " -- Away Messages");
	for(unsigned i = 0; i < myData.awayMessageNames.count() && i < myData.awayMessages.count(); i++)
		config.writeEntry(myData.awayMessageNames[i], myData.awayMessages[i]);
	// now to sweep, and delete the ones that don't exist anymore
	if(config.hasGroup(userSection + " -- Away Messages"))
	{
		QMap<QString, QString> entryMap = config.entryMap(userSection + " -- Away Messages");
		QMap<QString, QString>::Iterator mapIterator;
		for(mapIterator = entryMap.begin(); mapIterator != entryMap.end(); mapIterator++)
		{
			if( myData.awayMessageNames.find(mapIterator.key()) == myData.awayMessageNames.end() )
				config.deleteEntry(mapIterator.key(), false);
		}
	}

	KSimpleConfig nickConfig("kitnicknamesrc");
	nickConfig.setGroup(userSection);
	for(QMap<QString, QString>::Iterator i = myData.nicknames.begin(); i != myData.nicknames.end(); i++)
		nickConfig.writeEntry(i.key(), i.data());
	// TODO: delete the ones that don't exist anymore?
}
void KitUserProfile::load(void)
{
	if(!isValid) return;
	KSimpleConfig config("kitprofilerc");

	config.setGroup(userSection);
	myData.name = config.readEntry("Screen Name");
	myData.password = config.readEntry("Password");
	myData.savePassword = config.readLongNumEntry("Save Password", false);

	myData.logging = config.readLongNumEntry("Logging", false);
	myData.timestamping = config.readLongNumEntry("Timestamping", true);
	myData.x = config.readLongNumEntry("Window X", -1);
	myData.y = config.readLongNumEntry("Window Y", -1);
	myData.width = config.readLongNumEntry("Window Width", -1);
	myData.height = config.readLongNumEntry("Window Height", -1);
	myData.listColumns = config.readLongNumEntry("List Columns", 31);

	myData.foregroundColor = config.readColorEntry("Foreground Color", &black);
	myData.backgroundColor = config.readColorEntry("Background Color", &white);
	myData.bold = config.readBoolEntry("Bold", false);
	myData.fontSize = config.readLongNumEntry("Font Size", 0);
	myData.icqMode = config.readBoolEntry("ICQ Mode", 0);

	myData.keepAlive = config.readLongNumEntry("Keep Connection Alive", true);
	myData.autoConnect = config.readBoolEntry("Automatically Connect", false);
	myData.useCustomServer = config.readLongNumEntry("Use Custom Server", false);
	myData.server = config.readEntry("Custom Server");
	myData.serverPort = config.readLongNumEntry("Custom Server Port", 0);
	myData.authorizer = config.readEntry("Custom Authorizer");
	myData.authorizerPort = config.readLongNumEntry("Custom Authorizer Port", 0);

	myData.personalInformation = config.readEntry("Personal Information");
	myData.dir.firstName = config.readEntry("Directory First Name");
	myData.dir.middleName = config.readEntry("Directory Middle Name");
	myData.dir.lastName = config.readEntry("Directory Last Name");
	myData.dir.maidenName = config.readEntry("Directory Maiden Name");
	myData.dir.city = config.readEntry("Directory City");
	myData.dir.state = config.readEntry("Directory State");
	myData.dir.country = config.readEntry("Directory Country");
	myData.dir.email = config.readEntry("Directory Email");
	myData.dir.allowWebSearches = config.readBoolEntry("Directory Allow Web Searches", false);
	
	QString conf;
	if( config.hasKey("Configuration"))
	{
		conf = config.readEntry( QString("Configuration") );
		tocParseConfig(conf, &(myData.names), &(myData.permit), &(myData.deny), &(myData.permitStatus));
	}
	else
	{
		myData.permitStatus = TOC_PERMITALL;
		myData.names.reset();
		myData.permit.reset();
		myData.deny.reset();
	}
	if(config.hasGroup(userSection + " -- Away Messages"))
	{
		QMap<QString, QString> entryMap = config.entryMap(userSection + " -- Away Messages");
		QMap<QString, QString>::Iterator mapIterator;
		for(mapIterator = entryMap.begin(); mapIterator != entryMap.end(); mapIterator++)
		{
			myData.awayMessageNames += mapIterator.key();
			if(mapIterator.data() == QString::null)
				myData.awayMessages += QString(" ");
			else
				myData.awayMessages += mapIterator.data();
		}
	}

	KSimpleConfig nickConfig("kitnicknamesrc");
	nickConfig.setGroup(userSection);
	QMap<QString, QString> entryMap = nickConfig.entryMap(userSection);
	for(QMap<QString, QString>::Iterator i = entryMap.begin(); i != entryMap.end(); i++)
		myData.nicknames.insert(i.key(), i.data());
}

#include "kituserprofile.moc"
