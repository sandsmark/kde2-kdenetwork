/*
* pop.cpp -- Implementation of class KPop3Drop.
* Author:	Sirtaj Singh Kang
* Version:	$Id: pop.cpp 66491 2000-10-05 20:31:49Z dfaure $
* Generated:	Tue Apr 21 18:12:41 EST 1998
*/

#include<stdio.h>

#include<kconfigbase.h>

#include<mimelib/pop.h>

#include"utils.h"
#include"pop.h"
#include"popcfg.h"
#include"dropdlg.h"

KPop3Drop::KPop3Drop()
  : KPollableDrop(),
    _port( DefaultPort ),
    _user(""),
    _password(""),
    _savePassword( false ),
    _valid (false ),
    _pop( 0 )
{
}

void KPop3Drop::setPopServer(const QString & server, int port)
{
	_server = server;
	_port	= port;
}

void KPop3Drop::setUser(const QString & user, const QString & password, 
	bool savepass )
{
	_user = user;
	_password = password;
	_savePassword = savepass;

	_valid = true;
}

void KPop3Drop::recheck()
{
	if( _pop == 0 ) {
		_pop = new DwPopClient;
	}

	//kdDebug() << "POP3: server = " << _server << endl;
	int ret = _pop->Open(_server.ascii(), _port );

	if( !ret ) {
		_valid = false;
		return;
	}

	//kdDebug() << "POP3: user = " << _user << endl;
	ret = _pop->User(_user.ascii());

	if( ret == '-' ) {
		_valid = false;
		_pop->Quit();
		_pop->Close();

		return;
	}

	//kdDebug() << "POP3: password = " << _password << endl;
	ret = _pop->Pass(_password.ascii());

	if( ret == '-' ) {
		_valid = false;
		_pop->Quit();
		_pop->Close();

		return;
	}

	ret = _pop->Stat();

	if( ret == '-' ) {
		_valid = false;
		_pop->Quit();
		_pop->Close();

		return;
	}

	DwString response = _pop->SingleLineResponse();

	int newcount=0, octets=0;

	sscanf( response.c_str(), "+OK %d %d", &newcount, &octets );

	_pop->Quit();
	_pop->Close();

	if( newcount != count() ) {
		emit changed( newcount );
	}

	_valid = true;
	return;
}

bool KPop3Drop::valid()
{
	return _valid;
}

KPop3Drop::~KPop3Drop()
{
	delete _pop;
}

KMailDrop* KPop3Drop::clone() const 
{
	KPop3Drop *clone = new KPop3Drop;

	*clone = *this;

	return clone;
}

bool KPop3Drop::readConfigGroup( const KConfigBase& cfg )
{
	QString val;
	KPollableDrop::readConfigGroup( cfg );

	val = cfg.readEntry(fu(HostConfigKey));
	if( val.isEmpty() ) { _valid = false; return false; }
	setPopServer( val, cfg.readNumEntry(fu(PortConfigKey), DefaultPort ) );

	_user = cfg.readEntry(fu(UserConfigKey));
	if( _user.isEmpty() ) { _valid = false; return false; }

	_password = cfg.readEntry(fu(PassConfigKey));

	if( _password.isEmpty() ) {
		_savePassword = false;
	}
	else {
		_savePassword = true;
		decrypt( _password );
	}

	return true;
}

bool KPop3Drop::writeConfigGroup( KConfigBase& cfg ) const
{
	KPollableDrop::writeConfigGroup( cfg );
	QString p;

	if( _savePassword == true ) {
		p = _password;
		encrypt( p );
	}

	cfg.writeEntry(fu(HostConfigKey), _server );
	cfg.writeEntry(fu(PortConfigKey), _port );
	cfg.writeEntry(fu(UserConfigKey), _user );
	cfg.writeEntry(fu(PassConfigKey), p );

	return true;
}

KPop3Drop& KPop3Drop::operator = ( const KPop3Drop& other )
{
	setPopServer( other._server, other._port );
	setUser( other._user,other._password );
	setFreq( other.freq() );

	return *this;
}

void KPop3Drop::addConfigPage( KDropCfgDialog *dlg )
{
	dlg->addConfigPage( new KPopCfg( this ) );

	KPollableDrop::addConfigPage( dlg );
}

void KPop3Drop::encrypt( QString& str )
{
	unsigned int i, val;
	unsigned int len = str.length();
	QString result;

	for ( i=0; i < len; i++ )
	{
		char c = str[ i ].latin1();

		val = c - ' ';
		val = (255-' ') - val;
		result[i] = (char)(val + ' ');
	}
	result[i] = '\0';
}

void KPop3Drop::decrypt( QString& str )
{
	encrypt( str );
}

const char *KPop3Drop::HostConfigKey = "host";
const char *KPop3Drop::PortConfigKey = "port";
const char *KPop3Drop::UserConfigKey = "user";
const char *KPop3Drop::PassConfigKey = "pass";
const char *KPop3Drop::SavePassConfigKey = "savepass";
const int  KPop3Drop::DefaultPort	= 110;
