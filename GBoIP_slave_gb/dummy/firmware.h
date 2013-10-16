// firmware.h -*-c-*-


#ifndef FIRMWARE_h
#define FIRMWARE_h


#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "net/enc28j60.h"
#include "net/ip_arp_udp_tcp.h"
#include "net/net.h"
#include "config.h"


#define TransNumGwmac 1


void arpresolverResultCallback(uint8_t *ip, uint8_t refnum, uint8_t *mac);


#endif
