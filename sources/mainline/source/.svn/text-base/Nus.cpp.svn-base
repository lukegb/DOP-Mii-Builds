#include <cstdio>
#include <string.h>
#include <cstdlib>
#include <ogc/es.h>
#include <unistd.h>

#include "Nus.h"
#include "Http.h"
#include "Tools.h"
#include "Network.h"
#include "Gecko.h"
#include "FileSystem.h"
#include "Settings.h"

using namespace IO;

void NUS::ExtractTmdVersion(string &filename, u64 titleId, u16 *version, char *content, u8 **outbuf)
{		
	gprintf("Getting Version from TMD File\n");
	tmd *pTMD = (tmd *)SIGNATURE_PAYLOAD((signed_blob *)*outbuf);
	*version = pTMD->title_version;
	filename = GetCacheFolder(titleId, *version);
	char tmp[16];
	sprintf(tmp, "tmd.%u", *version);
	filename.append(tmp);
	memset(tmp, 0, sizeof(tmp));
	gprintf("New Filename = %s\n", filename.c_str());
}

int NUS::GetFile(u64 titleId, u16 *version, char *content, u8 **outbuf, u32 *outlen) 
{	
	int ret = 0;
	char defaultUrl[256] = "";
	string cachefile;
	string alternateUrl;
	string folder;
	FILE *fp = NULL;
	u32 titleId1 = TITLEID1(titleId);
	u32 titleId2 = TITLEID2(titleId);
	bool deviceMounted = false;	
	int retry = 5;
	HTTP *http = new HTTP();		

	if (Tools::StringStartsWithI(Settings::Instance().NUS.CacheFolder, "sd:/") && SD::Mount()) deviceMounted = true;
	if (Tools::StringStartsWithI(Settings::Instance().NUS.CacheFolder, "usb:/") && USB::Mount()) deviceMounted = true;

	gprintf("\nTitleID1 = %08X, TitleID2 = %08X, Content = %s, Version = %u\n", titleId1, titleId2, content, *version);

	cachefile = GetCacheFileName(titleId, *version, content);

	// If version is = 0, check for existence X:/XXX/XXX/tmd
	// This is used for NUS offline mode
	if (strcmpi(content, "TMD") == 0 && *version == 0 && deviceMounted)
	{				
		if (File::Exists(cachefile.c_str()))
		{
			printf("Loading [tmd] from Cache...");
			ret = File::ReadBinary(cachefile.c_str(), outbuf, outlen);
			ExtractTmdVersion(cachefile, titleId, version, content, outbuf);
			if (ret > 0) 
			{
				printf("Done\n");
				goto checkbuf;
			}
			else printf("Failed (%d)\n", ret);
		}
		else gprintf("File (%s) not found.\n", cachefile.c_str());
	}
	//else gprintf("No Card Found or content != TMD or Version != 0\n");

	// If Version != 0 then we know what version of a file we want
	// so continue normal operations

	if (*version != 0 && deviceMounted && File::Exists(cachefile.c_str())) 
	{		
		printf("Loading [%s] from Cache... ", content);
		Spinner::Start();
		ret = File::ReadBinary(cachefile.c_str(), outbuf, outlen);		
		Spinner::Stop();
		printf("\b.Done\n");
		if (ret > 0) goto checkbuf;
		printf("Failed (%d)\n", ret);
	}
	//else gprintf("No Card Found or Version == 0\n");
    	
	gprintf("NusGetObject::NetworkInit...");
	Network::Startup();
    
	alternateUrl = GetAlternateUrl(titleId, *version, content);
	sprintf(defaultUrl, "%s/%08X%08X/%s", NusPath, titleId1, titleId2, content);	
	
	while (1)
	{				
		if (alternateUrl.size())
		{
			gcprintf("Downloading [%s] from ALT...", content);
			Spinner::Start();				
			ret = http->Request(alternateUrl.c_str(), (u32)(1 << 31));		
			Spinner::Stop();

			switch (http->Status)
			{
			case HttpStatus::OK: gcprintf("\b.Done\n"); break;
			case HttpStatus::NotFound: gcprintf("\b.Not Found\n"); break;
			case HttpStatus::ErrorConnect: 
				gcprintf("\b.Connect Error (%d)\n", http->ErrorCode); 
				break;
			case HttpStatus::ErrorReceive:
				gcprintf("\b.Receive Error (%d)\n", http->ErrorCode);
				break;
			default:
				gcprintf("\n>> ErrorCode (%d), StatusCode (%u)", http->ErrorCode, http->StatusCode);
				break;
			}
		}
		else http->Status = HttpStatus::NotFound;

		if (http->Status != HttpStatus::OK)
		{
			gcprintf("Downloading [%s] from NUS...", content);
			Spinner::Start();
			ret = http->Request(defaultUrl, (u32)(1 << 31));			
			Spinner::Stop();

			switch (http->Status)
			{
			case HttpStatus::OK: gcprintf("\b.Done\n"); break;
			case HttpStatus::NotFound:
				gcprintf("\b.Not Found\n");
				ret = -404;
				goto final;
				break;
			case HttpStatus::ErrorConnect:
				gcprintf("\b.Connect Error (%d)\n", http->ErrorCode); 
				retry--;
				if (retry < 0) goto final;
				break;
			case HttpStatus::ErrorReceive:
				gcprintf("\b.Receive Error (%d)\n", http->ErrorCode);
				retry--;
				if (retry < 0) goto final;
				break;
			default:
				gcprintf("\b.Error (%u)\n", (int)http->StatusCode);
				retry--;
				if (retry < 0) goto final;
				break;
			}
		}

		if (http->Status == HttpStatus::OK) break;
	}
	ret = http->GetResult(outbuf, outlen);

	if (ret < 0) goto final;

	// if version = 0 and file is TMD file then lets extract the version and return it
	// so that the rest of the files will save to the correct folder
	if (!strcmpi(content, "TMD") && *version == 0 && *outbuf != NULL && *outlen != 0)
	{
		ExtractTmdVersion(cachefile, titleId, version, content, outbuf);
	}

	if (*version != 0 && deviceMounted) 
	{			
		folder = GetCacheFolder(titleId, *version);
		
		if (!Directory::CreateTree(folder.c_str()))
		{
			gprintf("\n>> Could Not Create Folder Tree for (%s)\n", folder.c_str());
		}
		else
		{
			gprintf("\nWriting File (%s)\n", cachefile.c_str());
			fp = fopen(cachefile.c_str(), "wb");
			if (fp) 
			{
				fwrite(*outbuf, *outlen, 1, fp);
				fclose(fp);
			}
			else gprintf("\nCould not write file %s\n", cachefile.c_str());
		}
	}

checkbuf:
	if (((int)*outbuf & 0xF0000000) == 0xF0000000) ret = (int)*outbuf;

final:	
	memset(defaultUrl, 0, sizeof(defaultUrl));
	folder.clear();
	cachefile.clear();
	alternateUrl.clear();
	delete http; http = NULL;
	return ret;
}

