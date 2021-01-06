/**********************************************************************

 The IO LAG Controller

 $$Id: ioLAG.cpp 77798 2001-01-13 12:48:56Z hausmann $$

**********************************************************************/


#include "ioLAG.h"
#include "control_message.h"
#include <iostream>
#include "config.h"

#include <kdebug.h>

KSircIOLAG::KSircIOLAG(KSircProcess *_proc)
  : QObject(),
    KSircMessageReceiver(_proc)
{
  proc = _proc;
  setBroadcast(FALSE);
  startTimer(30000);
//  startTimer(5000);
//(proc->getWindowList())["!all"]->control_message(SET_LAG, "99");
}


KSircIOLAG::~KSircIOLAG()
{
  killTimers();
}

void KSircIOLAG::sirc_receive(QString str, bool)
{

  if(str.contains("*L*")){
    int s1, s2;
    s1 = str.find("*L* ") + 4;
    s2 = str.length();
    if(s1 < 0 || s2 < 0){
      kdDebug() << "Lag mesage broken: " << str << endl;
      return;
    }
    QString lag = str.mid(s1, s2 - s1);
    //    std::cerr << "Lag: " << str << endl;
    //    std::cerr << "Setting lag to: " << lag << endl;
    (proc->getWindowList())["!all"]->control_message(SET_LAG, lag);
  }

}

void KSircIOLAG::control_message(int, QString)
{
}

void KSircIOLAG::timerEvent ( QTimerEvent * )
{
  QString cmd = "/lag\n";
  emit outputLine(cmd);
}

#include "ioLAG.moc"
