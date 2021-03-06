#undef QT_NO_ASCII_CAST

#include "kpgp.moc"
#include "kpgpbase.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#include <qregexp.h>
#include <qcursor.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qtextcodec.h>

#include <kdebug.h>
#include <kapp.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstaticdeleter.h>
#include <kcharsets.h>
#include <kpassdlg.h>


Kpgp *Kpgp::kpgpObject = 0L;
static KStaticDeleter<Kpgp> kpgpod;

Kpgp::Kpgp()
  : publicKeys(),
    passphrase(0), passphrase_buffer_len(0), havePassPhrase(false)
{
  if (!kpgpObject) {
    kdDebug(5100) << "creating new pgp object" << endl;
  }
  kpgpObject=kpgpod.setObject(this);
  pgp = 0;

  config = new KSimpleConfig("kpgprc" );

  init();
}

Kpgp::~Kpgp()
{
  if (kpgpObject == this) kpgpObject = kpgpod.setObject(0);
  clear(TRUE);
  delete config;
}

// ----------------- public methods -------------------------

void
Kpgp::init()
{
  wipePassPhrase();

  // read kpgp config file entries
  readConfig();

  // do we have a pgp executable
  checkForPGP();
  // create the KpgpBase object later when it is
  // needed to avoid the costly check done for
  // the autodetection of PGP 2/6
  //assignPGPBase();
  delete pgp;
  pgp=0;

  // get public keys
  // No! This takes time since pgp takes some ridicules
  // time to start, blocking everything (pressing any key _on_
  // _the_ _machine_ _where_ _pgp_ _runs: helps; ??? )
  // So we will ask for keys when we need them.
  //publicKeys = pgp->pubKeys(); This can return 0!!!

  needPublicKeys = true;
}

void
Kpgp::readConfig()
{
  storePass = config->readBoolEntry("storePass");
  showEncryptionResult = config->readBoolEntry("showEncryptionResult", true);
  pgpType = ( Kpgp::PGPType) config->readNumEntry("pgpType",tAuto);
  flagEncryptToSelf = config->readBoolEntry("encryptToSelf", true);
}

void
Kpgp::writeConfig(bool sync)
{
  config->writeEntry("storePass",storePass);
  config->writeEntry("showEncryptionResult", showEncryptionResult);
  config->writeEntry("pgpType",(int) pgpType);
  config->writeEntry("encryptToSelf",flagEncryptToSelf);

  if(sync)
    config->sync();

  delete pgp;
  pgp = 0;
}

void
Kpgp::setUser(const QString aUser)
{
  if (pgpUser != aUser) {
    pgpUser = aUser;
    wipePassPhrase();
  }
}

const QString
Kpgp::user(void)
{
  return pgpUser;
}

void
Kpgp::setEncryptToSelf(bool flag)
{
  flagEncryptToSelf = flag;
}

bool
Kpgp::encryptToSelf(void)
{
  return flagEncryptToSelf;
}

bool
Kpgp::storePassPhrase(void) const
{
  return storePass;
}

void
Kpgp::setStorePassPhrase(bool flag)
{
  storePass = flag;
}


bool
Kpgp::setMessage(const QCString mess, const QCString aCharset)
{
  int index;
  int retval = 0;

  if (0 == pgp) assignPGPBase();

  clear();

  charset = aCharset;

  if(havePgp)
    retval = pgp->setMessage(mess);
  else
  {
    errMsg = i18n("Couldn't find PGP executable.\n"
		  "Please check your PATH is set correctly.");
    return FALSE;
  }


  // "-----BEGIN PGP" must be at the beginning of a line
  if (((index = mess.find("-----BEGIN PGP")) != -1) &&
       ((index == 0) || (mess[index-1] == '\n')))
  {
    // The following if-condition is always true!
    // Proof:
    // If (havePgp) and (mess contains "-----BEGIN PGP")
    //   then retval is true, i.e. != 0.
    // If (!havePgp) and (mess contains "-----BEGIN PGP")
    //   then the previous if-branch is executed and this method is left.
    // If (mess doesn't contain "-----BEGIN PGP")
    //   then this if-branch is never reached.
    // Therefore the test for (retval != 0) can be omitted.
    // if(retval != 0)
      errMsg = pgp->lastErrorMessage();

    // front and backmatter...
    front = mess.left(index);
    // "-----END PGP" must be at the beginning of a line
    index = mess.find("\n-----END PGP",index);
    if (index >= 0)
    {
      // begin the search for the next '\n' after "-----END PGP"
      index = mess.find("\n",index+13);
      back  = mess.right(mess.length() - index);
    }

    return TRUE;
  }
  //  kdDebug(5100) << "Kpgp: message does not contain PGP parts" << endl;
  return FALSE;
}

