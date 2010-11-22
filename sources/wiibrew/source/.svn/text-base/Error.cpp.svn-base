#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include "Error.h"

char EsError::Message[115] = {};
char* EsError::ToString(int errorCode)
{	
	memset(Message, 0, sizeof(Message));
	sprintf(Message, "Success");
	if (errorCode > -1) return Message;

	switch (errorCode)
	{
		case -106: sprintf(Message, "(-106) Invalid TMD when using ES_OpenContent or access denied."); break;
		case -1009: sprintf(Message, "(-1009) Read failure (short read)."); break;
		case -1010: sprintf(Message, "(-1010) Write failure (short write)."); break;
		case -1012: sprintf(Message, "(-1012) Invalid signature type."); break;
		case -1015: sprintf(Message, "(-1015) Invalid value for byte at 0x180 in ticket (valid:0,1,2)"); break;
		case -1017: sprintf(Message, "(-1017) Wrong IN or OUT size, wrong size for a part of the vector, vector alignment problems, non-existant ioctl."); break;
		case -1020: sprintf(Message, "(-1020) ConsoleID mismatch"); break;
		case -1022: sprintf(Message, "(-1022) Content did not match hash in TMD."); break;
		case -1024: sprintf(Message, "(-1024) Memory allocation failure."); break;
		case -1026: sprintf(Message, "(-1026) Incorrect access rights."); break;
		case -1028: sprintf(Message, "(-1028) No ticket installed."); break;
		case -1029: sprintf(Message, "(-1029) Installed Ticket/TMD is invalid"); break;
		case -1035: sprintf(Message, "(-1035) Title with a higher version is already installed."); break;
		case -1036: sprintf(Message, "(-1036) Required sysversion(IOS) is not installed."); break;
		case -2008: sprintf(Message, "(-2008) Invalid parameter(s)."); break;		
		case -2011: sprintf(Message, "(-2011) Signature check failed."); break;
		case -2013: sprintf(Message, "(-2013) Keyring is full (contains 0x20 keys)."); break;
		case -2014: sprintf(Message, "(-2014) Bad has length (!=20)"); break;
		case -2016: sprintf(Message, "(-2016) Unaligned Data."); break;
		case -4100: sprintf(Message, "(-4100) Wrong Ticket-, Cert size or invalid Ticket-, Cert data"); break;
		default: sprintf(Message, "(%d) Unknown Error", errorCode);
	}
	
	return Message;	
}

char NandError::Message[50] = {};
char* NandError::ToString(int errorCode)
{
	memset(Message, 0, sizeof(Message));
	sprintf(Message, "Success");
	if (errorCode > -1) return Message;

	switch (errorCode)
	{
		case -1: sprintf(Message, "(-1) Access Denied."); break;
		case -2: sprintf(Message, "(-2) File Exists."); break;
		case -4: sprintf(Message, "(-4) Invalid Argument."); break;
		case -6: sprintf(Message, "(-6) File not found."); break;
		case -8: sprintf(Message, "(-8) Resource Busy."); break;
		case -12: sprintf(Message, "(-12) Returned on ECC error."); break;
		case -22: sprintf(Message, "(-22) Alloc failed during request."); break;
		case -102: sprintf(Message, "(-102) Permission denied."); break;
		case -103: sprintf(Message, "(-103) Returned for \"corrupted\" NAND."); break;
		case -105: sprintf(Message, "(-105) File exists."); break;
		case -106: sprintf(Message, "(-106) File not found."); break;
		case -107: sprintf(Message, "(-107) Too many fds open."); break;
		case -108: sprintf(Message, "(-108) Memory is full."); break;
		case -109: sprintf(Message, "(-190) Too many fds open."); break;
		case -110: sprintf(Message, "(-110) Path Name is too long."); break;
		case -111: sprintf(Message, "(-111) FD is already open."); break;
		case -114: sprintf(Message, "(-114) Returned on ECC error."); break;
		case -115: sprintf(Message, "(-115) Directory not empty."); break;
		case -116: sprintf(Message, "(-116) Max Directory Depth Exceeded."); break;
		case -118: sprintf(Message, "(-118) Resource busy."); break;
		case -119: sprintf(Message, "(-119) Fatal Error."); break;
		default: sprintf(Message, "(%d) Unknown Error", errorCode);
	}

	return Message;
}
