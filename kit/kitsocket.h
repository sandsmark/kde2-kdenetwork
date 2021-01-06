// kitsocket.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef KITSOCKET_H
#define KITSOCKET_H

#include <ksock.h>
#include <qtimer.h>
#include "tsfbuffer.h"

bool sflapConnect(int);

#define ERRNO_SWITCH(good, bad) switch(errno)\
                                { case EINTR: case EAGAIN: good;\
                                default: bad; }

// 0.25 seconds between worker calls
#define KITSOCKET_WORKER_PERIOD 250

class KitSocket : public QObject
{
	Q_OBJECT

	public:
		KitSocket(const QString &host, unsigned short port, int timeOut = 30);
		~KitSocket();

		virtual bool connect(void);

		void writeFrame(sflap_frame &);
		void writeData(const QString &data);
		void writeSignon(unsigned ver, unsigned short tlv, const QString &name);
		void writeKeepAlive(void);

		int readFrame(sflap_frame &);
		int read(char *, int);
		int read(QString &);

		void setKeepAlive(bool);
		void setPaused(bool);
	signals:
		void readData(void);
		void disconnected(void);
	protected slots:
		void _worker(void);
	protected:
		inline int sock() {return ksocket ? ksocket->socket() : -1;};
		bool connectSocket(void);
		bool connectSFLAP(void);
		void disconnectSocket(void);
	private:
		bool paused, keepAlive;
		int keepalivecount;

		KSocket *ksocket;
		QString wHost;
		unsigned short wPort;
		int wTimeout;

		QTimer timer;
		int out_sequence, in_sequence;
		TSFBuffer outBuffer, inBuffer;
};

#endif
