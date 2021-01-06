// taim.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include "version.h"

#include <kapp.h>
#include <klocale.h>
#include <knotifyclient.h>
#include "taim.h"
#include "kitsocket.h"

// * class TAim - public *
TAim::TAim(QObject *parent, const char *name)
	: QObject(parent, name)
{
	status = TAIM_OFFLINE;
	socket = 0;
	username = QString::null;
	r_password = QString::null;
	userInfo = QString::null;

	serverConfig.permitStatus = 1;
	serverConfig.buddyList.reset();
	serverConfig.permitList.reset();
	serverConfig.denyList.reset();

	currentConfig.permitStatus = 1;
	currentConfig.buddyList.reset();
	currentConfig.permitList.reset();
	currentConfig.denyList.reset();

	keepAlive = false;
	usingCustomServer = false;
	
	updateIdleTime = false;
	isIdle = false;

	connect(&idleTimer, SIGNAL(timeout()), this, SLOT(updateIdleness()));
}
TAim::~TAim()
{
	emit endProgress();
	setStatus(TAIM_OFFLINE);
}
void TAim::setBuddyList(const TBuddyList &a)
{
	TBuddy buddy;
	TBuddyList newList = a;
	// reset all buddies
	for(int i = 0; i < newList.getCount(); i++)
	{
		newList.setStatus(i, TAIM_OFFLINE);
		newList.setUserClass(i, 0);
		newList.setSignonTime(i, 0);
		newList.setIdleTime(i, 0);
		newList.setEvil(i, 0);
	}
	// get the current values
	for(int i = 0; i < currentConfig.buddyList.getCount(); i++)
	{
		int j = newList.getNum( currentConfig.buddyList.getName(i) );
		if(j != -1)
		{
			currentConfig.buddyList.get(&buddy, i);
			newList.setStatus(j, buddy.status);
			newList.setUserClass(j, buddy.userClass);
			newList.setSignonTime(j, buddy.signonTime);
			newList.setIdleTime(j, buddy.idleTime);
			newList.setEvil(j, buddy.evil);
		}
	}
	currentConfig.buddyList = newList;
	sendConfig();
	emit configChanged();
}
void TAim::setPermitList(const TBuddyList &a)
{
	if(a != currentConfig.permitList)
	{
		currentConfig.permitList = a;
		sendPermissions();
	}
}
void TAim::setDenyList(const TBuddyList &a)
{
	if(a != currentConfig.denyList)
	{
		currentConfig.denyList = a;
		sendPermissions();
	}
}
void TAim::setPermissions(int a)
{
	int newStatus = ( 1 <= a && 4 >= a ) ? a : 1;
	if(newStatus != currentConfig.permitStatus)
	{
		currentConfig.permitStatus = newStatus;
		sendPermissions();
	}
}
void TAim::doKeepAlive(int doit)
{
	keepAlive = doit ? true : false;
	if(socket != 0) socket->setKeepAlive(keepAlive);
}
void TAim::doIdleTime(bool doit)
{
	updateIdleTime = doit;
}
void TAim::useCustomServer(int use)
{
	usingCustomServer = use ? true : false;
}
void TAim::setCustomServer(const QString &s, int sp, const QString &a, int ap)
{
	server = s;
	serverPort = sp;
	authorizer = a;
	authorizerPort = ap;
}
void TAim::setServerBuddyList(const TBuddyList &a)
{
	serverConfig.buddyList = a;
	setConfig();
}
void TAim::setServerPermitList(const TBuddyList &a)
{
	serverConfig.permitList = a;
	setConfig();
}
void TAim::setServerDenyList(const TBuddyList &a)
{
	serverConfig.denyList = a;
	setConfig();
}
void TAim::setServerPermissions(int a)
{
	serverConfig.permitStatus = a;
	setConfig();
}

