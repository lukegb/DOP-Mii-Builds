#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <gccore.h>
#include <malloc.h>
#include <wiiuse/wpad.h>
#include <mxml.h>
#include <sstream>
#include <iomanip>

#include "Title.h"
#include "Controller.h"
#include "Error.h"
#include "FileSystem.h"
#include "Gecko.h"
#include "Global.h"
#include "rijndael.h"
#include "sha1.h"
#include "System.h"
#include "Nand.h"
#include "Nus.h"
#include "Video.h"
#include "Tools.h"
#include "Patcher.h"
#include "Titles_xml.h"
#include "Title.h"
#include "Wad.h"
#include "SysMenuMatrix.h"
#include "SysTitle.h"
#include "SysCheck.h"

#define HAVE_AHBPROT ((*(vu32*)0xcd800064 == 0xFFFFFFFF) ? 1 : 0)

using namespace std;
using namespace IO;
using namespace Titles;

const u8 AesCommonKey[16] = { 0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4, 0x48, 0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7 };

Title::Title(u32 titleId1, u32 titleId2)
{
	TitleId = TITLEID(titleId1, titleId2);
	Initialize();
}

Title::Title(u64 titleId)
{
	TitleId = titleId;
	Initialize();
}

void Title::Initialize()
{
	Type = TitleType::UNKNOWN;
	ContentCount = 0;
	TicketSize = 0;
	TmdSize = 0;
	CrlSize = 0;
	Ticket = NULL;
	Tmd = NULL;
	Crl = NULL;
	EncryptedBuffer = NULL;
	DecryptedBuffer = NULL;
	BufferSize = NULL;
	StoredTMD = NULL;

	char titlename[256];
	sprintf(titlename, "%08X-%08X", TITLEID1(TitleId), TITLEID2(TitleId));
	Name = titlename;
	memset(titlename, 0, 256);

	DetermineTitleType();
}

Title::~Title()
{
	this->Clear();
	this->Name.clear();	
	delete this->StoredTMD;
}

void Title::Clear()
{	
	delete Ticket; Ticket = NULL;
	delete Tmd; Tmd = NULL;
	delete Crl; Crl = NULL;

	for (u32 i = 0; i < ContentCount; i++)
	{
		if (EncryptedBuffer && EncryptedBuffer[i]) delete EncryptedBuffer[i];
		if (DecryptedBuffer && DecryptedBuffer[i]) delete DecryptedBuffer[i];
	}

	delete EncryptedBuffer; EncryptedBuffer = NULL;
	delete DecryptedBuffer; DecryptedBuffer = NULL;
	delete BufferSize; BufferSize = NULL;

	ContentCount = 0;
	TicketSize = 0;
	TmdSize = 0;
	CrlSize = 0;	
}

void Title::DetermineTitleType()
{
	char titleIdStr[16];
	mxml_node_t* xTree = NULL;
	mxml_node_t* xTop = NULL;
	mxml_node_t* xTitle = NULL;
	u32 titleId1 = TITLEID1(TitleId);
	u32 titleId2 = TITLEID2(TitleId);

	sprintf(titleIdStr, "%08X%08X", titleId1, titleId2);

	Type = TitleType::UNKNOWN;

	xTree = mxmlLoadString(NULL, (char*)Titles_xml, MXML_NO_CALLBACK);
	if (!xTree) goto end;

	xTop = mxmlFindElement(xTree, xTree, "titles", NULL, NULL, MXML_DESCEND_FIRST);
	if (!xTop) goto end;

	for  
	( 
		xTitle = mxmlFindElement(xTop, xTop, "title", "id", titleIdStr, MXML_DESCEND_FIRST); xTitle != NULL;
		xTitle = mxmlFindElement(xTitle, xTop, "title", "id", titleIdStr, MXML_NO_DESCEND)
	)	
	{
		const char* type = mxmlElementGetAttr(xTitle, "type");

		if (!strcmpi(type, "IOS")) Type = TitleType::IOS;
		else if (!strcmpi(type, "SYSMENU")) Type = TitleType::SYSMENU;
		else if (!strcmpi(type, "CHANNEL")) Type = TitleType::CHANNEL;
		else if (!strcmpi(type, "BOOT2")) Type = TitleType::BOOT2;

		const char* name = mxmlElementGetAttr(xTitle, "name");
		if (name) this->Name = name;
	}

	// if still unknown lets try and figure it out by titleid
	if (Type == TitleType::UNKNOWN)
	{
		if ((TitleId >= 0x100000004ull) & (TitleId <= 0x1000000FFull))
		{
			this->Type = TitleType::IOS;
			sprintf(titleIdStr, "IOS%u\n", titleId2);
			this->Name = titleIdStr;
		}
	}

end:
	memset(titleIdStr, 0, 16);
	mxmlDelete(xTitle);
	mxmlDelete(xTop);
	mxmlDelete(xTree);
}

void Title::GetTitleKey(signed_blob *signedTicket, u8 *key) 
{
	static u8 iv[16] ATTRIBUTE_ALIGN(32);
	static u8 keyin[16] ATTRIBUTE_ALIGN(32);
	static u8 keyout[16] ATTRIBUTE_ALIGN(32);

	tik *ticket = (tik*)SIGNATURE_PAYLOAD(signedTicket);
	u8 *encKey = (u8*)&ticket->cipher_title_key;
	memcpy(keyin, encKey, sizeof(keyin));
	memcpy(iv, &ticket->titleid, sizeof(ticket->titleid));
	
	aes_set_key((u8*)AesCommonKey);
	aes_decrypt(iv, keyin, keyout, sizeof(keyin));
	memcpy(key, keyout, sizeof(keyout));

	memset(iv, 0, sizeof(iv));
	memset(keyin, 0, sizeof(keyin));
	memset(keyout, 0, sizeof(keyout));
}

void Title::Encrypt()
{
	u8 key[16];
	GetTitleKey(Ticket, key);
	aes_set_key(key);

	for (u32 i = 0; i < ContentCount; i++) 
	{
		EncryptBuffer(i, DecryptedBuffer[i], EncryptedBuffer[i], BufferSize[i]);
	}
}

void Title::EncryptBuffer(u16 index, u8 *source, u8 *dest, u32 len) 
{
	u8 iv[16];
	memset(iv, 0, 16);
	memcpy(iv, &index, 2);
	aes_encrypt(iv, source, dest, len);
}

void Title::Decrypt() 
{
	//u8 *key = (u8*)Tools::AllocateMemory(sizeof(u8) * 16);
	u8 key[16];
	GetTitleKey(Ticket, key);
	aes_set_key(key);

	for (u32 i = 0; i < ContentCount; i++) 
	{
		DecryptBuffer(i, EncryptedBuffer[i], DecryptedBuffer[i], BufferSize[i]);
		printf("\b..");
	}

	//delete key; key = NULL;
}

void Title::DecryptBuffer(u16 index, u8 *source, u8 *dest, u32 len) 
{
	u8 iv[16];
	memset(iv, 0, 16);
	memcpy(iv, &index, 2);
	aes_decrypt(iv, source, dest, len);
}

