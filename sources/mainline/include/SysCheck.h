#ifndef _SYSCHECK_H_
#define _SYSCHECK_H_

class SysCheck
{
public:
	static bool CheckFlashAccess();
	static bool CheckFakeSign();
	static bool CheckEsIdentify();
	static bool CheckNandPermissions();
	static int  RemoveBogusTicket();
	static int  RemoveBogusTitle();
};

#endif