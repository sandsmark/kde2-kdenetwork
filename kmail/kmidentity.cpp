// kmidentity.cpp

#include "kmidentity.h"
#include "kfileio.h"

#include <kconfig.h>
#include <kapp.h>

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kmessagebox.h>


//-----------------------------------------------------------------------------
QStringList KMIdentity::identities()
{
  KConfig* config = kapp->config();
  KConfigGroupSaver saver( config, "Identity" );

  QStringList result = config->readListEntry( "IdentityList" );
  if (!result.contains( i18n( "Default" ))) {
    result.remove( "unknown" );
    result.prepend( i18n( "Default" ));
  }
  return result;
}


//-----------------------------------------------------------------------------
void KMIdentity::saveIdentities( QStringList ids, bool aWithSync )
{
  KConfig* config = kapp->config();
  KConfigGroupSaver saver( config, "Identity" );

  if (ids.contains( i18n( "Default" )))
    ids.remove( i18n( "Default" ));
  config->writeEntry( "IdentityList", ids );

  if (aWithSync) config->sync();
}


//-----------------------------------------------------------------------------
KMIdentity::KMIdentity( QString id )
{
  mIdentity = id;
}


//-----------------------------------------------------------------------------
KMIdentity::~KMIdentity()
{
}


//-----------------------------------------------------------------------------
void KMIdentity::readConfig(void)
{
  KConfig* config = kapp->config();
  struct passwd* pw;
  char str[80];
  int i;

  KConfigGroupSaver saver( config, (mIdentity == i18n( "Default" )) ? 
		     QString("Identity") : "Identity-" + mIdentity );

  mFullName = config->readEntry("Name");
  if (mFullName.isEmpty())
  {
    pw = getpwuid(getuid());
    if (pw)
    {
      mFullName = pw->pw_gecos;

      i = mFullName.find(',');
      if (i>0) mFullName.truncate(i);
    }
  }


  mEmailAddr = config->readEntry("Email Address");
  if (mEmailAddr.isEmpty())
  {
    pw = getpwuid(getuid());
    if (pw)
    {
      gethostname(str, 79);
      mEmailAddr = QString(pw->pw_name) + "@" + str;

    }
  }

  mVCardFile = config->readEntry("VCardFile");
  mOrganization = config->readEntry("Organization");
  mPgpIdentity = config->readEntry("PGP Identity");
  mReplyToAddr = config->readEntry("Reply-To Address");
  mSignatureFile = config->readEntry("Signature File");
  mUseSignatureFile = config->readBoolEntry("UseSignatureFile", false);
  mSignatureInlineText = config->readEntry("Inline Signature");
  if (mIdentity == i18n( "Default" ))
    mTransport = QString::null;
  else
    mTransport = config->readEntry("Transport");
}


//-----------------------------------------------------------------------------
void KMIdentity::writeConfig(bool aWithSync)
{
  KConfig* config = kapp->config();

  KConfigGroupSaver saver( config, (mIdentity == i18n( "Default" )) ? 
		     QString("Identity") : "Identity-" + mIdentity );

  config->writeEntry("Identity", mIdentity);
  config->writeEntry("Name", mFullName);
  config->writeEntry("Organization", mOrganization);
  config->writeEntry("PGP Identity", mPgpIdentity);
  config->writeEntry("Email Address", mEmailAddr);
  config->writeEntry("Reply-To Address", mReplyToAddr);
  config->writeEntry("Signature File", mSignatureFile);
  config->writeEntry("Inline Signature", mSignatureInlineText );
  config->writeEntry("UseSignatureFile", mUseSignatureFile );
  config->writeEntry("VCardFile", mVCardFile);
  config->writeEntry("Transport", mTransport);

  if (aWithSync) config->sync();
}


//-----------------------------------------------------------------------------
bool KMIdentity::mailingAllowed(void) const
{
  return (!mFullName.isEmpty() && !mEmailAddr.isEmpty());
}


//-----------------------------------------------------------------------------
void KMIdentity::setFullName(const QString &str)
{
  mFullName = str;
}


//-----------------------------------------------------------------------------
void KMIdentity::setOrganization(const QString &str)
{
  mOrganization = str;
}


//-----------------------------------------------------------------------------
void KMIdentity::setPgpIdentity(const QString &str)
{
  mPgpIdentity = str;
}


//-----------------------------------------------------------------------------
void KMIdentity::setEmailAddr(const QString &str)
{
  mEmailAddr = str;
}


//-----------------------------------------------------------------------------
void KMIdentity::setVCardFile(const QString &str)
{
  mVCardFile = str;
}


//-----------------------------------------------------------------------------
QString KMIdentity::fullEmailAddr(void) const
{
  if (mFullName.isEmpty()) return mEmailAddr;

  const QString specials("()<>@,.;:[]");

  QString result;

  // add DQUOTE's if necessary:
  bool needsQuotes=false;
  for (unsigned int i=0; i < mFullName.length(); i++) {
    if ( specials.contains( mFullName[i] ) )
      needsQuotes = true;
    else if ( mFullName[i] == '\\' || mFullName[i] == '"' ) {
      needsQuotes = true;
      result += '\\';
    }
    result += mFullName[i];
  }

  if (needsQuotes) {
    result.insert(0,'"');
    result += '"';
  }

  result += " <" + mEmailAddr + '>';

  return result;
}

//-----------------------------------------------------------------------------
void KMIdentity::setReplyToAddr(const QString& str)
{
  mReplyToAddr = str;
}


//-----------------------------------------------------------------------------
void KMIdentity::setSignatureFile(const QString &str)
{
    mSignatureFile = str;
}


//-----------------------------------------------------------------------------
void KMIdentity::setSignatureInlineText(const QString &str )
{
    mSignatureInlineText = str;
}


//-----------------------------------------------------------------------------
void KMIdentity::setUseSignatureFile( bool flag )
{
  mUseSignatureFile = flag;
}

//-----------------------------------------------------------------------------
void KMIdentity::setTransport(const QString &str)
{
  mTransport = str;
}

//-----------------------------------------------------------------------------
QString KMIdentity::signature(void) const
{
  QString result, sigcmd;

  if( mUseSignatureFile == false ) { return mSignatureInlineText; }

  if (mSignatureFile.isEmpty()) return QString::null;

  if (mSignatureFile.right(1)=="|")
  {
    KTempFile tmpf;
    int rc;

    tmpf.setAutoDelete(true);
    // signature file is a shell script that returns the signature
    if (tmpf.status() != 0) {
      QString wmsg = i18n("Failed to create temporary file\n%1:\n%2").arg(tmpf.name()).arg(strerror(errno));
      KMessageBox::information(0, wmsg);
      return QString::null;
    }
    tmpf.close();

    sigcmd = mSignatureFile.left(mSignatureFile.length()-1);
    sigcmd += " >";
    sigcmd += tmpf.name();
    rc = system(sigcmd.local8Bit());

    if (rc != 0)
    {
      QString wmsg = i18n("Failed to execute signature script\n%1:\n%2").arg(sigcmd).arg(strerror(errno));
      KMessageBox::information(0, wmsg);
      return QString::null;
    }
    result = QString::fromLocal8Bit(kFileToString(tmpf.name(), TRUE, FALSE));
  }
  else
  {
    result = QString::fromLocal8Bit(kFileToString(mSignatureFile));
  }

  return result;
}
