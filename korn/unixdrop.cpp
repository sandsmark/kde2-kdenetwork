/*
* unixdrop.cpp -- Implementation of class KUnixDrop.
* Author:	Sirtaj Singh Kang
* Version:	$Id: unixdrop.cpp 93086 2001-04-20 15:05:12Z taj $
* Generated:	Sun Nov 30 23:28:07 EST 1997
*/


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include<dropdlg.h>

#include<qfileinfo.h>
#include<qdatetime.h>
#include<qfile.h>
#include<qapplication.h>
#include<kdebug.h>

#include<kconfigbase.h>

#include"utils.h"
#include"unixcfg.h"
#include"unixdrop.h"

#define MAXSTR (1024)

static bool checkfrom(const char *buffer);
static const char *compareHeader(const char *header, const char *field);


KUnixDrop::KUnixDrop()
	: KPollableDrop(),
	_lastMod	( new QDateTime ),
	_info		( new QFileInfo ),
	_lockInfo	( new QFileInfo ),
	_lastSize	( 0 ),
	_valid		( false ),
	_buffer		( 0 )

{
}

KUnixDrop::~KUnixDrop()
{
	stopMonitor();

	delete _lastMod;
	delete _info;
	delete _lockInfo;

	if( _buffer ) {
		delete [] _buffer;
	}
}

void KUnixDrop::recheck()
{
	if( valid() && !locked() && touched() ) {
	  //kdDebug() << "KUnixDrop: checking" << endl;
		int messages = doCount();

		if( messages != count() ) {
			emit changed( messages );
		}
	}
#if 0
	else {
	    kdDebug() << "KUnixDrop: not checking - mbox " << (valid()?"":"not ") << "valid, " << (locked()?"":"not ") << "locked, " << (touched()?"":"not ") << "touched" << endl;
	}
#endif
}

bool KUnixDrop::valid()
{
	return _valid;
}

bool KUnixDrop::touched()
{
	_info->refresh();

	if ( !_info->exists() ) {
		if( _lastSize == 0  ) return false;
		_lastSize = 0;
		return true;
	}

	if(_info->size() != _lastSize
			|| (_info->lastModified() > *_lastMod) ) {

		_lastSize = _info->size();
		*_lastMod = _info->lastModified();

		return true;
	}

	return false;
}

bool KUnixDrop::locked()
{
	_lockInfo->refresh();
	return _lockInfo->exists();
}

int KUnixDrop::doCount()
{

	_info->refresh();

	if (  !_info->exists() ){
		return 0;
	}

  //kdDebug() << "KUnixDrop: counting.." << endl;
	QFile mbox(_file);
        char *buffer = lineBuffer();
        int count=0, msgCount=0;
        bool inHeader = false;
        bool hasContentLen = false;
        bool msgRead = false;
        long contentLength=0;

        if(!mbox.open(IO_ReadOnly)) {
                qWarning("countMail: file open error");
                return 0;
        }

	buffer[MAXSTR-1] = 0;

	while( mbox.readLine(buffer, MAXSTR-2) > 0 ) {
		// read a line from the mailbox

		if( !strchr(buffer, '\n') && !mbox.atEnd() ){
			// read till the end of the line if we
			// haven't already read all of it.

			int c;

			while( (c=mbox.getch()) >=0 && c !='\n' )
				;
		}

		if( !inHeader && checkfrom(buffer) ) {
			// check if this is the start of a message
			hasContentLen = false;
			inHeader = true;
			msgRead = false;
		}
		else if ( inHeader ) {
			// check header fields if we're already in one

			if (compareHeader(buffer, "Content-Length")){
				hasContentLen = true;
				contentLength = atol(buffer+15);
			}

			if (compareHeader(buffer, "Status")) {
				const char *field = buffer;
				field += 7;
				while(field && (*field== ' '||*field == '\t'))
					field++;

				if ( *field == 'N' || *field == 'U' )
					msgRead = false;
				else
					msgRead = true;
			}
			else if (buffer[0] == '\n' ) {
				if( hasContentLen ) {
					mbox.at( mbox.at() + contentLength);
				}

				inHeader = false;

				if ( !msgRead ) {
					count++;
				}
			} 
		}//in header

		if( ++msgCount >= 1000 ) {
			qApp->processEvents();
			msgCount = 0;
		}
	}//while

	mbox.close();
	//kdDebug() << count << " messages" << endl;
	return count;
}





