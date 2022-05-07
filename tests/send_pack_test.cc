#include <iostream>
#include "../src/rtp.h"
#include "../src/tcp_client.h"
#include <thread>

#pragma comment(lib,"ws2_32.lib")

//测试打包与发包

int main()
{
	char buff[128] = {};

	sockets::Init();
	
	rtp_parameter_t param;
	memset(&param, 0x0, sizeof(rtp_parameter_t));
	param.version = 2;
	param.pt = 97;
	param.ext = 1;

	rtp_session_t sess;
	sess.seq_number = 557;
	sess.timestamp = 1024;
	sess.ssrc = 243324332;

	for (int i = 0; i < sizeof(buff); ++i)
	{
		buff[i] = i % 127;
	}
	//new rtp_packet
	rtp_packet_t* rtp_packet = rtp_alloc(sizeof(buff));
	//fill rtp_packet
	rtp_pack(rtp_packet, &param, &sess, buff, sizeof(buff));

	bool flag = sockets::isIp2("123.60.90.46");
	//fill ext
	void* ext_body = malloc(4);
	memset(ext_body, 0x0, 4);
	rtp_add_ext_hdr(rtp_packet, 10, 1, ext_body);
	//send
	int sockfd = sockets::Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	for (int i = 0; i < 10; ++i)
	{
		dump(rtp_packet, "Send: ");
		int packet_len = rtp_len(rtp_packet);
		char* pbuf = (char*)malloc(packet_len);
		int ret = rtp_copy(rtp_packet, pbuf, packet_len);
		if (ret == 0)
		{
			sockets::SendUdp(sockfd, pbuf, packet_len, 0, "123.60.90.46", 9900);
		}
		sess.seq_number += 1;
		sess.timestamp += 1024;
		//repack
		rtp_pack(rtp_packet, &param, &sess, buff, sizeof(buff));
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	sockets::Destroy();

	return 0;
}