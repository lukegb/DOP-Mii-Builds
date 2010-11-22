#include <stdio.h>
#include <cstdlib>
#include <ogcsys.h>
#include <malloc.h>
#include <wiiuse/wpad.h>
#include <algorithm>
#include <unistd.h>

#include "System.h"
#include "Video.h"
#include "Tools.h"
#include "Gecko.h"
#include "Nand.h"
#include "Network.h"
#include "svnrev.h"
#include "FileSystem.h"
#include "Global.h"
#include "Settings.h"

using namespace IO;

SystemState System::State = SystemState::Running;
signed_blob* System::CertProperty::Value = NULL;
u32 System::CertProperty::Size = 0;
System::CertProperty System::Cert;
bool System::ShutdownRequested = false;

System::CertProperty::operator signed_blob*()
{
	if (!Value)
	{	
		gprintf(">> Loading System Cert\n");
		int file = Nand::OpenRead("/sys/cert.sys");
		if (file > 0)
		{	
			Size = Nand::GetFileSize(file);
			Value = (signed_blob*)Tools::AllocateMemory(Size);
			int ret = Nand::Read(file, (u8*)Value, Size);
			Nand::Close(file);
			if (ret != (int)Size) Value = (signed_blob*)NULL;
		}
	}
	return Value;
}

void System::ShutdownDevices()
{		
	//gprintf("Shutting Down WPAD\n"); 
	WPAD_Flush(0);
	WPAD_Disconnect(0);
	WPAD_Shutdown();

	//gprintf("Shutting Down Network\n"); 
	Network::ShutDown();

	//gprintf("\nShutting Down SD\n"); 
	SD::Unmount();

	//gprintf("Shutting Down USB\n"); 
	USB::Shutdown();

	//gprintf("Shutting Down ISFS\n");
	Nand::Shutdown();
}

void System::Shutdown()
{	
	gprintf("\nSystem::ShutDown()\n");
	ShutdownDevices();
	if (System::State == SystemState::PowerOff)
	{		
		gprintf("Powering Off Console\n");
			
		int ret;
		if (CONF_GetShutdownMode() == CONF_SHUTDOWN_IDLE) 
		{
			ret = CONF_GetIdleLedMode();		
			if (ret >= CONF_LED_OFF && ret <= CONF_LED_BRIGHT) ret = STM_SetLedMode(ret);

			ret = STM_ShutdownToIdle();
		} 
		else ret = STM_ShutdownToStandby();
	}
	else
	{
		gprintf("\nReturning to Loader");
		Console::SetPosition(Console::Rows-1, 0);	
		Console::ClearLine();
		printf("Returning To Loader");
		fflush(stdout);
		VIDEO_WaitVSync();
		exit(0);
	}
}

void* System::ResetCallback()
{
	gprintf("Reset Button Pressed\n");
	System::Exit();
	return NULL;
}

void* System::PowerCallback()
{
	/* Poweroff console */
	System::State = SystemState::PowerOff;
	return NULL;
}

void System::Initialize()
{
    /* Initialize video subsytem */
	InitGecko(); 
	gprintf("\n\nDOP-Mii (r%s)\n", SVN_REV_STR);
	gprintf("Initializing Wii\n");
	gprintf("VideoInit\n"); Video::Initialize();
    gprintf("PAD_Init\n"); PAD_Init();
	gprintf("WPAD_Init\n"); WPAD_Init();	
	USB::Startup(); // Wake Up USB Drive
	Settings::Instance().Load();

    /* Set RESET/POWER button callback */
	WPAD_SetPowerButtonCallback((WPADShutdownCallback)PowerCallback);
    SYS_SetResetCallback((resetcallback)ResetCallback);
    SYS_SetPowerCallback((powercallback)PowerCallback);
}

void System::Exit(bool forceExit) 
{
	System::State = SystemState::Exit;
	if (forceExit) System::Shutdown();
}

void System::ExitToPriiloader()
{
	// Set the magic word
	*(vu32*)0x8132FFFB = 0x4461636f;
	// Going down...
	SYS_ResetSystem(SYS_RESTART,0,0);
	// Wait for next frame
	VIDEO_WaitVSync();
}

int System::ReloadIOS(u32 version, bool initWPAD)
{
	// The following needs to be shutdown before reload	
	ShutdownDevices();
	gprintf("Reloading IOS%d\n", version);		
	usleep(5000);
	int ret = IOS_ReloadIOS(version);
	usleep(5000);
	if (initWPAD) 
	{
		gprintf("Reinitializing WPAD\n");
		WPAD_Init();
	}
	return ret;
}

int System::GetInstalledTitleIdList(u64List &list)
{
	u64 *titles = NULL;
	u32 length;
	u32 numTitles;
	int ret = 0;

	/* Get number of titles */
	ret = ES_GetNumTitles(&numTitles);
	if (ret < 0) return ret;

	/* Calculate buffer lenght */
	length = ROUND_UP(sizeof(u64) * numTitles, 32);

	/* Allocate memory */
	titles = (u64*)Tools::AllocateMemory(length);
	if (!titles) return -1;

	/* Get titles */
	ret = ES_GetTitles(titles, numTitles);
	if (ret < 0) goto end;
	
	for (u32 i = 0; i < numTitles; i++)
	{
		list.push_back(titles[i]);
	}

end:
	/* Free memory */
	delete titles; titles = NULL;
	return ret;	
}

int System::GetInstalledIosIdList(u32List &list)
{	
	u64List titles;

	int ret = GetInstalledTitleIdList(titles);
	if (ret < 0) return ret;

	for (u64Iterator titleId = titles.begin(); titleId < titles.end(); ++titleId)
	{
		u32 titleId1 = TITLEID1(*titleId);
		u32 titleId2 = TITLEID2(*titleId);

		if (titleId1 == 1 && titleId2 > 2 && titleId2 < 256)
		{
			list.push_back(titleId2);
		}
	}

	return 0;
}

int System::GetEmptyIosIdSlots(u32List &list)
{
	int ret = -1;
	u32List installedList;

	ret = GetInstalledIosIdList(installedList);
	if (ret < 0) goto final;

	for (u32 i = 4; i < 0xFF; i++) { list.push_back(i); }

	for (u32Iterator i = installedList.begin(); i < installedList.end(); ++i)
	{
		for (u32Iterator e = list.begin(); e < list.end(); ++e)
		{
			if (*e == *i) 
			{
				list.erase(e);
				break;
			}
		}
	}

	sort(list.rbegin(), list.rend());

final:
	installedList.clear();
	return ret;
}
