/*-------------------------------------------------------------

DOP-Mii - by Lunatik, Arikado
Supported Developers: giantpune, Lunatik, SifJar, and PhoenixTank

Based on Dop-IOS - install and patch any IOS by marc
Based on tona's shop installer (C) 2008 tona

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <gccore.h>
#include <malloc.h>
#include <wiiuse/wpad.h>
#include <string>
#include <algorithm>
#include <sstream>

#include "Global.h"
#include "Controller.h"
#include "FileSystem.h"
#include "SysCheck.h"
#include "SysTitle.h"
#include "System.h"
#include "Title.h"
#include "Tools.h"
#include "Video.h"
#include "Nand.h"
#include "Nus.h"
#include "Boot2.h"
#include "Settings.h"
#include "Main.h"
#include "Scripting.h"
#include "Identify.h"
#include "RuntimeIOSPatch.h"

#define HAVE_AHBPROT ((*(vu32*)0xcd800064 == 0xFFFFFFFF) ? 1 : 0)

#warning REMINDER: change ProgramVersion in include/Global.h


using namespace IO;
using namespace Titles;

/* 
libOGC wants to default to IOS36. 
I say we use latest installed so we can possibly get SDHC support
*/
extern "C" s32 __IOS_LoadStartupIOS() 
{
	if (HAVE_AHBPROT)
	{
		/* Let's not muck up our privs */
		return 0;
	}
	int ret = 0;

	ret = __ES_Init();
	if(ret < 0) return ret;

	u32List list;
	System::GetInstalledIosIdList(list);
	sort(list.rbegin(), list.rend());	
	for (u32Iterator version = list.begin(); version < list.end(); ++version)
	{
		if (*version < 81) // Currently latest IOS starts at 80
		{
			ret = __IOS_LaunchNewIOS(*version);
			if (ret > -1) break;
		}
	}
	list.clear();
	if (ret < 0) return ret;
	return 0;
}

int Main::InstallFakeSignIOS36()
{
	int ret = 0;
	signed_blob *stmd = NULL;
	IosRevisionIterator rev;
    
    int iosVersion = 0;
	int iosToUse = 0;
	bool isAHBPROT = HAVE_AHBPROT;
	
	//First try to install via AHBPROT
	
	if(isAHBPROT)
	{
	   gcprintf("Trying to install via AHBPROT...\n");
	   
	   //Patch current IOS via AHBPROT if not already patched
	   CurrentIOS = IosMatrix->Item((u32)IOS_GetVersion());
	   gprintf("Checking to see if patching current IOS via AHBPROT is necessary\n");
	   if (!SysCheck::CheckFakeSign()) 
	   {
		   gcprintf("Patching current IOS via AHBPROT...\n");
		   Console::PrintSolidLine();
		   RuntimeIOSPatch::Apply();
		   gcprintf("\n\n...COMPLETE\n");
	   }
	   
       IosMatrixIterator IOS;
	   
/*       IOS->Id = 0x100000024ull;
	   rev->Id = 1024;*/
        IOS = IosMatrix->Item(36u);
	rev = IOS->Revisions.Item(3351);
		
	iosVersion = SysTitle::GetVersion(0x100000036ull);
        if (iosVersion != 0)
        {
	   //Delete IOS 36
	   gcprintf("Removing IOS 36...\n");
	   ret = UninstallIOS(IOS);
	   if (ret < 0)
	   {
	      gcprintf("\n>> ERROR! Failed To Remove IOS 36\n");
		  goto error;
	   }
	   gcprintf("...COMPLETE!\n");
        } else {
           gcprintf("You do not have IOS 36 already installed.\n");
        }
	   
	   //Install lower version of IOS 36 that has fakesigning
	   gcprintf("Installing an older IOS 36 with fakesignig...\n");
	   ret = InstallIOS(IOS, rev);
	   if (ret < 0)
	   {
	      gcprintf("\n>> ERROR! Failed To Remove IOS 36\n");
		  goto error;
	   }
	   gcprintf("...COMPLETE!\n");
	   gprintf("Fakesign ios installed via AHBPROT successfully!\n");
	   
	   goto final;
	
	}

    // Check to see if IOS 41 | 43 | 45 | 46 is currently installed
    // If it is installed see if it is a version that is supported, if so move on
    // if not move onto the next IOS

    // Check IOS36
    gcprintf("Checking IOS36 Version...");
    iosVersion = SysTitle::GetVersion(0x100000024ull);
    if (iosVersion <= 3351 && iosVersion > 0) 
    {        
        gcprintf("Version Usable. Starting Downgrade.\n");
        iosToUse = 36;
    } else gcprintf("Not Usable.\n");

    // Check IOS41
    if (iosToUse == 0)
    {
        gcprintf("Checking IOS41 Version...");
        iosVersion = SysTitle::GetVersion(0x100000029ull);    
        if (iosVersion == 0)
        {
            gcprintf("IOS41 v3348 not installed. Installing now.\n\n");
            rev = IosMatrix->Item(41u)->Revisions.Item(3348);
            rev->IgnoreAllPatches = true;
            ret = Title::Install(rev);
            if (ret > 0) iosToUse = 41;
        }
        else if (iosVersion <= 3348) 
        {
            gcprintf("Version Usable. Starting Downgrade.\n");
            iosToUse = 41;
        }
        else gcprintf("Not Usable.\n");
    }

    // Check IOS43
    if (iosToUse == 0)
    {
        gcprintf("Checking IOS43 Version...");
        iosVersion = SysTitle::GetVersion(0x10000002Bull);
        if (iosVersion == 0)
        {
            gcprintf("IOS43 v3348 not installed. Installing now.\n\n");
            rev = IosMatrix->Item(43u)->Revisions.Item(3348);
            rev->IgnoreAllPatches = true;
            ret = Title::Install(rev);
            if (ret > 0) iosToUse = 43;
        }
        else if (iosVersion <= 3348)
        {
            gcprintf("Version Usable. Starting Downgrade.\n");
            iosToUse = 43;
        }
        else gcprintf("Not Usable.\n");
    }

    // Check IOS45
    if (iosToUse == 0)
    {
        gcprintf("Checking IOS45 Version...");
        iosVersion = SysTitle::GetVersion(0x10000002Dull);
        if (iosVersion == 0)
        {
            gcprintf("IOS45 v3091 not installed. Installing now.\n\n");
            rev = IosMatrix->Item(45u)->Revisions.Item(3091);
            rev->IgnoreAllPatches = true;
            ret = Title::Install(rev);
            if (ret > 0) iosToUse = 45;
        }
        else if (iosVersion <= 3091)
        {
            gcprintf("Version Usable. Starting Downgrade.\n");
            iosToUse = 45;
        }
        else gcprintf("Not Usable.\n");        
    }

    // Check IOS46
    if (iosToUse == 0)
    {
        gcprintf("Checking IOS46 Version...");
        iosVersion = SysTitle::GetVersion(0x10000002Eull);
        if (iosVersion == 0)
        {
            gcprintf("IOS46 v3350 not installed. Installing now.\n\n");
            rev = IosMatrix->Item(46u)->Revisions.Item(3350);
            rev->IgnoreAllPatches = true;
            ret = Title::Install(rev);
            if (ret > 0) iosToUse = 46;
        }
        else if (iosVersion <= 3350)
        {
            gcprintf("Version Usable. Starting Downgrade.\n");
            iosToUse = 46;
        }
        else gcprintf("Not Usable.\n");        
    }

    // Check IOS52
    if (iosToUse == 0)
    {
        gcprintf("Checking IOS52 Version...");
        iosVersion = SysTitle::GetVersion(0x100000034ull);
        if (iosVersion == 0)
        {
            gcprintf("IOS52 v5661 not installed. Installing now.\n\n");
            rev = IosMatrix->Item(52u)->Revisions.Item(5661);
            rev->IgnoreAllPatches = true;
            ret = Title::Install(rev);
            if (ret > 0) iosToUse = 52;
        }
        else if (iosVersion <= 5661)
        {
            gcprintf("Version Usable. Starting Downgrade.\n");
            iosToUse = 52;
        }
        else gcprintf("Not Usable.\n");        
    }

    if (iosToUse == 0)
    {
        gcprintf("\n\n>> ERROR! No Useable IOSes found to downgrade IOS15.\n");
        ret = -1;
        goto error;
    }

    // Let's see if we are already on IOS36, if not then reload to IOS36
	if (IOS_GetVersion() != iosToUse) 
	{
		gcprintf("Current IOS is not %d. Loading IOS%d...\n\n", iosToUse, iosToUse);
		ret = System::ReloadIOS(iosToUse);
		if (ret < 0)
		{
			gcprintf("\n>> ERROR! Failed to load IOS%d: ErrorCode (%d)", iosToUse, ret);
			goto error;
		}
	}

	// snag stored TMD on IOS36 before reloading to IOS15	
	ret = SysTitle::GetTMD(0x100000024ull, &stmd);
	if (ret < 0)
	{
		gcprintf("\n>>ERROR! Failed to Stored TMD for IOS36: %s\n", EsError::ToString(ret));
		goto error;
	}
	
	if (SysTitle::GetVersion(0x10000000Full) != 257)
	{
		gcprintf("*** Downgrading IOS15 to v257 ***\n");
		ret = Title::Install(IosMatrix->Item(15u)->Revisions.Item(257));
		if (ret < 0)
		{
			gcprintf("\n>> ERROR! Failed To Downgrade IOS15 to v257\n");
			goto error;
		}
	}

	Console::PrintSolidLine(false);
	gcprintf("*** Reloading IOS15 (v257) ***\n");
	ret = System::ReloadIOS(15);
	if (ret < 0)
	{
		gcprintf("\n>> ERROR! Failed to reload IOS15: ErrorCode (%d)\n", ret);
		goto error;
	}

	gcprintf("*** Restoring IOS36 to v3351 ***\n\n");
	rev = IosMatrix->Item(36u)->Revisions.Item(3351);
	rev->ApplyFakeSignPatch = true;
	ret = Title::Install(rev, stmd);
	if (ret < 0)
	{
		gcprintf("\n>> ERROR! Failed To Restore IOS36 to v3551\n");
		goto error;
	}

	Console::PrintSolidLine(false);
	gcprintf("*** Reloading IOS36 (v3351) ***\n\n");
	ret = System::ReloadIOS(36);
	if (ret < 0)
	{
		gcprintf("\n>> ERROR! Failed to reload IOS15: ErrorCode (%d)\n", ret);
		goto error;
	}
	gcprintf("Would you like to restore IOS15 to 1031?\n");
	if (Console::PromptYesNo())
	{	
		gcprintf("\n*** Restoring IOS15 to v1031 ***\n");
		ret = Title::Install(IosMatrix->Item(15u)->Revisions.Item(1031));
		if (ret < 0)
		{
			gcprintf("\n>> ERROR! Failed To Restore IOS15 to v1031\n");
			goto error;
		}
	}

	if (ret > -1)
	{
		Console::ClearScreen();
		gcprintf("Installation of IOS36 (v3351) w/FakeSign was completed successfully!!!\n");
		gcprintf("You may now use IOS36 to install anything else.\n\n");
		gcprintf("Press any key to continue.\n");
		VIDEO_WaitVSync();
		Controller::WaitAnyKey();
		goto final;
	}

error:
	gcprintf("\nPress any key to return to the menu.\n");
	Controller::WaitAnyKey();

final:
	delete stmd; stmd = NULL;
	return ret;
}