int Title::SetContentCount(u32 count) 
{
	if (ContentCount > 0) 
	{		
		for (u32 i = 0; i < ContentCount; i++) 
		{
			if (EncryptedBuffer && EncryptedBuffer[i]) delete EncryptedBuffer[i];
			if (DecryptedBuffer && DecryptedBuffer[i]) delete DecryptedBuffer[i];
		}

		delete EncryptedBuffer; EncryptedBuffer = NULL;
		delete DecryptedBuffer; DecryptedBuffer = NULL;
		delete BufferSize; BufferSize = NULL;
	}

	ContentCount = count;
	if (count > 0) 
	{
		EncryptedBuffer = (u8**)Tools::AllocateMemory(4*count);
		DecryptedBuffer = (u8**)Tools::AllocateMemory(4*count);
		BufferSize = (u32*)Tools::AllocateMemory(4*count);

		for (u32 i = 0; i < count; i++) 
		{
			if (EncryptedBuffer) EncryptedBuffer[i] = NULL;
			if (DecryptedBuffer) DecryptedBuffer[i] = NULL;
		}

		if (!EncryptedBuffer || !DecryptedBuffer || !BufferSize) return -1;
	}
	return 0;
}

int Title::DowngradeTmdRevision() 
{
	// The revison of the tmd used as paramter here has to be >= the revision of the installed tmd
	int ret = 0;
	signed_blob *stmd = NULL;
	s32 file = 0;
	u32 stmdSize = 0;
	u8 *ttmd = NULL;
	char* tmdPath = (char*)"/tmp/title.tmd";

	bool editDirectly = (HAVE_AHBPROT);
	if (editDirectly) {
		sprintf(tmdPath, "/title/%08x/%08x/content/title.tmd", TITLEID1(this->TitleId), TITLEID2(this->TitleId));
		gcprintf("Detected NAND permissions, and AHBPROT is enabled... Editing TMD directly!\n");
	}

	gcprintf("Loading Stored TMD...");

	if (this->StoredTMD)
	{
		stmd = this->StoredTMD;
		stmdSize = SIGNED_TMD_SIZE(this->StoredTMD);
	}
	else 
	{
		ret = SysTitle::GetTMD(this->TitleId, &stmd);
		if (ret < 0)
		{
			gcprintf("\n>> ERROR: Failed To Get Stored TMD: %s\n", EsError::ToString(ret));
			goto end;
		}

		stmdSize = (u32)ret;
	}

	if (!IS_VALID_SIGNATURE(stmd))
	{
		gcprintf("\n>> ERROR! Invalid Stored TMD\n");
		ret = -1;
		goto end;
	}

	gcprintf("\nSetting existing TMD revision to 0...");
	if (!editDirectly)
		ret = ES_AddTitleStart(stmd, stmdSize, System::Cert, System::Cert.Size, NULL, 0);
	if (ret < 0) 
	{
		gcprintf("\n>> ERROR! ES_AddTitleStart: %s\n", EsError::ToString(ret));
		ES_AddTitleCancel();
		goto end;
	}
	gcprintf("Done\n");

	ret = Nand::Startup();
	if (ret < 0) 
	{
		gcprintf("\n>> ERROR! Nand::Startup: %s\n", NandError::ToString(ret));
		if (!editDirectly)
			ES_AddTitleCancel();
		goto end;
	}
	
    ret = Nand::Delete(tmdPath);
    if (ret < 0)
    {
        gcprintf(">> Nand::Delete = %s\n", NandError::ToString(ret));
	if (!editDirectly)
	        ES_AddTitleCancel();
        goto end;
    }

    ret = Nand::CreateFile(tmdPath, 0, 3, 3, 3);
    if (ret < 0)
    {
        gcprintf(">> Nand::CreateFile = %s\n", NandError::ToString(ret));
	if (!editDirectly)
	        ES_AddTitleCancel();
        goto end;
    }
	
	file = Nand::OpenReadWrite(tmdPath);
	if (file < 0) 
	{
		gcprintf("\n>> ERROR! Nand::OpenReadWrite: %s\n", NandError::ToString(file));
		if (!editDirectly)
			ES_AddTitleCancel();
		goto end;
	}
	
	ttmd = (u8*)stmd;
	ttmd[0x1dc] = 0;
	ttmd[0x1dd] = 0;

	ret = Nand::Write(file, (u8*)stmd, stmdSize);
	if (ret < 0) 
	{
		gcprintf("\n>> ERROR! Nand::Write: %s\n", NandError::ToString(ret));
		Nand::Close(file);
		if (!editDirectly)
			ES_AddTitleCancel();
		goto end;
	}	
	Nand::Close(file);
	if (!editDirectly)
		gprintf("\n>> ES_AddTitleFinish = %s\n", EsError::ToString(ES_AddTitleFinish()));	
end:
	delete stmd; stmd = NULL;
	return ret;
}

int Title::PerformInstall()
{
	int ret = 0;
	tmd *ptmd = NULL;	

	#ifdef DEBUG
	gprintf("Title Type = %d\n", (int)this->Type);
	gprintf("Title ID = %08X-%08X", TITLEID1(this->TitleId), TITLEID2(this->TitleId));
	#endif

	if (!Ticket || !Tmd || !EncryptedBuffer || !DecryptedBuffer || !BufferSize) 
	{
		gcprintf("\n>> ERROR: Title not initialized properly.\n");
		return -1;
	}

	if (this->Type != TitleType::CHANNEL)
	{
		u16 newVersion = ((tmd*)SIGNATURE_PAYLOAD(this->Tmd))->title_version;
		u16 oldVersion = SysTitle::GetVersion(this->TitleId);
		gprintf("Old Version = %u, New Version = %u\n", oldVersion, newVersion);
		if (oldVersion != 0 && oldVersion > newVersion)
		{
			ret = DowngradeTmdRevision();
			if (ret < 0) return ret;
		}
	}

	gcprintf("Installing...");
	Spinner::Start();

	ret = ES_AddTicket(this->Ticket, this->TicketSize, System::Cert, System::Cert.Size, Crl, CrlSize);
	if (ret < 0) 
	{
		gcprintf("\n>> ERROR! ES_AddTicket: %s\n", EsError::ToString(ret));
		ES_AddTitleCancel();
		goto end;
	}

	ret = ES_AddTicket(this->Ticket, this->TicketSize, System::Cert, System::Cert.Size, Crl, CrlSize);
	if (ret < 0) 
	{
		gcprintf("\n>> ERROR! ES_AddTicket: %s\n", EsError::ToString(ret));
		ES_AddTitleCancel();
		goto end;
	}

	gprintf("\n>> ES_AddTitleStart..");	
	ret = ES_AddTitleStart(this->Tmd, this->TmdSize, System::Cert, System::Cert.Size, Crl, CrlSize);
	if (ret < 0) 
	{
		gcprintf("\n>> ERROR! ES_AddTitleStart: %s\n", EsError::ToString(ret));
		ES_AddTitleCancel();
		goto end;
	}
	gcprintf("\b..");

	ptmd = (tmd*)SIGNATURE_PAYLOAD(this->Tmd);

	for (u32 i = 0; i < ContentCount; i++) 
	{
		tmd_content *content = &ptmd->contents[i];

		//gprintf("\n>> ES_AddContentStart");
		int cid = ES_AddContentStart(ptmd->title_id, content->cid);
		if (cid < 0) 
		{
			gcprintf("\n>> ERROR! ES_AddContentStart for content #%u cid %08X returned: %s\n", i, content->cid, EsError::ToString(cid));
			ES_AddTitleCancel();
			ret = cid;
			goto end;
		}
		
		//gprintf("\n>> ES_AddContentData");
		ret = ES_AddContentData(cid, EncryptedBuffer[i], BufferSize[i]);
		if (ret < 0) 
		{
			gcprintf("\n>> ERROR! ES_AddContentData for content #%u cid %08X returned: %s\n", i, content->cid, EsError::ToString(ret));
			ES_AddTitleCancel();
			goto end;
		}

		//gprintf("\n>> ES_AddContentFinish\n");
		ret = ES_AddContentFinish(cid);
		if (ret < 0) 
		{
			gcprintf("\n>> ERROR! ES_AddContentFinish for content #%u cid %08X returned: %s\n", i, content->cid, EsError::ToString(ret));
			ES_AddTitleCancel();
			goto end;
		}
		gcprintf("\b..");
	}

	ret = ES_AddTitleFinish();
	if (ret < 0) 
	{
		gcprintf("\n>> ERROR! ES_AddTitleFinish: %s\n", EsError::ToString(ret));
		ES_AddTitleCancel();
		goto end;
	}
	gcprintf("\b..");
	Spinner::Stop();
	gcprintf("\b.Done\n");

end:
	Spinner::Stop();
	return ret;
}

