#ifndef _NUS_H_
#define _NUS_H_

#include <string>

using namespace std;

class NUS
{
private:
	static void ExtractTmdVersion(string &filename, u64 titleId, u16 *version, char *content, u8 **outbuf);
	static string GetAlternateUrl(u64 titleId, u16 version, const char* content);
	static string GetCacheFileName(u64 titleId, u16 version, const char* content);
	static string GetCacheFolder(u64 titleId, u16 version);
	static void ReplaceTags(string &str, u64 titleId, u16 version);
public:
	static int GetFile(u64 titleId, u16 *version, char *content, u8 **outbuf, u32 *outlen);
};

#endif