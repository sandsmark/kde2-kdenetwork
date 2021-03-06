/** KPGP: Pretty good privacy en-/decryption class
 *        This code is under GPL V2.0
 *
 * @author Lars Knoll <knoll@mpi-hd.mpg.de>
 *
 * GNUPG support
 * @author "J. Nick Koston" <bdraco@the.system.is.halted.net> 
 *
 * PGP6 and other enhancements
 * @author Andreas Gungl <Andreas.Gungl@osp-dd.de>
 */
#ifndef KPGPBASE_H
#define KPGPBASE_H

#include <qstring.h>
#include <qstrlist.h>

class KpgpBase
{
public:

  enum{ 
    OK = 0,
      CLEARTEXT   =  0,
      RUN_ERR     =  1,
      ERROR       =  1,
      ENCRYPTED   =  2,
      SIGNED      =  4,
      GOODSIG     =  8,
      ERR_SIGNING = 16,
      UNKNOWN_SIG = 32,
      BADPHRASE   = 64,
      BADKEYS     =128,
      NO_SEC_KEY  =256, 
      MISSINGKEY  =512 };
  
  /** virtual class used internally by kpgp */
  KpgpBase();
  virtual ~KpgpBase();
  
  /** set and retrieve the message to handle */
  virtual bool setMessage(const QCString mess);
  virtual QCString message() const;

  /** manipulating the message */
  virtual int encrypt(const QStrList *, bool = false) { return OK; };
  virtual int sign(const char *) { return OK; };
  virtual int encsign(const QStrList *, const char * = 0,
		      bool = false) { return OK; };
  virtual int decrypt(const char * = 0) { return OK; };
  virtual QStrList pubKeys() { return OK; };
  virtual QString getAsciiPublicKey(QString) { return QString::null; };
  virtual int signKey(const char *, const char *) { return OK; };

  /** various functions to get the status of a message */
  bool isEncrypted() const;
  bool isSigned() const;
  bool isSigGood() const;
  bool unknownSigner() const;
  int getStatus() const;
  QString signedBy() const;
  QString signedByKey() const;
  QString encryptedFor() const;
  const QStrList *receivers() const;

  virtual QString lastErrorMessage() const;

  /// kpgp needs to access this function
  void clearOutput();

protected:
  virtual int run(const char *cmd, const char *passphrase = 0, bool onlyReadFromPGP = false);
  virtual int runGpg(const char *cmd, const char *passphrase = 0, bool onlyReadFromGnuPG = false);
  virtual void clear();

  QString addUserId();

  QCString input;
  QCString output;
  QString info;
  QString errMsg;
  QString signature;
  QString signatureID;
  QString requiredID;
  QStrList recipients;

  int status;

};

// ---------------------------------------------------------------------------

class KpgpBase2 : public KpgpBase
{

public:
  KpgpBase2();
  virtual ~KpgpBase2();

  virtual int encrypt(const QStrList *recipients,
		      bool ignoreUntrusted = false);
  virtual int sign(const char *passphrase);
  virtual int encsign(const QStrList *recipients, const char *passphrase = 0,
		      bool ingoreUntrusted = false);
  virtual int decrypt(const char *passphrase = 0);
  virtual QStrList pubKeys();
  virtual QString getAsciiPublicKey(QString _person);
  virtual int signKey(const char *key, const char *passphrase);
};

class KpgpBaseG : public KpgpBase
{

public:
  KpgpBaseG();
  virtual ~KpgpBaseG();

  virtual int encrypt(const QStrList *recipients,
		      bool ignoreUntrusted = false);
  virtual int sign(const char *passphrase);
  virtual int encsign(const QStrList *recipients, const char *passphrase = 0,
		      bool ingoreUntrusted = false);
  virtual int decrypt(const char *passphrase = 0);
  virtual QStrList pubKeys();
  virtual QString getAsciiPublicKey(QString _person);
  virtual int signKey(const char *key, const char *passphrase);
};


class KpgpBase5 : public KpgpBase
{

public:
  KpgpBase5();
  virtual ~KpgpBase5();

  virtual int encrypt(const QStrList *recipients,
		      bool ignoreUntrusted = false);
  virtual int sign(const char *passphrase);
  virtual int encsign(const QStrList *recipients, const char *passphrase = 0,
		      bool ingoreUntrusted = false);
  virtual int decrypt(const char *passphrase = 0);
  virtual QStrList pubKeys();
  virtual QString getAsciiPublicKey(QString _person);
  virtual int signKey(const char *key, const char *passphrase);
};


class KpgpBase6 : public KpgpBase2
{

public:
  KpgpBase6();
  virtual ~KpgpBase6();

  virtual int decrypt(const char *passphrase = 0);
  virtual QStrList pubKeys();

  virtual int isVersion6();
};

// ---------------------------------------------------------------------------
// inlined functions


inline bool 
KpgpBase::isEncrypted() const
{
  if( status & ENCRYPTED )
    return true;
  return false;
}

inline bool 
KpgpBase::isSigned() const
{
  if( status & SIGNED )
    return true;
  return false;
}

inline bool 
KpgpBase::isSigGood() const
{
  if( status & GOODSIG )
    return true;
  return false;
}

inline bool 
KpgpBase::unknownSigner() const
{
  if( status & UNKNOWN_SIG )
    return true;
  return false;
}

inline const QStrList *
KpgpBase::receivers() const
{
  if(recipients.count() <= 0) return 0;
  return &recipients;
}

inline int
KpgpBase::getStatus() const
{
  return status;
}

inline QString 
KpgpBase::signedBy(void) const
{
  return signature;
}

inline QString
KpgpBase::signedByKey(void) const
{
  return signatureID;
}

inline QString
KpgpBase::encryptedFor(void) const
{
  return requiredID;
}

#endif
