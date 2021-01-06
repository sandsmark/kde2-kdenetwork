/** KPGP: Pretty good privacy en-/decryption class
 *        This code is under GPL V2.0
 *
 * @author Lars Knoll <knoll@mpi-hd.mpg.de>
 *
 * GNUPG support
 * @author "J. Nick Koston" <bdraco@the.system.is.halted.net>
 *
 * PGP6 and other enhancements
 * @author Andreas Gungl <a.gungl@gmx.de>
 */
#ifndef KPGP_H
#define KPGP_H

#include <stdio.h>
#include <qstring.h>
#include <qstrlist.h>
#include <qdialog.h>
#include <qwidget.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qmultilineedit.h>

#include <kdialogbase.h>

class QLineEdit;
class QCursor;
class QCheckBox;
class QGridLayout;

class KSimpleConfig;
class KpgpBase;


class Kpgp
{
private:
    // the class running pgp
    KpgpBase *pgp;
protected:
    Kpgp();

public:
  virtual ~Kpgp();

  /** the following virtual function form the interface to the
      application using Kpgp
  */
  virtual void setBusy() =0;
  virtual bool isBusy() =0;
  virtual void idle() =0;

  virtual void readConfig();
  virtual void writeConfig(bool sync);
  virtual void init();

  /** sets the message to en- or decrypt
    returns TRUE if the message contains any pgp encoded or signed
    parts
    The format is then:
    <pre>
    frontmatter()
    ----- BEGIN PGP ...
    message()
    ----- END PGP ...
    backmatter()
    </pre>
    
    @p aCharset is only needed for when displaying the result to the
    user. It defaults to the local charset. */
  virtual bool setMessage(const QCString mess,
			  const QCString aCharset=0);
  /** gets the de- (or en)crypted message */
  virtual const QCString message(void);
  /** gets the part before the decrypted message */
  virtual const QCString frontmatter(void) const;
  /** gets the part after the decrypted message */
  virtual const QCString backmatter(void) const;

  /** decrypts the message if the passphrase is good.
    returns false otherwise */
  bool decrypt(void);
  /** encrypt the message for a list of persons. if sign is true then
      the message is signed with the key corresponding to the give user id. */
  bool encryptFor(const QStrList& receivers, const QString pgpUserId,
                  bool sign = TRUE);

protected:
  int doEncSign(QStrList persons, bool sign, bool ignoreUntrusted = false);

public:
  /** sign the message with the key corresponding to the give user id. */
  bool sign(const QString pgpUserId);
  /** sign a key in the keyring with users signature. */
  bool signKey(QString _key);
  /** get the known public keys. */
  const QStrList* keys(void);
  /** check if we have a public key for given person. */
  bool havePublicKey(QString person);
  /** try to get the public key for this person */
  QString getPublicKey(QString _person);
  /** try to ascii output of the public key of this person */
  QString getAsciiPublicKey(QString _person);

  /** is the message encrypted ? */
  bool isEncrypted(void);
  /** the persons who can decrypt the message */
  const QStrList* receivers(void);
  /** shows the secret key which is needed
    to decrypt the message */
  const QString KeyToDecrypt(void);

  /** is the message signed by someone */
  bool isSigned(void);
  /** it is signed by ... */
  QString signedBy(void);
  /** keyID of signer */
  QString signedByKey(void);
  /** is the signature good ? */
  bool goodSignature(void);

  /** Request the change of the passphrase of the actual secret
      key. TBI */
  bool changePassPhrase();

 /** set a user identity to use (if you have more than one...)
   * by default, pgp uses the identity which was generated last. */
  void setUser(const QString user);
  /** Returns the actual user identity. */
  const QString user(void);

  /** always encrypt message to oneself? */
  void setEncryptToSelf(bool flag);
  bool encryptToSelf(void);

  /** store passphrase in pgp object
    Problem: passphrase stays in memory.
    Advantage: you can call en-/decrypt without always passing the
    passphrase */
  void setStorePassPhrase(bool);
  bool storePassPhrase(void) const;

  /** clears everything from memory */
  void clear(bool erasePassPhrase = FALSE);

  /** returns the last error that occured */
  const QString lastErrorMsg(void) const;

  // what version of PGP/GPG should we use
  enum PGPType { tAuto, tGPG, tPGP2, tPGP5, tPGP6, tOff } pgpType;

  // did we find a pgp executable?
  bool havePGP(void) const;

