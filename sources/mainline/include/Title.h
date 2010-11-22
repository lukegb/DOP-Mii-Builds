#ifndef _TITLE_H_
#define _TITLE_H_

#include <string>
#include "IOSMatrix.h"
#include "ChannelMatrix.h"
#include "SysMenuMatrix.h"

#define GETWADSD 0
#define GETWADUSB 1
#define GETNUS 2

using namespace std;
using namespace Titles;

enum class TitleType
{
	UNKNOWN = 0,
	BOOT2,
	CHANNEL,
	IOS,
	SYSMENU
};

enum class TitleError
{
	General = -1,
	Success = 1,
	InvalidRevision = -0xFFFC,
	InvalidRegion = -0xFFFD,
	Cancelled = -0xFFFE,
	NotSupported = -0xFFFF
};

class Title
{
protected:
	signed_blob *StoredTMD;
public: // oh, ffs, cba to make each individual method public, so I'll just publicise the whole thing.
	bool ContainsModule(u8 *buffer, u32 bufferSize, char *module);
	int  CheckContentHashes();
	void Clear();
	void Decrypt();
	void DecryptBuffer(u16 index, u8 *source, u8 *dest, u32 len);		
	int  DeleteTicket();
	int  DeleteTitle();
	void DetermineTitleType();
	int  DowngradeTmdRevision();
	int  Download(u16 revision);
	void Encrypt();
	void EncryptBuffer(u16 index, u8 *source, u8 *dest, u32 len);
	int  Get() { return Get(0); }
	int  Get(u16 revision);
	int  Get(u16 revision, const char* wadFileName);
	int  Get(u16 revision, int getMethod);
	int  Get(u16 revision, const char* wadFileName, int getMethod);
	void Initialize();
	int  InstallIOS(IosRevisionIterator revision, u32 altSlot);
	int  InstallSysMenu(SysMenuMatrixIterator sysMenuItem);
	int  InstallChannel();
	int  ModuleIndex(char *module);
	int  PerformInstall();
	int  RemoveTicket();
	int  RemoveTitle();
	int  RemoveTitleContents();
	int  SetContentCount(u32 count);
	int  LoadFromWad(u16 revision, const char* filename);

	void ChangeTicketTitleId(u64 titleId);
	void ChangeTmdTitleId(u64 titleId);

	static void DisplayInstallStatus(int status, Title *title);
	static int  GetAlternateIosSlot();
public:
	u64 TitleId;

	TitleType Type;
	string Name;

	signed_blob *Ticket;
	u32 TicketSize;

	signed_blob *Tmd;
	u32 TmdSize;

	signed_blob *Crl;
	u32 CrlSize;

	u8 **EncryptedBuffer;
	u8 **DecryptedBuffer;
	u32 *BufferSize;
	u32 ContentCount;
	
	Title(u64 titleId);
	Title(u32 titleId1, u32 TitleId2);
public:
	~Title();
	static void GetTitleKey(signed_blob *signedTicket, u8 *key);	
	static int Install(IosRevisionIterator iosRevision);
	static int Install(IosRevisionIterator iosRevision, bool useAltSlot);
	static int Install(IosRevisionIterator iosRevision, signed_blob *storedTMD);
	static int Install(IosRevisionIterator iosRevision, signed_blob *storedTMD, bool useAltSlot);
	static int Install(ChannelIterator channel);
	static int Install(SysMenuMatrixIterator sysMenuItem);
	static int Uninstall(const u64 titleId);
	static int Uninstall(const u32 titleId1, const u32 titleId2);	
} ATTRIBUTE_ALIGN(32);

#endif