int Main::InstallIOS(IosMatrixIterator ios, IosRevisionIterator revision) 
{
	return InstallIOS(ios, revision, false);
}

int Main::InstallIOS(IosMatrixIterator ios, IosRevisionIterator revision, bool altSlot) 
{
	VIDEO_WaitVSync();
	Console::ClearScreen();
	gprintf("\n");
	gcprintf("Are you sure you want to install %s (v%d)?\n", ios->Name.c_str(), revision->Id);
	if (!Console::PromptYesNo()) return -1;
	gcprintf("\n");
	int ret = Title::Install(revision, altSlot);
	gcprintf("\nPress any key to return to the menu.\n");
	Controller::WaitAnyKey();
	return ret;
}

int Main::UninstallIOS(IosMatrixIterator ios) 
{
	VIDEO_WaitVSync();
	Console::ClearScreen();
	gprintf("\n");
	gcprintf("Are you sure you want to uninstall %s?\n", ios->Name.c_str());
	if (!Console::PromptYesNo()) return -1;
	gcprintf("\n");
	int ret = Title::Uninstall(ios->TitleId);
	gcprintf("\nPress any key to return to the menu.\n");
	Controller::WaitAnyKey();
	return ret;
}

void Main::RefreshIosMatrix()
{
	u32 tmpCurrentIOS = Settings::Instance().DefaultIOS;
	if (IosMatrix != NULL) 
	{
		tmpCurrentIOS = CurrentIOS->Id;
		delete IosMatrix;		
	}
	IosMatrix = new Titles::IosMatrix();
	CurrentIOS = IosMatrix->Item(tmpCurrentIOS);
}

bool Main::ShowAHBPROTMenu()
{

    while (System::State == SystemState::Running)
	{
		VIDEO_WaitVSync();
		Console::ClearScreen();
		
		printf("\nYou have AHBPROT available for use.\n\n");
		printf("To continue using AHBPROT press A\n");
		printf("To proceed with your selected option instead press B\n");
		
		VIDEO_WaitVSync();

		u32 button = 0;
		while (Controller::ScanPads(&button))
		{
		
			if (button == WPAD_BUTTON_HOME || button == WPAD_CLASSIC_BUTTON_HOME || button == PAD_TRIGGER_Z) System::Exit();
			
			if (button == WPAD_BUTTON_B || button == WPAD_CLASSIC_BUTTON_B || button == PAD_BUTTON_B)
            {  
			
               gprintf("\nUsing IOS instead of AHBPROT"); 
               return 0;
			   
			}
			
			if (button == WPAD_BUTTON_A || button == WPAD_CLASSIC_BUTTON_A || button == PAD_BUTTON_A) 
			{
			
			   gprintf("\nUsing AHBPROT instead of reloading IOS");
			   return 1;
			   
			}	
		}
	}
	
	gprintf("\nIf you are reading this, something is seriously broken.");
	
    return 0;
	
}

