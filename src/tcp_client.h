#ifndef CROSS_TCP_CLIENT_H
#define CROSS_TCP_CLIENT_H

#include <stdint.h>
#include <string>
#include <vector>
#if defined(_WIN32)
#include<ws2tcpip.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

//#pragma comment(lib,"ws2_32.lib");

namespace sockets
{
	void Init();
	void Destroy();
	//void perr_exit(const char *s);
	void err_exit(const char *s);
	int Connect(int fd, const struct sockaddr *sa, socklen_t salen);
	int Socket(int family, int type, int protocol);
	int Read(int fd, void *ptr, size_t nbytes);
	int Write(int fd, const void *ptr, size_t nbytes);
	void Close(int fd);

	void ParseIp(sockaddr_in*, const char* ip_str, int port);
	int SendUdp(int sockfd, const char* buf, int len, int flags, const sockaddr_in* addr);
	int SendUdp(int sockfd, const char* buf, int len, int flags, const char* ip_str, int port);

	void split(const std::string& text, const std::string& splitter, std::vector<std::string>& vecs);

	void getIpByDoname(const std::string&, std::vector<std::string>&);
	bool isIp(const std::string& str);
	bool isIp2(const std::string& str);
	bool isNum(const std::string& str);

	uint64_t hostToNetwork64(uint64_t host64);

	///上层API
	int TcpSocket();
	int UdpSocket();
	int ConnectTcp(const char* ip_str, int port, int timeout = 0);

}
#endif