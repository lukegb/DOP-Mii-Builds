#ifndef _BOOT2_H_
#define _BOOT2_H_

#define MaxBoot2Version 4
class Boot2
{
private:
	static int Install(u16 version, bool isinternal);
	static signed_blob* FindCert(signed_blob* certs, u32 certsSize, const char *match);
public:
	static int Install(u16 version);
};

#endif