int Title::Get(u16 revision)
{
	char wadFileName[256];

	if (Type == TitleType::IOS) sprintf(wadFileName, "%s-64-v%u.wad", Name.c_str(), revision);
	if (Type == TitleType::SYSMENU) sprintf(wadFileName, "System Menu-NUS-v%u.wad", revision);

	int ret = Get(revision, wadFileName);	
	memset(wadFileName, 0, 256);
	return ret;
}

int Title::Get(u16 revision, int getMethod)
{
	char wadFileName[256];

	if (Type == TitleType::IOS) sprintf(wadFileName, "%s-64-v%u.wad", Name.c_str(), revision);
	if (Type == TitleType::SYSMENU) sprintf(wadFileName, "System Menu-NUS-v%u.wad", revision);

	int ret = Get(revision, wadFileName, getMethod);	
	memset(wadFileName, 0, 256);
	return ret;
}

int Title::Get(u16 revision, const char* wadFileName, int getMethod)
{
	char filename[512];
	int ret;
	if (getMethod == GETWADSD) 
	{			
		if (SD::Mount() < 0) 
		{
			gcprintf("Could not load SD Card\n");
			sleep(2);
			return -1;
		}
		sprintf(filename, "sd:/wad/%s", wadFileName);
		ret = LoadFromWad(revision, filename);
		if (ret < 0) 
		{
			sprintf(filename, "sd:/%s", wadFileName);
			ret = LoadFromWad(revision, filename);
		}
		return ret;
	}
	if (getMethod == GETWADUSB) 
	{
		if (USB::Mount() < 0) 
		{
			gcprintf("Could not load USB Drive\n");
			sleep(2);
			return -1;
		}
		sprintf(filename, "usb:/wad/%s", wadFileName);
		ret = LoadFromWad(revision, filename);
		if (ret < 0) 
		{
			sprintf(filename, "usb:/%s", wadFileName);
			ret = LoadFromWad(revision, filename);
		}
		return ret;
	}
	if (getMethod == GETNUS) return Download(revision);
	return -1;
}

int Title::Get(u16 revision, const char* wadFileName) 
{
	u32 button;

	int selection = 2;
	const char* optionstring[4] = {"Load WAD from SD Card", "Load WAD from USB Storage", "Download from NUS", "Cancel"};

	this->Clear();

	while (System::State == SystemState::Running) 
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
				else selection = 3;
			}

			if (button == WPAD_BUTTON_RIGHT)
			{
				if (selection < 3) selection++;
				else selection = 0;
			}

			if (button == WPAD_BUTTON_A)
			{
				printf("\n");
				if (selection > 0 && selection < 3) return Get(revision, wadFileName, selection);
				if (selection == 3) return (int)TitleError::Cancelled;
			}
			if (button) break;
		}
	}
	return 0;
}

int Title::Download(u16 revision) 
{
	s32 ret;
	u8 *ticketBuffer = NULL;
	u8 *tmdBuffer = NULL;
	tmd *ptmd = NULL;
	u16 version = revision;   
	char filename[32]; 

	printf("\n");
	gprintf("\n[**** TMD - START ****]\n");
	if (revision > 0) sprintf(filename, "tmd.%u", version);
	else sprintf(filename, "tmd");   
	ret = NUS::GetFile(TitleId, &version, filename, &tmdBuffer, &this->TmdSize);
	if (ret < 0) 
	{
		gcprintf("Loading tmd failed, ret = %d\n", ret);
		return -1;
	}

	if (tmdBuffer == NULL || this->TmdSize == 0) 
	{
		gcprintf("TMD error\n");
		return -1;
	}

	this->TmdSize = SIGNED_TMD_SIZE((signed_blob *)tmdBuffer);
	this->Tmd = (signed_blob*)Tools::AllocateMemory(this->TmdSize);
	if (this->Tmd == NULL) 
	{
		gcprintf("ERROR! Title::Download: Out of memory for TMD\n");
		return -1;
	}
	memcpy(this->Tmd, tmdBuffer, this->TmdSize);	
	delete tmdBuffer; tmdBuffer = NULL;

	if (!IS_VALID_SIGNATURE(this->Tmd)) 
	{
		gcprintf("ERROR! Bad TMD signature!\n");
		return -1;
	}
	gprintf("[**** TMD - END   ****]\n");
    
	gprintf("\n[**** Ticket - Start ****]\n");
	ret = NUS::GetFile(TitleId, &version, (char*)"cetk", &ticketBuffer, &this->TicketSize);
	if (ret < 0) 
	{
		gcprintf("\n>> ERROR! Loading ticket failed, ret = %u\n", ret);
		return ret;
	}
	if (ticketBuffer == NULL || this->TicketSize == 0) 
	{
		gcprintf("\n>> ERROR! Ticket Not Found.\n");
		return -1;
	}

	this->TicketSize = SIGNED_TIK_SIZE((signed_blob *)ticketBuffer);
	this->Ticket = (signed_blob*)Tools::AllocateMemory(this->TicketSize);
	if (!this->Ticket) 
	{
		gcprintf("\n>> ERROR! Title::Download: Out of memory for Ticket\n");
		return -1;
	}
	memcpy(this->Ticket, ticketBuffer, this->TicketSize);
	delete ticketBuffer; ticketBuffer = NULL;

	if (!IS_VALID_SIGNATURE(Ticket)) 
	{
		gcprintf("\n>> ERROR: Bad ticket signature!\n");
		return -1;
	}

	/* Get TMD info */
	ptmd = (tmd*)SIGNATURE_PAYLOAD(this->Tmd);
	gprintf("[**** Ticket - End   ****]\n");

	gcprintf("Checking Title ID and Version...");
	if (TITLEID1(ptmd->title_id) != TITLEID1(TitleId) || TITLEID2(ptmd->title_id) != TITLEID2(TitleId)) 
	{
		gcprintf("Title has titleid: %016llX but expected was: %016llX\n", ptmd->title_id, TitleId);
		return -1;
	}

	if (ptmd->title_version != version) 
	{
		gcprintf("Title has revision: %u but expected was: %u\n", ptmd->title_version, version);
		return -1;
	}
	gcprintf("Done\n");

	ret = SetContentCount(ptmd->num_contents);
	if (ret < 0) 
	{
		gcprintf("ERROR! Title::Download: Out of memory for SetContentCount\n");
		return -1;
	}

	gcprintf("Loading Contents...\n");
	for (u32 cnt = 0; cnt < ptmd->num_contents; cnt++) 
	{
		tmd_content *content = &ptmd->contents[cnt];

		/* Encrypted content size */
		BufferSize[cnt] = ROUND_UP((u32)content->size, 64);

		sprintf(filename, "%08X", content->cid);
		
		ret = NUS::GetFile(TitleId, &version, filename, &EncryptedBuffer[cnt], &BufferSize[cnt]);

		if (BufferSize[cnt] % 16) 
		{
			gcprintf("Content %u size is not a multiple of 16\n", cnt);
			return -1;
		}

		gprintf("Content Size = %llu\n", content->size);
		if (BufferSize[cnt] < (u32)content->size) 
		{
			gcprintf("Content %u size is too small\n", cnt);
			return -1;
		}

		DecryptedBuffer[cnt] = (u8*)Tools::AllocateMemory(BufferSize[cnt]);
		if (!DecryptedBuffer[cnt]) 
		{
			gcprintf("ERROR! Title::Download: Out of memory for Decrypted Buffer\n");
			return -1;
		}	
	}	

	gcprintf("Load Contents Completed...\n");
	gcprintf("Decrypting Title...");
	Spinner::Start();
	Decrypt();
	Spinner::Stop();
	gcprintf("\b.Done\n");

	gcprintf("Checking hashes...");
	ret = CheckContentHashes();
	if (ret < 0) 
	{
		gcprintf("\n>>ERROR! Content Hashes Do Not Match!\n");
		return ret;
	}
	gcprintf("Done\n");

	return 1;
}

