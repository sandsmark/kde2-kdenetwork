/* User identity information
 *
 * Author: Stefan Taferner <taferner@kde.org>
 * This code is under GPL
 */
#ifndef kmidentity_h
#define kmidentity_h

#include <qstring.h>
#include <qstringlist.h>

/** User identity information */
class KMIdentity
{
public:
  /** Returns list of identity ids written to the config file */
  static QStringList identities();

  /** Save a list of identity ids in the config file */
  static void saveIdentities( QStringList ids, bool aWithSync = TRUE );

  /** Constructor loads config file */
  KMIdentity( QString id );

  /** Destructor saves config file */
  virtual ~KMIdentity();

  /** Read configuration from the global config */
  virtual void readConfig(void);

  /** Write configuration to the global config with optional sync */
  virtual void writeConfig(bool withSync=TRUE);

  /** Tests if there are enough values set to allow mailing */
  virtual bool mailingAllowed(void) const;

  /** Identity/nickname fot this collection */
  QString identity(void) const { return mIdentity; }

  /** Full name of the user */
  QString fullName(void) const { return mFullName; }
  virtual void setFullName(const QString&);

  /** The user's organization (optional) */
  QString organization(void) const { return mOrganization; }
  virtual void setOrganization(const QString&);

  /** The user's PGP identity */
  QString pgpIdentity(void) const { return mPgpIdentity; }
  virtual void setPgpIdentity(const QString&);

  /** email address (without the user name - only name@host) */
  QString emailAddr(void) const { return mEmailAddr; }
  virtual void setEmailAddr(const QString&);

  /** vCard to attach to outgoing emails */
  QString vCardFile(void) const { return mVCardFile; }
  QString VCardFile(void) const { return mVCardFile; }
  virtual void setVCardFile(const QString&);

  /** email address in the format "username <name@host>" suitable
    for the "From:" field of email messages. */
  QString fullEmailAddr(void) const;

  /** email address for the ReplyTo: field */
  QString replyToAddr(void) const { return mReplyToAddr; }
  virtual void setReplyToAddr(const QString&);

  /** name of the signature file (with path) */
  QString signatureFile(void) const { return mSignatureFile; }
  virtual void setSignatureFile(const QString&);

  /** inline signature */
  QString signatureInlineText(void) const { return mSignatureInlineText;}
  virtual void setSignatureInlineText(const QString&);

  /** Inline or signature from a file */
  bool useSignatureFile(void) { return mUseSignatureFile; }
  void setUseSignatureFile(bool);

  /** Returns the signature. This method also takes care of
    special signature files that are shell scripts and handles
    them correct. So use this method to rectreive the contents of
    the signature file. */
  virtual QString signature(void) const;

  /** The transport that is set for this identity. Used to link a
      transport with an identity. */
  QString transport(void) { return mTransport; }
  virtual void setTransport(const QString&);

protected:
  QString mIdentity, mFullName, mOrganization, mPgpIdentity, mEmailAddr;
  QString mReplyToAddr, mSignatureFile;
  QString mSignatureInlineText, mTransport;
  QString mVCardFile;
  bool    mUseSignatureFile;
};

#endif /*kmidentity_h*/
