#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <gccore.h>

#include "Gecko.h"
#include "Patcher.h"
#include "sha1.h"

int Patcher::BruteTMD(tmd *ptmd)
{
	u16 fill;
	for (fill=0; fill<65535; fill++) 
	{
		ptmd->fill3=fill;
		sha1 hash;
		SHA1((u8 *)ptmd, TMD_SIZE(ptmd), hash);
		if (hash[0]==0) return 0;
	}
	return -1;
}

void Patcher::ForgeTMD(signed_blob *signedTmd)
{
	ZeroSignature(signedTmd);
	BruteTMD((tmd*)SIGNATURE_PAYLOAD(signedTmd));
}

int Patcher::BruteTicket(tik *ticket) 
{
	u16 fill;
	for (fill=0; fill<65535; fill++) 
	{
		ticket->padding=fill;
		sha1 hash;
		SHA1((u8*)ticket, sizeof(ticket), hash);
		if (hash[0]==0) return 0;
	}
	return -1;
}

void Patcher::ForgeTicket(signed_blob *signedTicket)
{
	ZeroSignature(signedTicket);
	BruteTicket((tik*)SIGNATURE_PAYLOAD(signedTicket));
}

int Patcher::PatchFakeSign(u8 *buf, u32 size)
{
	u32 matchCount = 0;
	u8 hash1[] = {0x20,0x07,0x23,0xA2}; // oldHashCheck
	u8 hash2[] = {0x20,0x07,0x4B,0x0B};	// newHashCheck

	for (u32 i= 0; i<size-4; i++) 
	{
		if (!memcmp(buf + i, hash1, sizeof(hash1))) 
		{
			gcprintf("\n\t- Found Hash check @ 0x%X, patching... ", i);
			buf[i+1] = 0;
			i += 4;
			matchCount++;
			continue;
		}

		if (!memcmp(buf + i, hash2, sizeof(hash2))) 
		{
			gcprintf("\n\t- Found Hash check @ 0x%X, patching... ", i);
			buf[i+1] = 0;
			i += 4;
			matchCount++;
			continue;
		}
	}
	return matchCount;
}

int Patcher::PatchEsIdentity(u8 *buf, u32 size)
{
	u32 matchCount = 0;
	u8 identifyCheck[] = { 0x28, 0x03, 0xD1, 0x23 };
	
	for (u32 i = 0; i < size - 4; i++) 
	{
		if (!memcmp(buf + i, identifyCheck, sizeof(identifyCheck))) 
		{
			gcprintf("\n\t- Found ES_Identify check @ 0x%X, patching... ", i);
			buf[i+2] = 0;
			buf[i+3] = 0;
			i += 4;
			matchCount++;
			continue;
		}
	}
	return matchCount;

}

int Patcher::PatchNandPermissions(u8 *buf, u32 size)
{
	u32 i;
	u32 matchCount = 0;
	u8 oldTable[] = {0x42, 0x8B, 0xD0, 0x01, 0x25, 0x66};
	u8 newTable[] = {0x42, 0x8B, 0xE0, 0x01, 0x25, 0x66};

	for (i=0; i< size - sizeof(oldTable); i++) 
	{
		if (!memcmp(buf + i, oldTable, sizeof(oldTable))) 
		{
			gcprintf("\n\t- Found NAND Permission check @ 0x%X, patching... ", i);
			memcpy(buf + i, newTable, sizeof(newTable));
			i += sizeof newTable;
			matchCount++;
			continue;
		}
	}
	return matchCount;
}

void Patcher::ZeroSignature(signed_blob *sig)
{
	u8 *psig = (u8*)sig;
	memset(psig + 4, 0, SIGNATURE_SIZE(sig)-4);
}