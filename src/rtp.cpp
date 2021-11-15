#include "rtp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tcp_client.h"


rtp_packet_t* alloc_rtp(int payload_size)
{
	uint8_t* mem = (uint8_t*)malloc(payload_size + sizeof(rtp_packet_t));
	if (!mem)
	{
		return NULL;
	}
	rtp_packet_t* rtp = (rtp_packet_t*)mem;
	rtp->ptr_len = payload_size;
	rtp->ptr = mem + sizeof(rtp_packet_t);
	return rtp;
}

void pack_rtp(rtp_packet_t* rtp, rtp_parameter_t* param, rtp_session_t* session, const void* payload, int payload_len)
{
	rtp->hdr.cc = param->cc;
	rtp->hdr.extbit = param->ext;
	rtp->hdr.padbit = param->padding;
	rtp->hdr.version = param->version;
	rtp->hdr.paytype = param->pt;
	rtp->hdr.markbit = param->marker;
	rtp->hdr.seq_number = session->seq_number;
	rtp->hdr.timestamp = session->timestamp;
	rtp->hdr.ssrc = session->ssrc;

	rtp->hdr.seq_number = htons(rtp->hdr.seq_number);
	rtp->hdr.timestamp = htonl(rtp->hdr.timestamp);
	rtp->hdr.ssrc = htonl(rtp->hdr.ssrc);
	//assert(payload_len <= rtp->ptr_len)
	memcpy(rtp->ptr, payload, payload_len);
	rtp->ptr_len = payload_len;

}

int copy_rtp(rtp_packet_t* rtp, void* dest, int dest_len)
{
	if (rtp->ptr_len + sizeof(rtp_header_t) > dest_len)
	{
		return 1;
	}
	memcpy(dest, &rtp->hdr, rtp->ptr_len + sizeof(rtp_header_t));
	return 0;
}

int rtp_unpack_test(void* src, int len)
{
	if (len > sizeof(rtp_header_t))
	{
		return len;
	}
	return 0;
}

rtp_packet_t* rtp_unpack(void* src, int len)
{
	if (len <= sizeof(rtp_header_t))
	{
		return NULL;
	}
	rtp_packet_t* rtp = (rtp_packet_t*)malloc(len + RTP_DATA_OFF);
	if (!rtp)
	{
		return NULL;
	}
	memcpy(&rtp->hdr, src, len);
	rtp->ptr = &rtp->hdr;
	rtp->ptr_len = len - sizeof(rtp_header_t);
	rtp->hdr.seq_number = ntohs(rtp->hdr.seq_number);
	rtp->hdr.timestamp = ntohl(rtp->hdr.timestamp);
	rtp->hdr.ssrc = ntohl(rtp->hdr.ssrc);
	return rtp;
}

void dump(rtp_packet_t* rtp, const char* text)
{
	printf("%s seq: %u, ssrc: %u, ts: %u, pt: %u, payload_len: %d\n", text,
		rtp->hdr.seq_number,
		rtp->hdr.ssrc,
		rtp->hdr.timestamp,
		rtp->hdr.paytype,
		rtp->ptr_len);
}