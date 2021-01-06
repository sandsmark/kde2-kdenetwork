// kitsocket.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

extern "C"
{
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
}

#include "kitsocket.h"

#define print_frame(x, a)

bool sflapConnect(int sock)
{
	if(sock < 0) return false;
	int bytes;
	while(true)
	{
		bytes = ::write(sock, "FLAPON\r\n\r\n", 10);
		if(bytes == 10) return true;
		if(bytes > -1) return false;
		ERRNO_SWITCH(break, return false);
	}
}

// * class KitSocket *
KitSocket::KitSocket(const QString &host, unsigned short port, int timeout)
	: wHost(host),
	wPort(port),
	wTimeout(timeout),
	timer(this)
{
	keepAlive = true;
	paused = false;
	ksocket = 0;
	QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(_worker()));
}
KitSocket::~KitSocket()
{
	timer.disconnect(SIGNAL(timeout()));
	disconnectSocket();
}
// * KitSocket -- public *
bool KitSocket::connect(void)
{
	if(connectSocket()) if(connectSFLAP()) return true;
	disconnectSocket();
	return false;
}
void KitSocket::writeFrame(sflap_frame &a)
{
	outBuffer.writeFrame(a);
}
void KitSocket::writeData(const QString &data)
{
	sflap_frame frame;

	frame.asterisk = '*';
	frame.type = SFLAP_DATA;
	frame.sequence = htons(++out_sequence & 0xFFFF);
	frame.data_length = htons( data.length() + 1 );
	strncpy(frame.data, data.local8Bit().data(), data.local8Bit().length() + 1);

	writeFrame(frame);
}
void KitSocket::writeSignon(unsigned ver, unsigned short tlv, const QString &_name)
{
	QCString name(_name.local8Bit());

	sflap_frame frame;

	frame.asterisk = '*';
	frame.type = SFLAP_SIGNON;
	frame.sequence = htons(++out_sequence & 0xFFFF);
	frame.data_length = htons( 8 + name.length() );

	// version
	*(unsigned int *)(frame.data) = htonl( ver );
	// TLV flag (whatever that is)
	*(unsigned short *)(frame.data + 4) = htons( tlv );
	// normalized user name length
	*(unsigned short *)(frame.data + 6) = htons( name.length() );
	// normalized user name
	strncpy(frame.data + 8, name.data(), name.length());

	writeFrame(frame);
}
void KitSocket::writeKeepAlive(void)
{
	sflap_frame frame;

	frame.asterisk = '*';
	frame.type = SFLAP_KEEP_ALIVE;
	frame.sequence = htons(++out_sequence & 0xFFFF);
	frame.data_length = 0;

	writeFrame(frame);
}
int KitSocket::readFrame(sflap_frame &a)
{
	int i;
	i = inBuffer.readFrame(a);
	if(i == 0) return a.type; else return i;
}
int KitSocket::read(char *data, int len)
{
	sflap_frame frame;
	int i = readFrame(frame);
	if(i != -1) memcpy( (void *)data, (void *)frame.data, len );
	return i;
}
int KitSocket::read(QString &data)
{
	char scratchData[SFLAP_DATA_SIZE];
	int i = read((char *)&scratchData, SFLAP_DATA_SIZE);
	data = (char *)&scratchData;
	return i;
}
// * KitSocket -- public slots *
void KitSocket::setKeepAlive(bool a)
{
	keepAlive = a;
}
void KitSocket::setPaused(bool a)
{
	paused = a;
}
// * KitSocket -- protected *
bool KitSocket::connectSocket(void)
{
	if(ksocket) return false;
	outBuffer.clear();
	inBuffer.clear();
	ksocket = new KSocket(wHost.latin1(), wPort, wTimeout);
	ksocket->enableRead(false);
	ksocket->enableWrite(false);
	if(sock() < 0) return false;

	int flags = fcntl(sock(), F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(sock(), F_SETFL, flags);
	return true;
}
bool KitSocket::connectSFLAP(void)
{
	if(!ksocket) return false;
	if(sflapConnect(sock()))
	{
		timer.start( KITSOCKET_WORKER_PERIOD, false );
		return true;
	}
	return false;
}
void KitSocket::disconnectSocket()
{
	if(!ksocket) return;
	delete ksocket;
	ksocket = 0;
	emit disconnected();
}
void KitSocket::_worker(void)
{
	int bytes;
	sflap_frame frame;
	// send the server a keepalive every 15 minutes
	if(++keepalivecount >= (15 * 60 * 1000 / KITSOCKET_WORKER_PERIOD))
	{
		keepalivecount = 0;
		if(keepAlive) writeKeepAlive();
	}

	// check for stuff to read from the server
	bytes = ::read(sock(), (void *)&frame, 6);
	if(bytes < 0)
	{
		ERRNO_SWITCH(break,
		print_frame("KitSocket::_worker() -- reading -- bytes < 0", frame); disconnectSocket());
	}
	else if(bytes == 6)
	{
		int datatoread = ntohs(frame.data_length);
		int dataread = 0;
			 
		while(dataread < datatoread)
		{
			int retval = ::read( sock(), frame.data + dataread, datatoread - dataread);
			if(retval > 0) dataread += retval;
		}

		frame.data[ datatoread ] = 0;
		inBuffer.writeFrame(frame);
		emit readData();
	}
	else
	{
		print_frame("KitSocket::_worker() -- reading -- header incomplete.", frame);
		disconnectSocket();
	}

	// check for things to write to the server
	if(paused || (outBuffer.readFrame(frame) == -1)) return;
	bytes = ::write( sock(), (void *)&frame, 6);
	if(bytes < 0)
	{
		ERRNO_SWITCH(break,
		print_frame("KitSocket::_worker() -- writing -- bytes < 0", frame); disconnectSocket());
	}
	else if(bytes == 6)
	{
		bytes = ::write( sock(), (void *)&(frame.data), ntohs(frame.data_length) );
		if(bytes < ntohs(frame.data_length))
		{
			print_frame("KitSocket::_worker() -- writing -- body incomplete.", frame);
			disconnectSocket();
			return;
		}
	}
	else
	{
		print_frame("KitSocket::_worker() -- writing -- header incomplete.", frame);
		disconnectSocket();
	}
}

#include "kitsocket.moc"
