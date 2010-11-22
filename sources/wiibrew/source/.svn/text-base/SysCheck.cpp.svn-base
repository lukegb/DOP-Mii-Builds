#include <stdio.h>
#include <cstdlib>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <gccore.h>
#include <fat.h>
#include <sdcard/wiisd_io.h>
#include <wiiuse/wpad.h>
#include <stdexcept>

#include "Error.h"
#include "SU_ticket_dat.h"
#include "SysCheck.h"
#include "Tools.h"
#include "Nand.h"
#include "Gecko.h"
#include "Global.h"
#include "System.h"

using namespace IO;

bool SysCheck::CheckFlashAccess()
{
	gprintf("CheckFlashAccess::Nand::OpenRead(/dev/flash) = ");
	int ret = Nand::OpenRead("/dev/flash");
	gprintf("%s\n", NandError::ToString(ret));
	if (ret < 0) return false;
	Nand::Close(ret);
	return true;
}

bool SysCheck::CheckFakeSign()
{
	// We are expecting an error here, but depending on the error it will mean
	// that it is valid for fakesign. If we get -2011 it is definately not patched with fakesign.
	// Lower Revisions will return -106 if it can fakesign	

	gprintf("CheckFakeSign::ES_AddTicket = ");
	int ret = ES_AddTicket((signed_blob*)SU_ticket_dat, SU_ticket_dat_size, System::Cert, System::Cert.Size, NULL, 0);
	gprintf("%s\n", EsError::ToString(ret));
	if (ret > -1) RemoveBogusTicket();
	return (ret > -1 || ret == -1028 || ret == -106);
}

bool SysCheck::CheckEsIdentify()
{
	gprintf("CheckESIdentify::ES_Identify = ");	
	int ret = Identify::AsSuperUser();
	gprintf("%s\n", EsError::ToString(ret)); 
	return (ret > -1);
}

bool SysCheck::CheckNandPermissions()
{
	gprintf("CheckNandPermissions::Nand::OpenReadWrite(/title/00000001/00000002/content/title.tmd) = ");
	int ret = Nand::OpenReadWrite("/title/00000001/00000002/content/title.tmd");
	gprintf("%s\n", NandError::ToString(ret));
	Nand::Close(ret);
	if (ret < 0) return false;
	return true;
}

int SysCheck::RemoveBogusTicket()
{
	int ret = 0;
	u64 titleId = 0x100000000ULL;
	u32 views;
	tikview *viewdata = NULL;
	char filename[30] = "/ticket/00000001/00000000.tik";

	// We going to try the nice way first
	ret = ES_GetNumTicketViews(titleId, &views);
	if (ret < 0) 
	{
		gprintf("\n>> ERROR! ES_GetNumTicketViews: %s\n", EsError::ToString(ret));
		return ret;
	}

	if (!views) { gprintf("Bogus Ticket Not Found\n"); return 0; }

	viewdata = (tikview*)Tools::AllocateMemory(sizeof(tikview) * views);
	if (!viewdata) 
	{
		gcprintf("\n>>ERROR! Out Of Memory\n");
		viewdata = NULL; 
		ret = -1;
		goto end;
	}
	ret = ES_GetTicketViews(titleId, viewdata, views);
	if (ret < 0) 
	{
		gprintf("\n>> ERROR! ES_GetTicketViews: %s\n", EsError::ToString(ret));
		goto end;
	}

	for (u32 i = 0; i < views; i++) 
	{
		tikview v = viewdata[i];
		if (viewdata[i].titleid == titleId) 
		{
			ret = ES_DeleteTicket(&viewdata[i]);
			break;
		}
	}	

end:
	// Ticket Not Found
	if (ret == -106) ret = 0;

	// If nice way fails lets try forcing it
	if (ret < 0) 
	{
		ret = Nand::Delete(filename);
		if (ret < 0) gprintf("\n>> ERROR! Nand::Delete: %s\n", NandError::ToString(ret));
	}

	delete viewdata; viewdata = NULL;
	return ret;
}

int SysCheck::RemoveBogusTitle()
{
	int ret = 0;
	u64 titleId = 0x100000000ULL;

	// Lets try the nice way first
	ret = ES_DeleteTitleContent(titleId);
	if (ret > -1) ret = ES_DeleteTitle(titleId);

	// Title Not Found
	if (ret == -106) ret = 0;

	// We asked the nice way. Let's try doing it manually
	if (ret < 0)
	{
		ret = Nand::Delete("/title/00000001/00000000");
		if (ret < 0) gprintf("\n>> ERROR! Nand::Delete Failed: %s\n", EsError::ToString(ret));
	}

	return ret;
}