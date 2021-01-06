#undef QT_NO_ASCII_CAST
#undef QT_NO_COMPAT

#include "kpgpbase.h"
#include "kpgp.h"

#include <kapp.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include <qregexp.h>

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/poll.h>  /* for polling of GnuPG output */
#include <stdlib.h>
#include <errno.h>
#include <klocale.h>

#include <config.h>

KpgpBase::KpgpBase()
{
  status = OK;
}

KpgpBase::~KpgpBase()
{

}

bool
KpgpBase::setMessage(const QCString mess)
{
  int index;

  clear();
  input = mess;

  // "-----BEGIN PGP" must be at the beginning of a line
  if(((index = input.find("-----BEGIN PGP")) != -1) &&
     ((index == 0) || (input[index-1] == '\n'))) {
    decrypt();
    return true;
  }
  return false;
}

QCString
KpgpBase::message() const
{
  // do we have a deciphered text?
  if(!output.isEmpty()) return output;

  // no, then return the original one
  //kdDebug(5100) << "KpgpBase: No output!" << endl;
  return input;
}

int
KpgpBase::run(const char *cmd, const char *passphrase, bool onlyReadFromPGP)
{
  /* the pipe ppass is used for to pass the password to
   * pgp. passing the password together with the normal input through
   * stdin doesn't seem to work as expected (at least for pgp5.0)
   */
  char str[1025] = "\0";
  int pin[2], pout[2], perr[2], ppass[2];
  int len, len2;
  FILE *pass;
  pid_t child_pid;
  int childExitStatus;
  struct pollfd pollin, pollout, pollerr;
  int pollstatus;

  /* set the ID which one a passphrase is required for
   * to the current PGP identity
   */
  requiredID = Kpgp::getKpgp()->user();

  if(passphrase)
  {
    pipe(ppass);

    pass = fdopen(ppass[1], "w");
    fwrite(passphrase, sizeof(char), strlen(passphrase), pass);
    fwrite("\n", sizeof(char), 1, pass);
    fclose(pass);
    close(ppass[1]);

    // tell pgp which fd to use for the passphrase
    QString tmp;
    tmp.sprintf("%d",ppass[0]);
    setenv("PGPPASSFD",tmp,1);

    //Uncomment these lines for testing only! Doing so will decrease security!
    //kdDebug(5100) << "pgp PGPPASSFD = " << tmp << endl;
    //kdDebug(5100) << "pgp pass = " << passphrase << endl;
  }
  else
    unsetenv("PGPPASSFD");

  //Uncomment these lines for testing only! Doing so will decrease security!
  //kdDebug(5100) << "pgp cmd = " << cmd << endl;
  //kdDebug(5100) << "pgp input = " << QString(input)
  //          << "input length = " << input.length() << endl;

  info = "";
  output = "";

  pipe(pin);
  pipe(pout);
  pipe(perr);

  QApplication::flushX();
  if(!(child_pid = fork()))
  {
    /*We're the child.*/
    close(pin[1]);
    dup2(pin[0], 0);
    close(pin[0]);

    close(pout[0]);
    dup2(pout[1], 1);
    close(pout[1]);

    close(perr[0]);
    dup2(perr[1], 2);
    close(perr[1]);

    execl("/bin/sh", "sh", "-c", cmd,  NULL);
    _exit(127);
  }

  /*Only get here if we're the parent.*/
  close(pin[0]);
  close(pout[1]);
  close(perr[1]);

  // poll for "There is data to read."
  pollout.fd = pout[0];
  pollout.events = POLLIN;
  pollerr.fd = perr[0];
  pollerr.events = POLLIN;

  // poll for "Writing now will not block."
  pollin.fd = pin[1];
  pollin.events = POLLOUT;

  if (!onlyReadFromPGP) {
    if (!input.isEmpty()) {
      // write to pin[1] one line after the other to prevent dead lock
      for (unsigned int i=0; i<input.length(); i+=len2) {
        len2 = 0;

        // check if writing now to pin[1] will not block (5 ms timeout)
        //kdDebug(5100) << "Polling pin[1]..." << endl;
        pollstatus = poll(&pollin, 1, 5);
        if (pollstatus == 1) {
          //kdDebug(5100) << "Status for polling pin[1]: " << pollin.revents << endl;
          if (pollin.revents & POLLERR) {
            kdDebug(5100) << "PGP seems to have hung up" << endl;
            break;
          }
          else if (pollin.revents & POLLOUT) {
            // search end of next line
            if ((len2 = input.find('\n', i)) == -1)
              len2 = input.length()-i;
            else
              len2 = len2-i+1;

            //kdDebug(5100) << "Trying to write " << len2 << " bytes to pin[1] ..." << endl;
            len2 = write(pin[1], input.mid(i,len2).data(), len2);
            //kdDebug(5100) << "Wrote " << len2 << " bytes to pin[1] ..." << endl;
          }
        }
        else if (!pollstatus) {
          //kdDebug(5100) << "Timeout while polling pin[1]: "
          //              << pollin.revents << endl;
        }
        else if (pollstatus == -1) {
          kdDebug(5100) << "Error while polling pin[1]: "
                        << pollin.revents << endl;
        }

        if (pout[0] >= 0) {
          do {
            // check if there is data to read from pout[0]
            //kdDebug(5100) << "Polling pout[0]..." << endl;
            pollstatus = poll(&pollout, 1, 0);
            if (pollstatus == 1) {
              //kdDebug(5100) << "Status for polling pout[0]: " << pollout.revents << endl;
              if (pollout.revents & POLLIN) {
                //kdDebug(5100) << "Trying to read " << 1024 << " bytes from pout[0]" << endl;
                if ((len = read(pout[0],str,1024))>0) {
                  //kdDebug(5100) << "Read " << len << " bytes from pout[0]" << endl;
                  str[len] ='\0';
                  output += str;
                }
                else
                  break;
              }
            }
            else if (pollstatus == -1) {
              kdDebug(5100) << "Error while polling pout[0]: "
                            << pollout.revents << endl;
            }
          } while ((pollstatus == 1) && (pollout.revents & POLLIN));
        }

        if (perr[0] >= 0) {
          do {
            // check if there is data to read from perr[0]
            //kdDebug(5100) << "Polling perr[0]..." << endl;
            pollstatus = poll(&pollerr, 1, 0);
            if (pollstatus == 1) {
              //kdDebug(5100) << "Status for polling perr[0]: " << pollerr.revents << endl;
              if (pollerr.revents & POLLIN) {
                //kdDebug(5100) << "Trying to read " << 1024 << " bytes from perr[0]" << endl;
                if ((len = read(perr[0],str,1024))>0) {
                  //kdDebug(5100) << "Read " << len << " bytes from perr[0]" << endl;
                  str[len] ='\0';
                  info += str;
                }
                else
                  break;
              }
            }
            else if (pollstatus == -1) {
              kdDebug(5100) << "Error while polling perr[0]: "
                            << pollerr.revents << endl;
            }
          } while ((pollstatus == 1) && (pollerr.revents & POLLIN));
        }

        // abort writing to PGP if PGP hung up
        if ((pollstatus == 1) &&
            ((pollout.revents & POLLHUP) || (pollerr.revents & POLLHUP))) {
          kdDebug(5100) << "PGP hung up" << endl;
          break;
        }
      }
    }
    else // if input.isEmpty()
      write(pin[1], "\n", 1);
    //kdDebug(5100) << "All input was written to pin[1]" << endl;
  }
  close(pin[1]);

  pid_t waitpidRetVal;

  do {
    //kdDebug(5100) << "Checking if PGP is still running..." << endl;
    childExitStatus = 0;
    waitpidRetVal = waitpid(child_pid, &childExitStatus, WNOHANG);
    //kdDebug(5100) << "waitpid returned " << waitpidRetVal << endl;
    if (pout[0] >= 0) {
      do {
        // check if there is data to read from pout[0]
        //kdDebug(5100) << "Polling pout[0]..." << endl;
        pollstatus = poll(&pollout, 1, 0);
        if (pollstatus == 1) {
          //kdDebug(5100) << "Status for polling pout[0]: " << pollout.revents << endl;
          if (pollout.revents & POLLIN) {
            //kdDebug(5100) << "Trying to read " << 1024 << " bytes from pout[0]" << endl;
            if ((len = read(pout[0],str,1024))>0) {
              //kdDebug(5100) << "Read " << len << " bytes from pout[0]" << endl;
              str[len] ='\0';
              output += str;
            }
            else
              break;
          }
        }
        else if (pollstatus == -1) {
          kdDebug(5100) << "Error while polling pout[0]: "
                        << pollout.revents << endl;
        }
      } while ((pollstatus == 1) && (pollout.revents & POLLIN));
    }

    if (perr[0] >= 0) {
      do {
        // check if there is data to read from perr[0]
        //kdDebug(5100) << "Polling perr[0]..." << endl;
        pollstatus = poll(&pollerr, 1, 0);
        if (pollstatus == 1) {
          //kdDebug(5100) << "Status for polling perr[0]: " << pollerr.revents << endl;
          if (pollerr.revents & POLLIN) {
            //kdDebug(5100) << "Trying to read " << 1024 << " bytes from perr[0]" << endl;
            if ((len = read(perr[0],str,1024))>0) {
              //kdDebug(5100) << "Read " << len << " bytes from perr[0]" << endl;
              str[len] ='\0';
              info += str;
            }
            else
              break;
          }
        }
        else if (pollstatus == -1) {
          kdDebug(5100) << "Error while polling perr[0]: "
                        << pollerr.revents << endl;
        }
      } while ((pollstatus == 1) && (pollerr.revents & POLLIN));
    }
  } while (waitpidRetVal == 0);

  close(pout[0]);
  close(perr[0]);

  unsetenv("PGPPASSFD");
  if(passphrase)
    close(ppass[0]);

  // Did the child exit normally?
  if (WIFEXITED(childExitStatus) != 0) {
    // Get the return code of the child
    childExitStatus = WEXITSTATUS(childExitStatus);
    kdDebug(5100) << "PGP exited with exit status " << childExitStatus 
                  << endl;
  }
  else {
    childExitStatus = -1;
    kdDebug(5100) << "PGP exited abnormally!" << endl;
  }

  //Uncomment these lines for testing only! Doing so will decrease security!
  //kdDebug(5100) << "pgp output = " << QString(output) << endl;
  //kdDebug(5100) << "pgp info = " << info << endl;

  /* Make the information visible, so that a user can
   * get to know what's going on during the pgp calls.
   */
  kdDebug(5100) << info << endl;

  return childExitStatus;
}

