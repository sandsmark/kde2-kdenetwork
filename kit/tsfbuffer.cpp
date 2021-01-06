// tsfbuffer.cpp
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#include "config.h"

#include <qglobal.h>
#include "tsfbuffer.h"

// *******************
// * class TSFBuffer *
// *******************
TSFBuffer::TSFBuffer()
{
	head = 0;
	tail = 0;
	receivedFrame = false;
}
TSFBuffer::~TSFBuffer()
{
}
void TSFBuffer::clear()
{
	while(head != 0)
	{
		sfb_entry *temp = head;
		head = head->next;
		if(head != 0) head->prev = 0;
		delete temp;
	}
	receivedFrame = false;
}
void TSFBuffer::writeFrame(sflap_frame &frame)
{
	sfb_entry *cur;
	sfb_entry *temp = new sfb_entry;
	temp->frame = frame;
	if(!receivedFrame)
	{
		receivedFrame = true;
		firstInSequence = frame.sequence;
	}
	if(head == 0)
	{
		// first entry
		temp->next = 0;
		temp->prev = 0;
		head = temp;
		tail = head;
		return;
	}
	// this is complicated, to handle wraparound
	if(
	   (tail->frame.sequence < frame.sequence) ||
	   ( (tail->frame.sequence > firstInSequence) && (frame.sequence <= firstInSequence) )
	  )
	{
		// new tail
		temp->prev = tail;
		temp->next = 0;
		tail->next = temp;
		tail = temp;
		return;
	}
	cur = tail->prev;
	while( (cur != 0) && (cur->frame.sequence > frame.sequence) )
		cur = cur->prev;

	if(cur == 0)
	{
		// new head
		temp->prev = 0;
		temp->next = head;
		head->prev = temp;
		head = temp;
		return;
	}
	// somewhere in the middle
	temp->prev = cur;
	temp->next = cur->next;
	cur->next = temp;
	temp->next->prev = temp;

}
int TSFBuffer::readFrame(sflap_frame &frame)
{
	if(!head) return -1;
	frame = head->frame;
	sfb_entry *temp = head;
	head = head->next;
	if(head != 0) head->prev = 0;
	delete temp;
	return 0;
}