int Title::LoadFromWad(u16 revision, const char *filename)
{
	int ret = 0;
	FILE *file = NULL;
	WadHeader *header = NULL;
	u32 offset = 0;
	tmd_content *content = NULL;
	tmd *tmdData = NULL;

	file = File::OpenBinary(filename);
	if (!file)
	{
		gcprintf("\n>> ERROR! Could not open file: %s\n", filename);
		return -1;
	}

	// WAD Header
	gprintf("Loading WAD Header...\n");
	ret = File::ReadOffsetAlloc(file, offset, sizeof(WadHeader), (u8**)&header);
	if (ret < 0 || !header)
	{
		gcprintf("\n>> ERROR! Could not read WAD Header, ErrorCode(%d)", ret);
		goto error;
	}
	else offset += ROUND_UP(header->HeaderSize, 64);

	if (header->CertsSize == 0 || header->TicketSize == 0 || header->TmdSize == 0)
	{
		gcprintf("\n>> ERROR! Certs, Ticket and/or TMD has size 0\n");
		gcprintf(">> Sizes -> Certs (%u), Ticket (%u), TMD (%u)\n", header->CertsSize, header->TicketSize, header->TmdSize);
		goto error;
	}

    /* WAD certificates */
	/*
    (*title)->CertsSize = header->CertsSize;
	ret = Wad::ReadAlloc(fp, (void **)&(*title)->Certs, offset, header->CertsSize);
    if (ret < 0) 
	{
        gcprintf("Error reading the certs, ret = %d\n", ret);
        goto err;
    } 
	else offset += ROUND_UP(header->CertsSize, 64);

    if (!IS_VALID_SIGNATURE((signed_blob *)(*title)->Certs)) 
	{
        gcprintf("Error: Bad certs signature!\n");
        ret = -1;
        goto err;
    }
	*/
	// Skip Certs since we are using the system's cert
	offset += ROUND_UP(header->CertsSize, 64);

	// WAD CRL
	this->CrlSize = header->CrlSize;
	if (header->CrlSize)
	{
		gprintf("Loading CRL...\n");
		ret = File::ReadOffsetAlloc(file, offset, header->CrlSize, (u8**)&this->Crl);
		if (ret < 0)
		{
			gcprintf("\n>> ERROR! Could not read the CRL, ErrorCode (%d)\n", ret);
			goto error;
		}
		else offset += ROUND_UP(header->CrlSize, 64);
	}
	else this->Crl = NULL;

	// WAD Ticket
	gprintf("Loading Ticket...\n");
	this->TicketSize = header->TicketSize;
	ret = File::ReadOffsetAlloc(file, offset, header->TicketSize, (u8**)&this->Ticket);
	if (ret < 0)
	{
		gcprintf("\n>> ERROR! Could not read the Ticket. ErrorCode (%d)\n", ret);
		goto error;
	}
	else offset += ROUND_UP(header->TicketSize, 64);

	if (!IS_VALID_SIGNATURE(this->Ticket))
	{
		gcprintf("\n>> ERROR! Bad Ticket Signature\n");
		ret = -1;
		goto error;
	}

	// WAD TMD
	gprintf("Loading TMD...\n");
	this->TmdSize = header->TmdSize;
	ret = File::ReadOffsetAlloc(file, offset, header->TmdSize, (u8**)&this->Tmd);
	if (ret < 0)
	{
		gcprintf("\n>> ERROR! Could not read the TMD. ErrorCode (%d)\n", ret);
		goto error;
	}
	else offset += ROUND_UP(header->TmdSize, 64);

	if (!IS_VALID_SIGNATURE(this->Tmd))
	{
		gcprintf("\n>> ERROR! Bad TMD Signature\n");
		ret = -1;
		goto error;
	}

	// Get TMD INfO
	tmdData = (tmd*)SIGNATURE_PAYLOAD(this->Tmd);
	if (!tmdData)
	{
		gcprintf("\n>> ERROR! TMD Data is null\n");
		ret = -1;
		goto error;
	}

	gcprintf("Checking TitleID and Revision...\n");
	if (tmdData->title_id != TitleId)
	{
		gcprintf("WAD has TitleID: %016llX but expected wad: %016llX\n", tmdData->title_id, TitleId);
		ret = -1;
		goto error;
	}
	
	if (revision != 0 && tmdData->title_version != revision)
	{
		gcprintf("WAD has Revision: %u but expected was %u\n", tmdData->title_version, revision);
		ret = -1;
		goto error;
	}

	ret = SetContentCount(tmdData->num_contents);
	if (ret < 0)
	{
		gcprintf("\n>> ERROR! Out Of Memory\n");
		goto error;
	}

	gcprintf("Loading Contents..");
	for (u32 i = 0; i < tmdData->num_contents; i++)
	{
		content = &tmdData->contents[i];

		// Encrypted Content Size 
		this->BufferSize[i] = ROUND_UP((u32)content->size, 64);
		this->EncryptedBuffer[i] = (u8*)Tools::AllocateMemory(this->BufferSize[i]);
		this->DecryptedBuffer[i] = (u8*)Tools::AllocateMemory(this->BufferSize[i]);

		if (!this->EncryptedBuffer[i] || !this->DecryptedBuffer[i])
		{
			gcprintf("\n>> ERROR! Out Of Memory\n");
			ret = -1;
			goto error;
		}

		ret = File::ReadOffset(file, offset, this->BufferSize[i], &this->EncryptedBuffer[i]);
		if (ret < 0)
		{
			gcprintf("\n>> ERROR! Error Reading Content #%u. ErrorCode (%d)\n", i, ret);
			goto error;
		}
		
		offset += this->BufferSize[i];
		printf("\b..");
	}
	printf("Done\n");

	gcprintf("Decrypting Title...");
	Spinner::Start();
	Decrypt();
	Spinner::Stop();
	gcprintf("\b.Done\n");

	gcprintf("Checking Hashes...");
	ret = CheckContentHashes();
	if (ret < 0) 
	{
		gcprintf("\n>> ERROR! Content Hashes Do Not Match\n");
		goto error;
	}
	gcprintf("Done\n");

	goto end;

error:
	this->Clear();

end:
	if (file) fclose(file);
	delete header; header = NULL;
	return ret;
}