int
KpgpBase::runGpg(const char *cmd, const char *passphrase, bool onlyReadFromGnuPG)
{
  /* the pipe ppass is used for to pass the password to
   * pgp. passing the password together with the normal input through
   * stdin doesn't seem to work as expected (at least for pgp5.0)
   */
  char str[1025] = "\0";
  int pin[2], pout[2], perr[2], ppass[2];
  int len, len2;
  FILE *pass;
  pid_t child_pid;
  int childExitStatus;
  char gpgcmd[1024] = "\0";
  struct pollfd pollin, pollout, pollerr;
  int pollstatus;

  /* set the ID which one a passphrase is required for
   * to currently nothing
   */
  requiredID = QString::null;

  if(passphrase)
  {
    pipe(ppass);

    pass = fdopen(ppass[1], "w");
    fwrite(passphrase, sizeof(char), strlen(passphrase), pass);
    fwrite("\n", sizeof(char), 1, pass);
    fclose(pass);
    close(ppass[1]);

    //Uncomment these lines for testing only! Doing so will decrease security!
    //kdDebug(5100) << "pass = " << passphrase << endl;
  }

  //Uncomment these lines for testing only! Doing so will decrease security!
  //kdDebug(5100) << "pgp cmd = " << cmd << endl;
  //kdDebug(5100) << "pgp input = " << QString(input)
  //          << "input length = " << input.length() << endl;

  info = "";
  output = "";

  pipe(pin);
  pipe(pout);
  pipe(perr);


    if(passphrase) {
      snprintf(gpgcmd, 1023, "LANGUAGE=C gpg --passphrase-fd %d %s",ppass[0],cmd);
    } else {
      snprintf(gpgcmd, 1023, "LANGUAGE=C gpg %s",cmd);
    }

  QApplication::flushX();
  if(!(child_pid = fork()))
  {
    /*We're the child.*/
    close(pin[1]);
    dup2(pin[0], 0);
    close(pin[0]);

    close(pout[0]);
    dup2(pout[1], 1);
    close(pout[1]);

    close(perr[0]);
    dup2(perr[1], 2);
    close(perr[1]);

    //#warning FIXME: there has to be a better way to do this
     /* this is nasty nasty nasty (but it works) */
    if(passphrase) {
      snprintf(gpgcmd, 1023, "LANGUAGE=C gpg --passphrase-fd %d %s",ppass[0],cmd);
    } else {
      snprintf(gpgcmd, 1023, "LANGUAGE=C gpg %s",cmd);
    }

    //kdDebug(5100) << "pgp cmd = " << gpgcmd << endl;

    execl("/bin/sh", "sh", "-c", gpgcmd,  NULL);
    _exit(127);
  }

  /*Only get here if we're the parent.*/
  close(pin[0]);
  close(pout[1]);
  close(perr[1]);

  // poll for "There is data to read."
  pollout.fd = pout[0];
  pollout.events = POLLIN;
  pollerr.fd = perr[0];
  pollerr.events = POLLIN;

  // poll for "Writing now will not block."
  pollin.fd = pin[1];
  pollin.events = POLLOUT;

  if (!onlyReadFromGnuPG) {
    if (!input.isEmpty()) {
      // write to pin[1] one line after the other to prevent dead lock
      for (unsigned int i=0; i<input.length(); i+=len2) {
        len2 = 0;

        // check if writing now to pin[1] will not block (5 ms timeout)
        //kdDebug(5100) << "Polling pin[1]..." << endl;
        pollstatus = poll(&pollin, 1, 5);
        if (pollstatus == 1) {
          //kdDebug(5100) << "Status for polling pin[1]: " << pollin.revents << endl;
          if (pollin.revents & POLLERR) {
            kdDebug(5100) << "GnuPG seems to have hung up" << endl;
            break;
          }
          else if (pollin.revents & POLLOUT) {
            // search end of next line
            if ((len2 = input.find('\n', i)) == -1)
              len2 = input.length()-i;
            else
              len2 = len2-i+1;

            //kdDebug(5100) << "Trying to write " << len2 << " bytes to pin[1] ..." << endl;
            len2 = write(pin[1], input.mid(i,len2).data(), len2);
            //kdDebug(5100) << "Wrote " << len2 << " bytes to pin[1] ..." << endl;
          }
        }
        else if (!pollstatus) {
          //kdDebug(5100) << "Timeout while polling pin[1]: "
          //              << pollin.revents << endl;
        }
        else if (pollstatus == -1) {
          kdDebug(5100) << "Error while polling pin[1]: "
                        << pollin.revents << endl;
        }

        if (pout[0] >= 0) {
          do {
            // check if there is data to read from pout[0]
            //kdDebug(5100) << "Polling pout[0]..." << endl;
            pollstatus = poll(&pollout, 1, 0);
            if (pollstatus == 1) {
              //kdDebug(5100) << "Status for polling pout[0]: " << pollout.revents << endl;
              if (pollout.revents & POLLIN) {
                //kdDebug(5100) << "Trying to read " << 1024 << " bytes from pout[0]" << endl;
                if ((len = read(pout[0],str,1024))>0) {
                  //kdDebug(5100) << "Read " << len << " bytes from pout[0]" << endl;
                  str[len] ='\0';
                  output += str;
                }
                else break;
              }
            }
            else if (pollstatus == -1) {
              kdDebug(5100) << "Error while polling pout[0]: "
                            << pollout.revents << endl;
            }
          } while ((pollstatus == 1) && (pollout.revents & POLLIN));
        }

        if (perr[0] >= 0) {
          do {
            // check if there is data to read from perr[0]
            //kdDebug(5100) << "Polling perr[0]..." << endl;
            pollstatus = poll(&pollerr, 1, 0);
            if (pollstatus == 1) {
              //kdDebug(5100) << "Status for polling perr[0]: " << pollerr.revents << endl;
              if (pollerr.revents & POLLIN) {
                //kdDebug(5100) << "Trying to read " << 1024 << " bytes from perr[0]" << endl;
                if ((len = read(perr[0],str,1024))>0) {
                  //kdDebug(5100) << "Read " << len << " bytes from perr[0]" << endl;
                  str[len] ='\0';
                  info += str;
                }
                else break;
              }
            }
            else if (pollstatus == -1) {
              kdDebug(5100) << "Error while polling perr[0]: "
                            << pollerr.revents << endl;
            }
          } while ((pollstatus == 1) && (pollerr.revents & POLLIN));
        }

        // abort writing to GnuPG if GnuPG hung up
        if ((pollstatus == 1) &&
            ((pollout.revents & POLLHUP) || (pollerr.revents & POLLHUP))) {
          kdDebug(5100) << "GnuPG hung up" << endl;
          break;
        }
      }
    }
    else
      write(pin[1], "\n", 1);
    //kdDebug(5100) << "All input was written to pin[1]" << endl;
  }
  close(pin[1]);

  pid_t waitpidRetVal;

  do {
    //kdDebug(5100) << "Checking if GnuPG is still running..." << endl;
    childExitStatus = 0;
    waitpidRetVal = waitpid(child_pid, &childExitStatus, WNOHANG);
    //kdDebug(5100) << "waitpid returned " << waitpidRetVal << endl;
    if (pout[0] >= 0) {
      do {
        // check if there is data to read from pout[0]
        //kdDebug(5100) << "Polling pout[0]..." << endl;
        pollstatus = poll(&pollout, 1, 0);
        if (pollstatus == 1) {
          //kdDebug(5100) << "Status for polling pout[0]: " << pollout.revents << endl;
          if (pollout.revents & POLLIN) {
            //kdDebug(5100) << "Trying to read " << 1024 << " bytes from pout[0]" << endl;
            if ((len = read(pout[0],str,1024))>0) {
              //kdDebug(5100) << "Read " << len << " bytes from pout[0]" << endl;
              str[len] ='\0';
              output += str;
            }
            else break;
          }
        }
        else if (pollstatus == -1) {
          kdDebug(5100) << "Error while polling pout[0]: "
                        << pollout.revents << endl;
        }
      } while ((pollstatus == 1) && (pollout.revents & POLLIN));
    }

    if (perr[0] >= 0) {
      do {
        // check if there is data to read from perr[0]
        //kdDebug(5100) << "Polling perr[0]..." << endl;
        pollstatus = poll(&pollerr, 1, 0);
        if (pollstatus == 1) {
          //kdDebug(5100) << "Status for polling perr[0]: " << pollerr.revents << endl;
          if (pollerr.revents & POLLIN) {
            //kdDebug(5100) << "Trying to read " << 1024 << " bytes from perr[0]" << endl;
            if ((len = read(perr[0],str,1024))>0) {
              //kdDebug(5100) << "Read " << len << " bytes from perr[0]" << endl;
              str[len] ='\0';
              info += str;
            }
            else break;
          }
        }
        else if (pollstatus == -1) {
          kdDebug(5100) << "Error while polling perr[0]: "
                        << pollerr.revents << endl;
        }
      } while ((pollstatus == 1) && (pollerr.revents & POLLIN));
    }
  } while (waitpidRetVal == 0);

  close(pout[0]);
  close(perr[0]);

  if(passphrase)
    close(ppass[0]);

  // Did the child exit normally?
  if (WIFEXITED(childExitStatus) != 0) {
    // Get the return code of the child
    childExitStatus = WEXITSTATUS(childExitStatus);
    kdDebug(5100) << "GnuPG exited with exit status " << childExitStatus 
                  << endl;
  }
  else {
    childExitStatus = -1;
    kdDebug(5100) << "GnuPG exited abnormally!" << endl;
  }

  //Uncomment these lines for testing only! Doing so will decrease security!
  //kdDebug(5100) << "pgp output = " << QString(output) << endl;
  //kdDebug(5100) << "pgp info = " << info << endl;

  /* Make the information visible, so that a user can
   * get to know what's going on during the gpg calls.
   */
  kdDebug(5100) << info << endl;

  return childExitStatus;
}

