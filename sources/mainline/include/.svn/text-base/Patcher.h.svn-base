#ifndef _PATCHER_H_
#define _PATCHER_H_

class Patcher
{
public:
	static int BruteTMD(tmd *ptmd);
	static void ForgeTMD(signed_blob *signedTmd);

	static int BruteTicket(tik *ticket);
	static void ForgeTicket(signed_blob *signedTicket);

	static int PatchFakeSign(u8 *buf, u32 size);
	static int PatchEsIdentity(u8 *buf, u32 size);
	static int PatchNandPermissions(u8 *buf, u32 size);
	static void ZeroSignature(signed_blob *sig);
};

#endif