string NUS::GetAlternateUrl(u64 titleId, u16 version, const char* content)
{
	if (!Settings::Instance().NUS.AlternateUrl.size()) return "";
	string result = Settings::Instance().NUS.AlternateUrl;
	if (result.rfind("/") != (result.size()-1)) result.push_back('/');
	ReplaceTags(result, titleId, version);
	result.append(content);
	return result;
}

string NUS::GetCacheFolder(u64 titleId, u16 version)
{
	if (!Settings::Instance().NUS.CacheFolder.size()) return "";
	string result = Settings::Instance().NUS.CacheFolder;
	if (result.rfind("/") != (result.size()-1)) result.push_back('/');
	ReplaceTags(result, titleId, version);
	return result;
}

string NUS::GetCacheFileName(u64 titleId, u16 version, const char* content)
{
	string result = GetCacheFolder(titleId, version);
	if (!result.size()) return "";
	result.append(content);
	return result;
}

void NUS::ReplaceTags(string &data, u64 titleId, u16 version)
{
	char tmp[8];

	// Replace TITLEID1
	sprintf(tmp, "%08X", TITLEID1(titleId));
	Tools::StringReplaceI(data, "{TITLEID1}", tmp);

	// Replace TITLEID2
	sprintf(tmp, "%08X", TITLEID2(titleId));
	Tools::StringReplaceI(data, "{TITLEID2}", tmp);

	// Replace Version
	sprintf(tmp, "%u", version);
	Tools::StringReplaceI(data, "{VERSION}", tmp);

	memset(tmp, 0, sizeof(tmp));
}