const QCString
Kpgp::frontmatter(void) const
{
  return front;
}

const QCString
Kpgp::backmatter(void) const
{
  return back;
}

const QCString
Kpgp::message(void)
{
  if (0 == pgp) assignPGPBase();

  return pgp->message();
}

bool
Kpgp::prepare(bool needPassPhrase)
{
  if (0 == pgp) assignPGPBase();

  if(!havePgp)
  {
    errMsg = i18n("Couldn't find PGP executable.\n"
		       "Please check your PATH is set correctly.");
    return FALSE;
  }

  if((pgp->getStatus() & KpgpBase::NO_SEC_KEY))
    return FALSE;

  if(needPassPhrase && !havePassPhrase) {
    QString ID = pgp->encryptedFor();
    KpgpPass passdlg(0, i18n("OpenPGP Security Check"), true, ID);
    int n = 0;
    while (this->isBusy()) { n++; this->idle(); }
    int passdlgResult = passdlg.exec();
    for ( int j = 0 ; j < n ; j++ ) this->setBusy();
    if (passdlgResult == QDialog::Accepted) {
      if (!setPassPhrase(passdlg.passphrase())) {
	// ### set new errmsg after message freeze:
	//	  errMsg = i18n("Out of memory or passphrase too long\n"
	//			"(Must be less than 1024 characters).");
	errMsg = i18n("The passphrase is missing.");
	return FALSE;
      }
    } else
      wipePassPhrase();
  }
  return TRUE;
}

void
Kpgp::wipePassPhrase(bool freeMem)
{
  if ( passphrase ) {
    if ( passphrase_buffer_len )
      memset( passphrase, 0x00, passphrase_buffer_len );
    else {
      kdDebug(5100) << "wipePassPhrase: passphrase && !passphrase_buffer_len ???" << endl;
      passphrase = 0;
    }
  }
  if ( freeMem && passphrase ) {
    free( passphrase );
    passphrase = 0;
    passphrase_buffer_len = 0;
  }
  havePassPhrase = false;
}

bool
Kpgp::decrypt(void)
{
  int retval;

  if (0 == pgp) assignPGPBase();

  // do we need to do anything?
  if(!pgp->isEncrypted()) return TRUE;
  // everything ready
  if(!prepare(TRUE)) return FALSE;
  // ok now try to decrypt the message.
  retval = pgp->decrypt(passphrase);
  // erase the passphrase if we do not want to keep it
  cleanupPass();

  if((retval & KpgpBase::BADPHRASE))
  {
    //kdDebug(5100) << "Kpgp: bad passphrase" << endl;
    wipePassPhrase();
  }

  if(retval & KpgpBase::ERROR)
  {
    errMsg = pgp->lastErrorMessage();
    return false;
  }
  return true;
}

bool
Kpgp::sign(const QString pgpUserId)
{
  return encryptFor(0, pgpUserId, true);
}

