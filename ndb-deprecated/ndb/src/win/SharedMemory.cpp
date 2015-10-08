#include <windows.h>

#include "SharedMemory.h"

void* pshared_memory;

void InitSharedMemory()
{
	HANDLE hShared = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,(1<<20)*5,NDB_NAME);
	if(hShared==NULL || GetLastError()==ERROR_ALREADY_EXISTS){
		CloseHandle(hShared);
		return;
	}

	void* pv = MapViewOfFile(hShared,FILE_MAP_ALL_ACCESS,0,0,(1<<20)*5);

	pshared_memory = pv;
}
