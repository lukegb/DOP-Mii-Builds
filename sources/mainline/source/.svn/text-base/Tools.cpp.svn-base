#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <string>
#include <unistd.h>
#include <algorithm>

#include "Error.h"
#include "Tools.h"
#include "System.h"
#include "Gecko.h"
#include "SysTitle.h"
#include "Nand.h"
#include "WiiMote.h"
#include "DiscLight.h"
#include "Settings.h"

using namespace std;
using namespace IO;

lwp_t Spinner::Thread = LWP_THREAD_NULL;
bool Spinner::Running = false;

void *Spinner::Execute(void *args) 
{	
	const char* spinnerChars = "/-\\|";	
	uint spin  = 0;
	int led = 0;
	int ledDir = 1;

	Settings &settings = Settings::Instance();
	
	if (settings.FlashWiiLeds) DiscLight::SetLevel(100);

	while (Running) 
	{		
		if (!Running) break;				
		printf("\b%c", spinnerChars[spin++]);
		fflush(stdout);
		if (spin >= strlen(spinnerChars)) spin = 0;
		if (settings.FlashWiiMoteLeds)
		{
			led += ledDir;
			if (!led || led > 2) ledDir = -ledDir;
			WiiMote::SetLeds(WPAD_CHAN_0, WIIMOTE_LED_1 << led);
			WiiMote::SetLeds(WPAD_CHAN_1, WIIMOTE_LED_1 << led);
			WiiMote::SetLeds(WPAD_CHAN_2, WIIMOTE_LED_1 << led);
			WiiMote::SetLeds(WPAD_CHAN_3, WIIMOTE_LED_1 << led);
		}		
		if (settings.FlashWiiLeds) DiscLight::Toggle();
		if (!Running) break;
		usleep(50000);			
	}	

	if (settings.FlashWiiLeds) DiscLight::TurnOff();
	
	if (settings.FlashWiiMoteLeds)
	{
		WiiMote::SetLeds(WPAD_CHAN_0, WIIMOTE_LED_1);
		WiiMote::SetLeds(WPAD_CHAN_1, WIIMOTE_LED_2);
		WiiMote::SetLeds(WPAD_CHAN_2, WIIMOTE_LED_3);
		WiiMote::SetLeds(WPAD_CHAN_3, WIIMOTE_LED_4);
	}
	return NULL;
}

void Spinner::Start()
{
	if (Thread != LWP_THREAD_NULL) return;
	Running = true;
	LWP_CreateThread(&Thread, Spinner::Execute, NULL, NULL, 0, 64);
}

void Spinner::Stop()
{
	if (Running) 
	{		
		Running = false;
		LWP_JoinThread(Thread, NULL);
		Thread = LWP_THREAD_NULL;		
	}
}

void* Tools::AllocateMemory(u32 size)
{
	return memalign(32, (size+31)&(~31));
}

const char* Tools::GetRegionString(int regionId)
{
	if (regionId < 0) return "UNKNOWN";

	switch (regionId)
	{
		case CONF_REGION_JP: return "Japan (J)";
		case CONF_REGION_EU: return "Europe (E)";
		case CONF_REGION_US: return "North America (U)";
		case CONF_REGION_KR: return "Korea (K)";
		default: return "UNKNOWN";
	}
}

bool Tools::IsPriiloaderInstalled()
{
	bool retValue = false;
	int size = 0;
	u8 *buffer = NULL;
	const char* checkStr = "priiloader";

	string bootfile = SysTitle::GetBootFilename(TITLEID(1,2));
	if (bootfile.size() == 0) goto end;

	size = Nand::Read(bootfile.c_str(), &buffer);
	if (size < 0) goto end;
	
	for (u32 i = 0; i < size - strlen(checkStr); i++)
	{
		if (!strncmp((char*)buffer+i, checkStr, strlen(checkStr)))		
		{
			retValue = true;
			break;
		}
	}
	
end:
	bootfile.clear();
	delete buffer; buffer = NULL;
	return retValue;
}