QString
KpgpBase::addUserId()
{
  QString cmd;
  QString pgpUser = Kpgp::getKpgp()->user();

  if(!pgpUser.isEmpty())
  {
    cmd += " -u \"";
    cmd += pgpUser;
    cmd += "\"";
    return cmd;
  }
  return "";
}

void
KpgpBase::clear()
{
  input = QCString();
  output = QCString();
  info = QString::null;
  errMsg = QString::null;
  signature = QString::null;
  signatureID = QString::null;
  recipients.clear();
  status = OK;
}

void
KpgpBase::clearOutput()
{
  output = QString::null;
}

QString
KpgpBase::lastErrorMessage() const
{
  return errMsg;
}

// -------------------------------------------------------------------------

KpgpBaseG::KpgpBaseG()
  : KpgpBase()
{
}

KpgpBaseG::~KpgpBaseG()
{
}

int
KpgpBaseG::encrypt(const QStrList *_recipients, bool /*ignoreUntrusted*/)
{
  return encsign(_recipients, 0);
}

int
KpgpBaseG::sign(const char *passphrase)
{
  return encsign(0, passphrase);
}

int
KpgpBaseG::encsign(const QStrList *_recipients, const char *passphrase,
		   bool /*ignoreUntrusted*/)
{
  QString cmd, pers;
  int exitStatus = 0;
  output = "";

  if(_recipients != 0)
    if(_recipients->count() <= 0)
      _recipients = 0;

  if(_recipients != 0 && passphrase != 0)
    cmd = "--batch --escape-from --armor --always-trust --sign --encrypt --textmode ";
  else if( _recipients != 0 )
    cmd = "--batch --escape-from --armor --always-trust --encrypt --textmode ";
  else if(passphrase != 0 )
    cmd = "--batch --escape-from --armor --always-trust --clearsign ";
  else
  {
    kdDebug(5100) << "kpgpbase: Neither recipients nor passphrase specified." << endl;
    return OK;
  }

  if(passphrase != 0)
    cmd += addUserId();

  if(_recipients != 0)
  {
    QStrListIterator it(*_recipients);
    while( (pers=it.current()) )
    {
      cmd += " --recipient \"";
      cmd += pers;
      cmd += "\" ";
      ++it;
    }
    if(Kpgp::getKpgp()->encryptToSelf()) {
      cmd += " --recipient \"";
      cmd += Kpgp::getKpgp()->user();
      cmd += "\" ";
    }
    cmd += " --set-filename stdin ";
  }

  status = 0;
  exitStatus = runGpg(cmd, passphrase);

  if(exitStatus != 0)
    status = ERROR;

  if(_recipients != 0)
  {
    int index = 0;
    bool bad = FALSE;
    unsigned int num = 0;
    QString badkeys = "";
    while((index = info.find("Cannot find the public key",index))
	  != -1)
    {
      bad = TRUE;
      index = info.find("'",index);
      int index2 = info.find("'",index+1);
      badkeys += info.mid(index, index2-index+1) + ", ";
      num++;
    }
    if(bad)
    {
      badkeys.stripWhiteSpace();
      if(num == _recipients->count())
	errMsg.sprintf("Could not find public keys matching the\n"
		       "userid(s) %s.\n"
		       "Message is not encrypted.\n",
		       (const char *)badkeys);
      else
	errMsg.sprintf("Could not find public keys matching the\n"
		       "userid(s) %s. These persons won't be able\n"
		       "to read the message.",
		       (const char *)badkeys);
      status |= MISSINGKEY;
      status |= ERROR;
    }
  }
  if(passphrase != 0)
  {
    if(info.find("Pass phrase is good") != -1)
    {
      //kdDebug(5100) << "KpgpBase: Good Passphrase!" << endl;
      status |= SIGNED;
    }
    if( info.find("bad passphrase") != -1)
    {
      errMsg = i18n("Bad passphrase; couldn't sign");
      status |= BADPHRASE;
      status |= ERR_SIGNING;
      status |= ERROR;
    }
  }
  //kdDebug(5100) << "status = " << status << endl;
  return status;
}