void Main::ShowBoot2Menu()
{
	while (System::State == SystemState::Running)
	{
		VIDEO_WaitVSync();
		Console::ClearScreen();
		u32 version = 0;
		int ret = ES_GetBoot2Version(&version);
		if (ret < 0)
		{
			printf("ERROR! ES_GetBoot2Version: %s", EsError::ToString(ret));
			printf("Could not get boot2 version.\n");
			printf("It's possible your Wii is a boot2v4+ Wii, maybe not.");
		} 
		else printf("Your Boot2 version is: %u", version);


		if (ret == 700)
		{
			Console::SetFgColor(Color::Yellow, Bold::On);
			printf("PROCEED NO FURTHER AND REPORT THIS IMMEDIATELY TO THE DOP-MII WEBSITE");
			Console::ResetColors();
		}
                

		Console::PrintSolidLine();

		if (version == 0)
		{
			printf("There was a problem getting the Boot2 version.\n");
			printf("Please Exit DOP and try again\n");
		}
		else if (version < MaxBoot2Version)
		{
			printf("Your Boot2 is eligable to be upgraded to v%d.", MaxBoot2Version);
			printf("\n\n");
			Console::SetFgColor(Color::Yellow, Bold::On);
			printf("!!! IMPORTANT READ BELOW VERY CAREFULLY!!!\n");
			Console::ResetColors();
			printf("If you have BootMii installed as Boot2. Beyond our control whenever\n");
			printf("you upgrade your Boot2 it will get wiped out. You will need to reinstall\n");
			printf("BootMii as Boot2 after the installation is completed.\n");
			printf("\n");
			printf("Once you have upgraded your Boot2. You CANNOT downgrade your Boot2 at all.\n");
			printf("There is no known way to get around this at this time so before you decide\n");
			printf("to upgrade, please think about this very carefully.\n");
			printf("\n");
			printf("At this time there are no known issues preventing you from upgrading.\n");
			printf("But the team DOES NOT take any responsibility if you decide to upgrade.\n");
			printf("\n");
			printf("As a safety precaution, when you start the upgrade process you will be\n");
			printf("prompted 4 times to confirm that you truely want to upgrade your Boot2.\n");
		} 
		else if (version == MaxBoot2Version)
		{
			printf("You are currently running the latest supported Boot2 version.\n");
		}
		else 
		{
			printf("You are currently running a newer Boot2 version than we support.\n");
		}

		Console::SetRowPosition(Console::Rows - 5);
		Console::PrintSolidLine();
		printf("[B] Back");
		if (version && version < MaxBoot2Version) printf("  [+] Upgrade Boot2");
		Console::PrintSolidLine();

		printf("Current IOS: %s", CurrentIOS->Name.c_str());

		VIDEO_WaitVSync();

		u32 button = 0;
		while (Controller::ScanPads(&button))
		{
			if (button == WPAD_BUTTON_HOME) System::Exit();
			if (System::State != SystemState::Running) return;
			if (button == WPAD_BUTTON_B) return;

			if (button == WPAD_BUTTON_PLUS && version && version < MaxBoot2Version) 
			{
				if (UpgradeBoot2() > -1) ES_GetBoot2Version(&version);				
			}

			if (button) break;
		}
	}
}

int Main::UpgradeBoot2()
{
	int ret = 0;
	u32 button = 0;

	Console::ClearScreen();
	gcprintf("Are you sure you want to upgrade Boot2?\n");
	if (!Console::PromptYesNo()) return -0xFFFF;
	gcprintf("\nAre you really sure you want to upgrade Boot2?\n");
	if (!Console::PromptYesNo()) return -0xFFFF;
	gcprintf("\nAre you really really sure you want to upgrade Boot2?\n");
	if (!Console::PromptYesNo()) return -0xFFFF;

	gcprintf("\nAs a final confirmation please press and hold [%s,+,-] on the WiiMote\n", UpArrow);
	gcprintf("or [%s,R,L] on the GameCube Controller then press [A]\n\n", UpArrow);
	gcprintf("To Cancel either press [B] or [Home] to return to loader\n\n");

	while (Controller::ScanPads(&button))
	{
		if (button == WPAD_BUTTON_HOME) System::Exit();
		if (System::State != SystemState::Running) return -0xFFFF;
		if (button == WPAD_BUTTON_B) return -0xFFFF;

		if (button == (WPAD_BUTTON_UP+WPAD_BUTTON_A+WPAD_BUTTON_PLUS+WPAD_BUTTON_MINUS))
		{
			Console::SetFgColor(Color::Yellow, Bold::On);
			printf("!!! DO NOT POWER OFF THE WII !!!\n\n");
			Console::ResetColors();
			gcprintf("Installing Boot2...");
			Boot2::Install(4);
			gcprintf("Done\n");
			break;
		}
	}

	gcprintf("\nPress any key to return to the menu.\n");
	Controller::WaitAnyKey();
	
	return ret;
}

void Main::BuildSysCheckTable()
{
	int ret = 0;
	u32 titlesCount;
	u64 *titles = NULL;

	SysCheckTable.clear();

	ret = ES_GetNumTitles(&titlesCount);
	titles = (u64*)Tools::AllocateMemory(sizeof(u64) * titlesCount);
	ret = ES_GetTitles(titles, titlesCount);
 
	for (u32 i = 0; i < titlesCount; i++)
	{
		u64 titleId = titles[i];
		u32 titleId1 = TITLEID1(titleId);
		u32 titleId2 = TITLEID2(titleId);

		if (titleId1 != 1) continue; // skip non-system titles
		
		// skip system menu, BC, MIOS and possible other non-IOS titles
		if (titleId2 < 4 || titleId2 > 255) continue;
		
		{ // check if this is just a stub
			tmd *ptmd = NULL;
			SysTitle::GetTMD(titleId, &ptmd);
			u16 numContents = ptmd->num_contents;
			u16 titleVersion = ptmd->title_version;
			delete ptmd; ptmd = NULL;
					
			IosRevision *revision = IosMatrix::GetIosRevision(titleId2, titleVersion);		
			if (revision)
			{
				if (revision->IsStub || revision->Id < 1000) { delete revision; revision = NULL; continue; }
			}
			else 
			{
				if (numContents == 1) continue; // Is A Stub
				if (numContents == 3) continue; // May be a stub so skip to be safe
			}
		} // END CHECK
				
		SysCheckTable.push_back(titleId2);
	}
	delete titles; titles = NULL;
	
	sort(SysCheckTable.rbegin(), SysCheckTable.rend());
	gprintf("IOSES Found = %d\n", SysCheckTable.size()-1);	
}


void Main::ShowWelcomeScreen()
{
	//Basic scam warning, brick warning, and credits by Arikado
	VIDEO_WaitVSync();
	Console::ClearScreen();
	printf("Welcome to DOP-Mii!\n\n");
	printf("If you have paid for this software, you have been scammed.\n\n");
	printf("If misused, this software WILL brick your Wii.\n");
	printf("The authors of DOP-Mii are not responsible if your Wii is bricked.\n\n");
	printf("Website       : http://dop-mii.googlecode.com\n");
	printf("Forums        : http://groups.google.com/group/dop-mii\n");
	printf("Blog          : http://arikadosblog.blogspot.com\n");
	printf("Developers    : Lunatik, Arikado\n");
	printf("Other Credits : giantpune, SifJar, PheonixTank, Bushing\n\n");
	printf("Press A to continue. Press [Home] to exit.");		
	VIDEO_WaitVSync();

	u32 button;
	while (Controller::ScanPads(&button))
	{
		if (button == WPAD_BUTTON_HOME) System::Exit(true);		
		if (System::State != SystemState::Running) return;
		if (button == WPAD_BUTTON_A) return;		
	}
}