int Title::ModuleIndex(char *module)
{
	for (u32 i = 0; i < this->ContentCount; i++)
	{
		if (!DecryptedBuffer[i] || !BufferSize[i]) return -1;
		if (ContainsModule(DecryptedBuffer[i], BufferSize[i], module)) return i;
	}

	return -1;
}

int Title::CheckContentHashes()
{
	tmd_content *contents = TMD_CONTENTS((tmd*)SIGNATURE_PAYLOAD(this->Tmd));
	if (!contents) return -1;

	sha1 hash;
	for (u32 i = 0; i < this->ContentCount; i++)
	{
		SHA1(this->DecryptedBuffer[i], contents[i].size, hash);
		if (memcmp(contents[i].hash, hash, sizeof(hash)) != 0) return -1;
	}

	return 0;
}

bool Title::ContainsModule(u8 *buffer, u32 bufferSize, char *module)
{
	const char* iosVersionTag = "$IOSVersion:";

	for (u32 i = 0; i < (bufferSize - 64); i++)
	{
		if (!strncmp((char*)buffer+i, iosVersionTag, 10))
		{
			char version[128];
			while (buffer[i+strlen(iosVersionTag)] == ' ') i++; // skip spaces
			strlcpy(version, (char*)buffer + i + strlen(iosVersionTag), sizeof(version));
			i += 64;
			if (strncmp(version, module, strlen(module)) == 0) return true;
		}
	}
	
	return false;
}

int Title::Uninstall(const u64 titleId)
{
	return Uninstall(TITLEID1(titleId), TITLEID2(titleId));
}

int Title::Uninstall(const u32 titleId1, const u32 titleId2)
{
	int ret = 0;
	int contentsRet;
	int ticketRet;
	int titleRet;

	if (titleId1 == 1 && titleId2 == 1)
	{
		gcprintf("\n>> ERROR! Boot2 cannot be uninstalled!\n");
		return -1;
	}

/*	if (titleId1 == 1 && titleId2 == 2)
	{
		gcprintf("\n>> ERROR! System Menu cannot be uninstalled!\n");
		return -1;
	}*/
	Title *title = new Title(titleId1, titleId2);

	if (titleId1 == 1 && title->Type == TitleType::IOS)
	{
		// Make sure IOS is not used by system menu
		u64 sysMenuIOS = 0;
		ret = SysTitle::GetSysVersion(0x100000002ULL, &sysMenuIOS);
		if (ret < 0)
		{
			gcprintf("\n>> ERROR! Failed to get System Menu required IOS version.");
			gcprintf("\n>> ERROR! %s\n", EsError::ToString(ret));
			ret = -1;
			goto end;
		}

		if (sysMenuIOS == title->TitleId)
		{
			gcprintf("\n>> ERROR! This IOS is required for the currently installed System Menu.");
			gcprintf("\n>> ERROR! Uninstall Aborted!\n");
			ret = -1;
			goto end;
		}

		// Make sure we have SU Access
		if (!SysCheck::CheckNandPermissions())
		{
			gcprintf("\n");
			gcprintf("ERROR! Failed to get SU Access\n");
			gcprintf("Please use an IOS that has the NAND Permissions Patch applied.\n");
			ret = -1;
			goto end;
		}

		// Delete title and ticket at FS level.
		ticketRet = title->DeleteTicket();
		titleRet = title->DeleteTitle();
		contentsRet = titleRet;
	}
	else
	{
		// Remove title (contents and ticket)
		ticketRet = title->RemoveTicket();
		contentsRet	= title->RemoveTitleContents();
		titleRet = title->RemoveTitle();

		// Attempt forced uninstall if something fails
		if (ticketRet < 0 || contentsRet < 0 || titleRet < 0)
		{
			printf("\n\n");
			printf("At least one operation failed. Attempt low-level delete?\n");			
			if (Console::PromptContinue())
			{
				if (!SysCheck::CheckNandPermissions())
				{
					gcprintf("\n\n**** ERROR! *****\n");
					gcprintf("Failed to get SU Access\n");
					gcprintf("Please use and IOS that has the NAND Permissions Patch applied.\n");
					ret = -1;
					goto end;
				}
				ticketRet = title->DeleteTicket();
				titleRet = title->DeleteTitle();
				contentsRet = titleRet;
			}			
		}
	}
	
	if (ticketRet < 0 && contentsRet < 0 && titleRet < 0) ret = -1;
	else if (ticketRet < 0 || contentsRet < 0 || titleRet < 0) ret =  1;
	else ret =  0;

end:
	delete title; title = NULL;
	return ret;
}

int Title::DeleteTicket()
{
	int ret;
	char filepath[256];
	sprintf(filepath, "/ticket/%08x/%08x.tik", TITLEID1(TitleId), TITLEID2(TitleId));
	
	/* Delete ticket */
	gcprintf("Deleting Ticket File (%s)...", filepath);
	fflush(stdout);

	ret = Nand::Delete(filepath);
	if (ret < 0) gcprintf("\n>> ERROR! Nand::Delete: %s\n", NandError::ToString(ret));
	else gcprintf("Done\n");
	return ret;
}

int Title::DeleteTitle()
{
	int ret;
	char filepath[256];
	sprintf(filepath, "/title/%08x/%08x",  TITLEID1(TitleId), TITLEID2(TitleId));
	
	/* Remove title */
	gcprintf("Deleting Title Folder (%s)...", filepath);
	fflush(stdout);

	ret = Nand::Delete(filepath);
	if (ret < 0) gcprintf("\n>> ERROR! Nand::Delete: %s\n", NandError::ToString(ret));
	else gcprintf("Done\n");
	return ret;
}

