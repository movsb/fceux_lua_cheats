#ifndef __SHAREDMEMORY_H__
#define __SHAREDMEMORY_H__

#define NDB_NAME	"ndb_shared"

struct NDB_ShareMemory
{
	HWND hwndNdb;
	UINT uMsg;
};

#define NSMM_LOADGAME	0
struct NSM_LOADGAME{
	NDB_ShareMemory hdr;
	char fname[256];
	size_t size;
	unsigned char data[1];
};

extern void* pshared_memory;

void InitSharedMemory();


#endif//!__SHAREDMEMORY_H__