int
KpgpBaseG::decrypt(const char *passphrase)
{
  QString cmd;
  int index, index2;
  int exitStatus = 0;
  output = "";

  //if(((index = input.find("-----BEGIN PGP SIGNED MESSAGE-----")) != -1) &&
  //   ((index == 0) || (input[index-1] == '\n'))) {
  //      cmd = "--batch --set-filename stdin --verify";
  //}
  //else
  //    cmd = "--batch --set-filename stdin --decrypt";
  cmd = "--batch --decrypt";

  status = 0;
  exitStatus = runGpg(cmd, passphrase);

  if(exitStatus == -1) {
    errMsg = i18n("error running gpg");
    status = RUN_ERR;
    return status;
  }

  if( info.find("File contains key") != -1)
  {
    // FIXME: should do something with it...
  }

  if ((info.find("secret key not available") != -1)
      || ((info.find("key not found") != -1) && (info.find("Can't check signature") == -1)))
  {
    //kdDebug(5100) << "kpgpbase: message is encrypted" << endl;
    status |= ENCRYPTED;
    if((index = info.find("bad passphrase")) != -1)
    {
      if(passphrase != 0)
      {
	errMsg = i18n("Bad pass Phrase; couldn't decrypt");
	kdDebug(5100) << "KpgpBase: passphrase is bad" << endl;
	status |= BADPHRASE;
	status |= ERROR;
      }
      else {
	// Search backwards the user ID of the needed key
	index2 = info.findRev("\"", index) - 1;
	index = info.findRev("      \"", index2) + 7;
	// The conversion from UTF8 is necessary because gpg stores and
	// prints user IDs in UTF8
	requiredID = QString::fromUtf8(info.mid(index, index2 - index + 1));
	kdDebug(5100) << "KpgpBase: key needed is \"" << requiredID << "\"!" << endl;
      }
    }
    else
    {
      // no secret key fitting this message
      status |= NO_SEC_KEY;
      status |= ERROR;
      errMsg = i18n("Do not have the secret key for this message");
      kdDebug(5100) << "KpgpBase: no secret key for this message" << endl;
    }
    // check for persons
    index = info.find("can only be read by:");
    if(index != -1)
    {
      index = info.find("\n",index);
      int end = info.find("\n\n",index);

      recipients.clear();
      while( (index2 = info.find("\n",index+1)) <= end )
      {
	QString item = info.mid(index+1,index2-index-1);
	item.stripWhiteSpace();
	recipients.append(item);
	index = index2;
      }
    }
  }
  if((index = info.find("Signature made")) != -1)
  {
    //kdDebug(5100) << "KpgpBase: message is signed" << endl;
    status |= SIGNED;
    if ((info.find("Key matching expected") != -1)
        || (info.find("Can't check signature") != -1))
    {
      index = info.find("key ID ",index);
      signatureID = info.mid(index+7,8);
      signature = QString("unknown key ID %1 ").arg(signatureID);
      status |= UNKNOWN_SIG;
      status |= GOODSIG;
    }
    else if( info.find("Good signature") != -1 )
    {
      status |= GOODSIG;
      // get signer
      index = info.find("\"",index);
      index2 = info.find("\"", index+1);
      signature = info.mid(index+1, index2-index-1);

      // get key ID of signer
      index = info.find("key ID ",index2);
      signatureID = info.mid(index+7,8);
    }
    else if( info.find("BAD signature") != -1 )
    {
      //kdDebug(5100) << "BAD signature" << endl;
      status |= SIGNED;
      status |= ERROR;

      // get signer
      index = info.find("\"",index);
      index2 = info.find("\"", index+1);
      signature = info.mid(index+1, index2-index-1);

      // get key ID of signer
      index = info.find("key ID ",index2);
      signatureID = info.mid(index+7,8);
    }
    else if( info.find("Can't find the right public key") != -1 )
    {
      status |= UNKNOWN_SIG;
      status |= GOODSIG; // this is a hack...
      signature = i18n("??? (file ~/.gnupg/pubring.gpg not found)");
      signatureID = "???";
    }
    else
    {
      status |= ERROR;
      signature = "";
      signatureID = "";
    }
  }
  //kdDebug(5100) << "status = " << status << endl;
  return status;
}

