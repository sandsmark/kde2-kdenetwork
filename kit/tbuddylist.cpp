// tbuddylist.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include "config.h"

#include <qstring.h>

#include "tbuddylist.h"
#include "aim.h"

// ********************
// * class TBuddyList *
// ********************
TBuddyList::TBuddyList()
{
	head = 0;
	tail = 0;
	count = 0;
	headG = 0;
	tailG = 0;
	countG = 0;
}
TBuddyList::TBuddyList(const TBuddyList &b)
{
	head = 0;
	tail = 0;
	count = 0;
	headG = 0;
	tailG = 0;
	countG = 0;
	for(int i = 0; i < b.count; i++)
	{
		if(b.getByNum(i) != 0)
			add( b.getByNum(i) );
	}
	for(int i = 0; i < b.countG; i++)
	{
		if(b.getByNumG(i) != 0)
			addGroup( b.getByNumG(i)->name );
	}
}
TBuddyList TBuddyList::operator=(const TBuddyList &b)
{
	TBuddy buddy;
	TBuddy *cur;

	reset();
	for(int i = 0; i < b.count; i++)
	{
		cur = b.getByNum(i);
		if(cur != 0)
		{
			buddy.name = cur->name;
			buddy.group = cur->group;
			buddy.userClass = cur->userClass;
			buddy.status = cur->status;
			buddy.signonTime = cur->signonTime;
			buddy.idleTime = cur->idleTime;
			buddy.evil = cur->evil;

			add( &buddy );
		}
	}
	TBuddyGroup *groupCur;
	QString name;
	for(int i = 0; i < b.countG; i++)
	{
		groupCur = b.getByNumG(i);

		if(groupCur != 0)
		{
			name = groupCur->name;
			addGroup( name );
		}
	}
	return *this;
}
TBuddyList::~TBuddyList()
{
	reset();
}