int Title::RemoveTicket()
{
	tikview *viewdata ATTRIBUTE_ALIGN(32);

	u32 views = 0;
	int ret;

	gcprintf("Removing tickets...");

	/* Get number of ticket views */
	ret = ES_GetNumTicketViews(TitleId, &views);
	if (ret < 0) 
	{
		gcprintf("\n>> ERROR! ES_GetNumTicketViews: %s\n", EsError::ToString(ret));
		return ret;
	}

	if (!views) 
	{
		gcprintf("No tickets found!\n");
		return 0;
	} 
	
	/* Get ticket views */
	viewdata = (tikview*)Tools::AllocateMemory(sizeof(tikview) * views);
	ret = ES_GetTicketViews(TitleId, viewdata, views);
	if (ret < 0) 
	{
		gcprintf("\n>> ERROR! ES_GetTicketViews: %s\n", EsError::ToString(ret));
		goto end;
	}

	/* Remove tickets */
	for (u32 cnt = 0; cnt < views; cnt++) 
	{
		ret = ES_DeleteTicket(&viewdata[cnt]);
		if (ret < 0) 
		{
			gcprintf("\n>> ERROR! ES_DeleteTicket (View = %d): %s\n", cnt, EsError::ToString(ret));
			goto end;
		}
	}
	gcprintf("Done!\n");

end:
	delete viewdata; viewdata = NULL;
	return ret;
}

int Title::RemoveTitle()
{
	/* Remove title */
	gcprintf("Removing Title...");
	fflush(stdout);

	int ret = ES_DeleteTitle(TitleId);
	if (ret < 0) gcprintf("\n>> ERROR! ES_DeleteTitle: %s\n", EsError::ToString(ret));
	else printf("Done\n");
	return ret;
}

int Title::RemoveTitleContents()
{
	/* Remove title contents */
	gcprintf("Removing Title Contents...");
	fflush(stdout);

	int ret = ES_DeleteTitleContent(TitleId);
	if (ret < 0) gcprintf("\n>>ERROR! ES_DeleteTitleContent: %s\n", EsError::ToString(ret));
	else gcprintf("Done\n");
	return ret;
}

int Title::InstallIOS(IosRevisionIterator revision, u32 altSlot)
{
	int ret = 0;	
	bool tikDirty = false;
	bool tmdDirty = false;
	bool applyFakeSignPatch = false;
	bool applyEsIdentifyPatch = false;
	bool applyNandPermissionPatch = false;	

	// Prompt for Patches
	if (revision->IsStub)
	{
		Console::SetFgColor(Color::Yellow, Bold::On);
		gcprintf("!!! This version is a Stub. !!!\n");
		Console::ResetColors();
		gcprintf("Are you sure you want to continue?\n");
		if (!Console::PromptYesNo()) return -1;
		gcprintf("\n");
	}
    else if (revision->IgnoreAllPatches) { /* Do Nothing */ }
	else 
	{
		if (revision->CanPatchFakeSign)
		{
			if (!(applyFakeSignPatch = revision->ApplyFakeSignPatch))
			{
				gcprintf("Apply FakeSign (Trucha) patch to %s?\n", Name.c_str());
				applyFakeSignPatch = Console::PromptYesNo();
				gcprintf("\n");
			}
		}

		if (revision->CanPatchEsIdentify)
		{
			if (!(applyEsIdentifyPatch = revision->ApplyEsIdentifyPatch))
			{
				gcprintf("Apply ES_Identify patch to %s?\n", Name.c_str());
				applyEsIdentifyPatch = Console::PromptYesNo();
				gcprintf("\n");
			}
		}

		if (revision->CanPatchNandPermissions)
		{
			if (!(applyNandPermissionPatch = revision->ApplyNandPermissionsPatch))
			{
				gcprintf("Apply NAND Permissions Patch?\n");
				applyNandPermissionPatch = Console::PromptYesNo();
				gcprintf("\n");
			}
		}
	}	

	revision->ResetApplyPatchFlags();

	gcprintf("Loading %s v%u into memory\n", Name.c_str(), revision->Id);
	
	ret = this->Get(revision->Id);
	if (ret < 0)
	{
		if (ret != (int)TitleError::Cancelled) gcprintf("\n>> ERROR! Title->Get: %d\n", ret);
		return ret;
	}

	tmd_content *p_cr = TMD_CONTENTS((tmd*)SIGNATURE_PAYLOAD(this->Tmd));

	if (applyFakeSignPatch || applyEsIdentifyPatch || applyNandPermissionPatch)
	{
		int index = ModuleIndex((char*)"ES:");
		if (index < 0)
		{
			gcprintf("\n>> ERROR! Could not identify ES Module.\n");
			return -1;
		}

		int fakeSignPatchCount = 0;
		if (applyFakeSignPatch)
		{
			gcprintf("Patching FakeSign (Trucha) bug into ES Module(#%u)...", index);
			fakeSignPatchCount = Patcher::PatchFakeSign(DecryptedBuffer[index], BufferSize[index]);
			gcprintf("\n\t- patched %u hash check(s)\n", fakeSignPatchCount);
		}

		int esIdentifyPatchCount = 0;
		if (applyEsIdentifyPatch)
		{
			gcprintf("Patching ES_Identify in ES module(#%u)...", index);
			esIdentifyPatchCount = Patcher::PatchEsIdentity(DecryptedBuffer[index], BufferSize[index]);
			gcprintf("\n\t- patch applied %u time(s)\n", esIdentifyPatchCount);
		}

		int nandPermissionPatchCount = 0;
		if (applyNandPermissionPatch)
		{
			gcprintf("Patching nand permissions in ES module(#%u)...", index);
			nandPermissionPatchCount = Patcher::PatchNandPermissions(DecryptedBuffer[index], BufferSize[index]);
			gcprintf("\n\t- patch applied %u time(s)\n", nandPermissionPatchCount);
		}

		if (fakeSignPatchCount > 0 || esIdentifyPatchCount > 0 || nandPermissionPatchCount > 0)
		{
			sha1 hash;
			SHA1(DecryptedBuffer[index], (u32)p_cr[index].size, hash);
			memcpy(p_cr[index].hash, hash, sizeof(hash));
			memset(hash, 0, sizeof(hash));
			tmdDirty = true;
		}		
	}

	if (altSlot)
	{
		u64 newTitleId = TITLEID(1,altSlot);

		ChangeTicketTitleId(newTitleId);
		ChangeTmdTitleId(newTitleId);
		this->TitleId = newTitleId;
		tmdDirty = tikDirty = true;
	}


	if (tmdDirty || tikDirty)
	{
		if (tikDirty) 
		{
			Patcher::ForgeTicket(this->Ticket);
			gcprintf("Ticket was Forged\n");
		}
		if (tmdDirty) 
		{
			Patcher::ForgeTMD(this->Tmd);
			gcprintf("TMD was Forged...\n");
		}
		gcprintf("Encrypting IOS...\n");
		Encrypt();
		gcprintf("Preparations complete\n\n");
	}

	ret = PerformInstall();
	if (ret < 0)
	{		
		if (ret == -1017 || ret == -2011) 
		{
			gcprintf("\n>> You need to use an IOS with FakeSign (Trucha) bug.\n");
		} 
		else if (ret == -1035) 
		{
			gcprintf("\n>> Has your installed IOS%u a higher revison than %u?\n", TITLEID2(TitleId), revision->Id);
		}

		return -1;
	}	
	return (int)TitleError::Success;
}