QStrList
KpgpBaseG::pubKeys()
{
  QString cmd;
  int index, index2;
  int exitStatus = 0;

  cmd = "--batch --list-keys";

  status = 0;
  exitStatus = runGpg(cmd,0,true);

  if(exitStatus != 0) {
    status = ERROR;
    return 0;
  }

  // now we need to parse the output
  QStrIList publicKeys;
  index = output.find("\n",1)+1; // skip first to "\n"
  while( (index = output.find("\n",index)) != -1)
  {
    //parse line
    QString line;
    if( (index2 = output.find("\n",index+1)) != -1)
    {
      int index3 = output.find("pub ",index);
      int index4 = output.find("uid ",index);
      if ((index4 != -1) && ((index4 < index3) || (index3 == -1)))
        index3 = index4;

      if( (index3 <index2) && (index3 != -1) )
      {
	line = output.mid(index3+31,index2-index3-31);
      }
      if(!line.isEmpty())
      {
	//kdDebug(5100) << "KpgpBase: found key for " << (const char *)line << "." << endl;
	publicKeys.append(line);
      }
    }
    else
      break;
    index = index2;
  }
  // sort the key list
  publicKeys.sort();
  //kdDebug(5100) << "finished reading keys" << endl;
  return publicKeys;
}

int
KpgpBaseG::signKey(const char *key, const char *passphrase)
{
  QString cmd;
  int exitStatus = 0;

  cmd = "--set-filename stdin ";
  cmd += addUserId();
  cmd += "--sign-key ";
  cmd += key;

  status = 0;
  exitStatus = runGpg(cmd,passphrase);

  if (exitStatus != 0)
    status = ERROR;

  return status;
}


QString KpgpBaseG::getAsciiPublicKey(QString _person)
{
  int exitStatus = 0;

  if (_person.isEmpty()) return _person;
  QCString toexec;
  toexec.sprintf("--batch --armor --export \"%s\"", _person.local8Bit().data());

  status = 0;
  exitStatus = runGpg(toexec.data(),0,true);
  if(exitStatus != 0) {
    status = ERROR;
    return QString::null;
  }

  return output;
}


// -------------------------------------------------------------------------

KpgpBase2::KpgpBase2()
  : KpgpBase()
{
}

KpgpBase2::~KpgpBase2()
{
}

int
KpgpBase2::encrypt(const QStrList *_recipients, bool /*ignoreUntrusted*/)
{
  return encsign(_recipients, 0);
}

int
KpgpBase2::sign(const char *passphrase)
{
  return encsign(0, passphrase);
}

int
KpgpBase2::encsign(const QStrList *_recipients, const char *passphrase,
		   bool /*ignoreUntrusted*/)
{
  QString cmd, pers;
  int exitStatus = 0;
  output = "";

  if(_recipients != 0)
    if(_recipients->count() <= 0)
      _recipients = 0;

  if(_recipients != 0 && passphrase != 0)
    cmd = "pgp +batchmode +language=en -seat ";
  else if( _recipients != 0 )
    cmd = "pgp +batchmode +language=en -eat";
  else if(passphrase != 0 )
    cmd = "pgp +batchmode +language=en -sat ";
  else
  {
    kdDebug(5100) << "kpgpbase: Neither recipients nor passphrase specified." << endl;
    return OK;
  }

  if(passphrase != 0)
    cmd += addUserId();

  if(_recipients != 0)
  {
    QStrListIterator it(*_recipients);
    while( (pers=it.current()) )
    {
      cmd += " \"";
      cmd += pers;
      cmd += "\"";
      ++it;
    }
    if(Kpgp::getKpgp()->encryptToSelf())
      cmd += " +EncryptToSelf";
  }
  cmd += " -f";

  status = 0;
  exitStatus = run(cmd, passphrase);

  if(exitStatus != 0)
    status = ERROR;

  if(_recipients != 0)
  {
    int index = 0;
    bool bad = FALSE;
    unsigned int num = 0;
    QString badkeys = "";
    if (info.find("Cannot find the public key") != -1)
    {
      index = 0;
      num = 0;
      while((index = info.find("Cannot find the public key",index))
	    != -1)
      {
        bad = TRUE;
        index = info.find("'",index);
        int index2 = info.find("'",index+1);
        if (num++)
          badkeys += ", ";
        badkeys += info.mid(index, index2-index+1);
      }
      if(bad)
      {
        badkeys.stripWhiteSpace();
        if(num == _recipients->count())
	  errMsg.sprintf("Could not find public keys matching the\n"
        	         "userid(s) %s.\n"
		         "Message is not encrypted.\n",
		         (const char *)badkeys);
        else
          errMsg.sprintf("Could not find public keys matching the\n"
		         "userid(s) %s. These persons won't be able\n"
		         "to read the message.",
		         (const char *)badkeys);
        status |= MISSINGKEY;
        status |= ERROR;
      }
    }
    if (info.find("skipping userid") != -1)
    {
      index = 0;
      num = 0;
      while((index = info.find("skipping userid",index))
	    != -1)
      {
        bad = TRUE;
        int index2 = info.find("\n",index+16);
        if (num++)
          badkeys += ", ";
        badkeys += info.mid(index+16, index2-index-16);
        index = index2;
      }
      if(bad)
      {
        badkeys.stripWhiteSpace();
        if(num == _recipients->count())
	  errMsg.sprintf("Public keys not certified with trusted signature for\n"
	                 "userid(s) %s.\n"
		         "Message is not encrypted.\n",
		         (const char *)badkeys);
        else
	  errMsg.sprintf("Public keys not certified with trusted signature for\n"
		         "userid(s) %s. These persons won't be able\n"
		         "to read the message.",
		         (const char *)badkeys);
        status |= BADKEYS;
        status |= ERROR;
        return status;
      }
    }
  }
  if(passphrase != 0)
  {
    if(info.find("Pass phrase is good") != -1)
    {
      //kdDebug(5100) << "KpgpBase: Good Passphrase!" << endl;
      status |= SIGNED;
    }
    if( info.find("Bad pass phrase") != -1)
    {
      errMsg = i18n("Bad passphrase; couldn't sign");
      status |= BADPHRASE;
      status |= ERR_SIGNING;
      status |= ERROR;
    }
  }
  if (info.find("Encryption error") != -1)
  {
    errMsg = i18n("PGP error occured. Please check\nyour PGP setup and key rings.");
    status |= NO_SEC_KEY;
    status |= BADKEYS;
    status |= ERROR;
  }
  //kdDebug(5100) << "status = " << status << endl;
  return status;
}