// * class TAim - public slots *
void TAim::setStatus(int newstatus)
{
	// this guards against a double delete
	if(newstatus == status) return;
	status = newstatus;
	if(socket)
	{
		delete socket;
		socket = 0;
	}
	if(status == TAIM_ONLINE) tocConnect();
	emit statusChanged(status);
}
void TAim::setUserName(const QString &a)
{
	if(a == username) return;
	if(status != TAIM_OFFLINE) setStatus(TAIM_OFFLINE);

	username = a;
	serverConfig.permitStatus = 1;
	serverConfig.buddyList.reset();
	serverConfig.permitList.reset();
	serverConfig.denyList.reset();
	
	emit userNameChanged();
}
void TAim::setPassword(const QString &a)
{
	r_password = tocRoast(a);
}
void TAim::changePassword(QString oldPassword, QString newPassword)
{
	if(!socket) return;
	QString data = QString("toc_change_passwd %1 %2").arg(tocProcess(oldPassword)).arg(tocProcess(newPassword));
	socket->writeData(data);
}
void TAim::setUserInfo(const QString &a)
{
	userInfo = a;
	sendUserInfo();
	endIdleness();
}
void TAim::requestUserInfo(const QString &name)
{
	if(!socket) return;
	QString data;
	data = QString::fromLatin1("toc_get_info %1").arg(tocNormalize(name));
	socket->writeData(data);
	endIdleness();
}
void TAim::sendIM(QString message, QString target, bool isAuto)
{
	if(!socket) return;
	QString data;
	data = QString::fromLatin1("toc_send_im %1 %2").arg(tocNormalize(target)).arg(tocProcess(message));
	if(isAuto) data += " auto";
	socket->writeData(data);
	endIdleness();
}
void TAim::sendWarning(QString target, bool isAnonymous)
{
	if(!socket) return;
	QString data;
	data.sprintf("toc_evil %s %s", tocNormalize(target).latin1(), isAnonymous ? "anon" : "norm");
	socket->writeData(data);
	endIdleness();
}
void TAim::requestDirectory(const QString &target)
{
	if(!socket) return;
	QString data = QString::fromLatin1("toc_get_dir %1").arg(tocNormalize(target));
	socket->writeData(data);
	endIdleness();
}
void TAim::setDirectory(const directory &dir)
{
	if(!socket) return;
	QString data = QString::fromLatin1("toc_set_dir %1:%2:%3:%4:%5:%6:%7:%8:");
	data.arg(tocProcess(dir.firstName));
	data.arg(tocProcess(dir.middleName));
	data.arg(tocProcess(dir.lastName));
	data.arg(tocProcess(dir.maidenName));
	data.arg(tocProcess(dir.city));
	data.arg(tocProcess(dir.state));
	data.arg(tocProcess(dir.country));
	data.arg(tocProcess(dir.email));
	if(dir.allowWebSearches) data += "T";
	socket->writeData(data);
	endIdleness();
}
void TAim::searchDirectory(const directory &dir)
{
	if(!socket) return;
	QString data = QString::fromLatin1("toc_dir_search %1:%2:%3:%4:%5:%6:%7:%8");
	data.arg(tocProcess(dir.firstName));
	data.arg(tocProcess(dir.middleName));
	data.arg(tocProcess(dir.lastName));
	data.arg(tocProcess(dir.maidenName));
	data.arg(tocProcess(dir.city));
	data.arg(tocProcess(dir.state));
	data.arg(tocProcess(dir.country));
	data.arg(tocProcess(dir.email));
	socket->writeData(data);
	endIdleness();
}

void TAim::cancelProgress(void)
{
	emit endProgress();
	setStatus(TAIM_OFFLINE);
}

// * class TAim - protected slots *
// *******************************************************************************
#define tocConnectError(msg2) { emit endProgress();\
                                emit displayError(QString(msg2));\
                                setStatus(TAIM_OFFLINE);\
                                return; }

