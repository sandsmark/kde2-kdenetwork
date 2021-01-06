// tbuddylist.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef TBUDDYLIST_H
#define TBUDDYLIST_H

#include <qstring.h>

class TBuddy
{
public:
	QString name;
	int group;
	int status;
	int userClass;
	int signonTime;
	int idleTime;
	int evil;

	TBuddy *prev;
	TBuddy *next;

	friend inline int operator==(const TBuddy &, const TBuddy &);
	friend inline int operator!=(const TBuddy &, const TBuddy &);
};

int operator==(const TBuddy &a, const TBuddy &b)
{
	if(a.name != b.name) return false;
	if(a.group != b.group) return false;
	if(a.userClass != b.userClass) return false;
	return true;
}

int operator!=(const TBuddy &a, const TBuddy &b)
{
	return !(a == b);
}

class TBuddyGroup
{
public:
	QString name;

	TBuddyGroup *prev;
	TBuddyGroup *next;

	friend inline int operator==(const TBuddyGroup &, const TBuddyGroup &);
	friend inline int operator!=(const TBuddyGroup &, const TBuddyGroup &);
};

int operator==(const TBuddyGroup &a, const TBuddyGroup &b)
{
	return a.name == b.name;
}

int operator!=(const TBuddyGroup &a, const TBuddyGroup &b)
{
	return a.name != b.name;
}

class TBuddyList
{
	public:
		TBuddyList();
		TBuddyList(const TBuddyList &);
		TBuddyList operator=(const TBuddyList &);
		~TBuddyList();

		int add(const TBuddy *);
		int del(int num);
		int del(const QString &name);
		QString getName(int num) const;
		int getNum(const QString &name) const;
		int getGroup(int num) const;
		int getStatus(int num) const;
		int get(TBuddy *, int num) const;
		int get(TBuddy *, const QString &name) const;
		inline int getCount(void) const {return count;};

		int setStatus(int num, int status);
		int setUserClass(int num, int userClass);
		int setSignonTime(int num, int time);
		int setIdleTime(int num, int time);
		int setEvil(int num, int evil);

		int addGroup(const QString &name);
		int delGroup(int num);
		int delGroup(const QString &name);
		int renameGroup(const QString &oldname, const QString &newname);
		QString getNameGroup(int num) const;
		int getNumGroup(const QString &name) const;
		inline int getCountGroup(void) const {return countG;};

		void reset(void);

		friend inline int operator==(const TBuddyList &, const TBuddyList &);
		friend inline int operator!=(const TBuddyList &, const TBuddyList &);
	private:
		TBuddy *getByNum(int) const;
		TBuddyGroup *getByNumG(int) const;

		TBuddy *head;
		TBuddy *tail;
		int count;

		TBuddyGroup *headG;
		TBuddyGroup *tailG;
		int countG;
};

int operator==(const TBuddyList &a, const TBuddyList &b)
{
	for(int i = 0; i < a.count; i++)
	{
		TBuddy *aitem = a.getByNum(i);
		TBuddy *bitem = b.getByNum(i);
		if(!bitem || *aitem != *bitem)
			return false;
	}
	for(int i = 0; i < a.countG; i++)
	{
		TBuddyGroup *aitem = a.getByNumG(i);
		TBuddyGroup *bitem = b.getByNumG(i);
		if(!bitem || *aitem != *bitem)
			return false;
	}
	return true;
}

int operator!=(const TBuddyList &a, const TBuddyList &b)
{
	return !(a == b);
}

#endif
