#include <gccore.h>
#include "Identify.h"
#include "Nand.h"
#include "Tools.h"
#include "Error.h"
#include "Gecko.h"
#include "System.h"
#include "SysTitle.h"
#include "Patcher.h"
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
	if (ret > -1) return ret;

	keyId = 0;

	signed_blob *stmd = NULL;
	signed_blob *sticket = NULL;
	tmd *ptmd = NULL;
	tik *pticket = NULL;
	u64 titleId = 0x100000002ULL;	

	stmd = (signed_blob*)Tools::AllocateMemory(520);
	sticket = (signed_blob*)Tools::AllocateMemory(STD_SIGNED_TIK_SIZE);

	memset(stmd, 0, 520);
	memset(sticket, 0, STD_SIGNED_TIK_SIZE);

	stmd = &stmd[0];
	sticket = &sticket[0];
	*stmd = *sticket = 0x10001;
	ptmd = (tmd*)SIGNATURE_PAYLOAD(stmd);
	pticket = (tik*)SIGNATURE_PAYLOAD(sticket);

	strcpy(ptmd->issuer, "Root-CA00000001-CP00000004");
	ptmd->title_id = titleId;
	ptmd->num_contents = 1;
	Patcher::ForgeTMD(stmd);

	strcpy(pticket->issuer, "Root-CA00000001-XS00000003");
	pticket->ticketid =  0x000038A45236EE5FULL;
	pticket->titleid = titleId;

	memset(pticket->cidx_mask, 0xFF, 0x20);
	Patcher::ForgeTicket(sticket);

	ret = ES_Identify(System::Cert, System::Cert.Size, stmd, 520, sticket, STD_SIGNED_TIK_SIZE, &keyId);

	delete stmd; stmd = NULL;
	delete sticket; sticket = NULL;
	return ret;
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