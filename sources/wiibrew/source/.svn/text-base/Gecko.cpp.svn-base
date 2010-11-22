#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "Gecko.h"
#include "BuildType.h"

extern  "C"
{
	void gcprintf(const char *fmt, ...)
	{
		char astr[4096];
		va_list ap;
		va_start(ap,fmt);
		vsprintf(astr, fmt, ap);
		va_end(ap);

		gprintf(astr);	
		printf(astr);	
		memset(astr, 0, sizeof(astr));
	}

#ifdef NETDEBUG
#include <sys/types.h>
#include <network.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <debug.h>

	/* It opens a connection to the host name rhost that is listening
	  on the specified port. It returns the socket descriptor, or -1
	  in case of failure.
	*/
	static int clientsocket(const char *rhost, unsigned short port)
	{
	  struct hostent *ptrh;  /* pointer to a host table entry     */
	  struct sockaddr_in sad;/* structure to hold server's address*/
	  int    fd;             /* socket descriptor                 */
	  struct in_addr *myaddr;/* IP address holder                 */

	  memset((char *)&sad, 0, sizeof(sad)); /* clear sockaddr structure */
	  memset((char *)&myaddr, 0, sizeof(myaddr)); /* and IP */
	  sad.sin_family = AF_INET;  /* set family to Internet */
	  sad.sin_port = htons((u_short)port); 
	  /* Convert host name to equivalent IP address and copy sad */
	  if (!inet_aton(rhost, myaddr)) {
	    ptrh = net_gethostbyname(rhost);
	    if (((char *)ptrh) == NULL) {
	      fprintf(stderr, "invalid host: %s\n", rhost);
	      return (-1);
	    }
	    memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);
	  } else {
	    memcpy(&sad.sin_addr, rhost, strlen(rhost));
	  }
	  
	  /* Create a socket */
	  fd = net_socket(PF_INET, SOCK_STREAM, 0);
	  if (fd < 0) {
	    fprintf(stderr, "socket creation failed\n");
	    return (-1);;
	  }
	  
	  /* Connect the socket to the specified server */
	  if (net_connect(fd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
	    fprintf(stderr, "connect failed\n");
	    return (-1);
	  }

	  return fd;
	}

	static int _net_print_socket = -1;

	int net_print_init(const char *rhost, unsigned short port)
	{
		int	sk = -1;

		if ( _net_print_socket < 0 ) {
			if (rhost==NULL){
				rhost = DEFAULT_NET_PRINT_HOST;
			}
			if (port <= 0 ){
				port = DEFAULT_NET_PRINT_PORT;
			}

			sk = clientsocket( rhost, port);
			if ( sk >= 0 ) {
				_net_print_socket = sk;
				net_print_string( __FILE__, __LINE__, "net_print_init() successful, socket=%d\n", _net_print_socket);
			}
		}


		return _net_print_socket;
	}

	int net_print_string( const char* file, int line, const char* format, ...)
	{
		va_list	ap;
		int len;
		int ret;
		char buffer[512];

		va_start(ap, format);

		if ( _net_print_socket < 0 ) {
			return	-1;
		}

		len = 0;
		if ( file != NULL) {
			len = sprintf( buffer, "%s:%d, ", file, line);
		}

		len += vsprintf( buffer+len, format, ap);
		va_end(ap);

		ret = net_send( _net_print_socket, buffer, len, 0);
		return ret;
	}


	
	//redo!
	void gprintf(const char *fmt, ...)
	{
		char astr[4096];
		va_list ap;
		va_start(ap,fmt);
		vsprintf( astr, fmt, ap );
		va_end(ap);
		net_print_string( NULL, 0, astr);
		memset(astr, 0, sizeof(astr));
	} 
	
	void InitGecko()
	{
		printf("Beginning debugging to %s @ %i\n", DEFAULT_NET_PRINT_HOST, DEFAULT_NET_PRINT_PORT);
		net_print_init(DEFAULT_NET_PRINT_HOST, DEFAULT_NET_PRINT_PORT);
		DEBUG_Init(100,5656);
	}
	
	void net_print_shutdown() {
		gprintf("Shutting down network debugging...\n");
		net_close(_net_print_socket);
		_net_print_socket = -1;
	}
#else /* NETDEBUG */
int net_print_init(const char *rhost, unsigned short port) { return 0; }
int net_print_string( const char* file, int line, const char* format, ...) { return 0; }
int net_print_binary( int format, const void* binary, int len) { return 0; }
void net_print_shutdown() {}
#ifdef DEBUG
	bool geckoinit = false;

	//using the gprintf from crediar because it is smaller than mine
	void gprintf(const char *fmt, ...)
	{
		if (!(geckoinit)) return;
		char astr[4096];
		va_list ap;
		va_start(ap,fmt);
		vsprintf( astr, fmt, ap );
		va_end(ap);
		usb_sendbuffer_safe( 1, astr, strlen(astr) );
		memset(astr, 0, sizeof(astr));
	} 

	void InitGecko()
	{
		if (usb_isgeckoalive(EXI_CHANNEL_1))
		{
			usb_flush(EXI_CHANNEL_1);
			geckoinit = true;
		}
	}
#else
	void gprintf(const char *fmt, ...) {}
	void InitGecko() {}
#endif
#endif /* NETDEBUG */

}
