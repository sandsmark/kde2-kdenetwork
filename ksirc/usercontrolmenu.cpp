#include "usercontrolmenu.h"

#include <iostream>
#include "config.h"

extern KConfig *kConfig;

QList<UserControlMenu> UserControlMenu::UserMenu;

UserControlMenu::UserControlMenu(const QString& _title,
				 const QString& _action,
				 int _accel,
				 int _type,
				 bool _op_only)
{
  title = _title;
  action = _action;
  accel = _accel;
  type = (itype) _type;
  op_only = _op_only;
}

UserControlMenu::~UserControlMenu()
{
}

QList<UserControlMenu> *UserControlMenu::parseKConfig()
{

  // Reset the UserMenu to nothing before we start.

  UserMenu.clear();

  kConfig->setGroup("UserMenu");
  int items = kConfig->readNumEntry("Number");
  if(items == 0){

    // We found nothing, so let's use some defaults.

    UserMenu.setAutoDelete(TRUE);
    UserMenu.append(new UserControlMenu("Refresh Nicks",
					 "refresh",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu);
    UserMenu.append(new UserControlMenu("Follow",
					 "follow $$dest_nick",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu("unFollow",
					 "unfollow $$dest_nick",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu); // Defaults to a seperator
    UserMenu.append(new UserControlMenu("Whois",
					 "/whois $$dest_nick",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu("Ping",
					 "/ping $$dest_nick",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu("Version",
					 "/ctcp $$dest_nick VERSION",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu); // Defaults to a seperator
    UserMenu.append(new UserControlMenu("Abuse",
					 "/me slaps $$dest_nick around with a small 50lb Unix Manual",
					 0, UserControlMenu::Text));
    UserMenu.append(new UserControlMenu); // Defaults to a seperator
    UserMenu.append(new UserControlMenu("Kick",
					 "/kick $$dest_nick",
					 0,
					 UserControlMenu::Text,
					 TRUE));
    UserMenu.append(new UserControlMenu("Ban",
					 "/ban $$dest_nick",
					 0,
					 UserControlMenu::Text,
					 TRUE));
    UserMenu.append(new UserControlMenu("UnBan",
					 "/unban $$dest_nick",
					 0,
					 UserControlMenu::Text,
					 TRUE));
    UserMenu.append(new UserControlMenu());
    UserMenu.append(new UserControlMenu("Op",
					 "/op $$dest_nick",
					 0,
					 UserControlMenu::Text,
					 TRUE));
    UserMenu.append(new UserControlMenu("Deop",
					 "/deop $$dest_nick",
					 0,
					 UserControlMenu::Text,
					 TRUE));
    UserMenu.append(new UserControlMenu());
    UserMenu.append(new UserControlMenu("Voice",
                                         "/mode $$dest_chan +v $$dest_nick",
                                         0,
                                         UserControlMenu::Text,
                                         TRUE));
    UserMenu.append(new UserControlMenu("Devoice",
                                         "/mode $$dest_chan -v $$dest_nick",
                                         0,
                                         UserControlMenu::Text,
                                         TRUE));
  }
  else{
    QString key, cindex, title, action;
    int accel, type, oponly;
    for(int i = 0; i < items; i++){
      cindex.sprintf("%d", i);
      key = "MenuType-" + cindex;
      type = kConfig->readNumEntry(key);
      if(type == UserControlMenu::Seperator)
	UserMenu.append(new UserControlMenu());
      else if(type == UserControlMenu::Text){
	key = "MenuTitle-" + cindex;
	title = kConfig->readEntry(key);
	key = "MenuAction-" + cindex;
	action = kConfig->readEntry(key);
	key = "MenuAccel-" + cindex;
	accel = kConfig->readNumEntry(key);
	key = "MenuOpOnly-" + cindex;
	oponly = kConfig->readNumEntry(key);
	
	UserMenu.append(new UserControlMenu(title, action, accel, type, (bool) oponly));
      }
    }
  }

  return &UserMenu;

}

void UserControlMenu::writeKConfig()
{

  kConfig->setGroup("UserMenu");

  int items = (int) UserMenu.count();

  kConfig->writeEntry("Number", items);

  QString key;
  QString cindex;
  UserControlMenu *ucm;
  int type;

  for(int i = 0; i < items; i++){
    ucm = UserMenu.at(i);
    cindex.sprintf("%d", i);
    key = "MenuType-" + cindex;
    type = ucm->type;
    kConfig->writeEntry(key, (int) type);
    // Do nothing for a seperator since it defaults accross
    if(type == UserControlMenu::Text){
      key = "MenuTitle-" + cindex;
      kConfig->writeEntry(key, ucm->title);
      key = "MenuAction-" + cindex;
      kConfig->writeEntry(key, ucm->action);
      key = "MenuAccel-" + cindex;
      kConfig->writeEntry(key, (int) ucm->accel);
      key = "MenuOpOnly-" + cindex;
      kConfig->writeEntry(key, (int) ucm->op_only);
    }
  }
  kConfig->sync();

}