int
KpgpBase2::decrypt(const char *passphrase)
{
  QString cmd;
  int index, index2;
  int exitStatus = 0;
  output = "";


  cmd = "pgp +batchmode +language=en -f";

  status = 0;
  exitStatus = run(cmd, passphrase);

  // pgp2.6 has sometimes problems with the ascii armor pgp5.0 produces
  // this hack can solve parts of the problem
  if(info.find("ASCII armor corrupted.") != -1)
  {
    kdDebug(5100) << "removing ASCII armor header" << endl;
    int index1 = input.find("-----BEGIN PGP SIGNED MESSAGE-----");
    if(index1 != -1)
      index1 = input.find("-----BEGIN PGP SIGNATURE-----", index1);
    else
      index1 = input.find("-----BEGIN PGP MESSAGE-----");
    index1 = input.find("\n", index1);
    index2 = input.find("\n\n", index1);
    input.remove(index1, index2 - index1);
    exitStatus = run(cmd, passphrase);
  }

  if(exitStatus == -1) {
    errMsg = i18n("error running PGP");
    status = RUN_ERR;
    return status;
  }

  if( info.find("File contains key") != -1)
  {
    // FIXME: should do something with it...
  }

  if(info.find("You do not have the secret key") != -1)
  {
    //kdDebug(5100) << "kpgpbase: message is encrypted" << endl;
    status |= ENCRYPTED;
    if( info.find("Bad pass phrase") != -1)
    {
      if(passphrase != 0)
      {
	errMsg = i18n("Bad passphrase; couldn't decrypt");
	kdDebug(5100) << "KpgpBase: passphrase is bad" << endl;
	status |= BADPHRASE;
	status |= ERROR;
      }
    }
    else
    {
      // no secret key fitting this message
      status |= NO_SEC_KEY;
      status |= ERROR;
      errMsg = i18n("Do not have the secret key for this message");
      kdDebug(5100) << "KpgpBase: no secret key for this message" << endl;
    }
    // check for persons
    index = info.find("can only be read by:");
    if(index != -1)
    {
      index = info.find("\n",index);
      int end = info.find("\n\n",index);

      recipients.clear();
      while( (index2 = info.find("\n",index+1)) <= end )
      {
	QString item = info.mid(index+1,index2-index-1);
	item.stripWhiteSpace();
	recipients.append(item);
	index = index2;
      }
    }
  }
  if((index = info.find("File has signature")) != -1)
  {
    //kdDebug(5100) << "KpgpBase: message is signed" << endl;
    status |= SIGNED;
    if( info.find("Key matching expected") != -1)
    {
      index = info.find("Key ID ",index);
      signatureID = info.mid(index+7,8);
      signature = QString("unknown key ID %1 ").arg(signatureID);
      status |= UNKNOWN_SIG;
      status |= GOODSIG;
    }
    else if( info.find("Good signature") != -1 )
    {
      status |= GOODSIG;
      // get signer
      index = info.find("\"",index);
      index2 = info.find("\"", index+1);
      signature = info.mid(index+1, index2-index-1);

      // get key ID of signer
      index = info.find("key ID ",index2);
      signatureID = info.mid(index+7,8);
    }
    else if( info.find("Can't find the right public key") != -1 )
    {
      status |= UNKNOWN_SIG;
      status |= GOODSIG; // this is a hack...
      signature = i18n("??? (file ~/.pgp/pubring.pgp not found)");
      signatureID = "???";
    }
    else
    {
      status |= ERROR;
      signature = "";
      signatureID = "";
    }
  }
  //kdDebug(5100) << "status = " << status << endl;
  return status;
}

QStrList
KpgpBase2::pubKeys()
{
  QString cmd;
  int index, index2;
  int exitStatus = 0;

  cmd = "pgp +batchmode +language=en -kv -f";

  status = 0;
  exitStatus = run(cmd,0,true);

  if(exitStatus != 0) {
    status = ERROR;
    return 0;
  }

  //truncate trailing "\n"
  if (output.length() > 1) output.truncate(output.length()-1);

  QStrList publicKeys;
  index = output.find("\n",1)+1; // skip first to "\n"
  while( (index = output.find("\n",index)) != -1)
  {
    //parse line
    QString line;
    if( (index2 = output.find("\n",index+1)) != -1)
      // skip last line
    {
      int index3 = output.find("pub ",index);

      if( (index3 >index2) || (index3 == -1) )
      {
	// second adress for the same key
	line = output.mid(index+1,index2-index-1);
	line = line.stripWhiteSpace();
      } else {
	// line with new key
	int index3 = output.find(
	  QRegExp("/[0-9][0-9]/[0-9][0-9] "),
	  index);
	line = output.mid(index3+7,index2-index3-7);
      }
      //kdDebug(5100) << "KpgpBase: found key for " << (const char *)line << endl;
      publicKeys.append(line);
    }
    else
      break;
    index = index2;
  }

  return publicKeys;
}

int
KpgpBase2::signKey(const char *key, const char *passphrase)
{
  QString cmd;
  int exitStatus = 0;

  cmd = "pgp +batchmode +language=en -ks -f";
  cmd += addUserId();
  if(passphrase != 0)
  {
    QString aStr;
    aStr.sprintf(" \"-z%s\"",passphrase);
    cmd += aStr;
  }
  cmd += key;

  status = 0;
  exitStatus = run(cmd);

  if (exitStatus != 0)
    status = ERROR;

  return status;
}


QString KpgpBase2::getAsciiPublicKey(QString _person)
{
  int exitStatus = 0;

  if (_person.isEmpty()) return _person;
  QCString toexec;
  toexec.sprintf("pgp +batchmode +force +language=en -kxaf \"%s\"", _person.local8Bit().data());

  status = 0;
  exitStatus = run(toexec.data(),0,true);

  if(exitStatus != 0) {
    status = ERROR;
    return QString::null;
  }

  return output;
}


// -------------------------------------------------------------------------

KpgpBase5::KpgpBase5()
  : KpgpBase()
{
}
KpgpBase5::~KpgpBase5()
{
}

int
KpgpBase5::encrypt(const QStrList *_recipients, bool ignoreUntrusted)
{
  return encsign(_recipients, 0, ignoreUntrusted);
}

int
KpgpBase5::sign(const char *passphrase)
{
  return encsign( 0, passphrase);
}

