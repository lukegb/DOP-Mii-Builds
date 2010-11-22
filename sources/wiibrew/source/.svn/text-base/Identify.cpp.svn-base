#include <gccore.h>
#include "Identify.h"
#include "Nand.h"
#include "Tools.h"
#include "Error.h"
#include "Gecko.h"
#include "System.h"
#include "SysTitle.h"
#include "SM_ticket_dat.h"
#include "SU_ticket_dat.h"
#include "SU_tmd_dat.h"

using namespace IO;

int Identify::AsSuperUser()
{
	int ret = -1;
	u32 keyId = 0;

	/* Try prebuilt tmd/ticket first, otherwise try building a forged tmd/ticket */
	/* Some IOSes unpatched will work with the prebuilt tmd/ticket, others don't. */
	/* Basically gotta love a buggy IOS */
	ret = ES_Identify(System::Cert, System::Cert.Size, (signed_blob*)SU_tmd_dat, SU_tmd_dat_size, (signed_blob*)SU_ticket_dat, SU_ticket_dat_size, &keyId);
	return ret;
	
	// Currently, no forgery :P
}

int Identify::AsSystemMenu()
{
	int ret = -1;
	signed_blob *stmd = NULL;
	signed_blob *sticket = NULL;
	u32 stmdSize = 0;
	u32 sticketSize = 0;
	u32 keyId = 0;
	u64 titleId = 0x100000002ULL;

	ret = SysTitle::GetTMD(titleId, &stmd);
	if (ret < 0)
	{
		gprintf("\n>> ERROR! Reading TMD Failed: %s\n", NandError::ToString(ret));
		goto final;
	}
	else stmdSize = (u32)ret;

	sticket = (signed_blob*)SM_ticket_dat;
	sticketSize = SIGNED_TIK_SIZE(sticket);

	ret = ES_Identify(System::Cert, System::Cert.Size, stmd, stmdSize, sticket, sticketSize, &keyId);

final:
	delete stmd; stmd = NULL;
	return ret;
}