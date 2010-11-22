#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <algorithm>

#include "Nand.h"
#include "Gecko.h"
#include "Error.h"
#include "Tools.h"

using namespace std;
using namespace IO;

/* Constants */
#define BASE_PATH "/tmp"

bool Nand::Initialized = false;

int Nand::Startup() 
{
	if (Initialized) return 0;
    int ret = ISFS_Initialize();
	Initialized = (ret == ISFS_OK);
	return ret;
}

void Nand::Shutdown()
{
	if (!Initialized) return;
	ISFS_Deinitialize();
	Initialized = false;
}

int Nand::CreateDir(u64 titleId) 
{
	Startup();
    char dirpath[ISFS_MAXPATH];

    /* Generate dirpath */
    sprintf(dirpath, BASE_PATH "/%08x", (u32)(titleId & 0xFFFFFFFF));

    /* Create directory */
	return CreateDir(dirpath);
}

int Nand::CreateDir(string path)
{
	return CreateDir(path.c_str());
}

int Nand::CreateDir(const char* path)
{
	Startup();
	return ISFS_CreateDir(path, 0, ISFS_OPEN_RW, ISFS_OPEN_RW, ISFS_OPEN_RW);
}

int Nand::CreateFile(u64 titleId, const char *filename) 
{
	Startup();
    char filepath[ISFS_MAXPATH];

    /* Generate filepath */
    sprintf(filepath, BASE_PATH "/%08x/%s", (u32)(titleId & 0xFFFFFFFF), filename);

    /* Create file */
    return ISFS_CreateFile(filepath, 0, ISFS_OPEN_RW, ISFS_OPEN_RW, ISFS_OPEN_RW);
}

int Nand::CreateFile(const char *filepath, u8 attributes, u8 ownerPerm, u8 groupPerm, u8 otherPerm)
{
	Startup();
	return ISFS_CreateFile(filepath, attributes, ownerPerm, groupPerm, otherPerm);
}

int Nand::OpenFile(u64 titleId, const char *filename, u8 mode) 
{
	Startup();
    char filepath[ISFS_MAXPATH];

    /* Generate filepath */
    sprintf(filepath, BASE_PATH "/%08x/%s", (u32)(titleId & 0xFFFFFFFF), filename);

    /* Open file */
	return ISFS_Open(filepath, mode);
}

int Nand::Open(const char* filepath, u8 mode)
{
	Startup();
	return ISFS_Open(filepath, mode);
}

int Nand::OpenRead(const char* filepath)
{
	Startup();
	return ISFS_Open(filepath, ISFS_OPEN_READ);	
}

int Nand::OpenReadWrite(const char* filepath)
{
	Startup();
	return ISFS_Open(filepath, ISFS_OPEN_RW);
}

int Nand::Close(int fp) 
{
	if (fp < 1) return 0;
	return ISFS_Close(fp);
}

int Nand::Read(int fp, u8 *buffer, u32 length) 
{
	Startup();
    /* Read file */
    return ISFS_Read(fp, buffer, length);
}

int Nand::Read(const char* filepath, u8 **buffer)
{
	int ret = 0;
	int file = 0;
	u32 size = 0;

	file = Nand::OpenRead(filepath);
	if (file < 1) return file;

	size = Nand::GetFileSize(file);
	if (size == 0) { ret = -1; goto end; }

	*buffer = (u8*)Tools::AllocateMemory(size);
	if (!buffer)
	{
		gprintf("Out Of Memory!\n");
		return -1;
	}

	ret = Nand::Read(file, *buffer, size);
	if (!buffer)
	{
		gprintf(">> ERROR! Nand::Read - Null Buffer\n");
		ret = -1;
	}	
end:
	Nand::Close(file);
	return ret;
}

int Nand::Write(int file, u8 *buffer, u32 length) 
{
	Startup();
    return ISFS_Write(file, buffer, length);
}