int Title::InstallSysMenu(SysMenuMatrixIterator sysMenuItem)
{
	int ret = 0;
	u64 iosTitleId;
	u16 iosRevision;
	Title *ios = NULL;

	if (sysMenuItem->Revision < 97) return (int)TitleError::InvalidRevision;

	bool priiloaderInstalled = Tools::IsPriiloaderInstalled();

	gprintf("Installing Revision %u\n", sysMenuItem->Revision);

	gcprintf("The System Menu is a critical piece of the Wii.\n");
	gcprintf("Before continuing please make sure that you have BootMii installed\n");
	gcprintf("and that you also have a backup of your NAND before continuing.\n");
	gcprintf("During the installation process please make sure you do not turn off the Wii.\n\n");		

	if (priiloaderInstalled)
	{
		if (!SysCheck::CheckNandPermissions())
		{
			Console::SetFgColor(Color::Yellow, Bold::On);
			gcprintf("Priiloader Detected.\n");
			gcprintf("The IOS you are using cannot gain SU access.\n");
			gcprintf("The installer will not be able to Backup & Restore Priiloader.\n");
			gcprintf("It is highly recommended that you exit and run an IOS\n");
			gcprintf("that has the NAND Permissions Patch before continuing.\n\n");
		}
		else
		{
			Console::SetFgColor(Color::Green, Bold::On);
			gcprintf("Priiloader Detected.\n");
			gcprintf("During the installation process Priiloader will be temporarily backed up.\n");
			gcprintf("You will be prompted after the installation is completed to check if you\n");
			gcprintf("would like to restore Priiloader back to its backed up state.\n\n");
		}
		Console::ResetColors();
	}

	if (!Console::PromptContinue()) return int(TitleError::Cancelled);
	gcprintf("\n");

	if (priiloaderInstalled) Tools::BackupPriiloader();

	if (CONF_GetRegion() != sysMenuItem->RegionId) 
	{
		gcprintf("\nThe System Menu you are trying to install is not supported by this region.\n");
		if (!Console::PromptContinue()) 
		{
			ret = (int)TitleError::InvalidRegion;
			goto final;
		}
	}

	gcprintf("\nChecking for Required IOS%u (v%u)...\n", sysMenuItem->IosRequired, sysMenuItem->IosRevision);
	iosTitleId = TITLEID(1, sysMenuItem->IosRequired);
	iosRevision = SysTitle::GetVersion(iosTitleId);

	// Check Version numbers and let users know what is going on.
	if (iosRevision == 0)
	{
		gcprintf("IOS%u (v%u) is not installed.\n", sysMenuItem->IosRequired, sysMenuItem->IosRevision);
		gcprintf("This is required for the System Menu to run.\n\n");
		gcprintf("Would you like to install the required IOS now?\n");
	}
	else if (iosRevision != sysMenuItem->IosRevision)
	{
		gcprintf("IOS%u (v%u) found. Expecting (v%u)\n\n", sysMenuItem->IosRequired, iosRevision, sysMenuItem->IosRevision);
		gcprintf("Would you like to install the required IOS now?\n");
	}

	// If Version numbers don't match then install IOS.
	if (iosRevision != sysMenuItem->IosRevision)
	{
		if (!Console::PromptYesNo())
		{
			ret = (int)TitleError::Cancelled;
			goto final;
		}
		gcprintf("\n");
		gcprintf("Starting Installation of IOS%u (v%u)\n", sysMenuItem->IosRequired, sysMenuItem->IosRevision);
		ios = new Title(iosTitleId);

		gcprintf("Getting Required Files...\n");
		ret = ios->Get(sysMenuItem->IosRevision);
		if (ret < 0)
		{
			if (ret != (int)TitleError::Cancelled) gcprintf("\n>> ERROR! Failed to Get Required IOS. ErrorCode (%d)\n", ret);
			goto final;
		}

		ret = ios->PerformInstall();
		if (ret < 0)
		{
			gcprintf("\n>> ERROR! IOS Installation Failed. Cancelling System Menu Install\n");
			goto final;
		}	
		gcprintf("\n");
	}
	
	// Install System Menu
	gcprintf("\nStarting Installation of %s...\n", sysMenuItem->Name.c_str());
	gcprintf("Getting Required Files...\n");
	ret = this->Get(sysMenuItem->Revision);
	if (ret < 0)
	{
		if (ret != (int)TitleError::Cancelled) gcprintf("\n>> ERROR! Failed to get %s. ErrorCode (%d)\n", sysMenuItem->Name.c_str(), ret);
		goto final;
	}

	gcprintf("\n");
	gcprintf("During the installation process it is important that you\n");
	Console::SetFgColor(Color::Yellow, Bold::On);
	gcprintf("!!!DO NOT POWER OFF THE WII!!!\n");
	Console::ResetColors();
	gcprintf("Failure to do so can cause your Wii to be bricked.\n");
	gcprintf("\n");
	gcprintf("Ready to install %s?\n", sysMenuItem->Name.c_str());
	if (!Console::PromptYesNo()) { ret = (int)TitleError::Cancelled; goto final; }
	gcprintf("\n");

	ret = this->PerformInstall();
	if (ret < 0) goto final;

	if (priiloaderInstalled)
	{
		gcprintf("\n");
		gcprintf("Do you want to restore Priiloader?\n");
		if (Console::PromptYesNo())
		{
			gprintf("\n");
			int pret = Tools::RestorePriiloader();
			if (pret < 0)
			{
				gcprintf("\n");
				gcprintf("Failed To Restore PriiLoader. ErrorCode = %d\n", pret);
			}
		}
	}
	
	ret = (int)TitleError::Success;

final:
	delete ios; ios = NULL;
	return ret;
}

int Title::Install(ChannelIterator channel)
{
	int ret = -1;
	Title *title = NULL;
	Title *subTitle = NULL;
	char wadFileName[256];

	// Get Titles
	gcprintf("Loading %s...\n", channel->Name.c_str());
	title = new Title(channel->TitleId);
	title->Type = TitleType::CHANNEL; // Incase it is a region only channel.
	sprintf(wadFileName, "%s-NUS.wad", channel->Name.c_str());
	ret = title->Get(0, wadFileName);
	if (ret < 0)
	{
		if (ret != (int)TitleError::Cancelled) gcprintf("\n>> ERROR! Title->Get: %d\n", ret);
		goto end;
	}
	
	if (channel->SubTitleId > 0)
	{
		subTitle = new Title(channel->SubTitleId);
		subTitle->Type = TitleType::CHANNEL;
		gprintf("Sub Title Type = %d\n", (int)subTitle->Type);
		gcprintf("\n");
		gcprintf("Loading Secondary Title %08X-%08X\n", TITLEID1(channel->SubTitleId), TITLEID2(channel->SubTitleId));		
		memset(wadFileName, 0, 256);
		sprintf(wadFileName, "%s-%s-NUS.wad", channel->Name.c_str(), channel->Region.c_str());
		ret = subTitle->Get(0, wadFileName);
		if (ret < 0)
		{
			if (ret != (int)TitleError::Cancelled) gcprintf("\n>> ERROR! SubTitle->Get: %d\n", ret);
			goto end;
		}
	}

	// Install titles
	gcprintf("\n");
	gcprintf("Starting Installation of %s\n", channel->Name.c_str());
	ret = title->PerformInstall();
	if (ret < 0) goto end;

	if (channel->SubTitleId > 0)
	{
		gcprintf("\n");
		gcprintf("Starting Installation of %s\n", subTitle->Name.c_str());
		ret = subTitle->PerformInstall();
		if (ret < 0) goto end;
	}

	ret = (int)TitleError::Success;

end:
	memset(wadFileName, 0, 256);
	DisplayInstallStatus(ret, title);
	delete subTitle; subTitle = NULL;
	delete title; title = NULL;
	return ret;
}

