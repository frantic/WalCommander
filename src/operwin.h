/*
 * Part of Wal Commander GitHub Edition
 * https://github.com/corporateshark/WalCommander
 * walcommander@linderdaum.com
 */

#pragma once

#include <mutex>

#include "swl.h"

using namespace wal;


typedef int ( * volatile OperCallback )( void* cbData );

class OperThreadWin;

class OperThreadNode
{
	friend class OperThreadWin;
	friend void* __123___OperThread( void* );

	OperThreadWin* volatile win;

	std::vector<char> threadInfo; //++volatile !!!
	OperThreadNode* volatile prev, *volatile next;
	volatile bool stopped;
	std::mutex mutex;
	void* volatile data;

	void* volatile cbData;
	std::condition_variable cbCond;
	volatile int  cbRet;
	OperCallback cbFunc;
public:
	OperThreadNode( OperThreadWin* w, const char* info, void* d )
		:  win( w ), threadInfo( new_char_str( info ) ), prev( 0 ), next( 0 ),
		   stopped( false ), data( d ),
		   cbData( 0 ), cbRet( -1 ), cbFunc( 0 )
	{}

	std::mutex& GetMutex() { return mutex; }

	void* Data() { return data; } //можно вызывать и работать с данными только заблакировав mutex получаемый через GetMutex
	bool NBStopped() { return stopped; } //можно вызывать только заблакировав mutex получаемый через GetMutex

	int CallBack( OperCallback f, void* data ); // ret < 0 if stopped

	//!!! id >= 2
	bool SendSignal( int id ) //обязательно запускать при НЕзалоченном mutex
	{
		std::lock_guard<std::mutex> lock( mutex );
		if ( stopped || id <= 1 ) { return false; }
		return WinThreadSignal( id );
	}

	~OperThreadNode();
private:
	OperThreadNode() {}
	CLASS_COPY_PROTECTION( OperThreadNode );
};

//ф-ция потока, не должна пропускать исключений
//поток может посылать сигналы только через tData
typedef void ( *OperThreadFunc )( OperThreadNode* node );


class OperThreadWin: public Win
{
	friend void* __123___OperThread( void* );
	int nextThreadId;
	int NewThreadID() { int n = nextThreadId; nextThreadId = ( ( nextThreadId + 1 ) % 0x10000 ); return n + 1; }

	int threadId;
	OperThreadNode* tNode;
	bool cbExecuted;
public:
	OperThreadWin( WTYPE t, unsigned hints = 0, int nId = 0, Win* _parent = nullptr, const crect* rect = nullptr )
		: Win( t, hints, _parent, rect, nId ), nextThreadId( 0 ), tNode( 0 ), cbExecuted( false ) {}

	void RunNewThread( const char* info, OperThreadFunc f, void* data ); //может быть исключение
	virtual void OperThreadSignal( int info );
	virtual void OperThreadStopped();

	void StopThread();

	static void DBGPrintStoppingList();

	virtual void ThreadSignal( int id, int data );
	virtual void ThreadStopped( int id, void* data );
	virtual ~OperThreadWin();
};
