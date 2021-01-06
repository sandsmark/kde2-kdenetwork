#include <stdio.h>

#include "baserules.h"
#include "config.h"
#include <kconfig.h>

extern global_config *kSircConfig;
extern KConfig *kConfig;

void KSMBaseRules::sirc_receive(QString, bool)
{
}

void KSMBaseRules::control_message(int, QString)
{
}

filterRuleList *KSMBaseRules::defaultRules()
{
  filterRule *fr;
  filterRuleList *frl = new filterRuleList();
  frl->setAutoDelete(TRUE);
  if(kSircConfig->filterKColour == TRUE){
    // 3 stage rule to filter:
    // ~blah~<something> stuff with ~ here and ~ there
    // Here's the path follows
    // =>~blah~;;;<something> with ~ here and ~ there
    // =>~blah~;;;<something> with ~~ here and ~~ there
    // =>~blah~<something> with ~~ here and ~ there
    // Basic problem was getting it not to change ~blah~ into ~~blah~~
    fr = new filterRule();
    fr->desc = "Add marker to second ~";
    fr->search = "^~\\S+~";
    fr->from = "^~(\\S+)~";
    fr->to = "~;;;$1~;;;";
    frl->append(fr);
    fr = new filterRule();
    fr->desc = "Escape kSirc Control Codes";
    fr->search = "~";
    fr->from = "(?g)~(?!;;;)";
    fr->to = "$1~~";
    frl->append(fr);
    fr = new filterRule();
    fr->desc = "Remove marker to second";
    fr->search = "^~;;;\\S+~;;;";
    fr->from = "^~;;;(\\S+)~;;;";
    fr->to = "~$1~";
    frl->append(fr);

  }
  else{
    // If we don't escape all ~'s at least esacpe the ones in
    // email/part joins etc.
    fr = new filterRule();
    fr->desc = "Search for dump ~'s";
    fr->search = "\\W~\\S+@\\S+\\W";
    fr->from = "~(\\S+@)";
    fr->to = "~~$1";
    frl->append(fr);
  }
  if(kSircConfig->filterMColour == TRUE){
    fr = new filterRule();
    fr->desc = "Remove mirc Colours";
    fr->search = "\\x03";
    fr->from = "(?g)\\x03(?:\\d{0,2},{0,1}\\d{0,2})";
    fr->to = "";
    frl->append(fr);
  }
  if( kSircConfig->nickFHighlight >= 0){
    fr = new filterRule();
    fr->desc = "Highlight nicks in colours";
    fr->search = "^(?:~\\S+~)<\\S+>";
    fr->from = "<(\\S+)>";
    if(kSircConfig->nickBHighlight >= 0)
      sprintf(to, "<~%d,%d$1~c>", kSircConfig->nickFHighlight,
	                          kSircConfig->nickBHighlight);
    else
      sprintf(to, "<~%d$1~c>", kSircConfig->nickFHighlight);
    fr->to = to;
    frl->append(fr);
  }
  if( kSircConfig->usHighlight >= 0){
    QString nick = kSircConfig->nickName;
    if ( !nick.isEmpty() ) {
	if(nick.length() > 83){
	    qDebug("Nick too long");
	    nick.truncate( 83 );
	}
	sprintf(match_us,
		"(?i)<\\S+>.*%s.*", nick.latin1());
	sprintf(to_us,
		"$1~%d", kSircConfig->usHighlight);
	fr = new filterRule();
	fr->desc = "Highlight our nick";
	fr->search = match_us;
	fr->from = "(<\\S+>)";
	fr->to = to_us;
	frl->append(fr);
    }
  }

  // Default rules alays in place
  fr = new filterRule();
  fr->desc = "Remove Just bold in parts and joins";
  fr->search = "\\*\\x02\\S+\\x02\\*";
  fr->from = "\\*\\x02(\\S+)\\x02\\*";
  fr->to = "\\*$1\\*";
  frl->append(fr);

  return frl;
}
