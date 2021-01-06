// tsfbuffer.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef TSFBUFFER_H
#define TSFBUFFER_H

// framing for SFLAP connections

#define SFLAP_DATA_SIZE 8192

typedef struct sflap_frame
{
	unsigned char asterisk;
	unsigned char type;
	unsigned short sequence;
	unsigned short data_length;
	char data[SFLAP_DATA_SIZE];
};

#define SFLAP_SIGNON 1
#define SFLAP_DATA 2
#define SFLAP_KEEP_ALIVE 5

#define SFLAP_HEADERSIZE 6

// *******************
// * class TSFBuffer *
// *******************

// sflap_frame buffer entry
typedef struct sfb_entry
{
	sfb_entry *prev;
	sfb_entry *next;
	sflap_frame frame;
};

class TSFBuffer
{
	public:
		TSFBuffer();
		~TSFBuffer();
		
		void writeFrame(sflap_frame &);
		int readFrame(sflap_frame &);

		void clear(void);
	private:
		sfb_entry *head;
		sfb_entry *tail;

		unsigned short firstInSequence;
		int receivedFrame;
};

#endif