  /** Should PGP/GnuPG be used? */
  bool usePGP(void) const { return (havePGP() && (pgpType != tOff)); }

  // show the result of encryption/signing?
  void setShowCipherText(bool);
  bool showCipherText(void) const;

  // FIXME: key management

  // static methods

  /** return the actual pgp object */
  static Kpgp *getKpgp();

  /** get the kpgp config object */
  static KSimpleConfig *getConfig();

private:
  /** Set pass phrase */
  bool setPassPhrase(const char* pass);

  // test if the PGP executable is found and if there is a passphrase
  // set or given. Returns TRUE if everything is ok, and FALSE (together
  // with some warning message) if something is missing.
  bool prepare(bool needPassPhrase=FALSE);

  /** cleanup passphrase if it should not be stored. */
  void cleanupPass() { if (!storePass) wipePassPhrase(); }

  /** Wipes and optionally frees the memory used to hold the
      passphrase. */
  void wipePassPhrase(bool free=false);

  // transform an adress into canonical form
  QString canonicalAdress(QString _person);

  //Select public key from a list of all public keys
  QString SelectPublicKey(QStrList pbkeys, const char *caption);

  bool checkForPGP(void);
  void assignPGPBase(void);

  static Kpgp *kpgpObject;
  KSimpleConfig *config;

  QStrList publicKeys;
  QCString front;
  QCString back;
  QCString charset;
  QString errMsg;

  bool storePass : 1;
  char * passphrase;
  size_t passphrase_buffer_len;

  QString pgpUser;
  bool flagEncryptToSelf : 1;

  bool havePgp : 1;
  bool havePGP5 : 1;
  bool haveGpg : 1;
  bool havePassPhrase : 1;
  bool needPublicKeys : 1;
  bool showEncryptionResult : 1;
};

// -------------------------------------------------------------------------

class KPasswordEdit;

/** the passphrase dialog */
class KpgpPass : public KDialogBase
{
  Q_OBJECT

  public:
    KpgpPass( QWidget *parent=0, const QString &caption=QString::null,
	      bool modal=true, const QString &keyID=QString::null);
    virtual ~KpgpPass();

    const char * passphrase();

  private:
    KPasswordEdit *lineedit;
};

// -------------------------------------------------------------------------
/** the key dialog */
class KpgpKey : public KDialogBase
{
  Q_OBJECT

  public:
    KpgpKey( QStrList *keys, QWidget *parent=0, const char *name=0,
	     bool modal=true );
    virtual ~KpgpKey();

    static QString getKeyName(QWidget *parent = 0, const QStrList *keys = 0 );

  private:
    QString getKey();

  private:
    QComboBox *combobox;
    QCursor *cursor;
};


// -------------------------------------------------------------------------
/** a widget for configuring the pgp interface. Can be included into
    a tabdialog. This widget by itself does not provide an apply/cancel
    button mechanism. */
class KpgpConfig : public QWidget
{
  Q_OBJECT

  public:
    KpgpConfig(QWidget *parent = 0, const char *name = 0, bool encrypt =true);
    virtual ~KpgpConfig();

    virtual void setValues();
    virtual void applySettings();

  protected:
    Kpgp *pgp;
    QCheckBox *storePass;
    QCheckBox *encToSelf;
    QCheckBox *showCipherText;
    QButtonGroup *radioGroup;
    QRadioButton *autoDetect;
    QRadioButton *useGPG;
    QRadioButton *usePGP2x;
    QRadioButton *usePGP5x;
    QRadioButton *usePGP6x;
    QRadioButton *useNoPGP;

};

// -------------------------------------------------------------------------
class KpgpSelDlg: public KDialogBase
{
  Q_OBJECT

  public:
    KpgpSelDlg( const QStrList &keyList, const QString &recipent,
		QWidget *parent=0, const char *name=0, bool modal=true );
    virtual ~KpgpSelDlg() {};

    virtual const QString key(void) const {return mkey;};

  protected slots:
    virtual void slotOk();
    virtual void slotCancel();

  private:
    QListBox *mListBox;
    QString  mkey;
    QStrList mKeyList;
};

// -------------------------------------------------------------------------
class KpgpCipherTextDlg: public KDialogBase
{
  Q_OBJECT

  public:
    KpgpCipherTextDlg( const QCString & text, const QCString & charset=0, QWidget *parent=0,
                       const char *name=0, bool modal=true );
    virtual ~KpgpCipherTextDlg() {};

  private:
    QMultiLineEdit *mEditBox;
};

#endif

