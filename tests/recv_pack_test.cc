#include <iostream>
#include "../src/rtp.h"
#include "../src/tcp_client.h"
#include <thread>

#pragma comment(lib,"ws2_32.lib")

//测试收包与解包

int main()
{
	char buff[1500] = {};
	sockets::Init();
	
	int sockfd = sockets::Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6000);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	//绑定套接字到一个本地地址
	if (bind(sockfd, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		std::cout << "Failed bind()" << std::endl;
		return 0;
	}

	sockaddr_in clientAddr;
	int nLen = sizeof(addr);
	for(;;)
	{
		int nRecv = recvfrom(sockfd, buff, sizeof(buff), 0, (sockaddr*)&clientAddr, &nLen);
		std::cout << "nrecv:" << nRecv << std::endl;
		if (nRecv > 0)
		{
			/*for (int i = 0; i < nRecv; i++) {
				printf("%2x\n", buff[i]);
			}*/
			//buff[nRecv] = '\0';
			//std::cout << "\n------------\nrecv::" << inet_ntoa(clientAddr.sin_addr) << ": " << buff << std::endl;
			rtp_packet_t* un_rtp = rtp_unpack(buff, nRecv);
			dump(un_rtp, "receive packet: ");
		}
	}

	sockets::Destroy();

	return 0;
}