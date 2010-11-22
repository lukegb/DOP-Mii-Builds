#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "Global.h"
#include "IOSMatrix.h"

using namespace Titles;

enum class SystemState : int
{
	Running,
	Exit,
	PowerOff
};

class System
{
private:
	class CertProperty
	{
	private:
		static signed_blob* Value;
	public:		
		static u32 Size;
		operator signed_blob*();
	};
// Fields
private:
	static bool ShutdownRequested;
public: 
	static SystemState State;
// Methods
private: 
	static void* ResetCallback();
	static void* PowerCallback();

public: // Methods
	static CertProperty Cert;

	static void Initialize();
	static void Exit() { Exit(false); }
	static void Exit(bool forceExit);
	static void ExitToPriiloader();
	static int  ReloadIOS(IosMatrixIterator item) { return ReloadIOS(item->Id, true); }
	static int  ReloadIOS(IosMatrixIterator item, bool initWPAD) { return ReloadIOS(item->Id, initWPAD); }
	static int  ReloadIOS(u32 version) { return ReloadIOS(version, true); }
	static int  ReloadIOS(u32 version, bool initWPAD);
	static void Shutdown();
	static void ShutdownDevices();
	static int  GetInstalledTitleIdList(u64List &list);
	static int  GetInstalledIosIdList(u32List &list);
	static int  GetEmptyIosIdSlots(u32List &list);
};

#endif