#define whitespace(c)    (c == ' ' || c == '\t')

#define skip_white(c)	 while(c && (*c) && whitespace(*c) ) c++
#define skip_nonwhite(c) while(c && (*c) && !whitespace(*c) ) c++

#define skip_token(buf) skip_nonwhite(buf); if(!*buf) return false; \
	skip_white(buf); if(!*buf) return false;

static const char *month_name[13] = {
	"jan", "feb", "mar", "apr", "may", "jun",
	"jul", "aug", "sep", "oct", "nov", "dec", NULL
};

static const char *day_name[8] = {
	"sun", "mon", "tue", "wed", "thu", "fri", "sat", 0
};

static bool checkfrom(const char *buffer)
{

	/*
	A valid from line will be in the following format:
	
	From <user> <weekday> <month> <day> <hr:min:sec> [TZ1 [TZ2]] <year>
*/

	int day;
	int i;
	int found;

	/* From */
	
	if(!buffer || !*buffer)
		return false;

	if (strncmp(buffer, "From ", 5))
		return false;

	buffer += 5;

	skip_white(buffer);

	/* <user> */
	if(*buffer == 0) return false;
	skip_token(buffer);

	/* <weekday> */
	found = 0;
	for (i = 0; !found && day_name[i] != NULL; i++) {
		found = found || (strnicmp(day_name[i], buffer, 3) == 0);
	}

	if (!found)
		return false;
	
	skip_token(buffer);

	/* <month> */
	found = 0;
	
	for (i = 0; !found && month_name[i] != NULL; i++) {
		found = found || (strnicmp(month_name[i], buffer, 3) == 0);
	}

	if (!found)
		return false;

	skip_token(buffer);

	/* <day> */
	if ( (day = atoi(buffer)) < 0 || day < 1 || day > 31)
		return false;

	return true;
}

static const char *compareHeader(const char *header, const char *field)
{
        int len = strlen(field);

        if (strnicmp(header, field, len))
                return NULL;

        header += len;

        if( *header != ':' )
                return NULL;

        header++;

        while( *header && ( *header == ' ' || *header == '\t') )
                header++;

        return header;
}


void KUnixDrop::setFile(const QString & file)
{
	bool run = running();

	if( run ) {
		stopMonitor();
	}

	_file = file;

	// update monitors
	QString lockf = file;
	lockf += fu(".lockfile");

	_info->setFile( _file );
	_lockInfo->setFile( lockf );
	_lastMod->setTime_t( 0 );

	_valid = true;

	if( run ) {
		startMonitor();
	}
	kdDebug() << "KUnixDrop::setFile to " << file << endl;
}

KMailDrop *KUnixDrop::clone() const
{
	KUnixDrop *clone = new KUnixDrop;

	*clone = *this;

	return clone;
}

KUnixDrop& KUnixDrop::operator=( const KUnixDrop& other )
{
	setFreq( other.freq() );
	setFile( other.file() );

	return *this;
}

bool KUnixDrop::readConfigGroup ( const KConfigBase& cfg )
{
	KPollableDrop::readConfigGroup( cfg );

	QString box = cfg.readEntry(fu(FileConfigKey));
	
	if( box.isEmpty() ) {
		qWarning( "KUnixDrop::readConfigGroup: no file for '%s'.",
				caption().ascii() );

		_valid = false;

		return false;
	}

        setFile( box );

        return true;
}

bool KUnixDrop::writeConfigGroup ( KConfigBase& cfg ) const
{
	KPollableDrop::writeConfigGroup( cfg );

	cfg.writeEntry(fu(FileConfigKey), _file );

	return true;
}

void KUnixDrop::addConfigPage( KDropCfgDialog *dlg ) 
{
	dlg->addConfigPage( new KUnixCfg( this ) );

	KPollableDrop::addConfigPage( dlg );
}

char *KUnixDrop::lineBuffer()
{
	if( _buffer == 0 ) {
		_buffer = new char[MAXSTR];
		assert( _buffer != 0 );
	}

	return _buffer;
}

const char *KUnixDrop::FileConfigKey = "file";


