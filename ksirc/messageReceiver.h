
#ifndef KMESSAGERECEIVER_H
#define KMESSAGERECEIVER_H

#include <qstring.h>
#include <qlist.h>

class KSircProcess;

struct filterRule {
    const char *desc;
    const char *search;
    const char *from;
    const char *to;
};

typedef QList<filterRule> filterRuleList;

class KSircMessageReceiver
{
public:
  KSircMessageReceiver(KSircProcess *_proc);
  virtual ~KSircMessageReceiver();

    virtual void sirc_receive(QString str, bool broadcast = false) = 0;

    virtual void control_message(int, QString) = 0;

  bool getBroadcast();
  void setBroadcast(bool bd);

  virtual filterRuleList *defaultRules();

private:
  KSircProcess *proc;
  bool broadcast;

};

#endif