int Tools::BackupPriiloader()
{
	int ret = 0;
	const char* tempPath = "/tmp/priiloader";
	const char* dataPath = "/title/00000001/00000002/data";
	char fromfile[256];
	char tofile[256];

	gcprintf("Backing Up Priiloader...");
	gprintf("\n");

	ret = Nand::CreateDir(tempPath);
	if (ret < 0 && ret != -105) 
	{
		gcprintf("\n>> ERROR! CreateDir(%s): %s\n", NandError::ToString(ret));
		return ret;
	}

	/* 
	Backup XXXXXXX.app
	If this fails we need to return an error as this is the core file
	for priiloader. Without this nothing else matters.
	*/
	string bootfile = SysTitle::GetBootFilename(0x100000002ULL);
	sprintf(tofile, "%s/main.app", tempPath);
	gprintf("Copying File\n\t>> From:%s\n\t>> To %s\n", bootfile.c_str(), tofile);
	Spinner::Start();
	ret = Nand::CopyFile(bootfile.c_str(), tofile);
	Spinner::Stop();
	bootfile.clear();
	if (ret < 0) 
	{
		gprintf("\t>> ERROR! Copy File Failed: %s\n", NandError::ToString(ret));
		goto end;
	}
	printf("\b..");

	/* Copy loader.ini */
	sprintf(fromfile, "%s/loader.ini", dataPath);
	sprintf(tofile, "%s/loader.ini", tempPath);
	gprintf("Copying File\n\t>> From:%s\n\t>> To %s\n", fromfile, tofile);
	Spinner::Start();
	ret = Nand::CopyFile(fromfile, tofile);
	Spinner::Stop();
	if (ret < 0) { gprintf("\t>> ERROR! Copy File Failed: %s\n", NandError::ToString(ret));	ret = 0; }
	printf("\b..");

	/* Copy main.bin */
	sprintf(fromfile, "%s/main.bin", dataPath);
	sprintf(tofile, "%s/main.bin", tempPath);
	gprintf("Copying File\n\t>> From:%s\n\t>> To %s\n", fromfile, tofile);
	Spinner::Start();
	ret = Nand::CopyFile(fromfile, tofile);
	Spinner::Stop();
	if (ret < 0) { gprintf("\t>> ERROR! Copy File Failed: %s\n", NandError::ToString(ret));	ret = 0; }
	printf("\b..");

	/* Copy hacks.ini */
	sprintf(fromfile, "%s/hacks.ini", dataPath);
	sprintf(tofile, "%s/hacks.ini", tempPath);
	gprintf("Copying File\n\t>> From:%s\n\t>> To %s\n", fromfile, tofile);
	Spinner::Start();
	ret = Nand::CopyFile(fromfile, tofile);
	Spinner::Stop();
	if (ret < 0) { gprintf("\t>> ERROR! Copy File Failed: %s\n", NandError::ToString(ret));	ret = 0; }
	printf("\b..");

	/* Copy hacks_s.ini */		
	sprintf(fromfile, "%s/hacks_s.ini", dataPath);
	sprintf(tofile, "%s/hacks_s.ini", tempPath);
	gprintf("Copying File\n\t>> From:%s\n\t>> To %s\n", fromfile, tofile);
	Spinner::Start();
	ret = Nand::CopyFile(fromfile, tofile);
	Spinner::Stop();
	if (ret < 0) { gprintf("\t>> ERROR! Copy File Failed: %s\n", NandError::ToString(ret));	ret = 0; }

	gcprintf("\b.Done\n");
end:		
	bootfile.clear();
	memset(fromfile, 0, 256);
	memset(tofile, 0, 256);
	return ret;
}