bool
Kpgp::encryptFor(const QStrList& aPers, const QString pgpUserId, bool sign)
{
  QStrList persons, noKeyFor;
  char* pers;
  int status = 0;
  errMsg = "";
  int ret;
  QString aStr;

  if (0 == pgp) assignPGPBase();

  setUser(pgpUserId);
  persons.clear();
  noKeyFor.clear();

  if(!aPers.isEmpty())
  {
    QStrListIterator it(aPers);
    while((pers = it.current()))
    {
      QString aStr = getPublicKey(pers);
      if(!aStr.isEmpty())
        persons.append(aStr);
      else
        noKeyFor.append(pers);
      ++it;
    }
    if(persons.isEmpty())
    {
      int n = 0;
      while (this->isBusy()) { n++; this->idle(); }
      int ret = KMessageBox::warningContinueCancel(0,
			       i18n("Could not find the public keys for the\n"
				    "recipients of this mail.\n"
				    "Message will not be encrypted."),
                               i18n("PGP Warning"), i18n("C&ontinue"));
      for (int j = 0; j < n; j++) this->setBusy();
      if(ret == KMessageBox::Cancel) return false;
    }
    else if(!noKeyFor.isEmpty())
    {
      QString aStr = i18n("Could not find the public keys for\n");
      QStrListIterator it(noKeyFor);
      aStr += it.current();
      ++it;
      while((pers = it.current()))
      {
        aStr += ",\n";
        aStr += pers;
        ++it;
      }
      if(it.count() > 1)
        aStr += i18n("These persons will not be able to decrypt the message.\n");
      else
        aStr += i18n("This person will not be able to decrypt the message.\n");

      int n = 0;
      while (this->isBusy()) { n++; this->idle(); }
      int ret = KMessageBox::warningContinueCancel(0, aStr,
                               i18n("PGP Warning"),
			       i18n("C&ontinue"));
      for (int j = 0; j < n; j++) this->setBusy();
      if(ret == KMessageBox::Cancel) return false;
    }
  }

  status = doEncSign(persons, sign);

  // check for bad passphrase
  while(status & KpgpBase::BADPHRASE)
  {
    wipePassPhrase();
    int n = 0;
    while (this->isBusy()) { n++; this->idle(); }
    ret = KMessageBox::warningYesNoCancel(0,
				   i18n("You just entered an invalid passphrase.\n"
					"Do you want to try again, continue and\n"
					"leave the message unsigned, "
					"or cancel sending the message?"),
                                   i18n("PGP Warning"),
				   i18n("&Retry"), i18n("Send &unsigned"));
    for (int j = 0; j < n; j++) this->setBusy();
    if(ret == KMessageBox::Cancel) return false;
    if(ret == KMessageBox::No) sign = false;
    // ok let's try once again...
    status = doEncSign(persons, sign);
  }
  // check for bad keys
  if( status & KpgpBase::BADKEYS)
  {
    aStr = pgp->lastErrorMessage();
    aStr += "\n";
    aStr += i18n("Do you want to encrypt anyway, leave the\n"
		 "message as is, or cancel the message?");
    int n = 0;
    while (this->isBusy()) { n++; this->idle(); }
    ret = KMessageBox::warningYesNoCancel(0,aStr, i18n("PGP Warning"),
                           i18n("&Encrypt"), i18n("&Send unencrypted"));
    for (int j = 0; j < n; j++) this->setBusy();
    if(ret == KMessageBox::Cancel) return false;
    if(ret == KMessageBox::No)
    {
      pgp->clearOutput();
      return true;
    }
    if(ret == KMessageBox::Yes) status = doEncSign(persons, sign, true);
  }
  if( status & KpgpBase::MISSINGKEY)
  {
    aStr = pgp->lastErrorMessage();
    aStr += "\n";
    aStr += i18n("Do you want to leave the message as is,\n"
		 "or cancel the message?");
    int n = 0;
    while (this->isBusy()) { n++; this->idle(); }
    ret = KMessageBox::warningContinueCancel(0,aStr, i18n("PGP Warning"),
				   i18n("&Send as is"));
    for (int j = 0; j < n; j++) this->setBusy();
    if(ret == KMessageBox::Cancel) return false;
    pgp->clearOutput();
    return true;
  }
  // ### FIXME (after KDE2.2): Show a message box if an error occured
  //if( !(status & KpgpBase::ERROR) )
  //{
  //  if (showCipherText())
  // If an error occured override the value of showCipherText
  if (showCipherText() || (status & KpgpBase::ERROR))
    {
        int n = 0;
        while (this->isBusy()) { n++; this->idle(); }

        //kdDebug(5100) << "kpgp: show cipher text window" << endl;
        KpgpCipherTextDlg *cipherTextDlg = new KpgpCipherTextDlg( message(), charset );
        bool result = (cipherTextDlg->exec() == QDialog::Accepted);

        for (int j = 0; j < n; j++) this->setBusy();
        return result;
    }
    return true;
  //}

  // in case of other errors we end up here.
  errMsg = pgp->lastErrorMessage();
  return false;

}

int
Kpgp::doEncSign(QStrList persons, bool sign, bool ignoreUntrusted)
{
  int retval;

  if (0 == pgp) assignPGPBase();

  // to avoid error messages in case pgp is not installed
  if(!havePgp)
    return KpgpBase::OK;

  if(sign)
  {
    if(!prepare(TRUE)) return KpgpBase::ERROR;
    retval = pgp->encsign(&persons, passphrase, ignoreUntrusted);
  }
  else
  {
    if(!prepare(FALSE)) return KpgpBase::ERROR;
    retval = pgp->encrypt(&persons, ignoreUntrusted);
  }
  // erase the passphrase if we do not want to keep it
  cleanupPass();

  return retval;
}

