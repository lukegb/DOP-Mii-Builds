#ifndef _HTTP_H_
#define _HTTP_H_

#define TCP_CONNECT_TIMEOUT 5000
#define TCP_BLOCK_SIZE (16 * 1024)
#define TCP_BLOCK_RECV_TIMEOUT 4000
#define TCP_BLOCK_SEND_TIMEOUT 4000
#define HTTP_TIMEOUT 300000

enum class HttpStatus
{
	OK,
	ErrorConnect,
	BadRequest,
	ErrorStatus,
	ErrorTooBig,
	ErrorReceive,
	NotFound
};

class HTTP
{
private:
	u16 Port;
	u32 MaxSize;
	u8 *Data;
	u32 ContentLength;
	char *Host;
	char *Path;

	int  TcpConnect();
	int  TcpSocket();		
	bool TcpRead(const s32 s, u8 **buffer, const u32 length);
	char* TcpReadLine(const s32 s, const u16 maxLength, const u64 startTime, const u16 timeout);
	bool TcpWrite(const s32 s, const u8 *buffer, const u32 length);
public:
	HttpStatus Status;
	u32 StatusCode;
	int ErrorCode;
public:
	HTTP();
	~HTTP();
	bool Request(const char *url, const u32 maxSize);
	bool GetResult(u8 **content, u32 *length);
	static bool SplitURL(char **host, char **path, const char *url);
};

#endif

