/* -------------------------------------------------------------

   dict.h (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>
   (C) by Matthias Hölzer 1998

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   JobData          used for data transfer between Client and Interface
   DictAsyncClient  all network related stuff happens here in an asynchrous thread
   DictInterface    interface for DictAsyncClient, job management
   
   -------------------------------------------------------------*/
 
#ifndef _DICT_H_
#define _DICT_H_

#include <pthread.h>
#include <qlist.h>
#include <qstrlist.h>

class QSocketNotifier;
struct in_addr;


//********* JobData ******************************************


class JobData
{

public:
  
  enum QueryType {            //type of transaction
    TDefine=0,
    TGetDefinitions,
    TMatch,
    TShowDatabases,
    TShowDbInfo,
    TShowStrategies,
    TShowInfo,
    TUpdate
  };

  enum ErrType {             // error codes
    ErrNoErr=0,
    ErrCommunication,        // display result!
    ErrTimeout,
    ErrBadHost,
    ErrConnect,              // display result!
    ErrRefused,
    ErrNotAvailable,
    ErrSyntax,
    ErrCommandNotImplemented,
    ErrAccessDenied,
    ErrAuthFailed,
    ErrInvalidDbStrat,
    ErrNoDatabases,
    ErrNoStrategies,
    ErrServerError,          // display result!
    ErrMsgTooLong
  };
  
  JobData(QueryType Ntype,bool NnewServer,QString const& Nserver,int Nport,
          int NidleHold, int Ntimeout, int NpipeSize, bool NAuthEnabled,
          QString const& Nuser, QString const& Nsecret, unsigned int NheadLayout);
  ~JobData();
  
  QueryType type;
  ErrType error;
  
  bool canceled;
  int numFetched;
  char *result;           // utf-8 encoded...
  QStrList *matches;

  QCString query;         // utf-8 encoded...
  QStrList *defines;

  bool newServer;
  QString server;
  int port, timeout, pipeSize, idleHold;
  bool authEnabled;
  QString user, secret;
  QStrList databases,strategies;
  QCString strategy;
  unsigned int headLayout;
};


//********* DictAsyncClient ******************************************


class DictAsyncClient
{
  
public:

  DictAsyncClient(int NfdPipeIn, int NfdPipeOut);
  ~DictAsyncClient();

  static void* startThread(void* pseudoThis);

  void insertJob(JobData *newJob);
  void removeJob(); 
  
private:

  void waitForWork();       // main loop
  void define();
  bool getDefinitions();
  bool match();
  void showDatabases();
  void showDbInfo();
  void showStrategies();
  void showInfo();
  void update();

  void openConnection();       // connect, handshake and authorization
  void closeSocket();
  void doQuit();               // send "quit" without timeout, without checks, close connection
  bool waitForRead();          // used by getNextIntoBuffer()
  bool waitForWrite();         // used by sendBuffer() & connect()
  void clearPipe();            // remove start/stop signal

  bool sendBuffer();           // send cmdBuffer to the server
  bool getNextLine();          // set thisLine to next complete line of input
  bool nextResponseOk(int code); // reads next line and checks the response code
  bool getNextResponse(int &code); // reads next line and returns the response code
  void handleErrors();

  void resultAppend(const char* str);
  void resultAppend(const char* str,unsigned int length);
  void resultAppend(char chr);

  JobData *job;
  char *input;
  QCString  cmdBuffer;
  const unsigned int resultSizeInc, inputSize;
  char *thisLine, *nextLine, *inputEnd;
  unsigned int resultPos, resultSize;
  int fdPipeIn,fdPipeOut;      //IPC-Pipes to/from async thread
  int tcpSocket,timeout,idleHold;
};


//********* DictInterface *************************************************

class DictInterface : public QObject
{
  Q_OBJECT
 
public:
  
  DictInterface();
  ~DictInterface();

public slots:

  void serverChanged();     // inform the client when server settings get changed
  void stop();              // cancel all pending jobs
   
  void define(QCString query);
  void getDefinitions(QStrList* &query);
  void match(QCString query);
  void showDbInfo(QCString db); // fetch detailed db info
  void showDatabases();        // fetch misc. info...
  void showStrategies();
  void showInfo();
  void updateServer();         // get info about databases & strategies the server knows
    
signals:

  void infoReady();                   // updateServer done
  void resultReady(char* &result);     // define done, set to 0L if you want to keep it
  void matchReady(QStrList* &result);  // match done, set to 0L if you want to keep it
  void started(const QString &message);      // Client is active now, activate inidicator
  void stopped(const QString &message);      // Client is now halted, deactivate indicator

private slots:

  void clientDone();

private:

  JobData* generateQuery(JobData::QueryType type,QCString query);
  void insertJob(JobData* job);  // insert in job list, if nesscary cancel/remove previous jobs
  void startClient();            // send start signal
  void cleanPipes();             // empty the pipes, so that notifier stops firing

  QSocketNotifier *notifier;
  int fdPipeIn[2],fdPipeOut[2];    //IPC-Pipes to/from async thread
  pthread_t threadID;
  DictAsyncClient *client;
  QList<JobData> jobList;
  bool newServer,clientDoneInProgress;
};

extern DictInterface *interface;

#endif
