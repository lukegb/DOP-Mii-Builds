#include <cstdio>
#include <cstdarg>
#include <string.h>
#include <malloc.h>
#include <sys/dir.h>
#include <unistd.h>
#include <sdcard/wiisd_io.h>
#include <fat.h>
//#include "UsbStorage.h"
#include "FileSystem.h"
#include "Tools.h"
#include "Gecko.h"

#define CACHE 32
#define SECTORS 64
#define SECTORS_SD 32

namespace IO 
{
	void SD::Unmount()
	{
		fatUnmount("sd:/");
		__io_wiisd.shutdown();
	}

	bool SD::Mount()
	{			
		SD::Unmount();
		return fatMountSimple("sd", &__io_wiisd);
	}

	bool USB::isMounted = false;

	void USB::Unmount()
	{
		fatUnmount("usb:/");
		isMounted = false;
	}

	bool USB::Mount()
	{
		if (isMounted) return true;
		gprintf("Mounting USB Drive \n");
		USB::Unmount();

		// To Hopefully Wake Up The Drive
		fatMountSimple("usb", &__io_usbstorage);

		bool isInserted = __io_usbstorage.isInserted();
		gprintf("USB::IsInserted = %d\n", isInserted);
		if (!isInserted) return false;

		// USB Drive may be "sleeeeping". 
		// We need to try Mounting a few times to wake it up
		int retry = 10;		
		
		while (retry)
		{
			isMounted = fatMountSimple("usb", &__io_usbstorage);
			if (isMounted) break;
			sleep(1);
			retry--;
		}
		
		if (isMounted) gprintf("USB Drive Is Mounted\n");
		return isMounted;		
	}

	void USB::Startup()
	{
		USB::Unmount();
		// To Hopefully Wake Up The Drive				
		isMounted = fatMountSimple("usb", &__io_usbstorage);
	}

	void USB::Shutdown()
	{
		if (!isMounted) return;
		fatUnmount("usb:/");
		isMounted = false;
	}

	FILE* File::OpenBinary(const char* path)
	{
		return fopen(path, "rb");
	}

	bool File::Exists(const char *path)
	{
		FILE *fp = NULL;
		fp = fopen(path, "r");
		if (fp)
		{
			fclose(fp);
			fp = NULL;
			return true;
		}

		return false;
	}

	int File::ReadBinary(const char* path, u8 **outbuf, u32 *outlen)
	{
		return Read(path, "rb", outbuf, outlen);
	}

	int File::Read(const char *path, u8 **outbuf, u32 *outlen)
	{
		return Read(path, "r", outbuf, outlen);
	}

	int File::Read(const char *path, const char *mode, u8 **outbuf, u32 *outlen)
	{
		int ret;

		FILE *fp = NULL;
		fp = fopen(path, mode);
		if (fp)
		{
			gprintf("FileRead = %s\n", path);
			fseek(fp, 0, SEEK_END);
			*outlen = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			*outbuf = (u8*)Tools::AllocateMemory(*outlen);

			if (*outbuf == NULL) 
			{
				gcprintf("Out of memory: Size %d\n", *outlen);
				ret = -1;
			}		

			if (fread(*outbuf, *outlen, 1, fp) != 1) ret = -2; // Failed to read the file so return an error
			else ret = 1; // File successfully loaded so return

			fclose(fp);
			fp = NULL;
			return ret;
		}
		else return -1;
	}

	int File::ReadOffset(FILE *fp, u32 offset, u32 length, u8 **outbuf)
	{
		fseek(fp, offset, SEEK_SET);
		return fread(*outbuf, length, 1, fp);
	}

	int File::ReadOffsetAlloc(FILE *file, u32 offset, u32 length, u8 **outbuf)
	{
		u8 *buffer = NULL;
		int ret;

		/* Allocate memory */
		buffer = (u8*)Tools::AllocateMemory(length);
		if (!buffer) return -1;

		/* Read file */
		ret = ReadOffset(file, offset, length, &buffer);
		if (ret < 0) 
		{
			delete buffer; buffer = NULL;
			return ret;
		}
		DCFlushRange(buffer, length);

		/* Set pointer */
		*outbuf = buffer;
		return 0;
	}

	bool Directory::CreateTree(const char* fullpath)
	{    
		bool result = false;
		char dir[300];
		char *pch = NULL;
		u32 len;
		struct stat st;

		strlcpy(dir, fullpath, sizeof(dir));
		len = strlen(dir);
		if (len && len< sizeof(dir)-2 && dir[len-1] != '/');
		{
			dir[len++] = '/';
			dir[len] = '\0';
		}
		if (stat(dir, &st) != 0) // fullpath not exist?
		{ 
			while (len && dir[len-1] == '/') dir[--len] = '\0';	// remove all trailing /
			pch = strrchr(dir, '/');
			if (pch == NULL) goto final;
			*pch = '\0';
			if (CreateTree(dir)) 
			{
				*pch = '/';
				if (mkdir(dir, 0777) == -1) goto final;
			} 
			else goto final;
		}

		result = true;

	final:		
		memset(dir, 0, sizeof(dir));
		return result;
	}
};