void Main::ShowMainMenu()
{
	u32 button = 0;
	int selection = 0;
	u8 menuMax = 4;
	int scriptsd = 0;
	int scriptusb = 0;
	if (SD::Mount() && File::Exists("sd:/dopscript.xml")) {
		scriptsd = 1;
		menuMax = 5;
	}
	if (USB::Mount() && File::Exists("usb:/dopscript.xml")) {
		scriptusb = 1;
		menuMax = 6;
	}

	while (System::State == SystemState::Running)
	{
		VIDEO_WaitVSync();
		Console::ClearScreen();
		printf("%sIOS, BC, MIOS%s\n", (selection == 0 ? AnsiSelection : ""), AnsiNormal);
		printf("%sChannels%s\n", (selection == 1 ? AnsiSelection : ""), AnsiNormal);
		printf("%sSystem Menu%s\n", (selection == 2 ? AnsiSelection : ""), AnsiNormal);
		printf("%sBoot2%s\n", (selection == 3 ? AnsiSelection : ""), AnsiNormal);
		printf("%sScan the Wii's internals (SysCheck)%s\n", (selection == 4 ? AnsiSelection : ""), AnsiNormal);
		printf("\n\n");
		// TODO: Make this option choose using a filebrowser
		if (scriptsd)
			printf("%sRun the script dopscript.xml from SD%s", (selection == 5 ? AnsiSelection : ""), AnsiNormal);
		if (scriptusb)
			printf("%sRun the script dopscript.xml from USB%s", (selection == 6 ? AnsiSelection : ""), AnsiNormal);

		Console::SetRowPosition(Console::Rows-8);
		Console::PrintSolidLine();
		printf("[%s][%s]  Change Selection\n", UpArrow, DownArrow);
		printf("[A]     Select\n");
		printf("[B]     Back\n");
		printf("[Home]  Exit");

		Console::PrintSolidLine();
		printf("Current IOS: %s", CurrentIOS->Name.c_str());
		VIDEO_WaitVSync();
		
		while (Controller::ScanPads(&button))
		{
			if (button == WPAD_BUTTON_HOME) System::Exit();
			if (System::State != SystemState::Running) return;

			if (button == WPAD_BUTTON_B) return;

			if (button == WPAD_BUTTON_A)
			{					
				switch (selection)
				{
					case 0: ShowIosMenu(); break;
					case 1: ShowChannelsMenu(); break;
					case 2: ShowSysMenusMenu(); break;
					case 3: ShowBoot2Menu(); break;
					case 4: RunSysCheck(); break;
					case 5: 
						Scripting::RunScript("sd:/dopscript.xml");
						break;
					case 6:
						Scripting::RunScript("usb:/dopscript.xml");
						break;
				}
			}
			if (button == WPAD_BUTTON_DOWN) selection++;
			if (button == WPAD_BUTTON_UP) selection--;			

			if (button == WPAD_BUTTON_UP && selection == 5 && scriptsd == 0) selection++;
			if (button == WPAD_BUTTON_DOWN && selection == 5 && scriptsd == 0) selection--;
			if (button) break;
		}

		if (selection < 0) selection = menuMax;
		if (selection > menuMax) selection = 0;
	}
}

void Main::ShowIosMenu()
{
	RefreshIosMatrix();
	IosMatrixIterator ios = CurrentIOS;
	IosRevisionIterator rev = ios->Revisions.Last();
	int selection = 0;
	u32 button = 0;
	u16 installedVer = SysTitle::GetVersion(ios->TitleId);
	bool allowInstall = false;
	bool allowUninstall = false;

	while (System::State == SystemState::Running)
	{
		VIDEO_WaitVSync();
		Console::ClearScreen();

		Console::SetColors(Color::Blue, Bold::Off, Color::White, Bold::On);
		Console::PrintCenter((Console::Cols/2)-1, "Select the IOS to DOP.");
		Console::ResetColors();

		Console::SetColPosition(Console::Cols / 2); printf("\xB3");

		Console::SetColors(Color::Blue, Bold::Off, Color::White, Bold::On);
		Console::PrintCenter((Console::Cols/2)-1, "Currently Installed");
		Console::ResetColors();
		printf("\n");

		// Line #1 - IOS Selection
		printf("Title    : ");		  
		Console::SetFgColor(Color::Yellow, Bold::On);	
		printf("%-*s ", 2, (ios > IosMatrix->First()) ? "<<" : "");
		Console::ResetColors();

		if (selection == 0) Console::SetColors(Color::White, Color::Black);
		Console::PrintCenter(11, ios->Name.c_str());
		Console::ResetColors();
	
		Console::SetFgColor(Color::Yellow, Bold::On);
		printf(" %*s", 2, (ios < IosMatrix->Last()) ? ">>" : "");
		Console::ResetColors();

		Console::SetColPosition(Console::Cols / 2); printf("\xB3");
	
		// Print Currently Installed Revision
		Console::SetColPosition(Console::Cols/2); printf("\xB3");
		if (installedVer == 0) 
		{
			Console::PrintCenter((Console::Cols/2)-1, "Not Installed");
		}
		else 
		{
			char verString[15] = {};
			sprintf(verString, "v%u", installedVer);
			IosRevisionIterator revDetails = ios->Revisions.Item(installedVer);
			if (revDetails != (IosRevisionIterator)NULL && revDetails->IsStub) strcat(verString, " (STUB)");
			Console::PrintCenter((Console::Cols/2)-1, verString);
		}

		// Line #3 - Revision Selection
		printf("\n%s", "Version  : ");
		if (rev != (IosRevisionIterator)NULL && rev != ios->Revisions.end())
		{
			Console::SetFgColor(Color::Yellow, Bold::On);
			printf("%-*s ", 2, (rev > ios->Revisions.First()) ? "<<" : "");
			Console::ResetColors();

			if (selection == 1) Console::SetColors(Color::White, Color::Black);
			Console::PrintCenter(11, "v%d", rev->Id);
			Console::ResetColors();

			Console::SetFgColor(Color::Yellow, Bold::On);
			printf(" %*s", 2, (rev < ios->Revisions.Last()) ? ">>" : "");
			Console::ResetColors();
		}
		else Console::PrintCenter(18, (char*)"(N/A)");

		Console::SetColPosition((Console::Cols/2)); printf("\xB3");
		printf("\n");

		// Print Horizonal Line - Special Version
		for (u32 i = 0; i < (Console::Cols/2); i++) printf("\xC4");
		printf("\xC1");
		for (u32 i = 0; i < (Console::Cols/2); i++) printf("\xC4");
		
		Console::SetPosition(4, 0);
		Console::SetFgColor(Color::White, Bold::On);
		printf("IOS Description\n");
		Console::ResetColors();

		printf("%s\n", ios->Description.c_str());

		if (rev != (IosRevisionIterator)NULL)
		{
			if 
			(
				rev->Id > 0 && (rev->Description.size() > 0 || rev->Note.size() > 0 || rev->IsStub || !rev->NusAvailable ||
				rev->CanPatchFakeSign || rev->CanPatchEsIdentify || rev->CanPatchNandPermissions)
			)
			{
				Console::SetPosition(9, 0);
				Console::SetFgColor(Color::White, Bold::On);
				printf("Selected Version Details\n");
				Console::ResetColors();

				if (rev->Description.size() > 0) printf("Description: %s\n", rev->Description.c_str());
				if (rev->IsStub) 
				{
					Console::SetFgColor(Color::Yellow, Bold::On);
					printf("Version is a STUB!\n");
					Console::ResetColors();
				}
				if (rev->CanPatchFakeSign) printf("Can patch FakeSign (Trucha)\n");
				if (rev->CanPatchEsIdentify) printf("Can patch ES_Identify\n");
				if (rev->CanPatchNandPermissions) printf("Can patch NAND Permissions\n");
				if (!rev->NusAvailable)
				{
					Console::SetFgColor(Color::Yellow, Bold::On);
					printf("Not Available on NUS!");
					Console::ResetColors();
				}
                if (rev->DowngradeBlocked)
                {
                    Console::SetFgColor(Color::Yellow, Bold::On);
                    printf("Version cannot downgrade other IOSes\n");
                    Console::ResetColors();
                }
				if (rev->Note.size() > 0) 
				{
					Console::SetFgColor(Color::Yellow, Bold::On);
					printf("Note: %s\n", rev->Note.c_str());
					Console::ResetColors();
				}
			}
		}

		Console::SetPosition(Console::Rows-8, 0);
		if (rev != (IosRevisionIterator)NULL && rev->Id > 0)
		{
			printf("Downloadable : %s\n", rev->NusAvailable ? "Yes" : "No");
			printf("WAD Filename : %s-64-v%u.wad", ios->Name.c_str(), rev->Id); 
		}
		else printf("\n");
		Console::PrintSolidLine();
		printf("[\x11|\x10] Change Selection%*s", 5, "");
		
		if (installedVer > 0) 
		{
			allowUninstall = true;
			printf("[-] Uninstall");
		}
		else allowUninstall = false;

		printf("\n[\x1E|\x1F] Change Selection%*s", 5, "");

		if (ios->Revisions.size() > 0 && rev->Id > 0) 
		{
			allowInstall = true;
			printf("[+] Install");
		}
		else allowInstall = false;

		printf("\n[Home] Exit   [B] Back     ");

		if (allowInstall) printf("[Hold A, Press +] Install Into Another Slot");

		Console::PrintSolidLine();
		printf("Current IOS: %s", CurrentIOS->Name.c_str());
		VIDEO_WaitVSync();

		while (Controller::ScanPads(&button))
		{
			if (button == WPAD_BUTTON_HOME) System::Exit();
			if (System::State != SystemState::Running) return;

			if (button == (WPAD_BUTTON_A+WPAD_BUTTON_PLUS) && allowInstall) 
			{
				if (InstallIOS(ios, rev, true) > -1)
				{
					u32 tmpIosId = ios->Id;
					u16 tmpRevId = rev->Id;				
					RefreshIosMatrix();
					ios = IosMatrix->Item(tmpIosId);
					rev = ios->Revisions.Item(tmpRevId);
				}	
			}

			if (button == WPAD_BUTTON_PLUS && allowInstall)
			{
				if (InstallIOS(ios, rev, false) > -1) installedVer = SysTitle::GetVersion(ios->TitleId);
			}

			if (button == WPAD_BUTTON_MINUS && allowUninstall)
			{
				if (UninstallIOS(ios) > -1) installedVer = SysTitle::GetVersion(ios->TitleId);
			}

			if (button == WPAD_BUTTON_B) return;

			if (rev != (IosRevisionIterator)NULL)
			{
				if (button == WPAD_BUTTON_UP) selection--;
				if (button == WPAD_BUTTON_DOWN) selection++;
			}

			if (selection < 0) selection = 1;
			if (selection > 1) selection = 0;

			if (button == WPAD_BUTTON_RIGHT) 
			{
				if (selection == 0 && ios != IosMatrix->Last()) 
				{ 
					++ios; 
					installedVer = SysTitle::GetVersion(ios->TitleId);
					if (!ios->Revisions.empty())
					{
						rev = ios->Revisions.Last(); 					
						if (rev != ios->Revisions.First() && rev->IsStub && (rev-1)->NusAvailable) --rev;
					}
					else rev = (IosRevisionIterator) NULL;
				}
				if (selection == 1) ++rev;
			}

			if (button == WPAD_BUTTON_LEFT) 
			{
				if (selection == 0 && ios != IosMatrix->First()) 
				{
					--ios; 
					installedVer = SysTitle::GetVersion(ios->TitleId);
					if (!ios->Revisions.empty())
					{
						rev = ios->Revisions.Last(); 
						if (rev != ios->Revisions.First() && rev->IsStub && (rev-1)->NusAvailable) --rev;
					}
					else rev = (IosRevisionIterator)NULL;
				}
				if (selection == 1) --rev;
			}
			
			if (ios > IosMatrix->Last()) 
			{
				ios = IosMatrix->Last();		
				rev = ios->Revisions.Last();
			}
			if (ios < IosMatrix->First()) 
			{
				ios = IosMatrix->First();
				rev = ios->Revisions.Last();
			}

			if (rev != (IosRevisionIterator)NULL)
			{
				if (rev > ios->Revisions.Last()) rev = ios->Revisions.Last();
				if (rev < ios->Revisions.First()) rev = ios->Revisions.First();
			}

			if (button) break;
		}
	}
}

