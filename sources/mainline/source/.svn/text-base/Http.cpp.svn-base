#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <network.h>
#include <ogc/lwp_watchdog.h>
#include <sys/errno.h>
#include <fcntl.h>

#include "Http.h"
#include "Tools.h"
#include "Gecko.h"

HTTP::HTTP()
{
	Host = NULL;
	Path = NULL;
	Data = NULL;
}

HTTP::~HTTP()
{
	delete Host; Host = NULL;
	delete Path; Path = NULL;
}

int HTTP::TcpSocket() 
{
	s32 s, res;

	s = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (s < 0) 
	{
		printf ("net_socket failed: %d\n", s);
		return s;
	}

	res = net_fcntl (s, F_GETFL, 0);
	if (res < 0) 
	{
		printf ("F_GETFL failed: %d\n", res);
		net_close (s);
		return res;
	}

	res = net_fcntl (s, F_SETFL, res | 4);
	if (res < 0) 
	{
		printf ("F_SETFL failed: %d\n", res);
		net_close (s);
		return res;
	}

	return s;
}

int HTTP::TcpConnect() 
{
	struct hostent *hp;
	struct sockaddr_in sa;
	s32 s, res;
	s64 t;
	
	hp = net_gethostbyname(this->Host);
	if (!hp || !(hp->h_addrtype == PF_INET)) 
	{
		gprintf("net_gethostbyname failed: %d\n", errno);
		return errno;
	}

	s = TcpSocket();
	if (s < 0) return s;

	memset (&sa, 0, sizeof (struct sockaddr_in));
	sa.sin_family = PF_INET;
	sa.sin_len = sizeof (struct sockaddr_in);
	sa.sin_port = htons (this->Port);
	memcpy ((char *) &sa.sin_addr, hp->h_addr_list[0], hp->h_length);

	t = gettime ();
	while (1) 
	{
		if (ticks_to_millisecs (diff_ticks (t, gettime ())) > TCP_CONNECT_TIMEOUT) 
		{
			gprintf ("TcpConnect timeout\n");
			net_close (s);
			return -ETIMEDOUT;
		}

		res = net_connect (s, (struct sockaddr*) &sa, sizeof(struct sockaddr_in));

		if (res < 0) 
		{
			if (res == -EISCONN) break;

			if (res == -EINPROGRESS || res == -EALREADY) 
			{
				usleep (20000);
				continue;
			}

			printf ("net_connect failed: %d\n", res);
			net_close (s);
			return res;
		}

		break;
	}

	return s;
}

char* HTTP::TcpReadLine(const s32 s, const u16 maxLength, const u64 startTime, const u16 timeout)
{
	char *buf;
	u16 c;
	s32 res;
	char *ret;

	buf = (char*)Tools::AllocateMemory(maxLength);

	c = 0;
	ret = NULL;
	while (1) 
	{
		if (ticks_to_millisecs (diff_ticks (startTime, gettime ())) > timeout) break;

		res = net_read (s, &buf[c], 1);

		if ((res == 0) || (res == -EAGAIN)) 
		{
			usleep (20 * 1000);
			continue;
		}

		if (res < 0) 
		{
			printf ("TcpReadLine failed: %d\n", res);
			break;
		}

		if ((c > 0) && (buf[c - 1] == '\r') && (buf[c] == '\n')) 
		{
			if (c == 1) 
			{
				ret = strdup ("");
				break;
			}

			ret = strndup (buf, c - 1);
			break;
		}

		c++;
		if (c == maxLength) break;
	}

	delete buf; buf = NULL;
	return ret;
}

bool HTTP::TcpRead(const s32 s, u8 **buffer, const u32 length)
{
	u8 *p;
	u32 step, left, block, received;
	s64 t;
	s32 res;

	step = 0;
	p = *buffer;
	left = length;
	received = 0;

	t = gettime();
	while (left) 
	{
		if (ticks_to_millisecs (diff_ticks (t, gettime ())) > TCP_BLOCK_RECV_TIMEOUT) 
		{
			gprintf("TcpRead timeout\n");
			ErrorCode = -ETIMEDOUT;
			break;
		}

		block = left;
		if (block > 2048) block = 2048;

		res = net_read (s, p, block);

		if ((res == 0) || (res == -EAGAIN)) 
		{
			usleep (20000);
			continue;
		}

		if (res < 0) 
		{
			printf ("net_read failed: %d\n", res);
			break;
		}

		received += res;
		left -= res;
		p += res;

		if ((received / TCP_BLOCK_SIZE) > step) 
		{
			t = gettime ();
			step++;
		}
	}

	return left == 0;
}