int
KpgpBase5::encsign(const QStrList *_recipients, const char *passphrase,
		   bool ignoreUntrusted)
{
  QString in,cmd,pers;
  int exitStatus = 0;
  int index;
  // used to work around a bug in pgp5. pgp5 treats files
  // with non ascii chars (umlauts, etc...) as binary files, but
  // we want a clear signature
  bool signonly = false;

  output = "";

  if(_recipients != 0)
    if(_recipients->isEmpty())
      _recipients = 0;

  if(_recipients != 0 && passphrase != 0)
    cmd = "pgpe -ats -f +batchmode=1";
  else if( _recipients != 0 )
    cmd = "pgpe -at -f +batchmode=1 ";
  else if(passphrase != 0 )
  {
    cmd = "pgps -bat -f +batchmode=1 ";
    signonly = true;
  }
  else
  {
    errMsg = "Neither recipients nor passphrase specified.";
    return OK;
  }

  if(ignoreUntrusted) cmd += " +NoBatchInvalidKeys=off";

  if(passphrase != 0)
    cmd += addUserId();

  if(_recipients != 0)
  {
    QStrListIterator it(*_recipients);
    while( (pers=it.current()) )
    {
      cmd += " -r \"";
      cmd += pers;
      cmd += "\"";
      ++it;
    }
    if(Kpgp::getKpgp()->encryptToSelf())
      cmd += " +EncryptToSelf";
  }

  if (signonly)
  {
    input.append("\n");
    input.replace(QRegExp("[ \t]+\n"), "\n");   //strip trailing whitespace
  }
  //We have to do this otherwise it's all in vain


  status = 0;
  exitStatus = run(cmd, passphrase);

  if(exitStatus != 0)
    status = ERROR;

  // now parse the returned info
  if(info.find("Cannot unlock private key") != -1)
  {
    errMsg = i18n("The passphrase you entered is invalid.");
    status |= ERROR;
    status |= BADPHRASE;
  }
  if(!ignoreUntrusted)
  {
    QString aStr;
    index = -1;
    while((index = info.find("WARNING: The above key",index+1)) != -1)
    {
      int index2 = info.find("But you previously",index);
      int index3 = info.find("WARNING: The above key",index+1);
      if(index2 == -1 || (index2 > index3 && index3 != -1))
      {
	// the key wasn't valid, no encryption to this person
	// extract the person
	index2 = info.find("\n",index);
	index3 = info.find("\n",index2+1);
	aStr += info.mid(index2+1, index3-index2-1);
	aStr += ", ";
      }
    }
    if(!aStr.isEmpty())
    {
      aStr.truncate(aStr.length()-2);
      if(info.find("No valid keys found") != -1)
	errMsg = i18n("The key(s) you want to encrypt your message\n"
		      "to are not trusted. No encryption done.");
      else
	errMsg = i18n("The following key(s) are not trusted:\n%1\n"
			    "They will not be able to decrypt the message.")
		       .arg(aStr);
      status |= ERROR;
      status |= BADKEYS;
    }
  }
  if((index = info.find("No encryption keys found for")) != -1)
  {
    index = info.find(":",index);
    int index2 = info.find("\n",index);

    errMsg.sprintf("Missing encryption key(s) for: %s",
		   info.mid(index,index2-index).latin1());
    status |= ERROR;
    status |= MISSINGKEY;
  }

  if(signonly) {
    // dash-escape the input
    if (input[0] == '-')
      input = "- " + input;
    input.replace(QRegExp("\n-"), "\n- -");

    output = "-----BEGIN PGP SIGNED MESSAGE-----\n\n" + input + "\n" + output;
  }

  return status;
}

int
KpgpBase5::decrypt(const char *passphrase)
{
  int exitStatus = 0;
  QString in = "";
  output = "";

  status = 0;
  exitStatus = run("pgpv -f +batchmode=1", passphrase);

  if(exitStatus == -1) {
    errMsg = i18n("error running PGP");
    status = RUN_ERR;
    return status;
  }

  // lets parse the returned information.
  int index;

  index = info.find("Cannot decrypt message");
  if(index != -1)
  {
    //kdDebug(5100) << "message is encrypted" << endl;
    status |= ENCRYPTED;

    // ok. we have an encrypted message. Is the passphrase bad,
    // or do we not have the secret key?
    if(info.find("Need a pass phrase") != -1)
    {
      if(passphrase != 0)
      {
	errMsg = i18n("Bad passphrase; couldn't decrypt");
	kdDebug(5100) << "KpgpBase: passphrase is bad" << endl;
	status |= BADPHRASE;
	status |= ERROR;
      }
    }
    else
    {
      // we don't have the secret key
      status |= NO_SEC_KEY;
      status |= ERROR;
      errMsg = i18n("Do not have the secret key for this message");
      kdDebug(5100) << "KpgpBase: no secret key for this message" << endl;
    }
    // check for persons
    index = info.find("can only be decrypted by:");
    if(index != -1)
    {
      index = info.find("\n",index);
      int end = info.find("\n\n",index);

      recipients.clear();
      int index2;
      while( (index2 = info.find("\n",index+1)) <= end )
      {
	QString item = info.mid(index+1,index2-index-1);
	item.stripWhiteSpace();
	recipients.append(item);
	index = index2;
      }
    }
  }
  index = info.find("Good signature");
  if(index != -1)
  {
    //kdDebug(5100) << "good signature" << endl;
    status |= SIGNED;
    status |= GOODSIG;

    // get key ID of signer
    index = info.find("Key ID ");
    int index2 = info.find(",",index);
    signatureID = info.mid(index+7,index2-index-8);

    // get signer
    index = info.find("\"",index);
    index2 = info.find("\"", index+1);
    signature = info.mid(index+1, index2-index-1);
  }
  index = info.find("BAD signature");
  if(index != -1)
  {
    //kdDebug(5100) << "BAD signature" << endl;
    status |= SIGNED;
    status |= ERROR;

    // get key ID of signer
    index = info.find("Key ID ");
    int index2 = info.find(",",index);
    signatureID = info.mid(index+7,index2-index-8);

    // get signer
    index = info.find("\"",index);
    index2 = info.find("\"", index+1);
    signature = info.mid(index+1, index2-index-1);
  }
  index = info.find("Signature by unknown key");
  if(index != -1)
  {
    index = info.find("keyid: ",index);
    int index2 = info.find("\n",index);
    signatureID = info.mid(index+7,index2-index-7);
    signature = QString("unknown key ID %1 ").arg(signatureID);
    // FIXME: not a very good solution...
    status |= SIGNED;
    status |= GOODSIG;
  }

  //kdDebug(5100) << "status = " << status << endl;
  return status;
}

QStrList
KpgpBase5::pubKeys()
{
  int index,index2;
  int exitStatus = 0;

  status = 0;
  exitStatus = run("pgpk -l",0,true);

  if(exitStatus != 0) {
    status = ERROR;
    return 0;
  }

  // now we need to parse the output
  QStrList publicKeys;
  index = output.find("\n",1)+1; // skip first to "\n"
  while( (index = output.find("\n",index)) != -1)
  {
    //parse line
    QString line;
    if( (index2 = output.find("\n",index+1)) != -1)
    {
      int index3 = output.find("uid ",index);

      if( (index3 <index2) && (index3 != -1) )
      {
	line = output.mid(index3+5,index2-index3-5);
      }
      if(!line.isEmpty())
      {
	//kdDebug(5100) << "KpgpBase: found key for " << (const char *)line << "." << endl;
	publicKeys.append(line);
      }
    }
    else
      break;
    index = index2;
  }
  //kdDebug(5100) << "finished reading keys" << endl;
  return publicKeys;
}

QString KpgpBase5::getAsciiPublicKey(QString _person)
{
  int exitStatus = 0;

  if (_person.isEmpty()) return _person;
  QCString toexec;
  toexec.sprintf("pgpk -xa \"%s\"", _person.local8Bit().data());

  status = 0;
  exitStatus = run(toexec.data(),0,true);

  if(exitStatus != 0) {
    status = ERROR;
    return QString::null;
  }

  return output;
}