ChannelMatrix* Main::InitChannelMatrix()
{
	switch (SelectedRegion)
	{
		case 1: return new ChannelMatrix(CONF_REGION_EU);
		case 2: return new ChannelMatrix(CONF_REGION_JP);
		case 3: return new ChannelMatrix(CONF_REGION_KR);
		default: return new ChannelMatrix(CONF_REGION_US);
	}
}

void Main::ShowChannelsMenu()
{
	u32 button;
	u32 menuSelection = 0;
	const u32 menuMax = 1;
	u16 installedSubVer = 0;

	ChannelMatrix *matrix = InitChannelMatrix();

	ChannelIterator item = matrix->DefaultChannel();
	u16 installedVer = SysTitle::GetVersion(item->TitleId);		
	if (item->SubTitleId > 0) installedSubVer = SysTitle::GetVersion(item->SubTitleId);

	while (System::State == SystemState::Running)
	{
		if (item->SubTitleId == 0) installedSubVer = 0;
		VIDEO_WaitVSync();
		Console::ClearScreen();		

		Console::SetColors(Color::Blue, Bold::Off, Color::White, Bold::On);
		Console::PrintCenter((Console::Cols/2)-1, "Select the Channel to DOP.");
		Console::ResetColors();

		Console::SetColPosition(Console::Cols / 2); printf("\xB3");

		Console::SetColors(Color::Blue, Bold::Off, Color::White, Bold::On);
		Console::PrintCenter((Console::Cols/2)-1, "Currently Installed");
		Console::ResetColors();
		printf("\n");

		// Line #1 Channel Selection
		printf("Channel : ");
		Console::SetFgColor(Color::Yellow, Bold::On);
		printf("%-*s", 2, (item > matrix->First()) ? "<<" : "");
		Console::ResetColors();

		printf(" %s", (menuSelection == 0 ? AnsiSelection : AnsiNormal));
		Console::PrintCenter(21, "%s", item->Name.c_str());
		Console::ResetColors();

		Console::SetFgColor(Color::Yellow, Bold::On);
		printf(" %-*s", 2, (item < matrix->Last()) ? ">>" : "");
		Console::ResetColors();		

		// Print Currently Installed Revision
		Console::SetColPosition(Console::Cols/2); printf("\xB3");

		if (installedVer == 0) Console::PrintCenter((Console::Cols/2)-1, "Not Installed");
		else 
		{		
			char versionStr[256];
			sprintf(versionStr, "v%u", installedVer);			
			if (installedSubVer > 0) snprintf(versionStr, sizeof(versionStr), "%s | v%u", versionStr, installedSubVer);
			Console::PrintCenter((Console::Cols/2)-1, "%s", versionStr);
			memset(versionStr, 0, sizeof(versionStr));
		}

		// Line 2 - Region
		printf("\n");
		printf("Region  : ");
		Console::SetFgColor(Color::Yellow, Bold::On);
		printf("%-*s", 2, "<<");
		Console::ResetColors();

		printf(" %s", (menuSelection == 1 ? AnsiSelection : AnsiNormal));
		Console::PrintCenter(21, "%s", Regions[SelectedRegion].Name);
		Console::ResetColors();

		Console::SetFgColor(Color::Yellow, Bold::On);
		printf(" %-*s", 2, ">>");
		Console::ResetColors();

		// Print Horizonal Line - Special Version
		Console::SetColPosition(Console::Cols / 2); printf("\xB3");
		printf("\n");
		for (u32 i = 0; i < (Console::Cols/2); i++) printf("\xC4");
		printf("\xC1");
		for (u32 i = 0; i < (Console::Cols/2); i++) printf("\xC4");

		// Details
		Console::SetFgColor(Color::White, Bold::On);
		printf("Details\n");
		Console::ResetColors();

		printf("Primary WAD     : %s-NUS.wad\n", item->Name.c_str());
		if (item->SubTitleId != 0) 
		{
			printf("Secondary Title : %08X-%08X\n", TITLEID1(item->SubTitleId), TITLEID2(item->SubTitleId));
			printf("Secondary WAD   : %s-%c-NUS.wad\n", item->Name.c_str(), Regions[SelectedRegion].Char);
		}

		Console::SetRowPosition(Console::Rows - 9);
		Console::PrintSolidLine();
		printf("[%s][%s][%s][%s] Change Selection\n", LeftArrow, RightArrow, UpArrow, DownArrow);
		printf("[+]          Install\n");

		if (installedVer > 0) printf("[-]          Uninstall\n");
		else printf("\n");

		printf("[B]          Back\n");
		printf("[Home]       Exit");
		Console::PrintSolidLine();
		printf("Current IOS: %s", CurrentIOS->Name.c_str());

		VIDEO_WaitVSync();
		
		while (Controller::ScanPads(&button))
		{
			if (button == WPAD_BUTTON_HOME) System::Exit();
			if (System::State != SystemState::Running) goto end;
	
			if (button == WPAD_BUTTON_PLUS)
			{
				if (InstallChannel(item) > -1) 
				{
					installedVer = SysTitle::GetVersion(item->TitleId);
					if (item->SubTitleId > 0) installedSubVer = SysTitle::GetVersion(item->SubTitleId);					
				}
			}

			if (button == WPAD_BUTTON_MINUS && installedVer > 0)
			{
				if (UninstallChannel(item) > -1) 
				{
					installedVer = SysTitle::GetVersion(item->TitleId);
					if (item->SubTitleId > 0) installedSubVer = SysTitle::GetVersion(item->SubTitleId);
				}
			}

			if (button == WPAD_BUTTON_DOWN) menuSelection++;
			if (button == WPAD_BUTTON_UP) menuSelection--;

			if (menuSelection > menuMax) menuSelection = 0;
			if (menuSelection < 0) menuSelection = menuMax;

			if (button == WPAD_BUTTON_LEFT)
			{
				if (menuSelection == 0 && item > matrix->First()) 
				{
					--item;
					installedVer = SysTitle::GetVersion(item->TitleId);
					if (item->SubTitleId > 0) installedSubVer = SysTitle::GetVersion(item->SubTitleId);
				}
				if (menuSelection == 1) 
				{
					SelectedRegion--;
					delete matrix; matrix = NULL;
				}
			}
			if (button == WPAD_BUTTON_RIGHT)
			{
				if (menuSelection == 0 && item < matrix->Last())
				{
					++item;
					installedVer = SysTitle::GetVersion(item->TitleId);
					if (item->SubTitleId > 0) installedSubVer = SysTitle::GetVersion(item->SubTitleId);
				}
				if (menuSelection == 1) 
				{
					SelectedRegion++;
					delete matrix; matrix = NULL;
				}
			}

			if (SelectedRegion < 0) SelectedRegion = REGIONS_HI;
			if (SelectedRegion > REGIONS_HI) SelectedRegion = REGIONS_LO;

			if (!matrix)
			{
				matrix = InitChannelMatrix();
				item = matrix->DefaultChannel();
				installedVer = SysTitle::GetVersion(item->TitleId);
				if (item->SubTitleId > 0) installedSubVer = SysTitle::GetVersion(item->SubTitleId);
			}

			if (button == WPAD_BUTTON_B) goto end;
			if (button) break;
		}
	}

end:
	delete matrix; matrix = NULL;
}

