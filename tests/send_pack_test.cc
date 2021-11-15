#include <iostream>
#include "../src/rtp.h"
#include "../src/tcp_client.h"
#include <thread>

#pragma comment(lib,"ws2_32.lib")

//测试打包与发包

int main()
{
	char buff[1024] = {};

	sockets::Init();
	
	rtp_parameter_t param;
	memset(&param, 0x0, sizeof(rtp_parameter_t));
	param.version = 2;
	param.pt = 97;

	rtp_session_t sess;
	sess.seq_number = 557;
	sess.timestamp = 1024;
	sess.ssrc = 243324332;

	for (int i = 0; i < sizeof(buff); ++i)
	{
		buff[i] = i % 127;
	}
	//new rtp_packet
	rtp_packet_t* rtp_packet = alloc_rtp(sizeof(buff));
	//fill rtp_packet
	pack_rtp(rtp_packet, &param, &sess, buff, sizeof(buff));
	//send
	int sockfd = sockets::Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	for (int i = 0; i < 10; ++i)
	{
		dump(rtp_packet, "Send: ");
		sockets::SendUdp(sockfd, (const char*)RTP_BYTE(rtp_packet), RTP_LEN(rtp_packet), 0, "81.71.41.235", 9900);
		sess.seq_number += 1;
		sess.timestamp += 1024;
		//repack
		pack_rtp(rtp_packet, &param, &sess, buff, sizeof(buff));
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	sockets::Destroy();

	return 0;
}