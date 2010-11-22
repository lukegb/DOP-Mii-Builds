#include <stdio.h>
#include <cstdlib>
#include <gccore.h>
#include <malloc.h>
#include <string.h>

#include "Error.h"
#include "Gecko.h"
#include "Boot2.h"
#include "Tools.h"
#include "Wad.h"
#include "rijndael.h"
#include "sha1.h"
#include "Title.h"
#include "System.h"
#include "Video.h"
#include "Controller.h"
#include <wiiuse/wpad.h>
#include <ogc/pad.h>
#include "FileSystem.h"

#define ALIGN(a,b) ((((a)+(b)-1)/(b))*(b))

// Okay, now for the checksums:
u8 boot2v4tmdsha1[20] = { 0x1d, 0x32, 0x46, 0x11, 0x36, 0x9c, 0x88, 0x46, 0x0f, 0x30, 0x2d, 0xae, 0x52, 0x09, 0xf4, 0xae, 0x17, 0x6a, 0x54, 0xaf };
//u8 boot2v4tiksha1[20] = { 0xc2, 0xeb, 0xfd, 0xbb, 0x0a, 0xb6, 0xa3, 0x1c, 0x71, 0xf9, 0x5f, 0xfa, 0x70, 0x5a, 0x7e, 0x9a, 0xbc, 0x27, 0x0f, 0x4c };
u8 boot2v4tiksha1[20] = { 0x1a, 0x27, 0xd5, 0xa9, 0xda, 0xc5, 0x6b, 0x16, 0xa2, 0x43, 0xc2, 0x57, 0x17, 0x1a, 0x06, 0x23, 0xa9, 0x29, 0xb2, 0x49 };
u8 boot2v4contentsha1[20] = { 0x0a, 0x15, 0xbe, 0xbf, 0x59, 0x4b, 0x8f, 0x1e, 0xe9, 0xae, 0xc1, 0xa2, 0x60, 0x83, 0xac, 0x45, 0x2e, 0x67, 0xc2, 0x95 };


using namespace IO;

int Boot2::Install(u16 version)
{
	switch (version)
	{
		case 4:
			return Install(4, true);
		default: 
			gcprintf("Unknown Boot2 Version (%u)\n", version);
			return -1;
	}

	return 0;
}

