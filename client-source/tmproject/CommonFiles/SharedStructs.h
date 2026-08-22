 #pragma once
#include <cstddef>
/*
Este header, ira compartilhar as structs entre servidor e client a medida que o mesmo for sendo atualizado
outros headers também poder´ser criado como de constantes por exemplo.
*/

#ifndef _SHAREDSTRUCTS_ // Last updated 27/01/2013

#define _SHAREDSTRUCTS_

#define _MSG      	short		  Size;			\
					char		  KeyWord;		\
					char		  CheckSum;		\
					short		  Type;			\
					short		  ID;			\
					unsigned int  ClientTick;

struct MSG_HSTANDARD
{
	short Size;
	char KeyWord;
	char CheckSum;
	short Type;
	short ID;
	int Tick;
};

#define _PCKHEADER MSG_HSTANDARD Header

#ifdef _WYDCLIENT
typedef struct		
{
	WORD Size;
	BYTE KeyWord;
	BYTE CheckSum;
	WORD Type;
	WORD ID;
	DWORD Tick;
} MSG_STANDARD;
#endif // _WYDCLIENT
struct		  MSG_AccountLogin
{
	MSG_STANDARD Header;

	// WYD 7.48 fixes the account at byte 12, the password at byte 28 and the
	// client version at byte 40. Keeping this wire order is mandatory because
	// the Go server decodes the legacy 116-byte packet by those exact offsets.
	char AccountName[16];
	char AccountPassword[12];
	int  ClientVersion;
	int  DBNeedSave;
	char Zero[52];
	int AdapterName[4];

};

// The source-built client advertises its extended 48-byte score ABI through
// the legacy DBNeedSave field. Stock 7.48 leaves this field at zero/one; the
// Go server uses the marker only to select packet serializers and never as
// gameplay authority.
constexpr int WYD748_SOURCE_PROTOCOL_MARKER = 0x32435253; // "SRC2" in LE.

// These assertions make a future compiler/layout edit fail at build time
// instead of silently turning valid credentials into a different wire packet.
static_assert(sizeof(MSG_AccountLogin) == 116, "WYD 7.48 login packet must be 116 bytes");
static_assert(offsetof(MSG_AccountLogin, AccountName) == 12, "WYD 7.48 account offset changed");
static_assert(offsetof(MSG_AccountLogin, AccountPassword) == 28, "WYD 7.48 password offset changed");
static_assert(offsetof(MSG_AccountLogin, ClientVersion) == 40, "WYD 7.48 cliver offset changed");
#pragma pack(push, 1)

#pragma pack(pop)

#endif
