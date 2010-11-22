#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <malloc.h>
#include <gccore.h>

#include "SysTitle.h"
#include "Gecko.h"
#include "Tools.h"
#include "Nand.h"
#include "Error.h"

using namespace IO;

int SysTitle::GetTMD(u64 titleId, tmd **ptmd)
{
	int ret = 0;
	signed_blob *stmd = NULL;
	ret = GetTMD(titleId, &stmd);
	if (ret < 0) goto end;
	if (!stmd) { ret = -1; goto end; }

	*ptmd = (tmd*)Tools::AllocateMemory(SIGNED_TMD_SIZE(stmd));
	memcpy(*ptmd, SIGNATURE_PAYLOAD(stmd), SIGNED_TMD_SIZE(stmd));
end:
	delete stmd; stmd = NULL;
	return ret;
}

int SysTitle::GetTMD(u64 titleId, signed_blob **stmd)
{
	*stmd = NULL;	
	u32 size;

	/* Get TMD size */
	int ret = ES_GetStoredTMDSize(titleId, &size);

	/* If we get an error try getting it from the NAND directly */
	if (ret < 0)
	{
		gprintf("\n>> ES_GetTMDSize Failed with Error: %s\n", EsError::ToString(ret));
		gprintf(">> Attempting to get TMD from NAND\n");
		char filename[256];
		sprintf(filename, "/title/%08x/%08x/content/title.tmd", TITLEID1(titleId), TITLEID2(titleId));		
		ret = Nand::Read(filename, (u8**)stmd);
		gprintf("Filename = %s\n, Nand::Read = %d\n", filename, ret);
		memset(filename, 0, 256);
	}
	else 
	{
		/* Allocate memory */
		*stmd = (signed_blob*)Tools::AllocateMemory(size);
		if (!*stmd) return -1;

		/* Read TMD */
		ret = ES_GetStoredTMD(titleId, *stmd, size);
		if (ret < 0) { delete stmd; stmd = NULL; }

		ret = (int)size;
	}

	return ret;
}

int SysTitle::GetTMDView(u64 titleId, TmdView **tmdv)
{
	*tmdv = NULL;
	u32 size = 0;
	
	int ret = ES_GetTMDViewSize(titleId, &size);
	if (ret < 0) return ret;

	*tmdv = (TmdView*)Tools::AllocateMemory(size);
	if (!*tmdv) return -1;

	ret = ES_GetTMDView(titleId, (u8*)*tmdv, size);
	if (ret < 0) { delete *tmdv; *tmdv = NULL; }

	return ret;
}

u16 SysTitle::GetVersion(u64 titleId)
{
	u16 result;
	GetVersion(titleId, &result);
	return result;
}

int SysTitle::GetVersion(u64 titleId, u16 *version)
{
	*version = 0;
	TmdView *view = NULL;

	int ret = GetTMDView(titleId, &view);
	if (ret < 0) goto end;

	*version = view->TitleVersion;
end:
	delete view; view = NULL;
	return ret;
}

u64 SysTitle::GetSysVersion(u64 titleId)
{
	u64 result;
	GetSysVersion(titleId, &result);
	return result;
}

int SysTitle::GetSysVersion(u64 titleId, u64 *version)
{
	*version = 0;
	tmd *ptmd = NULL;
	int ret = GetTMD(titleId, &ptmd);
	if (ret < 0) goto end;
	*version = ptmd->sys_version;
end:
	delete ptmd; ptmd = NULL;
	return ret;
}

u32 SysTitle::GetSize(u64 titleId)
{
	u32 result;
	GetSize(titleId, &result);
	return result;
}

int SysTitle::GetSize(u64 titleId, u32 *outSize)
{
	*outSize = 0;
	tmd *ptmd = NULL;
	int ret = GetTMD(titleId, &ptmd);
	if (ret < 0) goto end;
	
	for (u32 count = 0; count < ptmd->num_contents; count++)
	{
		tmd_content *content = &ptmd->contents[count];
		*outSize += content->size;
	}

end:
	delete ptmd; ptmd = NULL;
	return ret;
}

string SysTitle::GetBootFilename(u64 titleId)
{
	int ret = 0;
	tmd *ptmd = NULL;
	string filename;

	ret = GetTMD(titleId, &ptmd);
	if (ret < 0 || ptmd == NULL) goto end;

	for (u32 i = 0; i < ptmd->num_contents; i++)
	{
		if (ptmd->contents[i].index == ptmd->boot_index)
		{
			char tmp[256];
			sprintf(tmp, "/title/%08x/%08x/content/%08x.app" , TITLEID1(titleId), TITLEID2(titleId), ptmd->contents[i].cid);
			filename = tmp;
			memset(tmp, 0, sizeof(tmp));
			break;
		}
	}

end:
	delete ptmd; ptmd = NULL;
	return filename;
}

int SysTitle::GetTicket(u64 titleId, signed_blob **sticket)
{
	*sticket = NULL;	
	char filename[256];

	sprintf(filename, "/ticket/%08x/%08x.tik", TITLEID1(titleId), TITLEID2(titleId));		
	int ret = Nand::Read(filename, (u8**)sticket);
	memset(filename, 0, 256);
	return ret;
}

