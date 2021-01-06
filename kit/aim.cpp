// aim.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include <qstring.h>
#include "aim.h"

QString tocNormalize(const QString &oldstr)
{
	if(oldstr == QString::null) return QString::null;

	QString workstr;

	unsigned int i = 0;
	while( i < oldstr.length() )
	{
		if( ( (char)oldstr[i] >= 'A' ) && ( (char)oldstr[i] <= 'Z' ) )
			workstr += ( (char)oldstr[i] + (char)32 );
		else if( (char)oldstr[i] != ' ' )
			workstr += oldstr[i];

		i++;
	}
	
	return workstr;
}
QString tocRoast(const QString &oldstr)
{
	const QString tocRoast_string = "Tic/Toc";
	const int tocRoast_length = tocRoast_string.length();

	QString newstr;
	QString workstr;
	unsigned char c;

	newstr = "0x";

	for(unsigned int i = 0; i < oldstr.length(); i++)
	{
		c = oldstr[i] ^ (tocRoast_string[i % tocRoast_length]);
		workstr.sprintf("%02x", c);
		newstr += workstr;
	}

	return newstr;
}
QString tocProcess(const QString &oldstr)
{
	/*Arguments
with whitespace characters should be enclosed in quotes.  Dollar signs, 
curly brackets, square brackets, parentheses, quotes, and backslashes 
must all be backslashed whether in quotes or not.*/
	QString newstr = "\"";
	for(unsigned int i = 0; i < oldstr.length(); i++)
	{
		switch( (char)(QChar)oldstr[i] )
		{
			case '\n':
				break;
			case '$':
			case '{':
			case '}':
			case '[':
			case ']':
			case '(':
			case ')':
			case '\'':
			case '"':
			case '\\':
				newstr += '\\';
				// fallthrough
			default:
				newstr += (char)(QChar)oldstr[i];
		}
	}
	newstr += "\"";
	return newstr;
}
void tocParseConfig(const QString data, TBuddyList *buddyList, TBuddyList *permitList, TBuddyList *denyList, int *permitStatus)
{
	QString workstr;
	QString holding;
	TBuddy buddy;
	int i;

	workstr = data;

	// which group buddies belong in
	int group = 0;
	// skip "CONFIG:"
	workstr.remove(0, 7);

	while(workstr.length() > 0)
	{
// leave workstr at the start of the line
#define tocParseToHolding( x ) workstr.remove(0, 2);\
                               i = workstr.find("\n");\
                               if(i != -1)\
                               {\
                                  holding = workstr.left(i);\
                                  workstr.remove(0, i + 1);\
                               }\
                               else\
                               {\
                                  holding = workstr;\
                                  workstr.remove(0, workstr.length());\
						 }

		switch((char)(QChar)workstr[0])
		{
			case 'm':
				tocParseToHolding("case m");
				*permitStatus = holding.toInt();
				break;
			case 'g':
				tocParseToHolding("case g");
				// now add the name to the config, saving the current group number
				group = buddyList->addGroup( holding );
				break;
			case 'b':
				tocParseToHolding("case b");
				// set relevant elements of buddy to add to list
				buddy.name = holding;
				buddy.group = group;
				buddy.status = TAIM_OFFLINE;
				// now add to list
				buddyList->add(&buddy);
				break;
			case 'p':
				tocParseToHolding("case p");
				// set relevant elements of buddy to add to list
				buddy.name = holding;
				buddy.group = 0;
				// now add to list
				permitList->add(&buddy);
				break;
			case 'd':
				tocParseToHolding("case d");
				// set relevant elements of buddy to add to list
				buddy.name = holding;
				buddy.group = 0;
				// now add to list
				denyList->add(&buddy);
				break;
			default:
				tocParseToHolding("default");
		}
	}
}
QString tocWriteConfig(const TBuddyList *buddyList, const TBuddyList *permitList, const TBuddyList *denyList, int permitStatus)
{
	QString data;
	data.sprintf("m %1i\n", permitStatus);

	int i = -1;
	int currentGroup = -1;
	while(++i < buddyList->getCount())
	{
		if(currentGroup < buddyList->getGroup(i))
		{
			currentGroup = buddyList->getGroup(i);
			data += "g " + buddyList->getNameGroup(currentGroup) + "\n";
		}
		data += "b " + buddyList->getName(i) + "\n";
	}
	i = -1;
	while(++i < permitList->getCount())
	{
		data += "p " + permitList->getName(i) + "\n";
	}
	i = -1;
	while(++i < denyList->getCount())
	{
		data += "d " + denyList->getName(i) + "\n";
	}
	return data;
}