void TAim::tocConnect(void)
{
	// don't bother if already connected
	if(socket) return;

	serverConfig.buddyList.reset();
	serverConfig.permitList.reset();
	serverConfig.denyList.reset();
	oldEvil = 0;
	lastListSent = QString::null;
	lastPermissionSent = -1;
	lastInfoSent = QString::null;
	lastConfigSent = QString::null;

	// reset all buddies -- they'll be refreshed on the UPDATE_BUDDYs after configChanged
	int i = -1;
	while( ++i < currentConfig.buddyList.getCount() )
	{
		currentConfig.buddyList.setStatus(i, TAIM_OFFLINE);
		currentConfig.buddyList.setUserClass(i, 0);
		currentConfig.buddyList.setSignonTime(i, 0);
		currentConfig.buddyList.setIdleTime(i, 0);
		currentConfig.buddyList.setEvil(i, 0);
	}

	if( username.isNull() )
		tocConnectError(i18n("No username specified."));

	if( r_password.isNull() )
		tocConnectError(i18n("No password specified."));

	emit initProgress(7, 0, i18n("Initializing connection to server..."));

	// initialize connection
	QString usingServer = usingCustomServer ? server : QString(TOC_HOST);
	unsigned short usingPort = usingCustomServer ? serverPort : TOC_PORT;
	socket = new KitSocket(usingServer, usingPort);

	if( !socket->connect() )
		tocConnectError(i18n("Failed to connect."));

	socket->setKeepAlive(keepAlive);
	QObject::connect( socket, SIGNAL(readData()), this, SLOT(tocConnect1()) );
	QObject::connect( socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()) );
	
	emit updateProgress(1, i18n("Waiting for reply..."));
}
void TAim::tocConnect1(void)
{
	if(!socket) return;
	char data[8192];

	emit updateProgress(2, i18n("Signing on..."));
	
	QObject::disconnect( socket, SIGNAL(readData()), this, SLOT(tocConnect1()) );
	QObject::connect( socket, SIGNAL(readData()), this, SLOT(tocConnect2()) );

	int i = socket->read((char *)&data, 8192);
	
	if(i == -1)
		tocConnectError(i18n("No data to read.  This shouldn't happen, since this function is called only by the emit readData."));
	
	if(i != SFLAP_SIGNON)
		tocConnectError(i18n("Missed signon frame from server"));

	socket->writeSignon( 1, 1, tocNormalize(username) );

	emit updateProgress(3, i18n("Sending username and password..."));
	
	if(usingCustomServer)
	{
		QString temp;
		temp.sprintf("%i", authorizerPort);
		tocSignon(authorizer, temp);
	}
	else
		tocSignon(TOC_AUTH_HOST, TOC_AUTH_PORT);
	
	emit updateProgress(4, i18n("Waiting for authorization..."));
}
void TAim::tocConnect2(void)
{
	if(!socket) return;
	QObject::disconnect( socket, SIGNAL(readData()), this, SLOT(tocConnect2()) );
	QString data;

	socket->read(data);

	if(data.left(6) == "ERROR:")
	{
		doError(data);
		setStatus(TAIM_OFFLINE);
		emit endProgress();
		return;
	}
	else if(data.left(8) != "SIGN_ON:")
		tocConnectError(i18n("Response to tocSignon is not SIGN_ON"));

	emit updateProgress(5, i18n("Checking for configuration..."));
	QObject::connect( socket, SIGNAL(readData()), this, SLOT(tocConnect3()) );
	QObject::connect( socket, SIGNAL(readData()), this, SLOT(tocServer()) );
}
void TAim::tocConnect3(void)
{
	if(!socket) return;
	QObject::disconnect( socket, SIGNAL(readData()), this, SLOT(tocConnect3()) );

	emit updateProgress(6, i18n("Sending configuration..."));

	sendUserInfo();
	sendConfig();

	emit updateProgress(7, i18n("Completing signon..."));

	tocInitDone();
	emit endProgress();

	// we need this to update the idle time, as necessary
	idleTime = QDateTime::currentDateTime();
	isIdle = false;
	idleTimer.start(60000);
}
void TAim::onDisconnect(void)
{
	idleTimer.stop();
}

/* Getting too many complaints
	tocConnectError(i18n("Disconnected."));
*/

// ***************************************************************************

#define case_tocServer( x, y ) if( data.left( y ) == x ) {
#define end_case_tocServer return; }

