/*
 * kcmlisa.cpp
 *
 * Copyright (c) 2000,2001 Alexander Neundorf <neundorf@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "kcmlisa.h"

#include "findnic.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>

#include <qlabel.h>
#include <qtooltip.h>
#include <qhbox.h>
#include <qgroupbox.h>
#include <qfile.h>
#include <qtextstream.h>
 
#include <kapp.h>
#include <kprocess.h>
#include <klocale.h>
#include <kmessagebox.h>

LisaSettings::LisaSettings(const QString& config, QWidget *parent)
:QVBox(parent)
,m_config(config,false,true)
{
   m_autoSetup=new QPushButton(i18n("Autosetup..."),this);
   setStretchFactor(m_autoSetup,0);

   m_useNmblookup=new QCheckBox(i18n("Send NetBIOS broadcasts using nmblookup for searching"),this);
   QToolTip::add(m_useNmblookup,i18n("only hosts running smb servers will answer"));
   setStretchFactor(m_useNmblookup,0);

   QHBox *hbox=new QHBox(this);
   hbox->setSpacing(10);
   QLabel *label=new QLabel(i18n("Scan these addresses"),hbox);
   QToolTip::add(label,i18n("enter ranges or whole subnets to ping, see README"));
   m_pingAddresses=new KRestrictedLine(hbox,"a","0123456789.-/;");
   QToolTip::add(m_pingAddresses,i18n("enter ranges or whole subnets to ping, see README"));
   setStretchFactor(hbox,0);

   hbox=new QHBox(this);
   hbox->setSpacing(10);
   label=new QLabel(i18n("Allowed addresses"),hbox);
   QToolTip::add(label,i18n("usually your network address/subnet mask (e.g. 192.168.0.0/255.255.255.0;)"));

   m_allowedAddresses=new KRestrictedLine(hbox,"a","0123456789./;");
   QToolTip::add(m_allowedAddresses,i18n("usually your network address/subnet mask (e.g. 192.168.0.0/255.255.255.0;)"));
   setStretchFactor(hbox,0);

   hbox=new QHBox(this);
   hbox->setSpacing(10);
   label=new QLabel(i18n("Broadcast network address"),hbox);
   QToolTip::add(label,i18n("your network address/subnet mask (e.g. 192.168.0.0/255.255.255.0;)"));

   m_broadcastNetwork=new KRestrictedLine(hbox,"a","0123456789./");
   QToolTip::add(m_broadcastNetwork,i18n("your network address/subnet mask (e.g. 192.168.0.0/255.255.255.0;)"));
   setStretchFactor(hbox,0);

   m_pingNames=new KEditListBox(i18n("Additionally check the following hosts"),this,"a",false, KEditListBox::Add|KEditListBox::Remove);
   QToolTip::add(m_pingNames,i18n("The names of the hosts you want to check"));
   setStretchFactor(m_pingNames,1);

   QGroupBox *gb=new QGroupBox(2,Horizontal,i18n("Advanced settings"),this);

   m_deliverUnnamedHosts=new QCheckBox(i18n("Report unnamed hosts"),gb);
   QToolTip::add(m_deliverUnnamedHosts,i18n("report hosts without DNS names"));
   setStretchFactor(m_deliverUnnamedHosts,0);

   m_secondScan=new QCheckBox(i18n("Always scan twice"),gb);
   QToolTip::add(m_secondScan,i18n("check the hosts twice"));
   setStretchFactor(m_secondScan,0);

   hbox=new QHBox(gb);
   hbox->setSpacing(10);
   label=new QLabel(i18n("Update interval"),hbox);
   QToolTip::add(label,i18n("search hosts after this number of seconds"));
   m_updatePeriod=new QSpinBox(30,1800,10,hbox);
   m_updatePeriod->setSuffix(i18n(" sec"));
   QToolTip::add(m_updatePeriod,i18n("search hosts after this number of seconds"));
   setStretchFactor(hbox,0);

   hbox=new QHBox(gb);
   hbox->setSpacing(10);
   label=new QLabel(i18n("Wait for replies after first scan"),hbox);
   QToolTip::add(label,i18n("how long to wait for replies to the ICMP echo requests from hosts"));
   m_firstWait=new QSpinBox(1,99,5,hbox);
   m_firstWait->setSuffix(i18n(" 1/100 sec"));
   QToolTip::add(m_firstWait,i18n("how long to wait for replies to the ICMP echo requests from hosts"));
   setStretchFactor(hbox,0);

   hbox=new QHBox(gb);
   hbox->setSpacing(10);
   label=new QLabel(i18n("Send pings at once"),hbox);
   QToolTip::add(label,i18n("send this number of ping packets at once"));
   m_maxPingsAtOnce=new QSpinBox(8,1024,5,hbox);
   QToolTip::add(m_maxPingsAtOnce,i18n("send this number of ping packets at once"));
   setStretchFactor(hbox,0);

   hbox=new QHBox(gb);
   hbox->setSpacing(10);
   label=new QLabel(i18n("Wait for replies after second scan"),hbox);
   QToolTip::add(label,i18n("how long to wait for replies to the ICMP echo requests from hosts"));
   m_secondWait=new QSpinBox(0,99,5,hbox);
   m_secondWait->setSuffix(i18n(" 1/100 sec"));
   QToolTip::add(m_secondWait,i18n("how long to wait for replies to the ICMP echo requests from hosts"));
   setStretchFactor(hbox,0);

   connect(m_secondScan,SIGNAL(toggled(bool)),m_secondWait,SLOT(setEnabled(bool)));

   connect(m_pingAddresses,SIGNAL(textChanged(const QString&)),this,SIGNAL(changed()));
   connect(m_allowedAddresses,SIGNAL(textChanged(const QString&)),this,SIGNAL(changed()));
   connect(m_broadcastNetwork,SIGNAL(textChanged(const QString&)),this,SIGNAL(changed()));

   connect(m_pingAddresses,SIGNAL(returnPressed()),this,SIGNAL(changed()));
   connect(m_allowedAddresses,SIGNAL(returnPressed()),this,SIGNAL(changed()));
   connect(m_broadcastNetwork,SIGNAL(returnPressed()),this,SIGNAL(changed()));

   connect(m_firstWait,SIGNAL(valueChanged(int)),this,SIGNAL(changed()));
   connect(m_secondWait,SIGNAL(valueChanged(int)),this,SIGNAL(changed()));
   connect(m_maxPingsAtOnce,SIGNAL(valueChanged(int)),this,SIGNAL(changed()));
   connect(m_secondScan,SIGNAL(toggled(bool)),this,SIGNAL(changed()));
   connect(m_deliverUnnamedHosts,SIGNAL(toggled(bool)),this,SIGNAL(changed()));     
   connect(m_updatePeriod,SIGNAL(valueChanged(int)),this,SIGNAL(changed()));
   connect(m_pingNames,SIGNAL(changed()),this,SIGNAL(changed()));
   connect(m_useNmblookup,SIGNAL(toggled(bool)),this,SIGNAL(changed()));
   connect(m_autoSetup,SIGNAL(clicked()),this,SLOT(autoSetup()));

   setMargin(10);
   setSpacing(15);
}

void LisaSettings::load()
{
   int secondWait=m_config.readNumEntry("SecondWait",-1);
   if (secondWait<0)
   {
      m_secondWait->setValue(0);
      m_secondScan->setChecked(FALSE);
      m_secondWait->setEnabled(FALSE);
   }
   else
   {
      m_secondWait->setValue(secondWait);
      m_secondScan->setChecked(TRUE);
      m_secondWait->setEnabled(TRUE);
   };
   m_deliverUnnamedHosts->setChecked(m_config.readNumEntry("DeliverUnnamedHosts",0));

   m_firstWait->setValue(m_config.readNumEntry("FirstWait",30));
   m_maxPingsAtOnce->setValue(m_config.readNumEntry("MaxPingsAtOnce",256));
   m_updatePeriod->setValue(m_config.readNumEntry("UpdatePeriod",300));
   m_pingAddresses->setText(m_config.readEntry("PingAddresses","192.168.0.0/255.255.255.0;192.168.100.0-192.168.100.254"));
   m_allowedAddresses->setText(m_config.readEntry("AllowedAddresses","192.168.0.0/255.255.0.0"));
   m_broadcastNetwork->setText(m_config.readEntry("BroadcastNetwork","192.168.0.0/255.255.0.0"));
   m_pingNames->insertStringList(m_config.readListEntry("PingNames",';'));
   int i=m_config.readNumEntry("SearchUsingNmblookup",1);
   m_useNmblookup->setChecked(i!=0);
};

void LisaSettings::save()
{
   if ( getuid()==0)
   {
      if (m_secondScan->isChecked())
         m_config.writeEntry("SecondWait",m_secondWait->value());
      else
         m_config.writeEntry("SecondWait",-1);

      if (m_useNmblookup->isChecked())
         m_config.writeEntry("SearchUsingNmblookup",1);
      else
         m_config.writeEntry("SearchUsingNmblookup",0);
      if (m_deliverUnnamedHosts->isChecked())
         m_config.writeEntry("DeliverUnnamedHosts",1);
      else
         m_config.writeEntry("DeliverUnnamedHosts",0);
      m_config.writeEntry("FirstWait",m_firstWait->value());
      m_config.writeEntry("MaxPingsAtOnce",m_maxPingsAtOnce->value());
      m_config.writeEntry("UpdatePeriod",m_updatePeriod->value());
      m_config.writeEntry("PingAddresses",m_pingAddresses->text());
      m_config.writeEntry("AllowedAddresses",m_allowedAddresses->text());
      m_config.writeEntry("BroadcastNetwork",m_broadcastNetwork->text());
      QStringList writeStuff;
      for (int i=0; i<m_pingNames->count(); i++)
         writeStuff.append(m_pingNames->text(i));
      m_config.writeEntry("PingNames",writeStuff,';');

      m_config.sync();

      chmod("/etc/lisarc",S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
   }
   else
   {
      //ok, now it gets harder
      //we are not root but we want to write into /etc ....
      //any idea how to do it better ?
      QString username("???");
      struct passwd *user = getpwuid( getuid() );
      if ( user )
         username=user->pw_name;
      QString tmpFilename=QString("/tmp/kcmlisa-%1-%2-%3").arg(username).arg(getpid()).arg(time(0));

      QFile tmpFile(tmpFilename);
      bool res=tmpFile.open(IO_ReadWrite);
      if (res)
      {
         QTextStream confStream(&tmpFile);
         if (m_secondScan->isChecked())
            confStream<<"SecondWait = "<<m_secondWait->value()<<"\n";
         else
            confStream<<"SecondWait = -1\n";

         if (m_useNmblookup->isChecked())
            confStream<<"SearchUsingNmblookup = 1\n";
         else
            confStream<<"SearchUsingNmblookup = 0\n";

            confStream<<"SearchUsingNmblookup = 1\n";
            confStream<<"SearchUsingNmblookup = 0\n";

         if (m_deliverUnnamedHosts->isChecked())
            confStream<<"DeliverUnnamedHosts = 1\n";
         else
            confStream<<"DeliverUnnamedHosts = 0\n";

         confStream<<"FirstWait = "<<m_firstWait->value()<<"\n";
         confStream<<"MaxPingsAtOnce = "<<m_maxPingsAtOnce->value()<<"\n";
         confStream<<"UpdatePeriod = "<<m_updatePeriod->value()<<"\n";
         confStream<<"PingAddresses = "<<m_pingAddresses->text().latin1()<<"\n";
         confStream<<"AllowedAddresses = "<<m_allowedAddresses->text().latin1()<<"\n";
         confStream<<"BroadcastNetwork = "<<m_broadcastNetwork->text().latin1()<<"\n";
         QString writeStuff;
         for (int i=0; i<m_pingNames->count(); i++)
            writeStuff=writeStuff+m_pingNames->text(i).latin1()+";";

         confStream<<"PingNames = "<<writeStuff.latin1()<<"\n";
         tmpFile.close();
         QString suCommand=QString("cp %1 /etc/lisarc; chmod 644 /etc/lisarc").arg(tmpFilename.latin1());
         KProcess proc;
         proc<<"kdesu"<<"-c"<<suCommand.latin1();
         KApplication::setOverrideCursor(Qt::waitCursor);
         proc.start(KProcess::Block);
         remove(tmpFilename.latin1());
         KApplication::restoreOverrideCursor();
      };
   };
};

void LisaSettings::autoSetup()
{
   NICList* nl=findNICs();
//   cerr<<"autoSetup() "<<nl->count()<<endl;
   if (nl->count()==0)
   {
      //ok, easy one :-)
      KMessageBox::sorry(0,i18n("Sorry, it seems you don't have any network interfaces installed on your system."));
   }
   else if (nl->count()==1)
   {
      //still easy
      //if the host part is less than 20 bits simply take it
      MyNIC *nic=nl->first();
      QString address=inet_ntoa(nic->addr.sin_addr);
      QString netmask=inet_ntoa(nic->netmask.sin_addr);
      QString addrMask(address+"/"+netmask+";");
//      int tmp=ntohl(nic->addr.sin_addr.s_addr);
      int tmp=ntohl(nic->netmask.sin_addr.s_addr);
      //if the host part is less than 20 bits simply take it
      //this might be a problem on 64 bit machines
      if (tmp>0xfffff000)
      {
         m_pingAddresses->setText(addrMask);
         m_broadcastNetwork->setText(addrMask);
         m_allowedAddresses->setText(addrMask);
         m_secondWait->setValue(0);
         m_secondScan->setChecked(FALSE);
         m_secondWait->setEnabled(FALSE);
         m_firstWait->setValue(30);
         m_maxPingsAtOnce->setValue(256);
         m_updatePeriod->setValue(300);
         m_useNmblookup->setChecked(false);
      }
      else
      {
         QString addrMask(address+"/"+netmask+";");
         m_pingAddresses->setText("");
         m_broadcastNetwork->setText(addrMask);
         m_allowedAddresses->setText(addrMask);
         m_secondWait->setValue(0);
         m_secondScan->setChecked(FALSE);
         m_secondWait->setEnabled(FALSE);
         m_firstWait->setValue(30);
         m_maxPingsAtOnce->setValue(256);
         m_updatePeriod->setValue(300);
         m_useNmblookup->setChecked(true);
      };
      KMessageBox::information(0,i18n("The LISa daemon is now configured correctly, \
hopefully.<br>Make sure that it is started with root privileges. A good idea would \
be to start it when your system boots. (<code>lisa --kde2</code>)"));
   }
   else
   {
      QString msg(i18n("You have more than one network interface installed, I can't \
                       setup automatically.<br><br>I found the following:<br><br>"));
      //not that easy to handle
      for (MyNIC* tmp=nl->first(); tmp!=0; tmp=nl->next())
      {
         msg+="<b>"+tmp->name+": </b>"+inet_ntoa(tmp->addr.sin_addr)+"/"+inet_ntoa(tmp->netmask.sin_addr)+";<br>";
      };
      KMessageBox::sorry(0,msg);
   };

   emit changed();
   delete nl;
};


#include "kcmlisa.moc"