int Main::InstallChannel(ChannelIterator channel)
{
	VIDEO_WaitVSync();
	Console::ClearScreen();
	gprintf("\n");
	gcprintf("Are you sure you want to install %s \n", channel->Name.c_str());
	if (!Console::PromptYesNo()) return -1;
	gcprintf("\n");
	int ret = Title::Install(channel);
	gcprintf("\nPress any key to return to the menu.\n");
	Controller::WaitAnyKey();
	return ret;
}

int Main::UninstallChannel(ChannelIterator channel)
{
	VIDEO_WaitVSync();
	Console::ClearScreen();
	gprintf("\n");
	gcprintf("Are you sure you want to uninstall %s?\n", channel->Name.c_str());
	if (!Console::PromptYesNo()) return -1;
	gcprintf("\n");

	int ret = 0;

	if (channel->SubTitleId > 0) 
	{
		gcprintf("Removing Secondary Title %08X-%08X\n", TITLEID1(channel->SubTitleId), TITLEID2(channel->SubTitleId));
		ret  = Title::Uninstall(channel->SubTitleId);
		gcprintf("\n");
	}

	gcprintf("Removing %s\n", channel->Name.c_str());
	if (ret > -1) ret = Title::Uninstall(channel->TitleId);
	gcprintf("\n");

	gcprintf("\nPress any key to return to the menu.\n");
	Controller::WaitAnyKey();
	return ret;
}

SysMenuMatrix* Main::InitSysMenuMatrix()
{
	switch (SelectedRegion)
	{
		case 1: return new SysMenuMatrix(CONF_REGION_EU);
		case 2: return new SysMenuMatrix(CONF_REGION_JP);
		case 3: return new SysMenuMatrix(CONF_REGION_KR);
		default: return new SysMenuMatrix(CONF_REGION_US);
	}
}

