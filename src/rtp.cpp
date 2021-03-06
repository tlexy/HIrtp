#include "rtp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tcp_client.h"


rtp_packet_t* rtp_alloc(int payload_size)
{
	uint8_t* mem = (uint8_t*)malloc(payload_size + sizeof(rtp_packet_t));
	if (!mem)
	{
		return NULL;
	}
	memset(mem, 0x0, payload_size + sizeof(rtp_packet_t));
	rtp_packet_t* rtp = (rtp_packet_t*)mem;
	rtp->payload_len = payload_size;
	return rtp;
}

void rtp_add_ext_hdr(rtp_packet_t* rtp, uint16_t profile, uint16_t len, void* ext_body)
{
	rtp->hdr.extbit = 1;
	rtp->hdr_ext.profile_specific = profile;
	rtp->hdr_ext.length = len;
	rtp->ext_body = ext_body;
	rtp->ext_len = len;
	rtp->hdr_ext.profile_specific = htons(rtp->hdr_ext.profile_specific);
	rtp->hdr_ext.length = htons(rtp->hdr_ext.length);
}

void* rtp_alloc_ext_hdr(rtp_packet_t* rtp, uint16_t profile, uint16_t len)
{
	rtp->hdr.extbit = 1;
	rtp->hdr_ext.profile_specific = profile;
	rtp->hdr_ext.length = len;
	rtp->ext_body = malloc(4*len);
	rtp->ext_len = len;
	rtp->hdr_ext.profile_specific = htons(rtp->hdr_ext.profile_specific);
	rtp->hdr_ext.length = htons(rtp->hdr_ext.length);
	memset(rtp->ext_body, 0x0, 4 * len);
	return rtp->ext_body;
}

void rtp_pack(rtp_packet_t* rtp, rtp_parameter_t* param, rtp_session_t* session, const void* payload, int payload_len)
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
	rtp->payload_len = payload_len;

	rtp->hdr.seq_number = htons(rtp->hdr.seq_number);
	rtp->hdr.timestamp = htonl(rtp->hdr.timestamp);
	rtp->hdr.ssrc = htonl(rtp->hdr.ssrc);
	//assert(payload_len <= rtp->ptr_len)
	memcpy(&rtp->arr, payload, payload_len);

}

int rtp_len(rtp_packet_t* rtp)
{
	int packet_len = rtp->payload_len + sizeof(rtp_hdr_t);
	if (rtp->hdr.extbit == 1)
	{
		packet_len = packet_len + sizeof(rtp_hdr_ext_t) + (rtp->ext_len * 4);
	}
	return packet_len;
}

int rtp_copy(rtp_packet_t* rtp, void* dest, int dest_len)
{
	int packet_len = rtp->payload_len + sizeof(rtp_hdr_t);
	if (rtp->hdr.extbit == 1)
	{
		packet_len = packet_len + sizeof(rtp_hdr_ext_t) + (rtp->ext_len * 4);
	}
	if (packet_len > dest_len)
	{
		return 1;
	}

	unsigned pos = 0;
	char* p = (char*)dest;
	memcpy(p + pos, &rtp->hdr, sizeof(rtp_hdr_t));
	pos += sizeof(rtp_hdr_t);
	if (rtp->hdr.extbit == 1)
	{
		memcpy(p + pos, &rtp->hdr_ext, sizeof(rtp_hdr_ext_t));
		pos += sizeof(rtp_hdr_ext_t);

		memcpy(p + pos, rtp->ext_body, rtp->ext_len * 4);
		pos = pos + rtp->ext_len * 4;
	}
	memcpy(p + pos, &rtp->arr, rtp->payload_len);
	return 0;
}

int rtp_unpack_test(void* src, int len)
{
	if (len > sizeof(rtp_hdr_t))
	{
		return len;
	}
	return 0;
}

rtp_packet_t* rtp_unpack(void* src, int len)
{
	if (len <= sizeof(rtp_hdr_t))
	{
		return NULL;
	}
	rtp_packet_t* tmp = (rtp_packet_t*)src;
	//头部长度，包含hdr及hdr_ext及hdr_body length
	int header_len = sizeof(rtp_hdr_t);
	if (tmp->hdr.extbit == 1)
	{
		header_len += sizeof(rtp_hdr_ext_t);
		tmp->hdr_ext.length = ntohs(tmp->hdr_ext.length);
		tmp->hdr_ext.profile_specific = ntohs(tmp->hdr_ext.profile_specific);
		header_len = header_len + tmp->hdr_ext.length*4;
	}
	int payload_len = len - header_len;
	if (payload_len < 1)
	{
		return NULL;
	}

	char* p = (char*)src;
	rtp_packet_t* rtp = (rtp_packet_t*)malloc(payload_len + sizeof(rtp_packet_t));
	if (!rtp)
	{
		return NULL;
	}
	memset(rtp, 0x0, payload_len + sizeof(rtp_packet_t));
	rtp->payload_len = payload_len;
	memcpy(&rtp->hdr, src, sizeof(rtp_hdr_t));
	int pos = sizeof(rtp_hdr_t);
	if (rtp->hdr.extbit == 1)
	{
		memcpy(&rtp->hdr, src, sizeof(rtp_hdr_ext_t) + sizeof(rtp_hdr_t));
		pos = sizeof(rtp_hdr_ext_t) + sizeof(rtp_hdr_t);
	}

	if (rtp->hdr_ext.length > 0)
	{
		rtp->ext_body = p + pos;
		rtp->ext_len = rtp->hdr_ext.length;
		pos += (rtp->ext_len * 4);
	}
	memcpy(rtp->arr, p + pos, payload_len);

	rtp->hdr.seq_number = ntohs(rtp->hdr.seq_number);
	rtp->hdr.timestamp = ntohl(rtp->hdr.timestamp);
	rtp->hdr.ssrc = ntohl(rtp->hdr.ssrc);
	return rtp;
}

void dump(rtp_packet_t* rtp, const char* text)
{
	if (!rtp)
	{
		return;
	}
	if (rtp->hdr.extbit == 0)
	{
		printf("%s seq: %u, ssrc: %u, ts: %u, pt: %u, payload_len: %d\n", text,
			rtp->hdr.seq_number,
			rtp->hdr.ssrc,
			rtp->hdr.timestamp,
			rtp->hdr.paytype,
			rtp->payload_len);
	}
	else
	{
		printf("%s seq: %u, ssrc: %u, ts: %u, pt: %u, payload_len: %d, ext_hdr_len: %d\n", text,
			rtp->hdr.seq_number,
			rtp->hdr.ssrc,
			rtp->hdr.timestamp,
			rtp->hdr.paytype,
			rtp->payload_len,
			rtp->hdr_ext.length);
	}
}