#ifndef _TOOLS_H_
#define _TOOLS_H_

#include "Global.h"
#include "gccore.h"

class Spinner
{
private:
	static lwp_t Thread;
	static bool Running;
	static void * Execute(void *args);
public:
	static void Start();
	static void Stop();
};

class Tools
{
public:
	static void* AllocateMemory(u32 size);
	static const char* GetRegionString(int regionId);

	static bool IsPriiloaderInstalled();
	static int  BackupPriiloader();
	static int  RestorePriiloader();
	static void StringReplaceI(string &str, const char* from, const char* to);
	static bool StringStartsWithI(string &str, const char* value);
};


#endif