bool
Kpgp::signKey(QString _key)
{
  if (0 == pgp) assignPGPBase();

  if (!prepare(TRUE)) return FALSE;
  if(pgp->signKey(_key, passphrase) & KpgpBase::ERROR)
  {
    errMsg = pgp->lastErrorMessage();
    return false;
  }
  return true;
}


const QStrList*
Kpgp::keys(void)
{
  if (0 == pgp) assignPGPBase();

  if (!prepare()) return NULL;

  if(publicKeys.isEmpty()) publicKeys = pgp->pubKeys();
  return &publicKeys;
}

bool
Kpgp::havePublicKey(QString _person)
{
  if (0 == pgp) assignPGPBase();

  if(!havePgp) return true;
  if (needPublicKeys)
  {
    publicKeys = pgp->pubKeys();
    needPublicKeys=false;
  }

  QString str;
  _person = canonicalAdress(_person);

  for(str=publicKeys.first(); str!=0; str=publicKeys.next())
  {
    if(str.find(_person,0,FALSE) != -1) // case insensitive search
      return TRUE;
  }

  // reread the database, if key wasn't found...
  publicKeys = pgp->pubKeys();

  for(str=publicKeys.first(); str!=0; str=publicKeys.next())
  {
    if(str.find(_person,0,FALSE) != -1) // case insensitive search
      return TRUE;
  }

  return FALSE;
}

QString
Kpgp::getPublicKey(QString _person)
{
  if (0 == pgp) assignPGPBase();

  // just to avoid some error messages
  if(!havePgp) return QString::null;
  if (needPublicKeys)
  {
    publicKeys = pgp->pubKeys();
    needPublicKeys=false;
  }

  QString address, str;

  bool rereadKeys = false;

  while (true) {
    // first search a key which matches the complete address
    for(str=publicKeys.first(); str!=0; str=publicKeys.next())
      if(str.find(_person,0,FALSE) != -1) // case insensitive search
        return str;

    // now search a key which matches the canonical mail address.
    address = canonicalAdress(_person);
    for(str=publicKeys.first(); str!=0; str=publicKeys.next())
      if(str.find(address,0,FALSE) != -1) // case insensitive search
        return str;

    // if we already re-read the keys abort looking for a matching key
    if (rereadKeys)
      break;

    // reread the database, because key wasn't found...
    // FIXME: Add a "Re-read keys" option to the key selection dialog
    publicKeys = pgp->pubKeys();
    rereadKeys = true;
  }

  // FIXME: let user set the key/ get from keyserver
  // no match until now, let the user choose the key:
  str = SelectPublicKey(publicKeys, _person);
  if (!str.isEmpty()) return str;

  return QString::null;
}

QString
Kpgp::getAsciiPublicKey(QString _person)
{
  if (0 == pgp) assignPGPBase();

  return pgp->getAsciiPublicKey(_person);
}

bool
Kpgp::isEncrypted(void)
{
  if (0 == pgp) assignPGPBase();

  return pgp->isEncrypted();
}

const QStrList*
Kpgp::receivers(void)
{
  if (0 == pgp) assignPGPBase();

  return pgp->receivers();
}

const QString
Kpgp::KeyToDecrypt(void)
{
  if (0 == pgp) assignPGPBase();

  return pgp->encryptedFor();
}

bool
Kpgp::isSigned(void)
{
  if (0 == pgp) assignPGPBase();

  return pgp->isSigned();
}

QString
Kpgp::signedBy(void)
{
  if (0 == pgp) assignPGPBase();

  return pgp->signedBy();
}

QString
Kpgp::signedByKey(void)
{
  if (0 == pgp) assignPGPBase();

  return pgp->signedByKey();
}

bool
Kpgp::goodSignature(void)
{
  if (0 == pgp) assignPGPBase();

  return pgp->isSigGood();
}

bool Kpgp::setPassPhrase(const char * aPass)
{
  // null out old buffer before we touch the new string.  So in case
  // aPass isn't properly null-terminated, we don't leak secret data.
  wipePassPhrase();

  if (aPass)
  {
    size_t newlen = strlen( aPass );
    if ( newlen >= 1024 ) {
      // rediculously long passphrase.
      // Maybe someone wants to trick us in malloc()'ing
      // huge buffers...
      return false;
    }
    if ( passphrase_buffer_len < newlen + 1 ) {
      // too little space in current buffer:
      // allocate a larger one.
      if ( passphrase )
	free( passphrase );
      passphrase_buffer_len = (newlen + 1 + 15) & ~0xF; // make it a multiple of 16.
      passphrase = (char*)malloc( passphrase_buffer_len );
      if (!passphrase) {
	passphrase_buffer_len = 0;
	return false;
      }
    }
    memcpy( passphrase, aPass, newlen + 1 );
    havePassPhrase = true;
  }
  return true;
}

