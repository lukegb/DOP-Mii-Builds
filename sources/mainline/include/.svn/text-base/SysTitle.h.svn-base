#ifndef _SYSTITLE_H_
#define _SYSTITLE_H_

#include <string>
#include "Global.h"
#include "TmdView.h"

using namespace std;

class SysTitle
{
public:
	static int GetTMD(u64 titleId, tmd **ptmd);
	static int GetTMD(u64 titleId, signed_blob **stmd);
	static int GetTMDView(u64 titleId, TmdView **tmdv);
	static u16 GetVersion(u64 titleId);
	static int GetVersion(u64 titleId, u16 *version);
	static u64 GetSysVersion(u64 titleId);
	static int GetSysVersion(u64 titleId, u64 *version);
	static u32 GetSize(u64 titleId);
	static int GetSize(u64 titleId, u32 *outSize);
	static int GetTicket(u64 titleId, signed_blob **sticket);
	static string GetBootFilename(u64 titleId);	
};

#endif