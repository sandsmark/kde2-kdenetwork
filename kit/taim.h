// taim.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef TAIM_H
#define TAIM_H

#include <qdatetime.h>
#include <qobject.h>
#include <qstring.h>
#include <qtimer.h>

#include "tbuddylist.h"
#include "aim.h"

class KitSocket;

typedef struct TAimConfig
{
	int permitStatus;
	TBuddyList buddyList;
	TBuddyList permitList;
	TBuddyList denyList;
};

class TAim : public QObject
{
Q_OBJECT

public:
	TAim(QObject *parent = 0, const char *name = 0);
	~TAim();

	inline const TBuddyList &buddyList(void) { return currentConfig.buddyList; };
	inline const TBuddyList &permitList(void) { return currentConfig.permitList; };
	inline const TBuddyList &denyList(void) { return currentConfig.denyList; };
	inline int permissions(void) { return currentConfig.permitStatus; };

	void setBuddyList(const TBuddyList &);
	void setPermitList(const TBuddyList &);
	void setDenyList(const TBuddyList &);
	void setPermissions(int);

	inline int getStatus(void) { return status; };
	inline QString getUserName(void) { return username; };

	void doKeepAlive(int);
	void doIdleTime(bool);

	void useCustomServer(int);
	void setCustomServer(const QString &, int, const QString &, int);

	// get at the configuration stored on the server
	inline const TBuddyList &serverBuddyList(void) { return serverConfig.buddyList; };
	inline const TBuddyList &serverPermitList(void) { return serverConfig.permitList; };
	inline const TBuddyList &serverDenyList(void) { return serverConfig.denyList; };
	inline int serverPermissions(void) { return serverConfig.permitStatus; };

	void setServerBuddyList(const TBuddyList &);
	void setServerPermitList(const TBuddyList &);
	void setServerDenyList(const TBuddyList &);
	void setServerPermissions(int);

	struct directory
	{
		QString firstName;
		QString middleName;
		QString lastName;
		QString maidenName;
		QString city;
		QString state;
		QString country;
		QString email;
		bool allowWebSearches;
	};

public slots:
	void setStatus(int);
	void setUserName(const QString &);
	void setPassword(const QString &);
	
	void changePassword(QString, QString);

	// setUserInfo ends idleness, if online
	void setUserInfo(const QString &);
	// requestUserInfo ends idleness
	void requestUserInfo(const QString &);
	// sendIM: message, target, auto
	// sendIM ends idleness
	void sendIM(QString, QString, bool);
	// sendWarning: target, anonymous
	// sendWarning ends idleness
	void sendWarning(QString, bool);
	// requestDirectory ends idleness
	void requestDirectory(const QString &);
	// setDirectory ends idleness
	void setDirectory(const directory &);
	// searchDirectory ends idleness
	void searchDirectory(const directory &);

	// Progress Bar for connecting sequence: when cancel is pressed
	void cancelProgress(void);
signals:
	void statusChanged(int);	
	void configChanged(void);
	void buddyChanged(int);
	void userNameChanged(void);
	void userInfoReceived(QString);
	// IMReceived: QString name, QString msg, bool auto
	void IMReceived(QString, QString, bool);
	void warningReceived(int, QString);
	void warningReduced(int, QString);

	// if password change is accepted, this happens
	void passwordChanged();
	
	// Progress Bar for connecting sequence
	// int max sequence number, int sequence number, QString message
	void initProgress(int, int, QString);
	// int sequence number, QString message
	void updateProgress(int, QString);
	void endProgress(void);

	// error messages (from server, and local)
	void displayError(QString);
protected slots:
	// part of the sign-on process.. connected in turn to socket::readData signal
	void tocConnect(void);
	void tocConnect1(void);
	void tocConnect2(void);
	void tocConnect3(void);
	void onDisconnect(void);
	// parsing commands from the server
	void tocServer(void);
	// idleTime
	void endIdleness(void);
	void updateIdleness(void);
private:
	int status;
	KitSocket *socket;

	TAimConfig currentConfig, serverConfig;

	int keepAlive;

	int usingCustomServer;
	QString server;
	int serverPort;
	QString authorizer;
	int authorizerPort;

	// so we don't send the buddy list n times
	QString lastListSent;
	int lastPermissionSent;
	QString lastInfoSent;
	QString lastConfigSent;

	// secondary thread host lookup stuff
	int hostThreadReturn;

	// user information and configuration
	QString username;
	QString r_password;
	QString userInfo;

	// save old warning level, so we can tell the difference between warnings and reductions
	int oldEvil;

	// idleness
	bool updateIdleTime;
	QDateTime idleTime;
	bool isIdle;
	QTimer idleTimer;

	// private functions
	void tocSignon(const QString &auth_host, const QString &auth_port);
	void tocInitDone(void);
	void setConfig(void);
	void sendConfig(void);
	void sendPermissions(void);
	void sendUserInfo(void);
	void doUpdate(const QString &);
	void doIM(const QString &);
	void doEviled(const QString &);
	void doError(const QString &);
};

#endif