int Boot2::Install(u16 version, bool isinternal)
{	
	if (!isinternal) return -1;
	const char* optionstring[3] = {"Load WAD from SD Card", "Load WAD from USB Storage", /*"Download from NUS",*/ "Cancel"};
	u8 selection = 0;
	char lookin[5] = "unst";
	u32 button = 0;
	while (System::State == SystemState::Running && strcmp(lookin, "unst") == 0) 
	{
		Console::ClearLine();
		gcprintf("%s<<%s %s", AnsiYellowBoldFG, AnsiNormal, AnsiSelection);
		Console::PrintCenterGC(27, optionstring[selection]);
		gcprintf("%s %s>>%s", AnsiNormal, AnsiYellowBoldFG, AnsiNormal);
		Console::ResetColors();
		while (Controller::ScanPads(&button))
		{
			if (button == WPAD_BUTTON_HOME) System::Exit();
			if (System::State != SystemState::Running) return -1;

			if (button == WPAD_BUTTON_LEFT)
			{
				if (selection > 0) selection--;
				else selection = 2;
			}

			if (button == WPAD_BUTTON_RIGHT)
			{
				if (selection < 2) selection++;
				else selection = 0;
			}

			if (button == WPAD_BUTTON_A)
			{
				printf("\n");
				if (selection == 0) 
				{			
					if (SD::Mount() < 0) 
					{
						gcprintf("Could not load SD Card\n");
						break;
					}
					strcpy(lookin, "sd:");
					break;
				}

				if (selection == 1) 
				{
					if (USB::Mount() < 0) 
					{
						gcprintf("Could not load USB Drive\n");
						break;
					}
					strcpy(lookin, "usb:");
					break;
				}
				if (selection == 2) return (int)TitleError::Cancelled;
			}
			if (button) break;
		}
	}
	if (System::State != SystemState::Running) return -1;
	
	
	// Try loading from /wad/
	char filename[50] = "";
	sprintf(filename, "%s/wad/Boot2v%u.wad", lookin, version);
	int ret = 0;
	if (File::Exists(filename)) {
	} else {
		// Try again from /
		sprintf(filename, "%s/Boot2v%u.wad", lookin, version);
		if (!File::Exists(filename)) {
			gcprintf("Could not locate Boot2v%u.wad. Exiting update procedure.\n", version);
			return -2;
		}
	}
	
	// LOAD IT!
	u8 *wad = NULL; // This should be assigned within ReadBinary...
	u32 wad_length;
	printf("Loading WAD into memory...");
	File::ReadBinary(filename, &wad, &wad_length);
	printf("done\n");
	// That's it! Now just hope the rest of the code works...
	
	
	signed_blob *certs = NULL;
	signed_blob *xsCert = NULL;
	signed_blob *cpCert = NULL;
	signed_blob *caCert = NULL;

	signed_blob *stmd = NULL;
	signed_blob *tmdCert = NULL;
	u32 tmdCertSize = 0;
	tmd *ptmd = NULL;

	signed_blob *ticket = NULL;
	signed_blob *ticketCert = NULL;
	u32 ticketCertSize = 0;

	u8 *contentEnc = NULL;
	u8 *contentDec = NULL;
	u32 contentSize = 0;
	u8 key[16];
	u8 iv[16];
	sha1 hash;

	tmd_content tmdContent;

	WadHeader *header = (WadHeader*)wad;

	if (header->HeaderSize != sizeof(WadHeader))
	{
		gcprintf("Invalid Boot2 Header Length\n");
		return -1;
	}
	u8 *wadp = wad + ROUND_UP(header->HeaderSize, 64);

	certs = (signed_blob*)Tools::AllocateMemory(header->CertsSize);
	if (!certs)
	{
		gcprintf("\n>> ERROR! Out Of Memory\n");
		ret = -1;
		goto end;
	}
	memcpy(certs, wadp, header->CertsSize);
	wadp += ROUND_UP(header->CertsSize, 64);

	// Skip CRL if included
	wadp += ROUND_UP(header->CrlSize, 64);

	ticket = (signed_blob*)Tools::AllocateMemory(header->TicketSize);
	if (!ticket)
	{
		gcprintf("\n>> ERROR! Out Of Memory\n");
		ret = -1;
		goto end;
	}
	memcpy(ticket, wadp, header->TicketSize);
	wadp += ROUND_UP(header->TicketSize, 64);

	if (!IS_VALID_SIGNATURE(ticket))
	{
		gcprintf("\n>> ERROR! Invalid Ticket\n");
		ret = -1;
		goto end;
	}

	stmd = (signed_blob*)Tools::AllocateMemory(header->TmdSize);
	if (!stmd)
	{
		gcprintf("\n>> ERROR! Out Of Memory\n");
		ret = -1;
		goto end;
	}
	memcpy(stmd, wadp, header->TmdSize);
	wadp += ROUND_UP(header->TmdSize, 64);

	xsCert = FindCert(certs, header->CertsSize, "XS");
	if (!xsCert) 
	{
		gcprintf("\n>> ERROR! No XS Cert\n");
		ret = -1;
		goto end;
	}

	cpCert = FindCert(certs, header->CertsSize, "CP");
	if (!cpCert)
	{
		gcprintf("\n>> ERROR! No CP Cert\n");
		ret = -1;
		goto end;
	}

	caCert = FindCert(certs, header->CertsSize, "CA");
	if (!caCert)
	{
		gcprintf("\n>> ERROR! No CA Cert\n");
		ret = -1;
		goto end;
	}

	ticketCertSize = SIGNED_CERT_SIZE(xsCert) + SIGNED_CERT_SIZE(caCert);
	ticketCert = (signed_blob*)Tools::AllocateMemory(ticketCertSize);
	if (!ticketCert)
	{
		gcprintf("\n>> ERROR! Out Of Memory\n");
		ret = -1; goto end;
	}
	memcpy(ticketCert, xsCert, SIGNED_CERT_SIZE(xsCert));
	memcpy(((u8*)ticketCert) + SIGNED_CERT_SIZE(xsCert), caCert, SIGNED_CERT_SIZE(caCert));

	tmdCertSize = SIGNED_CERT_SIZE(cpCert) + SIGNED_CERT_SIZE(caCert);
	tmdCert = (signed_blob*)Tools::AllocateMemory(tmdCertSize);
	if (!tmdCert)
	{
		gcprintf("\n>> ERROR! Out Of Memory\n");
		ret = -1; goto end;
	}
	memcpy(tmdCert, cpCert, SIGNED_CERT_SIZE(cpCert));
	memcpy(((u8*)tmdCert) + SIGNED_CERT_SIZE(cpCert), caCert, SIGNED_CERT_SIZE(caCert));

	ptmd = (tmd*)SIGNATURE_PAYLOAD(stmd);
	// Now to verify stmd's checksum...
	SHA1((u8*)stmd, header->TmdSize, hash);
	if (memcmp(boot2v4tmdsha1, hash, sizeof(hash) != 0))
	{
		gcprintf("\n>> ERROR! TMD hash does not match!\n");
		ret = -1; goto end;
	}
	
	// now for Ticket...
	SHA1((u8*)ticket, header->TicketSize, hash);
	if (memcmp(boot2v4tiksha1, hash, sizeof(hash) != 0))
	{
	      gcprintf("\n>> ERROR! Ticket hash does not match!\n");
	      ret = -1; goto end;
	}
	

	contentSize = ALIGN(ptmd->contents[0].size, 16);
	contentEnc = (u8*)Tools::AllocateMemory(contentSize);
	contentDec = (u8*)Tools::AllocateMemory(contentSize);
	if (!contentEnc || !contentDec)
	{
		gcprintf("\n>> ERROR! Out Of Memory\n");
		ret = -1; goto end;
	}	
	memcpy(contentEnc, wadp, contentSize);

	/* Decrypt Content and Check Hashes */
	Title::GetTitleKey(ticket, key);
	aes_set_key(key);
	
	memset(iv, 0, 16);
	aes_decrypt(iv, contentEnc, contentDec, contentSize);

	tmdContent = TMD_CONTENTS(ptmd)[0];
	
	SHA1(contentDec, tmdContent.size, hash);
	if (memcmp(tmdContent.hash, hash, sizeof(hash)) != 0)
	{
		gcprintf("\n>> ERROR! Invalid Content Hash\n");
		ret = -1; goto end;
	}
	
	// Double check against stored sha1
	SHA1(contentEnc, ptmd->contents[0].size, hash);
	if (memcmp(boot2v4contentsha1, hash, sizeof(hash)) != 0)
	{
		gcprintf("\n>> ERROR! Invalid content hash (vs. stored)\n");
	      	// DUMP the hash
		u8 i;
		for (i = 0; i < 20; i++) {
			printf("0x%02X, ", (unsigned char) hash[i]);
		}
 		printf("\b\b  \n");
		ret = -1; goto end;
	}
	
	printf("\n\nAll checks have passed. Now installing...\n");
	
	Spinner::Start();

	/* Now the scary part. Install Boot2 */
	ret = ES_ImportBoot(ticket, header->TicketSize, ticketCert, ticketCertSize,
		  stmd, header->TmdSize, tmdCert, tmdCertSize, contentEnc, contentSize);
	
	Spinner::Stop();
	printf("...done!");
	
	gprintf("ES_ImportBoot = %s\n", EsError::ToString(ret));

end:
	memset(hash, 0, sizeof(hash));
	delete contentDec; contentDec = NULL;
	delete contentEnc; contentEnc = NULL;
	delete stmd; stmd = NULL;
	delete tmdCert; tmdCert = NULL;
	delete ticket; ticket = NULL;
	delete ticketCert; ticketCert = NULL;
	delete certs; certs = NULL;
	return ret;
}

signed_blob* Boot2::FindCert(signed_blob *certs, u32 certsSize, const char *match)
{
	signed_blob *cp = certs;
	while ((u32)cp < ((u32)certs + certsSize))
	{
		cert_rsa2048 *rc = (cert_rsa2048*)SIGNATURE_PAYLOAD(cp);
		if (!strncmp(rc->cert_name, match, strlen(match))) return cp;
		cp = (signed_blob*)(((u32)cp) + SIGNED_CERT_SIZE(cp));
	}

	return NULL;
}