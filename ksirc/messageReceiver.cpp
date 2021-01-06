#include "messageReceiver.h"
#include "ksircprocess.h"
#include "config.h"

KSircMessageReceiver::KSircMessageReceiver( KSircProcess * _proc )
{
    proc = _proc;
    broadcast = TRUE;
}

KSircMessageReceiver::~KSircMessageReceiver()
{
}

void KSircMessageReceiver::setBroadcast(bool bd)
{
  broadcast = bd;
}

bool KSircMessageReceiver::getBroadcast()
{
  return broadcast;
}

filterRuleList *KSircMessageReceiver::defaultRules()
{
  return new filterRuleList();
}
