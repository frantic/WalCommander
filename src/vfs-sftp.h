/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "libconf.h"
#include "vfs.h"

#ifdef LIBSSH2_EXIST

#include <libssh2.h>
#include <libssh2_sftp.h>
#include "tcp_sock.h"

class FSSftp : public FS
{
	std::mutex infoMutex;
	FSSftpParam _infoParam; //должно быть то же самое что и в operParam просто мьютексв разные, и который просто mutex может блокироваться надолго (на период работы функции)

	std::mutex mutex;

	enum CONSTS { MAX_FILES = 64 };

	TCPSock _sock;
	LIBSSH2_SFTP_HANDLE* volatile fileTable[MAX_FILES];
	LIBSSH2_SESSION* volatile sshSession;
	LIBSSH2_SFTP* volatile sftpSession;

	FSSftpParam _operParam;
	void CloseSession();
	int CheckSession( int* err, FSCInfo* info );

	void WaitSocket( FSCInfo* info ); //throw int(errno) or int(-2) on stop

	void CheckSessionEagain(); //if err != ...EAGAIN then throw int(...
	void CheckSFTPEagain();
	void CheckSFTP( int err ); //cheak if not 0 then throw


	void CloseHandle( LIBSSH2_SFTP_HANDLE* h, FSCInfo* info );
public:
	FSSftp( FSSftpParam* param );

	virtual unsigned Flags();
	virtual bool IsEEXIST( int err );
	virtual bool IsENOENT( int err );
	virtual bool IsEXDEV( int err );
	virtual FSString StrError( int err );
	virtual bool Equal( FS* fs );

	virtual int OpenRead ( FSPath& path, int flags, int* err, FSCInfo* info );
	virtual int OpenCreate  ( FSPath& path, bool overwrite, int mode, int flags, int* err, FSCInfo* info );
	virtual int Close ( int fd, int* err, FSCInfo* info );
	virtual int Read  ( int fd, void* buf, int size, int* err, FSCInfo* info );
	virtual int Write ( int fd, void* buf, int size, int* err, FSCInfo* info );
	virtual int Seek  ( int fd, SEEK_FILE_MODE mode, seek_t pos, seek_t* pRet,  int* err, FSCInfo* info );
	virtual int Rename   ( FSPath&  oldpath, FSPath& newpath, int* err,  FSCInfo* info );
	virtual int MkDir ( FSPath& path, int mode, int* err,  FSCInfo* info );
	virtual int Delete   ( FSPath& path, int* err, FSCInfo* info );
	virtual int RmDir ( FSPath& path, int* err, FSCInfo* info );
	virtual int SetFileTime ( FSPath& path, FSTime aTime, FSTime mTime, int* err, FSCInfo* info );
	virtual int ReadDir  ( FSList* list, FSPath& path, int* err, FSCInfo* info );
	virtual int Stat  ( FSPath& path, FSStat* st, int* err, FSCInfo* info );
	virtual int FStat ( int fd, FSStat* st, int* err, FSCInfo* info );
	virtual int Symlink  ( FSPath& path, FSString& str, int* err, FSCInfo* info );
	virtual int StatVfs( FSPath& path, FSStatVfs* st, int* err, FSCInfo* info );

	virtual FSString Uri( FSPath& path );
	virtual ~FSSftp();

	void GetParam( FSSftpParam* p ) { if ( !p ) { return; } std::lock_guard<std::mutex> lock( infoMutex ); *p = _infoParam; }
};

void InitSSH();
#else
#ifdef LIBSSH_EXIST

#include <libssh/sftp.h>

class FSSftp : public FS
{
	Mutex infoMutex;
	FSSftpParam _infoParam; //должно быть то же самое что и в operParam просто мьютексв разные, и который просто mutex может блокироваться надолго (на период работы функции)

	Mutex mutex;

	enum CONSTS { MAX_FILES = 64 };

	sftp_file fileTable[MAX_FILES];

	ssh_session sshSession;
	sftp_session sftpSession;

	FSSftpParam _operParam;
	void CloseSession();
	int CheckSession( int* err, FSCInfo* info );
public:
	FSSftp( FSSftpParam* param );

	virtual unsigned Flags();
	virtual bool IsEEXIST( int err );
	virtual bool IsENOENT( int err );
	virtual bool IsEXDEV( int err );
	virtual FSString StrError( int err );

	virtual int OpenRead ( FSPath& path, int* err, FSCInfo* info );
	virtual int OpenCreate  ( FSPath& path, bool overwrite, int mode, int* err, FSCInfo* info );
	virtual int Close ( int fd, int* err, FSCInfo* info );
	virtual int Read  ( int fd, void* buf, int size, int* err, FSCInfo* info );
	virtual int Write ( int fd, void* buf, int size, int* err, FSCInfo* info );
	virtual int Rename   ( FSPath&  oldpath, FSPath& newpath, int* err,  FSCInfo* info );
	virtual int MkDir ( FSPath& path, int mode, int* err,  FSCInfo* info );
	virtual int Delete   ( FSPath& path, int* err, FSCInfo* info );
	virtual int RmDir ( FSPath& path, int* err, FSCInfo* info );
	virtual int SetFileTime ( FSPath& path, FSTime aTime, FSTime mTime, int* err, FSCInfo* info );
	virtual int ReadDir  ( FSList* list, FSPath& path, int* err, FSCInfo* info );
	virtual int Stat  ( FSPath& path, FSStat* st, int* err, FSCInfo* info );
	virtual int Symlink  ( FSPath& path, FSString& str, int* err, FSCInfo* info );

	virtual FSString Uri( FSPath& path );
	virtual ~FSSftp();

	void GetParam( FSSftpParam* p ) { if ( !p ) { return; } std::lock_guard<std::mutex> lock( &infoMutex ); *p = _infoParam; }
};

void InitSSH();
#else
inline void InitSSH() {};
#endif
#endif