int Nand::Write(const char *filepath, u8 *buffer, u32 length)
{
	Startup();

	if (!buffer) { gprintf(">> ERROR! Nand::Write - Null Buffer\n"); return -1; }
	if (!length) { gprintf(">> ERROR! Nand::Write - 0 Length\n"); return -1; }	

	int ret = Delete(filepath);
	if (ret < 0 && ret != -106) gprintf("\t>> ERROR! Nand::Write->Delete: %s\n", NandError::ToString(ret));

	ret = CreateFile(filepath, 0, ISFS_OPEN_RW, ISFS_OPEN_RW, ISFS_OPEN_RW);
	if (ret < 0) 
	{
		gprintf("\t>> ERROR! Nand::Write->CreateFile: %s\n", NandError::ToString(ret));
		return ret;
	}

	int file = OpenReadWrite(filepath);
	if (file < 0) 
	{
		gprintf("\t>> ERROR! Nand::Write->OpenReadWrite: %s\n", NandError::ToString(ret));
		return file;
	}

	ret = Nand::Write(file, buffer, length);
	if (ret < 0) gprintf("\t>> ERROR! Nand::Write->Write: %s\n", NandError::ToString(ret));
	Nand::Close(file);
	return ret;
}

int Nand::RemoveDir(u64 titleId) 
{
	Startup();
    char *dirlist = NULL;

    char dirpath[ISFS_MAXPATH], filepath[ISFS_MAXPATH];
    u32 cnt, idx, nb_files;
    int ret;

    /* Generate dirpath */
    sprintf(dirpath, BASE_PATH "/%08x", (u32)(titleId & 0xFFFFFFFF));

    /* Retrieve number of files */
    ret = ISFS_ReadDir(dirpath, NULL, &nb_files);
    if (ret < 0) return ret;

    /* There are files inside the directory */
    if (nb_files) 
	{
        /* Allocate memory */
		dirlist = (char*)Tools::AllocateMemory(ISFS_MAXPATH * nb_files);
        if (!dirlist) return -1;

        /* Retrieve filelist */
        ret = ISFS_ReadDir(dirpath, dirlist, &nb_files);
        if (ret < 0) goto out;

        for (cnt = idx = 0; cnt < nb_files; cnt++) 
		{
            char *ptr = dirlist + idx;

            /* Generate filepath */
            sprintf(filepath, "%s/%s", dirpath, ptr);

            /* Delete file */
            ret = ISFS_Delete(filepath);
            if (ret < 0) goto out;

            /* Move to next entry */
            idx += strlen(ptr) + 1;
        }
    }

    /* Delete directory */
    ret = ISFS_Delete(dirpath);

out:
    /* Free memory */
	delete dirlist; dirlist = NULL;
    return ret;
}

int Nand::Delete(const char* filepath)
{
	Startup();
	return ISFS_Delete(filepath);
}

int Nand::Seek(int fp, int where, int whence)
{
	Startup();
	return ISFS_Seek(fp, where, whence);
}

u32 Nand::GetFileSize(int fp)
{
	if (fp < 1) return 0;
	Startup();
	fstats *status = (fstats*)Tools::AllocateMemory(sizeof(fstats));
	int ret = ISFS_GetFileStats(fp, status);
	if (ret < 0) { ret = 0; goto end; }
	ret = status->file_length;
end:
	delete status; status = NULL;
	return ret;
}

int Nand::ReadDir(const char *path, StringList &list)
{
	Startup();
	char *dirlist = NULL;
	u32 count = 0;
	u32 index = 0;
	int ret = 0;

	ret = ISFS_ReadDir(path, NULL, &count);
	if (ret < ISFS_OK) return ret;

	if (count == 0) return ISFS_OK;
	
	dirlist = (char*)Tools::AllocateMemory(ISFS_MAXPATH * count);
	if (!dirlist) return ISFS_ENOMEM;

	ret = ISFS_ReadDir(path, dirlist, &count);
	if (ret < ISFS_OK) goto end;

	for (u32 i = 0; i < count; i++)
	{
		const char *name = dirlist + index;
		list.push_back(name);
		index += strlen(name) + 1;
	}

	sort(list.begin(), list.end());

end:
	delete dirlist; dirlist = NULL;
	return ret;
}

int Nand::CopyFile(string from, string to)
{
	return CopyFile(from.c_str(), to.c_str());
}

int Nand::CopyFile(const char *from, const char *to)
{
	u8 *buffer = NULL;
	int ret = 0;

	int size = Nand::Read(from, &buffer);
	if (size < 0) {ret = size; goto end;}

	if (!buffer) { gprintf(">> ERROR! Nand::CopyFile - Null Buffer\n"); return -1; }

	ret = Nand::Write(to, buffer, (u32)size);
end:
	delete buffer; buffer = NULL;
	return ret;
}