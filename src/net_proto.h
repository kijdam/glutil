/*
 * net_proto.h
 *
 *  Created on: Mar 9, 2014
 *      Author: reboot
 */
#ifdef _G_SSYS_NET

#ifndef NET_PROTO_H_
#define NET_PROTO_H_

#define PROT_CODE_BASELINE_PROTO_VERSION	0x76

#define BASELINE_PROTOCOL_VERSION_MAJOR		0
#define BASELINE_PROTOCOL_VERSION_MINOR		1

#include <glutil.h>

typedef struct __net_proto_24_bit_network_address
{
  uint8_t b_1;
  uint8_t b_2;
  uint8_t b_3;
} _np_netaddr24;

#pragma pack(push, 4)

typedef struct ___baseline_proto_header
{
  uint8_t prot_code;
  uint8_t ttl;
  _np_netaddr24 dt_address_src;
  _np_netaddr24 dt_address_dst;
  uint32_t content_length;
} _bp_header, *__bp_header;

#pragma pack(pop)

#define BP_HEADER_SIZE          sizeof(struct ___baseline_proto_header)

#include <memory_t.h>

mda pc_a;

#include <net_io.h>

p_s_cb net_baseline_prochdr;

void
net_proto_reset_to_baseline (__sock_o pso);
void
net_proto_na24_copy (_np_netaddr24 *src, _np_netaddr24 *dst);

#endif /* NET_PROTO_H_ */

#endif