#ifndef _GECKO_H_
#define _GECKO_H_

#ifdef __cplusplus
extern "C"
{
#endif

int net_print_init(const char *rhost, unsigned short port);
int net_print_string( const char* file, int line, const char* format, ...);
int net_print_binary( int format, const void* binary, int len);
void net_print_shutdown();

#define DEFAULT_NET_PRINT_PORT	5194 
#define	DEFAULT_NET_PRINT_HOST  "14.internal.lukegb.com" 

	void gcprintf(const char *fmt, ...);
	void gprintf(const char *fmt, ...);
	void InitGecko();

#ifdef __cplusplus
}
#endif

#endif

