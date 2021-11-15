#include <iostream>
#include "../src/rtp.h"
#include "../src/tcp_client.h"

#pragma comment(lib,"ws2_32.lib")

//测试打包与发包

static char buff[1024] = {};

void unpacket_rtp_packet(void*, int len)
{

}

int main1()
{
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
	//
	//dump(rtp_packet, "original packet: ");

	rtp_packet_t* un_rtp = rtp_unpack(RTP_BYTE(rtp_packet), RTP_LEN(rtp_packet));//sizeof(buff) + sizeof(rtp_header_t)
	dump(un_rtp, "receive packet: ");

	sockets::Destroy();

	return 0;
}