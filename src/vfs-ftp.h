/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include "vfs.h"
#include "tcp_sock.h"


enum FTP_ERRNO
{
	EFTP_BADPROTO  = -10,
	EFTP_NOTSUPPORTED = -11,
	EFTP_OUTOFRESOURCE_INTERNAL = -12,
	EFTP_INVALID_PARAMETER = -13,
	EFTP_NOTEXIST = -14,
	EFTP_EXIST = -15
};

//++volatile надо скорректировать
class FtpStatCache
{
	enum CONSTS { CACHE_TIMEOUT = 60 };
	struct Dir: public iIntrusiveCounter
	{
		time_t tim;
		cstrhash< clPtr<Dir>, char> dirs;
		clPtr< cstrhash<FSStat, char> >  statHash;
		Dir(): tim( 0 ) {}
	};

	std::mutex mutex;
	Dir root;
	int* pCharset;
	Dir* GetParent( FSPath& path );
public:
	FtpStatCache( int* pCs ): pCharset( pCs ) {}
	void PutStat( FSPath path, clPtr<cstrhash<FSStat, char> > statHash );
	int GetStat( FSPath path, FSStat* st ); //0 - ok, 1 - not cache for it, -1 - not exist in cache
	void Del( FSPath path );
private:
	FtpStatCache() {};
	FtpStatCache( const FtpStatCache& ) {};
	void operator = ( const FtpStatCache& ) {};
};

//++volatile надо скорректировать
class FTPNode: public iIntrusiveCounter
{
	TCPSyncBufProto ctrl;
	TCPSyncBufProto data;

	bool _passive;

	int ReadCode();
	void Passive( unsigned& passiveIp, int& passivePort );
	void Port( unsigned ip, int port );
	void OpenData( const char* cmd );
public:
	FTPNode();

	void Connect( unsigned ip, int port, const char* user, const char* password, bool passive );
	void Noop();
	void Ls( ccollect<std::vector<char> >& list );
	void Cwd( const char* path );
	void OpenRead( const char* fileName );
	void MkDir( const char* fileName );
	void RmDir( const char* fileName );
	void Rename( const char* from, const char* to );
	void OpenWrite( const char* fileName );
	int ReadData( void* s, int size ) { return data.Read( ( char* )s, size ); }
	void WriteData( void* s, int size ) { data.Write( ( char* )s, size ); }
	void CloseData();
	void Delete( const char* fileName );

	void SetStopFunc( CheckStopFunc f, void* d ) { ctrl.SetStopFunc( f, d ); data.SetStopFunc( f, d ); }
	void ClearStopFunc() { SetStopFunc( 0, 0 ); }

	~FTPNode();
};


class FtpIDCollection
{
	std::mutex mutex;
	cstrhash<int, unicode_t> byName;
	ccollect<std::vector<unicode_t> > byId;
	static unicode_t u0;
public:
	FtpIDCollection() {}

	const unicode_t* GetName( int id )
	{
		std::lock_guard<std::mutex> lock( mutex );
		return ( id >= 0 && id < byId.count() && byId[id].data() ) ? byId[id].data() : &u0;
	}

	int GetId( const unicode_t* name )
	{
		std::lock_guard<std::mutex> lock( mutex );
		int* n = byName.exist( name );

		if ( n ) { return *n; }

		std::vector<unicode_t> p = new_unicode_str( name );
		int t = byId.count();
		byId.append( p );
		byName[name] = t;
		return t;
	}
	~FtpIDCollection() {}
};

//++volatile надо скорректировать
class FSFtp : public FS
{
	std::mutex infoMutex;
	FSFtpParam _infoParam;

	FtpStatCache statCache; //has own mutex
	FtpIDCollection uids;   //has own mutex
	FtpIDCollection gids;   //has own mutex

	std::mutex mutex;
	FSFtpParam _param;

	enum { NODES_COUNT = 32 };

	struct Node: public iIntrusiveCounter
	{
		volatile bool busy;
		volatile time_t lastCall;
		/*volatile*/ clPtr<FTPNode> pFtpNode;

		Node(): busy( false ), lastCall( 0 ) {}
	};

	Node nodes[NODES_COUNT];

	int GetFreeNode( int* err, FSCInfo* info ); //return -1 if error

	int ReadDir_int ( FSList* list, cstrhash<FSStat, char>* pSList, FSPath& _path, int* err, FSCInfo* info );
public:
	FSFtp( FSFtpParam* param );

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
	virtual int Rename   ( FSPath&  oldpath, FSPath& newpath, int* err,  FSCInfo* info );
	virtual int MkDir ( FSPath& path, int mode, int* err,  FSCInfo* info );
	virtual int Delete   ( FSPath& path, int* err, FSCInfo* info );
	virtual int RmDir ( FSPath& path, int* err, FSCInfo* info );
	virtual int SetFileTime ( FSPath& path, FSTime aTime, FSTime mTime, int* err, FSCInfo* info );
	virtual int ReadDir  ( FSList* list, FSPath& path, int* err, FSCInfo* info );
	virtual int Stat  ( FSPath& path, FSStat* st, int* err, FSCInfo* info );

	virtual FSString Uri( FSPath& path );
	virtual ~FSFtp();

	void GetParam( FSFtpParam* p ) { if ( !p ) { return; } std::lock_guard<std::mutex> lock( infoMutex ); *p = _infoParam; }

	virtual unicode_t* GetUserName( int user, unicode_t buf[64] );
	virtual unicode_t* GetGroupName( int group, unicode_t buf[64] );
};