int Tools::RestorePriiloader()
{
	int ret = 0;

	const char* tempPath = "/tmp/priiloader";
	const char* dataPath = "/title/00000001/00000002/data";
	char fromfile[256];
	char tofile[256];
	string bootfile;
	string copyfile;

	gcprintf("\nRestoring Priiloader...");
	gprintf("\n");
	
	/* 
	Restore XXXXXXX.app
	If this fails we need to return an error as this is the core file
	for priiloader. Without this nothing else matters.
	*/
	
	// Just to be sure lets make sure the new boot file isn't already priiloader
	if (!Tools::IsPriiloaderInstalled())
	{
		// First copy the original file to the alternate filename
		bootfile = SysTitle::GetBootFilename(0x100000002ULL);
		copyfile = bootfile; 
		copyfile.replace(33, 1, "1");
		gprintf("Copying File\n\t>> From:%s\n\t>> To %s\n", bootfile.c_str(), copyfile.c_str());
		Spinner::Start();
		ret = Nand::CopyFile(bootfile, copyfile);
		Spinner::Stop();
		if (ret < 0)
		{
			gprintf("\t>> ERROR! Copy File Failed: %s\n", NandError::ToString(ret));
			goto end;
		}
		printf("\b..");

		// Now Copy priiloader back to boot file name.
		sprintf(fromfile, "%s/main.app", tempPath);
		gprintf("Copying File\n\t>> From:%s\n\t>> To %s\n", fromfile , bootfile.c_str());
		Spinner::Start();
		ret = Nand::CopyFile(fromfile, bootfile.c_str());
		Spinner::Stop();
		if (ret < 0) 
		{
			gprintf("\t>> ERROR! Copy File Failed: %s\n", NandError::ToString(ret));
			goto end;
		}
		printf("\b..");
	}
	
	/* Copy loader.ini */
	sprintf(fromfile, "%s/loader.ini", tempPath);
	sprintf(tofile, "%s/loader.ini", dataPath);	
	gprintf("Copying File\n\t>> From:%s\n\t>> To %s\n", fromfile, tofile);
	Spinner::Start();
	ret = Nand::CopyFile(fromfile, tofile);
	Spinner::Stop();
	if (ret < 0) { gprintf("\t>> ERROR! Copy File Failed: %s\n", NandError::ToString(ret));	ret = 0; }
	printf("\b..");

	/* Copy main.bin */
	sprintf(fromfile, "%s/main.bin", tempPath);
	sprintf(tofile, "%s/main.bin", dataPath);	
	gprintf("Copying File\n\t>> From:%s\n\t>> To %s\n", fromfile, tofile);
	ret = Nand::CopyFile(fromfile, tofile);
	if (ret < 0) { gprintf("\t>> ERROR! Copy File Failed: %s\n", NandError::ToString(ret));	ret = 0; }
	printf(".");

	/* Copy hacks.ini */
	sprintf(fromfile, "%s/hacks.ini", tempPath);
	sprintf(tofile, "%s/hacks.ini", dataPath);
	gprintf("Copying File\n\t>> From:%s\n\t>> To %s\n", fromfile, tofile);
	ret = Nand::CopyFile(fromfile, tofile);
	if (ret < 0) { gprintf("\t>> ERROR! Copy File Failed: %s\n", NandError::ToString(ret));	ret = 0; }
	printf(".");

	/* Copy hacks_s.ini */		
	sprintf(fromfile, "%s/hacks_s.ini", tempPath);
	sprintf(tofile, "%s/hacks_s.ini", dataPath);
	gprintf("Copying File\n\t>> From:%s\n\t>> To %s\n", fromfile, tofile);
	Spinner::Start();
	ret = Nand::CopyFile(fromfile, tofile);
	Spinner::Stop();
	if (ret < 0) { gprintf("\t>> ERROR! Copy File Failed: %s\n", NandError::ToString(ret));	ret = 0; }

	gcprintf("\b.Done\n");
end:
	memset(fromfile, 0, 256);
	memset(tofile, 0, 256);
	copyfile.clear();
	bootfile.clear();
	return ret;
}

void Tools::StringReplaceI(string &str, const char* from, const char* to) 
{		
	string lowerStr = str;
	string fromStr = from;
	
	transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
	transform(fromStr.begin(), fromStr.end(), fromStr.begin(), ::tolower);
		
	string::size_type pos = 0;
	while ((pos = lowerStr.find(fromStr, pos)) != string::npos)
	{
		lowerStr.replace(pos, fromStr.size(), to);
		str.replace(pos, fromStr.size(), to);
		pos++;
	}

	fromStr.clear();
	lowerStr.clear();
}

bool Tools::StringStartsWithI(std::string &str, const char* value)
{
	bool result = false;
	string lowerStr = str;
	string valueStr = value;
	
	transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
	transform(valueStr.begin(), valueStr.end(), valueStr.begin(), ::tolower);

	if (lowerStr.find(valueStr) == 0) result = true;

	valueStr.clear();
	lowerStr.clear();
	return result;
}