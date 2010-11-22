#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

namespace IO 
{
	class SD
	{
	public:
		static bool Mount();
		static void Unmount();
	};

	class USB
	{
	private:
		static bool isMounted;
	public:
		static void Startup();
		static void Shutdown();
		static bool Mount();
		static void Unmount();
	};

	class File
	{
	public:
		static bool Exists(const char* path);
		static FILE* OpenBinary(const char *path);
		static int ReadBinary(const char *path, u8 **outbuf, u32 *outlen);
		static int Read(const char *path, u8 **outbuf, u32 *outlen);
		static int Read(const char *path, const char *mode, u8 **outbuf, u32 *outlen);
		static int ReadOffset(FILE *fp, u32 offset, u32 length, u8 **outbuf);
		static int ReadOffsetAlloc(FILE *fp, u32 offset, u32 length, u8 **outbuf);
		static bool FolderCreateTree(const char *fullpath);
	};

	class Directory
	{
	public:
		static bool CreateTree(const char *fullpath);
	};
};

#endif