bool HTTP::TcpWrite(const s32 s, const u8 *buffer, const u32 length) 
{
	const u8 *p;
	u32 step, left, block, sent;
	s64 t;
	s32 res;

	step = 0;
	p = buffer;
	left = length;
	sent = 0;

	t = gettime ();
	while (left) 
	{
		if (ticks_to_millisecs (diff_ticks (t, gettime ())) > TCP_BLOCK_SEND_TIMEOUT) 
		{
			printf ("tcp_write timeout\n");
			break;
		}

		block = left;
		if (block > 2048) block = 2048;

		res = net_write (s, p, block);

		if ((res == 0) || (res == -56)) 
		{
			usleep (20000);
			continue;
		}

		if (res < 0) 
		{
			printf ("net_write failed: %d\n", res);
			break;
		}

		sent += res;
		left -= res;
		p += res;

		if ((sent / TCP_BLOCK_SIZE) > step) {
			t = gettime ();
			step++;
		}
	}

	return left == 0;
}

bool HTTP::SplitURL(char **host, char **path, const char *url) 
{
	const char *p;
	char *c;
	if (strncasecmp (url, "http://", 7)) return false;
	p = url + 7;
	c = strchr (p, '/');
	if (c[0] == 0) return false;
	*host = strndup (p, c - p);
	*path = strdup (c);
	return true;
}

bool HTTP::Request(const char *url, const u32 maxSize) 
{
	bool ret = false;
	int linecount;
	int socket = 0;
	char *request = NULL;
	char *r = NULL;

	Status = HttpStatus::NotFound;
	StatusCode = 0;
	ErrorCode = 0;

	if (Host) { delete Host; Host = NULL; }
	if (Path) { delete Path; Path = NULL; }
	if (Data) { delete Data; Data = NULL; }

	if (!SplitURL(&Host, &Path, url)) goto final;

	Port = 80;
	MaxSize = maxSize;
	StatusCode = 404;
	
	ContentLength = 0;

	socket = TcpConnect();
	//gprintf("tcp_connect(%s, %hu) = %d\n", Host, Port, s);
	if (socket < 0) 
	{
		Status = HttpStatus::ErrorConnect;
		ErrorCode = socket;
		goto final;
	}

	request = (char*)Tools::AllocateMemory(1024);
	r = request;
	r += sprintf(r, "GET %s HTTP/1.1\r\n", Path);
	r += sprintf(r, "Host: %s\r\n", Host);
	r += sprintf(r, "Cache-Control: no-cache\r\n\r\n");

	//gprintf("request = %s\n", request);

	if (!TcpWrite(socket, (u8*)request, strlen(request))) { ret = false; goto final; }

	linecount = 0;

	for (linecount=0; linecount < 32; linecount++) 
	{
		char *line = TcpReadLine(socket, 0xff, gettime(), (u16)HTTP_TIMEOUT);
		//gprintf("TcpReadLine returned %p (%s)\n", line, line?line:"(null)");
		if (!line) 
		{
			StatusCode = 400;
			Status = HttpStatus::BadRequest;
			break;
		}

		if (strlen (line) < 1) break;

		sscanf (line, "HTTP/1.%*u %u", &StatusCode);
		sscanf (line, "Content-Length: %u", &ContentLength);
		
		delete line; line = NULL;
	}
	//gprintf("content_length = %d, status = %d, linecount = %d\n", content_length, http_status, linecount);
	if (linecount == 32 || !ContentLength) StatusCode = 404;

	if (StatusCode == 404)
	{
		Status = HttpStatus::NotFound;
		goto final;
	}
	if (StatusCode != 200) 
	{
		Status = HttpStatus::ErrorStatus;
		goto final;
	}
	if (ContentLength > MaxSize) 
	{
		Status = HttpStatus::ErrorTooBig;
		goto final;
	}

	Data = (u8*)Tools::AllocateMemory(ContentLength);
	if (!TcpRead(socket, &Data, ContentLength))
	{
		delete Data; Data = NULL;
		Status = HttpStatus::ErrorReceive;
		goto final;
	}

	Status = HttpStatus::OK;
	ret = true;

final:
	delete request; request = NULL;
	delete Host; Host = NULL;
	delete Path; Path = NULL;
	net_close (socket);
	return ret;
}

bool HTTP::GetResult(u8 **content, u32 *length) 
{
	if (Status == HttpStatus::OK) 
	{
		*content = Data;
		*length = ContentLength;
	} 
	else 
	{
		*content = NULL;
		*length = 0;
	}	
	return true;
}