void Main::ShowSysMenusMenu()
{
	u32 button = 0;
	u32 menuSelection = 0;
	const u32 menuMax = 1;
	const u64 titleId = 0x100000002ULL;

	SysMenuMatrix *matrix = InitSysMenuMatrix();
	
	SelectedSysMenu = matrix->Last();
	bool priiloaderInstalled = Tools::IsPriiloaderInstalled();
	SysMenuItem *installed = SysMenuMatrix::GetRevisionInfo(SysTitle::GetVersion(titleId));
	
	while (System::State == SystemState::Running)
	{
		VIDEO_WaitVSync();

		Console::ClearScreen();

		Console::SetColors(Color::Blue, Bold::Off, Color::White, Bold::On);
		Console::PrintCenter((Console::Cols/2)-1, "Select the System Menu to DOP.");
		Console::ResetColors();

		Console::SetColPosition(Console::Cols / 2); printf("\xB3");

		Console::SetColors(Color::Blue, Bold::Off, Color::White, Bold::On);
		Console::PrintCenter((Console::Cols/2)-1, "Currently Installed");
		Console::ResetColors();
		printf("\n");

		/* Line #1 Selection & Currently Installed */
		printf("Selection : %s%-*s%s", (menuSelection == 0 ? AnsiSelection : AnsiNormal), 20, SelectedSysMenu->Name.c_str(), AnsiNormal);		
		Console::SetColPosition(Console::Cols / 2); printf("\xB3");
		Console::PrintCenter((Console::Cols/2)-1, (installed ? installed->Name.c_str() : "Unknown"));
		printf("\n");

		/* Line #2 Region & Currently Installed */
		printf("Region    : %s%-*s%s", (menuSelection == 1 ? AnsiSelection : AnsiNormal), 20, Regions[SelectedRegion].Name, AnsiNormal);
		Console::SetColPosition(Console::Cols / 2); printf("\xB3");
		Console::PrintCenter((Console::Cols/2)-1, (installed ? Tools::GetRegionString(installed->RegionId) : "Unknown"));
		printf("\n");

		// Print Horizonal Line - Special Version
		for (u32 i = 0; i < (Console::Cols/2); i++) printf("\xC4");
		printf("\xC1");
		for (u32 i = 0; i < (Console::Cols/2); i++) printf("\xC4");

		/* Middle Section */
		Console::SetFgColor(Color::White, Bold::On);
		printf("Selection Details\n"); 
		Console::ResetColors();
		
		printf("IOS Required        : IOS%u (v%u)\n", SelectedSysMenu->IosRequired, SelectedSysMenu->IosRevision);
		
		u16 installedVer = SysTitle::GetVersion(TITLEID(1, SelectedSysMenu->IosRequired));		
		if (installedVer == 0) printf("IOS Installed       : Not Installed\n");
		else printf("IOS Installed       : IOS%u (v%u)\n", SelectedSysMenu->IosRequired, installedVer);
		printf("IOS WAD             : IOS%u-64-v%u.wad\n", SelectedSysMenu->IosRequired, SelectedSysMenu->IosRevision);
		printf("System Menu WAD     : System Menu-NUS-v%u.wad\n", SelectedSysMenu->Revision);
		printf("Priiloader Detected : %s\n", (priiloaderInstalled ? "Yes" : "No"));

		if (installedVer != SelectedSysMenu->IosRevision) printf("\nNOTE: Installer will install required IOS\n");

		Console::SetRowPosition(Console::Rows - 8);
		Console::PrintSolidLine();
		printf("[%s][%s][%s][%s] Change Selection\n", LeftArrow, RightArrow, UpArrow, DownArrow);
		printf("[+]          Install\n");
		printf("[B]          Back\n");
		printf("[Home]       Exit");
		Console::PrintSolidLine();
		printf("Current IOS: %s", CurrentIOS->Name.c_str());
		VIDEO_WaitVSync();			

		while (Controller::ScanPads(&button))
		{
			if (button == WPAD_BUTTON_HOME) System::Exit();
			if (System::State != SystemState::Running) goto final;

			if (button == WPAD_BUTTON_PLUS)
			{
				Console::ClearScreen();
				if (Title::Install(SelectedSysMenu) > -1)
				{
					delete installed;
					installed = SysMenuMatrix::GetRevisionInfo(SysTitle::GetVersion(titleId));
				}
				gcprintf("Press Any key to return to the menu.\n");
				Controller::WaitAnyKey();
				break;
			}

			if (button == WPAD_BUTTON_DOWN) menuSelection++;
			if (button == WPAD_BUTTON_UP) menuSelection--;

			if (menuSelection > menuMax) menuSelection = 0;
			if (menuSelection < 0) menuSelection = menuMax;

			if (button == WPAD_BUTTON_LEFT)  
			{
				if (menuSelection == 0) --SelectedSysMenu;
				
				if (menuSelection == 1) 
				{
					SelectedRegion--;
					delete matrix; matrix = NULL;
				}
			}
			if (button == WPAD_BUTTON_RIGHT) 
			{
				if (menuSelection == 0) ++SelectedSysMenu;
				if (menuSelection == 1) 
				{
					SelectedRegion++;	
					delete matrix; matrix = NULL;
				}
			}
			if (SelectedRegion < REGIONS_LO) SelectedRegion = REGIONS_HI;
			if (SelectedRegion > REGIONS_HI) SelectedRegion = REGIONS_LO;

			if (!matrix)
			{
				matrix = InitSysMenuMatrix();
				SelectedSysMenu = matrix->Last();
			}

			if (SelectedSysMenu > matrix->Last()) SelectedSysMenu = matrix->First();
			if (SelectedSysMenu < matrix->First()) SelectedSysMenu = matrix->Last();

			if (button == WPAD_BUTTON_B) goto final;
			if (button) break;
		}
	}

final:
	delete installed; installed = NULL;
	delete matrix; matrix = NULL;
}

void Main::RunSysCheck()
{
	Console::ClearScreen();
	if (!Console::PromptContinue()) return;

	u32 deviceId = 0;
	ostringstream os;
	int iosTestCnt = 1;
	
	SysCheck::RemoveBogusTicket();
	SysCheck::RemoveBogusTitle();

	ES_GetDeviceID(&deviceId);

	VIDEO_WaitVSync();
	Console::ClearScreen();
	printf("[*] Region: %s\n", Tools::GetRegionString(CONF_GetRegion()));
	printf("[*] Device ID: %u\n", deviceId);
	printf("[*] Hollywood Version: 0x%x\n", SYS_GetHollywoodRevision());
	printf("[*] Getting certs.sys from the NAND...");
	printf("%s\n", (System::Cert) ? "[DONE]" : "[FAIL]");
	printf("\n");

	BuildSysCheckTable();	
	if (SysCheckTable.size() == 0)
	{
		printf(">> ERROR! This IOS cannot retrive an IOS list.\n");
		printf(">> ERROR! Please use a different IOS to run SysCheck\n\n");
		printf("Press any key to return to the menu\n");
		Controller::WaitAnyKey();
		goto final;
	}	
	
	printf("[*] Found %i IOSes that are safe to test.\n\n", SysCheckTable.size()-1);
	sleep(5);

	if (System::State != SystemState::Running) goto final;

	os << "\"DOP-Mii Report\"\n";
	os << "\"Region\", " << Tools::GetRegionString(CONF_GetRegion()) << endl;
	os << "\"Hollywood Version\", 0x" << hex << SYS_GetHollywoodRevision() << endl;
	os << "\"Wii Unique DeviceID\", " << dec << deviceId << endl << endl;	
	os << "IOS Version, FakeSign, ES_Identify, NAND, Flash" << endl;

	Console::ClearScreen();
	Console::SetFgColor(Color::White, Bold::On);
	printf("%-13s %-10s %-11s %-10s %-10s\n", "IOS Version", "FakeSign", "ES_Identify", "NAND", "Flash");
	Console::ResetColors();

	for (u32Iterator ios = SysCheckTable.begin(); ios < SysCheckTable.end(); ++ios)
	{	
		if (System::State != SystemState::Running) goto final;

		bool hasFakeSign = false;
		bool hasEsIdentify = false;
		bool hasNandPermissions = false;
		bool hasFlashAccess = false;

		VIDEO_WaitVSync();
		System::ReloadIOS(*ios, false);
		char iosString[50] = "";				
		sprintf(iosString, "%-3u (v%d)", *ios, IOS_GetRevision());
		gprintf("\nTesting IOS %s\n", iosString);
		printf("%-13s ", iosString);
		printf("%-10s ", (hasFakeSign = SysCheck::CheckFakeSign()) ? "Enabled" : "Disabled");
		printf("%-11s ", (hasEsIdentify = SysCheck::CheckEsIdentify()) ? "Enabled" : "Disabled");
		printf("%-10s ", (hasNandPermissions = SysCheck::CheckNandPermissions()) ? "Enabled" : "Disabled");
		printf("%-10s ", (hasFlashAccess = SysCheck::CheckFlashAccess()) ? "Enabled" : "Disabled");
		printf("\n");
		VIDEO_WaitVSync();

		os << "\"" << iosString << "\"" 
		   << "," << (hasFakeSign ? "Enabled" : "Disabled")
		   << "," << (hasEsIdentify ? "Enabled" : "Disabled")
		   << "," << (hasNandPermissions ? "Enabled" : "Disabled")
		   << "," << (hasFlashAccess ? "Enabled" : "Disabled")
		   << endl;

		if ((iosTestCnt % (Console::Rows - 2)) == 0) 
		{
			WPAD_Init();
			printf("Press [A] Continue, [Home] Return Loader");
			u32 button;
			while (Controller::ScanPads(&button))
			{
				if (button == WPAD_BUTTON_HOME) System::Exit();
				if (System::State != SystemState::Running) goto final;
				if (button == WPAD_BUTTON_A) break;
			}
			WPAD_Shutdown();
			Console::ClearLine();
		}

		iosTestCnt++;
	}
	
	System::ReloadIOS(CurrentIOS->Id, true);
		
	if (System::State != SystemState::Running) goto final;

	Console::PrintSolidLine();
	printf("*** SysCheck Complete ***\n\n");
	printf("Where would you like to save the report?\n");
	printf("[+] SD Card    [-] USB Device   [B] Don't Save\n");

	u32 button;
	while (Controller::ScanPads(&button))
	{
		if (button == WPAD_BUTTON_HOME) goto final;
		if (System::State != SystemState::Running) goto final;	

		if (button == WPAD_BUTTON_B) { printf("\n\n"); break;}
		if (button == WPAD_BUTTON_PLUS)
		{
			if (SD::Mount())
			{
				printf("\nCreating report to sd:/DOP-Mii-Report.csv...\n\n");
				FILE *logFile = fopen("sd:/DOP-Mii-Report.csv", "wb");
				fwrite(os.str().c_str(), 1, os.str().size(), logFile);
				fclose(logFile);
				printf("All done, you can find the report on the root of your SD Card\n\n");
			}
			else printf("Failed to create report onto your SD Card!\n\n");

			printf("Press any key to return to exit.");
			Controller::WaitAnyKey();

			break;
		}

		if (button == WPAD_BUTTON_MINUS)
		{		
			if (USB::Mount())
			{
				printf("\nCreating report to usb:/DOP-Mii-Report.csv\n\n");
				FILE *logFile = fopen("usb:/DOP-Mii-Report.csv", "wb");
				fwrite(os.str().c_str(), 1, os.str().size(), logFile);
				fclose(logFile);
				printf("All done, you can find the report on the root of your USB Device\n\n");
			}				
			else printf("Failed to create log on your USB device!\n\n");				

			printf("Press any key to return to exit.");
			Controller::WaitAnyKey();

			break;
		}
	}

final:
	SysCheckTable.clear();
	os.clear(); os.str("");	
}