bool
Kpgp::changePassPhrase()
{
  //FIXME...
  KMessageBox::information(0,i18n("Sorry, but this feature\nis still missing"));
  return FALSE;
}

void
Kpgp::clear(bool erasePassPhrase)
{
  if(erasePassPhrase)
    wipePassPhrase(true);
  front = 0;
  back = 0;
}

const QString
Kpgp::lastErrorMsg(void) const
{
  return errMsg;
}

bool
Kpgp::havePGP(void) const
{
  return havePgp;
}

void
Kpgp::setShowCipherText(bool flag)
{
  showEncryptionResult = flag;
}

bool
Kpgp::showCipherText(void) const
{
  return showEncryptionResult;
}


// ------------------ static methods ----------------------------
Kpgp *
Kpgp::getKpgp()
{
  if (!kpgpObject)
  {
      kdError(5100) << "there is no instance of kpgp available" << endl;
  }
  return kpgpObject;
}

KSimpleConfig *
Kpgp::getConfig()
{
  return getKpgp()->config;
}


// --------------------- private functions -------------------

// check if pgp 2.6.x or 5.0 is installed
// kpgp will prefer to user pgp 5.0
bool
Kpgp::checkForPGP(void)
{
  // get path
  QString path;
  QStrList pSearchPaths;
  int index = 0;
  int lastindex = -1;

  havePgp=FALSE;

  path = getenv("PATH");
  while((index = path.find(":",lastindex+1)) != -1)
  {
    pSearchPaths.append(path.mid(lastindex+1,index-lastindex-1));
    lastindex = index;
  }
  if(lastindex != (int)path.length() - 1)
    pSearchPaths.append( path.mid(lastindex+1,path.length()-lastindex) );

  QStrListIterator it(pSearchPaths);

  // first search for pgp5.0
  havePGP5=FALSE;
  while ( it.current() )
  {
    path = it.current();
    path += "/pgpe";
    if ( !access( path, X_OK ) )
    {
      kdDebug(5100) << "Kpgp: pgp 5 found" << endl;
      havePgp=TRUE;
      havePGP5=TRUE;
      break;
    }
    ++it;
  }

  haveGpg=FALSE;
  // lets try gpg
  it.toFirst();
  while ( it.current() )
  {
    path = it.current();
    path += "/gpg";
    if ( !access( path, X_OK ) )
    {
      kdDebug(5100) << "Kpgp: gpg found" << endl;
      havePgp=TRUE;
      haveGpg=TRUE;
      break;
    }
    ++it;
  }

  // lets try pgp2.6.x
  it.toFirst();
  while ( it.current() )
  {
       path = it.current();
       path += "/pgp";
       if ( !access( path, X_OK ) )
       {
            kdDebug(5100) << "Kpgp: pgp 2 or 6 found" << endl;
            havePgp=TRUE;
            break;
       }
       ++it;
  }

  if (!havePgp)
  {
    kdDebug(5100) << "Kpgp: no pgp found" << endl;
  }

  return havePgp;
}

void
Kpgp::assignPGPBase(void)
{
  if (pgp)
    delete pgp;

  if(havePgp)
  {
    switch (pgpType)
    {
      case tGPG:
        kdDebug(5100) << "Kpgp: assign pgp - gpg" << endl;
        pgp = new KpgpBaseG();
        break;

      case tPGP2:
        kdDebug(5100) << "Kpgp: assign pgp - pgp 2" << endl;
        pgp = new KpgpBase2();
        break;

      case tPGP5:
        kdDebug(5100) << "Kpgp: assign pgp - pgp 5" << endl;
        pgp = new KpgpBase5();
        break;

      case tPGP6:
        kdDebug(5100) << "Kpgp: assign pgp - pgp 6" << endl;
        pgp = new KpgpBase6();
        break;

      case tOff:
        // dummy handler
        kdDebug(5100) << "Kpgp: pgpBase is dummy " << endl;
        pgp = new KpgpBase();
        break;

      case tAuto:
        kdDebug(5100) << "Kpgp: assign pgp - auto" << endl;
      default:
        kdDebug(5100) << "Kpgp: assign pgp - default" << endl;
        if(havePGP5)
        {
          kdDebug(5100) << "Kpgp: pgpBase is pgp 5" << endl;
          pgp = new KpgpBase5();
          pgpType = tPGP5;
        }
        else if (haveGpg)
        {
          kdDebug(5100) << "Kpgp: pgpBase is gpg " << endl;
          pgp = new KpgpBaseG();
          pgpType = tGPG;
        }
        else
        {
          KpgpBase6 *pgp_v6 = new KpgpBase6();
          if (!pgp_v6->isVersion6())
          {
            kdDebug(5100) << "Kpgp: pgpBase is pgp 2 " << endl;
            delete pgp_v6;
            pgp = new KpgpBase2();
            pgpType = tPGP2;
          }
          else
          {
            kdDebug(5100) << "Kpgp: pgpBase is pgp 6 " << endl;
            pgp = pgp_v6;
            pgpType = tPGP6;
          }
        }
    } // switch
  }
  else
  {
    // dummy handler
    kdDebug(5100) << "Kpgp: pgpBase is dummy " << endl;
    pgp = new KpgpBase();
    pgpType = tOff;
  }
}

