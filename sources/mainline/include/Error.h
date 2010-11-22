#ifndef _ERROR_H_
#define _ERROR_H_

class EsError
{
private:
	static char Message[115];
public:
	static char* ToString(int errorCode);
};

class NandError
{
private:
	static char Message[50];
public:
	static char* ToString(int errorCode);
};

#endif