int
KpgpBase5::signKey(const char *key, const char *passphrase)
{
  QString cmd;
  int exitStatus = 0;

  if(passphrase == 0) return false;

  cmd = "pgpk -f +batchmode=1";
  cmd += key;
  cmd += addUserId();

  status = 0;
  exitStatus = run(cmd, passphrase);

  if (exitStatus != 0)
    status = ERROR;

  return status;
}

// -------------------------------------------------------------------------

KpgpBase6::KpgpBase6()
  : KpgpBase2()
{
}

KpgpBase6::~KpgpBase6()
{
}

int
KpgpBase6::decrypt(const char *passphrase)
{
  QString cmd;
  int index, index2;
  int exitStatus = 0;
  output = "";

  cmd = "pgp +batchmode +language=C -f";

  status = 0;
  exitStatus = run(cmd, passphrase);

  if(exitStatus == -1) {
    errMsg = i18n("error running PGP");
    status = RUN_ERR;
    return status;
  }

  // encrypted message
  if( info.find("File is encrypted.") != -1)
  {
    //kdDebug(5100) << "kpgpbase: message is encrypted" << endl;
    status |= ENCRYPTED;
    if((index = info.find("Key for user ID")) != -1)
    {
      // Find out the key for which the phrase is needed
      index  = info.find(":", index) + 2;
      index2 = info.find("\n", index);
      requiredID = info.mid(index, index2 - index);
      //kdDebug(5100) << "KpgpBase: key needed is \"" << requiredID << "\"!" << endl;
      //kdDebug(5100) << "KpgpBase: pgp user is \"" << pgpUser << "\"." << endl;

      // Test output length to find out, if the passphrase is
      // bad. If someone knows a better way, please fix this.
      if (!passphrase || !output.length())
      {
	errMsg = i18n("Bad passphrase; couldn't decrypt");
	//kdDebug(5100) << "KpgpBase: passphrase is bad" << endl;
        status |= BADPHRASE;
        status |= ERROR;
      }
    }
    else if( info.find("Secret key is required to read it.") != -1)
    {
      errMsg = i18n("Do not have the secret key for this message");
      //kdDebug(5100) << "KpgpBase: no secret key for this message" << endl;
      status |= NO_SEC_KEY;
      status |= ERROR;
    }
  }

  // signed message
  if(((index = info.find("File is signed.")) != -1)
    || (info.find("Good signature") != -1 ))
  {
    //kdDebug(5100) << "KpgpBase: message is signed" << endl;
    status |= SIGNED;
    if( info.find("signature not checked") != -1)
    {
      index = info.find("KeyID:",index);
      signatureID = info.mid(index+7,8);
      signature = QString("unknown key ID %1 ").arg(signatureID);
      status |= UNKNOWN_SIG;
      status |= GOODSIG;
    }
    else if((index = info.find("Good signature")) != -1 )
    {
      status |= GOODSIG;
      // get signer
      index = info.find("\"",index);
      index2 = info.find("\"", index+1);
      signature = info.mid(index+1, index2-index-1);

      // get key ID of signer
      index = info.find("KeyID:",index2);
      if (index == -1)
        signatureID = "???";
      else
        signatureID = info.mid(index+7,8);
    }
    else if( info.find("Can't find the right public key") != -1 )
    {
      status |= UNKNOWN_SIG;
      status |= GOODSIG; // this is a hack...
      signature = i18n("??? (file ~/.pgp/pubring.pkr not found)");
      signatureID = "???";
    }
    else
    {
      status |= ERROR;
      signature = "";
      signatureID = "";
    }
  }
  //kdDebug(5100) << "status = " << status << endl;
  return status;
}

QStrList
KpgpBase6::pubKeys()
{
  QString cmd;
  int index, index2;
  int exitStatus = 0;
  int compatibleMode = 1;

  cmd = "pgp +batchmode +language=C -kv -f";
  status = 0;
  exitStatus = run(cmd,0,true);

  if(exitStatus != 0) {
    status = ERROR;
    return 0;
  }

  //truncate trailing "\n"
  if (info.length() > 1) info.truncate(info.length()-1);

  QStrList publicKeys;
  index = info.find("bits/keyID",1); // skip first to "\n"
  if (index ==-1)
  {
    index = info.find("Type bits",1); // skip first to "\n"
    if (index == -1)
      return 0;
    else
      compatibleMode = 0;
  }

  while( (index = info.find("\n",index)) != -1)
  {
    //parse line
    QString line;
    if( (index2 = info.find("\n",index+1)) != -1)
      // skip last line
    {
      int index3;
      if (compatibleMode)
      {
        int index_pub = info.find("pub ",index);
        int index_sec = info.find("sec ",index);
        if (index_pub < 0)
          index3 = index_sec;
        else if (index_sec < 0)
          index3 = index_pub;
        else
          index3 = (index_pub < index_sec ? index_pub : index_sec);
      }
      else
      {
        int index_rsa = info.find("RSA ",index);
        int index_dss = info.find("DSS ",index);
        if (index_rsa < 0)
          index3 = index_dss;
        else if (index_dss < 0)
          index3 = index_rsa;
        else
          index3 = (index_rsa < index_dss ? index_rsa : index_dss);
      }

      if( (index3 >index2) || (index3 == -1) )
      {
	// second adress for the same key
	line = info.mid(index+1,index2-index-1);
	line = line.stripWhiteSpace();
      } else {
	// line with new key
	int index4 = info.find(
	  QRegExp("/[0-9][0-9]/[0-9][0-9] "),
	  index);
	line = info.mid(index4+7,index2-index4-7);
      }
      //kdDebug(5100) << "KpgpBase: found key for " << (const char *)line << endl;

      // don't add PGP's comments to the key list
      if (!line.startsWith("*** KEY EXPIRED ***") &&
          (line.find(QRegExp("^expires [0-9][0-9][0-9][0-9]/[0-9][0-9]/[0-9][0-9]")) < 0) && 
          !line.startsWith("*** DEFAULT SIGNING KEY ***")) {
        publicKeys.append(line);
      }
    }
    else
      break;
    index = index2;
  }

  // Also look for pgp key groups
  cmd = "pgp +batchmode +language=C -gv -f";
  exitStatus = run(cmd,0,true);

  if(exitStatus != 0) {
    status = ERROR;
    return 0;
  }

  index = 0;
  while ( (index = info.find("\n >", index)) != -1 ) {
    QString line;
    index += 4;
    index2 = info.find(" \"", index);
    line = info.mid(index, index2-index+1).stripWhiteSpace();

    //kdDebug() << "KpgpBase6: found key group for " << line << endl;
    publicKeys.append(line);
  }

  return publicKeys;
}

int
KpgpBase6::isVersion6()
{
  QString cmd;
  int exitStatus = 0;

  cmd = "pgp";

  exitStatus = run(cmd, 0, true);

  if(exitStatus == -1) {
    errMsg = i18n("error running PGP");
    status = RUN_ERR;
    return 0;
  }

  if( info.find("Version 6") != -1)
  {
    //kdDebug(5100) << "kpgpbase: pgp version 6.x detected" << endl;
    return 1;
  }

  //kdDebug(5100) << "kpgpbase: not pgp version 6.x" << endl;
  return 0;
}