QString
Kpgp::canonicalAdress(QString _adress)
{
  int index,index2;

  _adress = _adress.simplifyWhiteSpace();
  _adress = _adress.stripWhiteSpace();

  // just leave pure e-mail adress.
  if((index = _adress.find("<")) != -1)
    if((index2 = _adress.find("@",index+1)) != -1)
      if((index2 = _adress.find(">",index2+1)) != -1)
	return _adress.mid(index,index2-index);

  if((index = _adress.find("@")) == -1)
  {
    // local address
    char hostname[1024];
    gethostname(hostname,1024);
    QString adress = "<";
    adress += _adress;
    adress += '@';
    adress += hostname;
    adress += '>';
    return adress;
  }
  else
  {
    int index1 = _adress.findRev(" ",index);
    int index2 = _adress.find(" ",index);
    if(index2 == -1) index2 = _adress.length();
    QString adress = "<";
    adress += _adress.mid(index1+1 ,index2-index1-1);
    adress += ">";
    return adress;
  }
}

QString
Kpgp::SelectPublicKey(QStrList pbkeys, const char *caption)
{
  KpgpSelDlg dlg(pbkeys,caption);
  QString txt ="";

  int n = 0;
  while (this->isBusy()) { n++; this->idle(); }
  bool rej = dlg.exec() == QDialog::Rejected;
  for (int j = 0; j < n; j++) this->setBusy();
  if (rej)  return 0;
  txt = dlg.key();
  if (!txt.isEmpty())
  {
    return txt;
  }
  return 0;
}


//----------------------------------------------------------------------
//  widgets needed by kpgp
//----------------------------------------------------------------------

KpgpPass::KpgpPass(QWidget *parent, const QString &caption, bool modal, const QString &keyID )
  :KDialogBase( parent, 0, modal, caption,
                Ok|Cancel )
{
  QHBox *hbox = makeHBoxMainWidget();
  hbox->setSpacing( spacingHint() );
  hbox->setMargin( marginHint() );

  QLabel *label = new QLabel(hbox);
  label->setPixmap( BarIcon("pgp-keys") );

  QWidget *rightArea = new QWidget( hbox );
  QVBoxLayout *vlay = new QVBoxLayout( rightArea, 0, spacingHint() );

  if (keyID == QString::null)
    label = new QLabel(i18n("Please enter your OpenPGP passphrase"),rightArea);
  else
    label = new QLabel(i18n("Please enter the OpenPGP passphrase for\n\"%1\"").arg(keyID),
                       rightArea);
  lineedit = new KPasswordEdit( rightArea );
  lineedit->setEchoMode(QLineEdit::Password);
  lineedit->setMinimumWidth( fontMetrics().maxWidth()*20 );
  lineedit->setFocus();
  connect( lineedit, SIGNAL(returnPressed()), this, SLOT(slotOk()) );

  vlay->addWidget( label );
  vlay->addWidget( lineedit );

  disableResize();
}


KpgpPass::~KpgpPass()
{
}

const char * KpgpPass::passphrase()
{
  return lineedit->password();
}



// ------------------------------------------------------------------------

