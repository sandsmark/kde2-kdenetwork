// kituserprofile.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef KITUSERPROFILE_H
#define KITUSERPROFILE_H

#include <qobject.h>
#include <qdict.h>
#include <qstringlist.h>
#include <qcolor.h>

#include "tbuddylist.h"
#include "taim.h"

// listview header titles
#define COL_NAME 0
#define COL_IDLE 1
#define COL_WARN 2
#define COL_SIGNON 3
#define COL_CLASS 4
#define COL_NICK 5

struct KitUserData
{
	QString name;
	QString password;
	int savePassword;

	int keepAlive;
	bool autoConnect;
	int useCustomServer;
	QString server;
	int serverPort;
	QString authorizer;
	int authorizerPort;

	int logging;
	int timestamping;

	QStringList awayMessageNames;
	QStringList awayMessages;
	QMap<QString, QString> nicknames;

	int permitStatus;
	TBuddyList names;
	TBuddyList permit;
	TBuddyList deny;

	QString personalInformation;
	TAim::directory dir;

	int listColumns;
	int x, y, width, height, state;

	QColor foregroundColor, backgroundColor;
	bool bold;
	int fontSize;
	bool icqMode;
};

class KitUserProfile : public QObject
{
	Q_OBJECT

	public:
		KitUserProfile(const QString &user, QObject * = 0, const char * = 0);
		~KitUserProfile();

		void unlock(void);

		inline const KitUserData *read(void) const { return isValid ? (const KitUserData *)&myData : 0; };
		inline KitUserData *data(void) { return isValid ? &myData : 0; };
		inline bool valid(void) const { return isValid; };

		inline const QString &name(void) const { return isValid ? userSection : QString::null; };

		bool execEditor(void);
		QString execAwayPicker(void);
	public slots:
		void save(void);
		void load(void);
	signals:
		void updated(void);
	private:
		QString userSection;
		KitUserData myData;

		bool isValid;
};

#endif
