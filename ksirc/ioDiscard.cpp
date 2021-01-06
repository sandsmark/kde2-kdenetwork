/**********************************************************************

 The IO Discarder

 $$Id: ioDiscard.cpp 77798 2001-01-13 12:48:56Z hausmann $$

 Simple rule, junk EVERYTHING!!!

**********************************************************************/


#include "ioDiscard.h"
#include "config.h"

KSircIODiscard::~KSircIODiscard()
{
}

void KSircIODiscard::sirc_receive(QString, bool)
{
}

void KSircIODiscard::control_message(int, QString)
{
}