KpgpKey::KpgpKey( QStrList *keys, QWidget *parent, const char *name,
		  bool modal )
  :KDialogBase( parent, name, modal, i18n("Select key"), Ok|Cancel, Ok, true )
{
  QHBox *hbox = new QHBox( this );
  setMainWidget( hbox );
  hbox->setSpacing( spacingHint() );
  hbox->setMargin( marginHint() );

  QLabel *label = new QLabel(hbox);
  label->setPixmap( BarIcon("pgp-keys") );

  QWidget *rightArea = new QWidget( hbox );
  QVBoxLayout *vlay = new QVBoxLayout( rightArea, 0, spacingHint() );

  label = new QLabel(i18n("Please select the public key to insert"),rightArea);
  combobox = new QComboBox( FALSE, rightArea, "combo" );
  combobox->setFocus();
  if( keys != 0 )
  {
    combobox->insertStrList(keys);
  }
  vlay->addWidget( label );
  vlay->addWidget( combobox );


  setCursor( QCursor(arrowCursor) );
  cursor = kapp->overrideCursor();
  if( cursor != 0 )
    kapp->setOverrideCursor( QCursor(arrowCursor) );

  disableResize();
}


KpgpKey::~KpgpKey()
{
  if(cursor != 0)
    kapp->restoreOverrideCursor();
}


QString
KpgpKey::getKeyName(QWidget *parent, const QStrList *keys)
{
  KpgpKey pgpkey( (QStrList*)keys, parent );
  if (pgpkey.exec() == QDialog::Accepted)
    return pgpkey.getKey().copy();
  else
    return QString();
}


QString
KpgpKey::getKey()
{
  return combobox->currentText();
}


// ------------------------------------------------------------------------
KpgpConfig::KpgpConfig(QWidget *parent, const char *name, bool encrypt)
  : QWidget(parent, name), pgp( Kpgp::getKpgp() )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

  QGroupBox *group = new QGroupBox( i18n("Warning"), this );
  topLayout->addWidget( group );
  QVBoxLayout *vlay = new QVBoxLayout( group, KDialog::spacingHint() );
  vlay->addSpacing( fontMetrics().lineSpacing() );
  QLabel *label = new QLabel( i18n("<qt><b>Please check if encryption really "
  	"works before you start using it seriously. Also note that attachments "
	"are not encrypted by the PGP/GPG module.</b></qt>"), group );
  vlay->addWidget( label );

  group = new QGroupBox( i18n("Options"), this );
  topLayout->addWidget( group );
  vlay = new QVBoxLayout( group, KDialog::spacingHint() );
  vlay->addSpacing( fontMetrics().lineSpacing() );

  storePass = new QCheckBox( i18n("&Keep passphrase in memory"), group );
  vlay->addWidget( storePass );
  if (encrypt) {
    encToSelf = new QCheckBox( i18n("&Always encrypt to self"), group );
    vlay->addWidget( encToSelf );
  }
  else encToSelf = 0;
  showCipherText = new QCheckBox( i18n("&Show ciphered/signed text after composing"), group );
  vlay->addWidget( showCipherText );

  // Group for selecting the program for encryption/decryption
  radioGroup = new QButtonGroup( i18n("Encryption tool"), this );
  topLayout->addWidget( radioGroup );
  QVBoxLayout *vrlay = new QVBoxLayout( radioGroup, KDialog::spacingHint() );
  vrlay->addSpacing( fontMetrics().lineSpacing() );

  autoDetect = new QRadioButton(  i18n("Auto-&detect"), radioGroup );
  radioGroup->insert( autoDetect );
  vrlay->addWidget( autoDetect );

  useGPG = new QRadioButton( i18n("&GPG - Gnu Privacy Guard"), radioGroup );
  radioGroup->insert( useGPG );
  vrlay->addWidget( useGPG );

  usePGP2x = new QRadioButton( i18n("PGP Version &2.x"), radioGroup );
  radioGroup->insert( usePGP2x );
  vrlay->addWidget( usePGP2x );

  usePGP5x = new QRadioButton( i18n("PGP Version &5.x"), radioGroup );
  radioGroup->insert( usePGP5x );
  vrlay->addWidget( usePGP5x );

  usePGP6x = new QRadioButton( i18n("PGP Version &6.x"), radioGroup );
  radioGroup->insert( usePGP6x );
  vrlay->addWidget( usePGP6x );

  useNoPGP = new QRadioButton( i18n("Do&n't use any encryption tool"), radioGroup );
  radioGroup->insert( useNoPGP );
  vrlay->addWidget( useNoPGP );

  topLayout->addStretch(1);

  setValues();
}


KpgpConfig::~KpgpConfig()
{
}