void TAim::tocServer(void)
{
	if(!socket) return;
	QString data;
	int i;

	socket->read(data);

	case_tocServer("SIGN_ON", 7)
		sendConfig();
		// tell TSocket to resume writing
		socket->setPaused(false);
		tocInitDone();
	end_case_tocServer
	case_tocServer("CONFIG", 6)
		tocParseConfig(data, &(serverConfig.buddyList), &(serverConfig.permitList), &(serverConfig.denyList), &(serverConfig.permitStatus));
	end_case_tocServer
	case_tocServer("NICK", 4)
		// ignore
	end_case_tocServer
	case_tocServer("IM_IN", 5)
		doIM(data);
	end_case_tocServer
	case_tocServer("UPDATE_BUDDY", 12)
		doUpdate(data);
	end_case_tocServer
	case_tocServer("ERROR", 5)
		doError(data);
	end_case_tocServer
	case_tocServer("EVILED", 6)
		doEviled(data);
	end_case_tocServer
	case_tocServer("CHAT_JOIN", 9)
		// ignore until chat is added
	end_case_tocServer
	case_tocServer("CHAT_IN", 7)
		// ignore until chat is added
	end_case_tocServer
	case_tocServer("CHAT_UPDATE_BUDDY", 17)
		// ignore until chat is added
	end_case_tocServer
	case_tocServer("CHAT_INVITE", 11)
		// ignore until chat is added
	end_case_tocServer
	case_tocServer("CHAT_LEFT", 9)
		// ignore until chat is added
	end_case_tocServer
	case_tocServer("GOTO_URL", 8)
		// strip the GOTO_URL
		i = data.find(':');
		if(i > 0) data.remove(0, i + 1);
		// strip the suggested window title
		i = data.find(':');
		if(i > 0) data.remove(0, i + 1);
		// TODO: hack hack hack
		if(usingCustomServer)
			data.sprintf("http://%s:%i/%s", server.latin1(), serverPort, data.latin1());
		else
			data.sprintf("http://" TOC_HOST ":%i/%s", TOC_PORT, data.latin1());
		// send the url
		emit userInfoReceived(data);
	end_case_tocServer
	case_tocServer("PAUSE", 5)
		socket->setPaused(true);
	end_case_tocServer
	case_tocServer("ADMIN_PASSWD_STATUS", 19)
		data.remove(0, 20);
		if(data == "0")
		{
			emit passwordChanged();
		}
		else
		{
 			QString error = i18n("Error in changing the password.");
			emit displayError(error);
		}
	end_case_tocServer
}
void TAim::endIdleness(void)
{
	if(!socket) return;
	idleTime = QDateTime::currentDateTime();
	if(isIdle)
	{
		isIdle = false;
		socket->writeData( "toc_set_idle 0" );
	}
}	
void TAim::updateIdleness(void)
{
	if(!socket) return;
	// set idle after 10 minutes
	int difference = idleTime.secsTo(QDateTime::currentDateTime());
	if( !isIdle && (difference >= 600) )
	{
		isIdle = true;
		QString data;
		data.sprintf("toc_set_idle %i", difference);
		socket->writeData(data);
	}
}
// * TAim -- private *
void TAim::tocSignon(const QString &auth_host, const QString &auth_port)
{
	QString data;
	data.sprintf("toc_signon %s %s  %s %s %s \"TIK:\\$Revision: 98382 $\"",
	             auth_host.latin1(), auth_port.latin1(),
	             (tocNormalize(username)).latin1(), r_password.latin1(), TOC_LANG);

	// frame is set up, now send it
	socket->writeData(data);
}
void TAim::tocInitDone(void)
{
	QString data = "toc_init_done";
	socket->writeData(data);
}
void TAim::setConfig(void)
{
	if(!socket) return;
	QString conf, data;
	conf = tocWriteConfig( &(serverConfig.buddyList), &(serverConfig.permitList), &(serverConfig.denyList), serverConfig.permitStatus);
	data.sprintf("toc_set_config \"%s\"", conf.latin1());
	if(data != lastConfigSent)
	{
		socket->writeData(data);
		lastConfigSent = data;
	}
}
void TAim::sendConfig(void)
{
	if(!socket) return;
	QString data;
	// send buddyList
	data = "toc_add_buddy";
	for(int i = 0; i < currentConfig.buddyList.getCount(); i++)
	{
		data += " " + tocNormalize( currentConfig.buddyList.getName(i) );
	}
	if(data != lastListSent)
	{
		socket->writeData(data);
		lastListSent = data;
	}
}
void TAim::sendPermissions(void)
{
	if(!socket) return;
	QString data;
	// set permit mode
	switch(lastPermissionSent)
	{
		case TOC_PERMITSOME:
		case TOC_PERMITALL:
			socket->writeData("toc_add_permit");
			break;
		case TOC_DENYALL:
		case TOC_DENYSOME:
			socket->writeData("toc_add_deny");
			break;
	}
	switch(currentConfig.permitStatus)
	{
		case TOC_PERMITSOME:
			data = "toc_add_permit";
			for(int i = 0; i < currentConfig.permitList.getCount(); i++)
			{
				data += " " + tocNormalize( currentConfig.permitList.getName(i) );
			}
			socket->writeData(data);
			break;
		case TOC_DENYALL:
			socket->writeData("toc_add_permit");
			break;
		case TOC_DENYSOME:
			data = "toc_add_deny";
			for(int i = 0; i < currentConfig.denyList.getCount(); i++)
			{
				data += " " + tocNormalize( currentConfig.denyList.getName(i) );
			}
			socket->writeData(data);
		case TOC_PERMITALL:
			socket->writeData("toc_add_deny");
			break;
	}
	lastPermissionSent = currentConfig.permitStatus;
}
void TAim::sendUserInfo(void)
{
	if(!socket) return;
	QString data;
	data.sprintf("toc_set_info %s", tocProcess(userInfo).latin1());
	if(data != lastInfoSent)
	{
		socket->writeData(data);
		lastInfoSent = data;
	}
}
void TAim::doUpdate(const QString &data)
{
	TBuddy buddy;
	QString workstr;
	QString holding;
	int i;

	workstr = data;

	// skip the "UPDATE_BUDDY:"
	workstr.remove(0, 13);

	// name
	i = workstr.find(':');
	buddy.name = workstr.left(i);
	// skip past the colon
	workstr.remove(0, i + 1);

	// status:  T or F
	buddy.status = (workstr[0] == 'T');
	
	// skip the status and the colon
	workstr.remove(0, 2);

	// evil amount : 0 to 100
	i = workstr.find(':');
	holding = workstr.left(i);
	buddy.evil = holding.toInt();
	workstr.remove(0, i + 1);
	
	// signontime:  seconds since unix epoch
	i = workstr.find(':');
	holding = workstr.left(i);
	buddy.signonTime = holding.toInt();
	workstr.remove(0, i + 1);

	// idle time: minutes
	i = workstr.find(':');
	holding = workstr.left(i);
	buddy.idleTime = holding.toInt();
	workstr.remove(0, i + 1);

	// user class "A " == AOL  " A" == oscar admin  " U" == unconfirmed  " O" == normal
	if(workstr[0] == 'A')
		buddy.userClass = TOC_USER_AOL;
	else
	{
		switch((char)(QChar)workstr[1])
		{
			case 'A':
				buddy.userClass = TOC_USER_ADMIN;
				break;
			case 'U':
				buddy.userClass = TOC_USER_UNCONFIRMED;
				break;
			case 'O':
			default:
				buddy.userClass = TOC_USER_NORMAL;
		}
	}

	// store the information
	int currentNum = currentConfig.buddyList.getNum(buddy.name);
	if(currentNum != -1)
	{
		int oldstatus = currentConfig.buddyList.getStatus(currentNum);
		if(buddy.status != oldstatus)
		{
			QString message;
			if(buddy.status)
			{
				message = i18n("%1 is online.").arg(buddy.name);
				KNotifyClient::event("Buddy Online", message);
			}
			else
			{
				message = i18n("%1 is offline.").arg(buddy.name);
				KNotifyClient::event("Buddy Offline", message);
			}
		}
		currentConfig.buddyList.setStatus(currentNum, buddy.status);
		currentConfig.buddyList.setUserClass(currentNum, buddy.userClass);
		currentConfig.buddyList.setSignonTime(currentNum, buddy.signonTime);
		currentConfig.buddyList.setIdleTime(currentNum, buddy.idleTime);
		currentConfig.buddyList.setEvil(currentNum, buddy.evil);
		emit buddyChanged(currentNum);
	}
}
void TAim::doIM(const QString &data)
{
	QString workstr;
	QString senderName;
	bool isAuto;
	int i;
	
	workstr = data;
	// skip IM_IN:
	workstr.remove(0, 6);

	// get username
	i = workstr.find(':');
	// if no colons, then there's something wrong
	if(i == -1)
		return;
	else
	{
		senderName = workstr.left(i);
		workstr.remove(0, i + 1);
	}
	// get whether it's auto
	isAuto = ( (char)(QChar)workstr[0] == 'T' ) ? true : false;
	workstr.remove(0, 2);
	// the rest is the message
	emit IMReceived(senderName, workstr, isAuto);
}
void TAim::doEviled(const QString &data)
{
	QString workstr;
	int newEvil;
	int i;

	workstr = data;
	// skip EVILED:
	workstr.remove(0, 7);

	i = workstr.find(':');
	if(i == -1) return;
	
	newEvil = workstr.left(i).toInt();
	// remove new evil
	workstr.remove(0, i + 1);

	// check if it's anonymous, and if it's a warning or a reduction
	if(newEvil > oldEvil)
		emit warningReceived(newEvil, (workstr.isEmpty()) ? QString::null : workstr);
	else
		emit warningReduced(newEvil, (workstr.isEmpty()) ? QString::null : workstr);
}
void TAim::doError(const QString &data)
{
	int errorno = 0;
	int i;
	QString workstr;
	QString message;
	QString display;

	workstr = data;

	// skip ERROR:
	workstr.remove(0, 6);

	// extract error number
	// QString message used as a temporary here
	// after this, workstr holds the parameter
	i = workstr.find(':');
	if(i != -1)
	{
		message = workstr.left(i);
		errorno = message.toInt();
		workstr.remove(0, i + 1);
	}
	else
	{
		errorno = workstr.toInt();
		workstr.truncate(0);
	}

#define doError_errorCase( x, y ) case x: message = y; break;

	switch(errorno)
	{
		// general errors
		doError_errorCase(901, i18n("%1 is not currently available."));
		doError_errorCase(902, i18n("Warning of %1 not currently available."));
		doError_errorCase(903, i18n("A message has been dropped; you are exceeding the server speed limit."));
		// admin errors
		doError_errorCase(911, i18n("Error validating input."));
		doError_errorCase(912, i18n("Invalid account."));
		doError_errorCase(913, i18n("Error encountered while processing request."));
		doError_errorCase(914, i18n("Service unavailable."));
		// chat errors
		doError_errorCase(950, i18n("Chat in %1 is unavailable."));
		// Message and information errors
		doError_errorCase(960, i18n("You are sending messages too fast to %1."));
		doError_errorCase(961, i18n("You missed a message from %1 because the message was too big."));
		doError_errorCase(962, i18n("You missed a message from %1 because the message was sent too fast."));
		// Directory errors
		doError_errorCase(970, i18n("Search failed."));
		doError_errorCase(971, i18n("Search returned too many matches."));
		doError_errorCase(972, i18n("Search needs more qualifiers."));
		doError_errorCase(973, i18n("Directory service temporarily unavailable."));
		doError_errorCase(974, i18n("Email lookup is restricted."));
		doError_errorCase(975, i18n("Keyword ignored."));
		doError_errorCase(976, i18n("No Keywords."));
		doError_errorCase(977, i18n("Language not supported."));
		doError_errorCase(978, i18n("Country not supported."));
		doError_errorCase(979, i18n("Unknown directory failure: %1"));
		// Authorization errors
		doError_errorCase(980, i18n("Incorrect screen name or password.  Login failed."));
		doError_errorCase(981, i18n("The service is temporarily unavailable."));
		doError_errorCase(982, i18n("Your warning level is currently too high to sign on."));
		doError_errorCase(983, i18n("You have been connecting and disconnecting too frequently.  Wait 10 minutes and try again.  If you continue to try, you will need to wait even longer."));
		doError_errorCase(989, i18n("Unknown signon failure: %1"));
	}
#undef doError_errorCase

	// if parameter exists, use it
	if(workstr.length() > 0)
		display = message.arg(workstr);
	else
		display = message;

	// stop = 8
	emit displayError(display);
}

#include "taim.moc"