// *******************************
// * TBuddyList public functions *
// *******************************
void TBuddyList::reset(void)
{
	TBuddy *cur = head;
	TBuddy *next;
	while(cur != 0)
	{
		next = cur->next;
		delete cur;
		cur = next;
	}
	count = 0;
	head = 0;
	tail = 0;

	TBuddyGroup *curG = headG;
	TBuddyGroup *nextG;
	while(curG != 0)
	{
		nextG = curG->next;
		delete curG;
		curG = nextG;
	}
	countG = 0;
	headG = 0;
	tailG = 0;
}
int TBuddyList::add(const TBuddy *buddy)
{
	if(buddy == 0) return -1;

	TBuddy *cur;
	TBuddy *temp = new TBuddy;
	int retval;

	temp->name = buddy->name;
	temp->group = buddy->group;
	temp->status = buddy->status;
	temp->userClass = buddy->userClass;
	temp->signonTime = buddy->signonTime;
	temp->idleTime = buddy->idleTime;
	temp->evil = buddy->evil;

	// check to make sure not to add the same name twice
	cur = head;
	QString buddyNormedName = tocNormalize(buddy->name);
	while( cur != 0 )
	{
		QString existingName = tocNormalize(cur->name);
		if( existingName == buddyNormedName )
		{
			delete temp;
			return -1;
		}
		cur = cur->next;
	}
	// we're going to add one, so increment the count
	count++;
	// decide where to add
	if(head == 0)
	{
		// first entry
		temp->prev = 0;
		temp->next = 0;
		head = temp;
		tail = head;
		return 0;
	}
	if( (tail->group < buddy->group) || 
	    ( (tail->group == buddy->group) &&
	      ( tocNormalize(tail->name) < tocNormalize(buddy->name) )
             )
	  )
	{
		// new tail
		temp->prev = tail;
		temp->next = 0;
		tail->next = temp;
		tail = temp;
		return count;
	}
	cur = tail->prev;
	retval = count - 1;
	// repeat while cur isn't null, cur's group is above buddy's group, or
	// they're in the same group, but cur's name is above buddy's name
	while( (cur != 0) &&
	       ( (cur->group > buddy->group) ||
		    ( (cur->group == buddy->group) &&
		      (tocNormalize(cur->name) > tocNormalize(buddy->name) ) ) ) ) 
	{
		cur = cur->prev;
		retval--;
	}
	if(!cur)
	{
		// new head
		temp->prev = 0;
		temp->next = head;
		head->prev = temp;
		head = temp;
		return 0;
	}
	// some other random place
	temp->prev = cur;
	temp->next = cur->next;
	cur->next = temp;
	temp->next->prev = temp;
	return retval;
}
int TBuddyList::del(int num)
{
	TBuddy *cur = getByNum(num);
	if(cur == 0) return -1;

	if(cur->prev != 0)
		cur->prev->next = cur->next;

	if(cur->next != 0)
		cur->next->prev = cur->prev;

	if(cur == head) head = cur->next;
	if(cur == tail) tail = cur->prev;
	count--;
	delete cur;

	return 0;
}
int TBuddyList::del(const QString &name)
{
	int i = getNum(name);
	return del(i);
}
QString TBuddyList::getName(int num) const
{
	TBuddy *cur = getByNum(num);

	return (cur == 0 ? QString::null : cur->name);
}
int TBuddyList::getNum(const QString &name) const
{
	if(count == 0) return -1;

	TBuddy *cur = head;
	int curNum = 0;
	do
	{
		if( tocNormalize(cur->name) == tocNormalize(name) )
			return curNum;
		else
		{
			curNum++;
			cur = cur->next;
		}
	}
	while(cur != 0);
	return -1;
}
int TBuddyList::get(TBuddy *retval, int num) const
{
	if(retval == 0) return -1;
	TBuddy *cur = getByNum(num);
	if(cur == 0) return -1;
	retval->name = cur->name;
	retval->group = cur->group;
	retval->status = cur->status;
	retval->userClass = cur->userClass;
	retval->signonTime = cur->signonTime;
	retval->idleTime = cur->idleTime;
	retval->evil = cur->evil;
	retval->prev = 0;
	retval->next = 0;
	return 0;
}
int TBuddyList::get(TBuddy *retval, const QString &name) const
{
	int i = getNum(name);
	return get(retval, i);
}
int TBuddyList::getGroup(int num) const
{
	TBuddy *cur = getByNum(num);
	return (cur == 0 ? -1 : cur->group);
}
int TBuddyList::getStatus(int num) const
{
	TBuddy *cur = getByNum(num);
	return (cur == 0 ? -1 : cur->status);
}
int TBuddyList::setStatus(int num, int status)
{
	TBuddy *cur = getByNum(num);
	if(cur == 0) return -1;
	cur->status = status;
	return 0;
}
int TBuddyList::setUserClass(int num, int userClass)
{
	TBuddy *cur = getByNum(num);
	if(cur == 0) return -1;
	cur->userClass = userClass;
	return 0;
}
int TBuddyList::setSignonTime(int num, int time)
{
	TBuddy *cur = getByNum(num);
	if(cur == 0) return -1;
	cur->signonTime = time;
	return 0;
}
int TBuddyList::setIdleTime(int num, int time)
{
	TBuddy *cur = getByNum(num);
	if(cur == 0) return -1;
	cur->idleTime = time;
	return 0;
}
int TBuddyList::setEvil(int num, int evil)
{
	TBuddy *cur = getByNum(num);
	if(cur == 0) return -1;
	cur->evil = evil;
	return 0;
}
int TBuddyList::addGroup(const QString &name)
{
	TBuddyGroup *cur;
	TBuddyGroup *temp;
	
	temp = new TBuddyGroup;
	temp->name = name;
	
	cur = headG;
	while( cur != 0 )
	{
		if(cur->name == name)
		{
			delete temp;
			return -1;
		}
		cur = cur->next;
	}
	countG++;

	int retval;
	if(headG == 0)
	{
		temp->prev = 0;
		temp->next = 0;
		headG = temp;
		tailG = headG;
		return 0;
	}
	else
	{
		// normalize names for comparisons.. so they'll be sorted without case sensitivity
		QString normedName = tocNormalize(name);

		if( tocNormalize(tailG->name) < normedName )
		{
			temp->prev = tailG;
			temp->next = 0;
			tailG->next = temp;
			tailG = temp;
			return countG - 1;
		}
		else
		{
			cur = tailG->prev;
			retval = countG - 2;
			while( (cur != 0) && ( tocNormalize(cur->name) > normedName ) )
			{
				cur = cur->prev;
				retval--;
			}
			if(cur == 0)
			{
				temp->prev = 0;
				temp->next = headG;
				headG->prev = temp;
				headG = temp;
				retval = 0;
			}
			else
			{
				temp->prev = cur;
				temp->next = cur->next;
				cur->next = temp;
				temp->next->prev = temp;
			}
			// whether cur == 0 or not, we must unlock, renumber, and quit
			{
				TBuddy *cur = head;
				while(cur != 0)
				{
					if(cur->group >= retval)
						(cur->group)++;
					cur = cur->next;
				}
			}
			return retval;
		}
	}
}
int TBuddyList::delGroup(int num)
{
	TBuddyGroup *cur = getByNumG(num);
	if(cur == 0) return -1;

	if(cur->prev != 0)
		cur->prev->next = cur->next;
	
	if(cur->next != 0)
		cur->next->prev = cur->prev;
	
	countG--;
	if(cur == headG) headG = cur->next;
	if(cur == tailG) tailG = cur->prev;
	delete cur;


	// remove all buddies in the deleted group
	// renumber the buddies that follow
	{
		TBuddy *cur = head;
		// skip all those of group less than the deleted group
		while(cur != 0 && cur->group < num)
			cur = cur->next;
		// code taken from del(int)
		// delete all those in the deleted group
		while(cur != 0 && cur->group == num)
		{
			TBuddy *save = cur->next;

			if(cur->prev != 0)
				cur->prev->next = cur->next;
			if(cur->next != 0)
				cur->next->prev = cur->prev;
				
			if(cur == head) head = cur->next;
			if(cur == tail) tail = cur->prev;
			delete cur;
			count--;

			cur = save;
		}
		// decrement the group of all that follow
		while(cur != 0)
		{
			(cur->group)--;
			cur = cur->next;
		}
	}
	return 0;
}
int TBuddyList::delGroup(const QString &name)
{
	int i;
	i = getNumGroup(name);
	return delGroup(i);
}
int TBuddyList::renameGroup(const QString &oldname, const QString &newname)
{
	int i = getNumGroup(oldname);
	// die if oldname doesn't exist
	if(i == -1) return -1;
	// die if newname does exist
	if(getNumGroup(newname) != -1) return -1;

	TBuddyGroup *cur = getByNumG(i);
	cur->name = newname;
	return 0;
}
QString TBuddyList::getNameGroup(int num) const 
{
	TBuddyGroup *cur = getByNumG(num);

	return (cur == 0 ? QString::null : cur->name);
}
int TBuddyList::getNumGroup(const QString &name) const
{
	if(countG == 0) return -1;

	TBuddyGroup *cur = headG;
	int curNum = 0;
	do
	{
		if( cur->name == name )
			return curNum;
		else
		{
			curNum++;
			cur = cur->next;
		}
	} while(cur != 0);
	return -1;
}

// ********************************
// * TBuddyList private functions *
// ********************************
TBuddy *TBuddyList::getByNum(int num) const
{
	int curNum = 0;
	TBuddy *cur = head;
	do
	{
		if( curNum == num )
			return cur;
		else
		{
			curNum++;
			cur = cur->next;
		}
	}
	while(cur != 0);
	return 0;
}
TBuddyGroup *TBuddyList::getByNumG(int num) const
{
	int curNum = 0;
	TBuddyGroup *cur = headG;
	do
	{
		if( curNum == num )
			return cur;
		else
		{
			curNum++;
			cur = cur->next;
		}
	}
	while(cur != 0);
	return 0;
}