void
KpgpConfig::setValues()
{
  // set default values
  storePass->setChecked( pgp->storePassPhrase() );
  if (encToSelf) encToSelf->setChecked( pgp->encryptToSelf() );
  showCipherText->setChecked( pgp->showCipherText() );

  switch (pgp->pgpType)
  {
    case Kpgp::tGPG:
      useGPG->setChecked(1);
      break;
    case Kpgp::tPGP2:
      usePGP2x->setChecked(1);
      break;
    case Kpgp::tPGP5:
      usePGP5x->setChecked(1);
      break;
    case Kpgp::tPGP6:
      usePGP6x->setChecked(1);
      break;
    case Kpgp::tOff:
      useNoPGP->setChecked(1);
      break;
    case Kpgp::tAuto:
    default:
      autoDetect->setChecked(1);
  }
}

void
KpgpConfig::applySettings()
{
  pgp->setStorePassPhrase(storePass->isChecked());
  if (encToSelf) pgp->setEncryptToSelf(encToSelf->isChecked());
  pgp->setShowCipherText(showCipherText->isChecked());

  if (autoDetect->isChecked())
    pgp->pgpType = Kpgp::tAuto;
  else if (useGPG->isChecked())
    pgp->pgpType = Kpgp::tGPG;
  else if (usePGP2x->isChecked())
    pgp->pgpType = Kpgp::tPGP2;
  else if (usePGP5x->isChecked())
    pgp->pgpType = Kpgp::tPGP5;
  else if (usePGP6x->isChecked())
    pgp->pgpType = Kpgp::tPGP6;
  else if (useNoPGP->isChecked())
    pgp->pgpType = Kpgp::tOff;

  pgp->writeConfig(true);
}



// ------------------------------------------------------------------------
KpgpSelDlg::KpgpSelDlg( const QStrList &aKeyList, const QString &recipent,
			QWidget *parent, const char *name, bool modal )
  :KDialogBase( parent, name, modal, i18n("PGP Key Selection"), Ok|Cancel, Ok)
{
  QFrame *page = makeMainWidget();
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

  QLabel *label = new QLabel( page );
  label->setText(i18n("Select public key for recipient \"%1\"").arg(recipent));
  topLayout->addWidget( label );

  mListBox = new QListBox( page );
  mListBox->setMinimumHeight( fontMetrics().lineSpacing() * 7 );
  mListBox->setMinimumWidth( fontMetrics().maxWidth() * 25 );
  topLayout->addWidget( mListBox, 10 );

  mKeyList = aKeyList;
  mkey = "";

  for( const char *key = mKeyList.first(); key!=0; key = mKeyList.next() )
  {
    //insert only real keys:
    //if (!(QString)key.contains("matching key"))
    mListBox->insertItem(key);
  }
  if( mListBox->count() > 0 )
  {
    mListBox->setCurrentItem(0);
  }
}


void KpgpSelDlg::slotOk()
{
  int idx = mListBox->currentItem();
  if (idx>=0) mkey = mListBox->text(idx);
  else mkey = "";

  accept();
}


void KpgpSelDlg::slotCancel()
{
  mkey = "";
  reject();
}



// ------------------------------------------------------------------------
KpgpCipherTextDlg::KpgpCipherTextDlg( const QCString & text, const QCString & charset, QWidget *parent,
                                      const char *name, bool modal )
  :KDialogBase( parent, name, modal, i18n("OpenPGP Information"), Ok|Cancel, Ok)
{
  // FIXME (post KDE2.2): show some more info, e.g. the output of GnuPG/PGP
  QFrame *page = makeMainWidget();
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

  QLabel *label = new QLabel( page );
  label->setText(i18n("Result of the last encryption / sign operation:"));
  topLayout->addWidget( label );

  mEditBox = new QMultiLineEdit( page );
  mEditBox->setMinimumHeight( fontMetrics().lineSpacing() * 25 );
  int width = fontMetrics().maxWidth() * 50;
  if (width > QApplication::desktop()->width() - 100)
    width = QApplication::desktop()->width() - 100;
  mEditBox->setMinimumWidth( width );
  mEditBox->setReadOnly(true);
  topLayout->addWidget( mEditBox, 10 );

  QFont font=mEditBox->font();
  if (!charset.isNull())
    KGlobal::charsets()->setQFont(font,
      KGlobal::charsets()->charsetForEncoding(charset));
  mEditBox->setFont(font);

  QString unicodeText;
  if (charset.isNull())
    unicodeText = QString::fromLocal8Bit(text.data());
  else {
    bool ok=true;
    QTextCodec *codec = KGlobal::charsets()->codecForName(charset, ok);
    if(!ok)
      unicodeText = QString::fromLocal8Bit(text.data());
    else
      unicodeText = codec->toUnicode(text.data(), text.length());
  }

  mEditBox->setText(unicodeText);
}
