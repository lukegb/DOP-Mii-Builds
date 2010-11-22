#ifndef _NAND_H_
#define _NAND_H_

#include <string>
#include "Global.h"

namespace IO
{
	class Nand
	{	
	private:
		static bool Initialized;

	public:
		static int Startup();
		static void Shutdown();

		static int CreateDir(u64 titleId);
		static int CreateDir(const char *path);
		static int CreateDir(string path);
		static int CreateFile(u64 titleId, const char *filename);
		static int CreateFile(const char *filepath, u8 attributes, u8 ownerPerm, u8 groupPerm, u8 otherPerm);
		static int CopyFile(string from, string to);
		static int CopyFile(const char *from, const char *to);

		static int Open(const char* filepath, u8 mode);
		static int OpenRead(const char* filepath);
		static int OpenReadWrite(const char* filepath);

		static u32 GetFileSize(int fp);
		
		static int OpenFile(u64 titleId, const char *filename, u8 mode);

		static int Close(int fp);
		
		static int Read(int fp, u8 *buffer, u32 length);
		static int Read(const char* filepath, u8 **buffer);
		static int Write(int file, u8 *buffer, u32 length);
		static int Write(const char* filepath, u8 *buffer, u32 length);
		static int Seek(int fp, int where, int whence);

		static int ReadDir(const char* path, StringList &list);

		static int RemoveDir(u64 titleId);
		static int Delete(const char* filepath);		
	};
};

#endif