void Main::ShowInitialMenu()
{
	int selection = 0;
	u32List iosList;
	u32Iterator menuIOS;
	u8 maxMenu = 3;

	bool isAHBPROT = HAVE_AHBPROT;

	/* Get IOS versions */
	System::GetInstalledIosIdList(iosList);

	/* Sort list */
	sort(iosList.begin(), iosList.end());

	/* Set default version */
	for (menuIOS = iosList.begin(); menuIOS < iosList.end(); ++menuIOS)
	{		
		if (*menuIOS == (u32)Settings::Instance().DefaultIOS)
		{
			CurrentIOS = IosMatrix->Item(*menuIOS);
			break;
		}

		if ((int)*menuIOS == IOS_GetVersion()) CurrentIOS = IosMatrix->Item(*menuIOS);		
	}

	while (System::State == SystemState::Running) 
	{		
		VIDEO_WaitVSync();
		Console::ClearScreen();
		printf("Which IOS would you like to use to install other IOSes?\n");
		if (isAHBPROT)
			printf("Note: choosing the first or third options will result in an AHBPROT usage prompt.");
		
		printf("%sIOS: %u%s\n", (selection == 0 ? AnsiSelection : ""), *menuIOS, AnsiNormal);
		printf("%sInstall IOS36 (v%d) w/FakeSign%s\n", (selection == 1 ? AnsiSelection : ""), IOS36Version, AnsiNormal);
		printf("%sScan the Wii's internals (SysCheck)%s\n", (selection == 2 ? AnsiSelection : ""), AnsiNormal);
		printf("%sExit%s", (selection == 3 ? AnsiSelection : ""), AnsiNormal);	

		Console::SetRowPosition(Console::Rows-7);
		Console::PrintSolidLine();
		printf("[%s][%s] Change Selection\n", UpArrow, DownArrow);
		
		if (selection == 0) printf("[%s][%s] Change IOS\n", LeftArrow, RightArrow);
		else printf("\n");

		printf("[Home] Exit");
		Console::PrintSolidLine();
		printf("Current IOS: IOS%u", IOS_GetVersion());
		VIDEO_WaitVSync();

		u32 button;
		while (Controller::ScanPads(&button))
		{
			if (button == WPAD_BUTTON_HOME) System::Exit();
			if (System::State != SystemState::Running) return;

			if (button == WPAD_BUTTON_UP) selection--;
			if (button == WPAD_BUTTON_DOWN) selection++;

			if (selection < 0) selection = maxMenu;
			if (selection > maxMenu) selection = 0;

			if (selection == 0)
			{
				if (button == WPAD_BUTTON_LEFT && menuIOS != iosList.begin()) --menuIOS;
				if (button == WPAD_BUTTON_RIGHT && menuIOS != iosList.end()-1) ++menuIOS;
			}	

			if (button == WPAD_BUTTON_A) 
			{ 
				switch (selection)
				{
					case 0:
					
					    if (isAHBPROT) isAHBPROT = ShowAHBPROTMenu();
						
						if (isAHBPROT) 
						{
							CurrentIOS = IosMatrix->Item((u32)IOS_GetVersion());
							// Should do patchy?
							if (!SysCheck::CheckFakeSign()) {
								// Do patchy
								Console::ClearScreen();
								printf("One moment... Applying patches...\n");
								Console::PrintSolidLine();
								RuntimeIOSPatch::Apply();
								printf("\n\n...COMPLETE");
							}
						} else {
							gprintf("Loading IOS\n");
							VIDEO_WaitVSync();
							Console::ClearScreen();
							printf("Loading selected IOS...\n");
														
							CurrentIOS = IosMatrix->Item(*menuIOS);
							System::ReloadIOS(CurrentIOS);
						}

						if (!SysCheck::CheckFakeSign())
						{
							printf("\n\n>> ERROR! Choose an IOS that accepts fake signing! Press A to continue.");
									
							u32 button;
							while (Controller::ScanPads(&button))
							{
								if (button == WPAD_BUTTON_HOME) System::Exit();
								if (System::State != SystemState::Running) goto end;
								if (button == WPAD_BUTTON_A) break;
							}
						}
						else 
						{
							ShowMainMenu(); 
							if (System::State != SystemState::Running) goto end;
							selection = 0;
						}
						break;
					case 1:
						VIDEO_WaitVSync();
						Console::ClearScreen();
						if (!Console::PromptContinue()) break;

						VIDEO_WaitVSync();
						Console::ClearScreen();
						if (InstallFakeSignIOS36() > -1) selection = 0;
						break;
					case 2:
					    if (isAHBPROT) 
						   isAHBPROT = ShowAHBPROTMenu();
						if (!isAHBPROT)
						   RunSysCheck(); 
						isAHBPROT = HAVE_AHBPROT;
						break;
					case 3:
						VIDEO_WaitVSync();
						Console::ClearScreen();
						printf("Are you sure you want to exit?\n");
						if (Tools::IsPriiloaderInstalled()) {
						        gcprintf("[A] Yes    [B] No    [Home] Exit to Priiloader\n");
						} else {
							gcprintf("[A] Yes    [B] No    [Home] Exit to System Menu\n");
						}
						
					        u32 button;
					        while (Controller::ScanPads(&button))
					        {
					                if (System::State != SystemState::Running) break;
					                if (button == WPAD_BUTTON_HOME) System::ExitToPriiloader();
					                if (button == WPAD_BUTTON_A) System::Exit();
					                if (button == WPAD_BUTTON_B) break;
					        }
						break;				
				} // End Switch
			} // End If
			if (button) break;
		}
	}

end:
	iosList.clear();
}

void Main::Run()
{	
	ShowWelcomeScreen();
	if (System::State != SystemState::Running) return;

	ShowInitialMenu();
	return;
}

Main::Main()
{	
	IosMatrix = NULL;
	RefreshIosMatrix();
	switch (CONF_GetRegion()) 
	{
		case CONF_REGION_US: SelectedRegion = 0; break;
		case CONF_REGION_EU: SelectedRegion = 1; break;
		case CONF_REGION_JP: SelectedRegion = 2; break;
		case CONF_REGION_KR: SelectedRegion = 3; break;
		default: SelectedRegion = 0; break;
	}
}

Main::~Main()
{
	SysCheckTable.clear();	
	delete IosMatrix;
}

int main(int argc, char **argv)
{		
	System::Initialize();

	Main *main = new Main();
	main->Run();
	delete main;	
	
	System::Shutdown();
	return 0;
}