int Title::Install(IosRevisionIterator revision)
{
	return Install(revision, NULL, false);
}

int Title::Install(IosRevisionIterator revision, bool useAltSlot)
{
	return Install(revision, NULL, useAltSlot);
}

int Title::Install(IosRevisionIterator iosRevision, signed_blob *storedTMD)
{
	return Install(iosRevision, storedTMD, false);
}

int Title::GetAlternateIosSlot()
{
	int ret = -1;
	u32 button = 0;
	u32List emptyList;
	u32Iterator selected;

	ret = System::GetEmptyIosIdSlots(emptyList);
	if (ret < 0) goto final;

	emptyList.push_back(0);
	selected = emptyList.begin();

	printf("Select the Slot you would like to install this IOS in.\n");
	printf("[%s|%s] Change Selection [B] Cancel [Home] Return To Loader\n", LeftArrow, RightArrow);

	while (System::State == SystemState::Running)
	{
		Console::ClearLine();
		gcprintf("%s<<%s %s", AnsiYellowBoldFG, AnsiNormal, AnsiSelection);

		if (*selected == 0) Console::PrintCenterGC(10, "Cancel");
		else Console::PrintCenterGC(10, "IOS%u", *selected);
		
		Console::ResetColors();
		gcprintf(" %s>>%s", AnsiYellowBoldFG, AnsiNormal);

		while (Controller::ScanPads(&button))
		{
			if (button == WPAD_BUTTON_HOME) System::Exit();
			if (System::State != SystemState::Running) goto final;

			if (button == WPAD_BUTTON_B)
			{
				ret = (int)TitleError::Cancelled;
				goto final;
			}

			/* 
			Because the list is in reverse we need to make sure left is
			going up the vector and right is going down
			*/
			if (button == WPAD_BUTTON_LEFT)
			{
				if (selected == emptyList.end()-1) selected = emptyList.begin();
				else ++selected;
			}

			if (button == WPAD_BUTTON_RIGHT)
			{
				if (selected == emptyList.begin()) selected = emptyList.end()-1;
				else --selected;
			}


			if (button == WPAD_BUTTON_A)
			{
				ret = *selected;
				goto final;
			}
			
			if (button) break;
		}
	}

final:
	emptyList.clear();
	return ret;
}

int Title::Install(IosRevisionIterator iosRevision, signed_blob *storedTMD, bool useAltSlot)
{
	Title* title = new Title(iosRevision->TitleId);
	int ret = -1;
	int altSlot = 0;
	
	if (useAltSlot)
	{
		altSlot = GetAlternateIosSlot();
		gprintf("Alt Slot = %d\n", altSlot);
		gcprintf("\n\n");
		if (altSlot == (int)TitleError::Cancelled || altSlot == 0)
		{
			ret = (int)TitleError::Cancelled; // incase altSlot = 0
			goto error;
		}
		if (altSlot < 0) 
		{
			gcprintf(">> ERROR! Failed to retrieve the empty IOS slots. Error (%d)\n", ret);
			ret = altSlot;
			goto final;
		}
	}
	
	if (storedTMD)
	{
		if (!IS_VALID_SIGNATURE(storedTMD))
		{
			gcprintf("\n>> ERROR! Invalid Stored TMD\n");
			ret = -1;
			goto final;
		}
		u32 storedTmdSize = (u32)SIGNED_TMD_SIZE(storedTMD);
		title->StoredTMD = (signed_blob*)Tools::AllocateMemory(storedTmdSize);
		if (!title->StoredTMD)
		{
			gcprintf("\n>> ERROR! Out Of Memory\n");
			ret = -1;
			goto final;
		}
		memcpy(title->StoredTMD, storedTMD, storedTmdSize);
	}	
	
	ret = title->InstallIOS(iosRevision, altSlot);

error:
	DisplayInstallStatus(ret, title);

final:
	delete title; title = NULL;
	return ret;
}

int Title::Install(SysMenuMatrixIterator sysMenuItem)
{
	Title* title = new Title(0x100000002ull);
	int ret = title->InstallSysMenu(sysMenuItem);
	DisplayInstallStatus(ret, title);
	delete title; title = NULL;
	return ret;
}

void Title::DisplayInstallStatus(int status, Title *title)
{
	if (status == (int)TitleError::NotSupported) 
		gcprintf("\n%s is not supported by the installer at this time\n", title->Name.c_str());
	else if (status == (int)TitleError::Cancelled) 
		gcprintf("\nInstallation Cancelled\n", title->Name.c_str());
	else if (status == (int)TitleError::InvalidRevision)
	{
		gcprintf("\n");
		gcprintf(">> ERROR: Cancelling Installation.\n");
		gcprintf(">> Invalid Revision Detected.\n");
	}
	else if (status == (int)TitleError::InvalidRegion) 
	{
		gcprintf("\n");
		gcprintf(">> ERROR! Cancelling Installation.\n");
		gcprintf(">> This revision is not valid for your Wii's region.\n");
	}
	else if (status >= 0) 
		gcprintf("\nInstallation Completed\n");
	else 
		gcprintf("\nERROR! Installer returned error code (%d)\n", status);
}

void Title::ChangeTicketTitleId(u64 titleId)
{
	static u8 iv[16] ATTRIBUTE_ALIGN(32);
	static u8 keyin[16] ATTRIBUTE_ALIGN(32);
	static u8 keyout[16] ATTRIBUTE_ALIGN(32);

	tik *ptik = (tik*)SIGNATURE_PAYLOAD(this->Ticket);
	u8 *enckey = (u8*)ptik->cipher_title_key;

	memcpy(keyin, enckey, sizeof(keyin));
	memcpy(iv, &ptik->titleid, sizeof(ptik->titleid));

	aes_set_key((u8*)AesCommonKey);
	aes_decrypt(iv, keyin, keyout, sizeof(keyin));
	ptik->titleid = titleId;

	memset(iv, 0, sizeof(iv));
	memcpy(iv, &ptik->titleid, sizeof(ptik->titleid));

	aes_encrypt(iv, keyout, keyin, sizeof(keyout));
	memcpy(enckey, keyin, sizeof(keyin));

	memset(iv, 0, sizeof(iv));
	memset(keyin, 0, sizeof(keyin));
	memset(keyout, 0, sizeof(keyout));
}

void Title::ChangeTmdTitleId(u64 titleId)
{
	((tmd*)SIGNATURE_PAYLOAD(this->Tmd))->title_id = titleId;
}
