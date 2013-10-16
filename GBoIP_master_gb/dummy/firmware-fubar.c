// firmware.c -*-c-*-

#include "firmware.h"

// variablen initialisierung

volatile uint8_t i=0;
volatile uint8_t done=0;
volatile uint8_t clk_high=0;
volatile uint8_t rx = 1;
volatile uint8_t tx = 0;
volatile uint8_t ser_in = 0;
uint8_t ser_in_buffer = 0;
volatile uint8_t ser_out = 0;
volatile uint8_t ser_out_buffer = 0;
volatile uint8_t bit_counter = 0;

int main(void) {

// zuweisung udp package
char *message;
message = &ser_in_buffer;

//message = &ser__buffer

// timer initialisierung
TCCR0=(1<<CS00);
TIMSK|=(1<<TOIE0);
TCNT0=0;

// external interrupt konfiguration
//GICR = 1<<INT0;
//MCUCR = 1<<ISC01;

// enable interrupts
sei();

// Eingaenge / Ausgaenge konfigurieren
DDRC = 0xff;			//PORTC - status LED's
DDRD = 0b00010000;		//PD2 - external interrupt 0 captures the clock signal of the master gameboy, PD3 read data from local gameboy, PD4 write data to local gameboy
PORTD&=~(1<<PORTD4);		// set data line from GBoIP to gameboy low

// setup ethernet interface

	static uint8_t buffer[BufferSize + 1];
	int8_t   i;
	uint16_t received;

	// Initialise network interface.
	enc28j60Init(mymac);
	_delay_ms(100);
	// Magjack leds configuration, see enc28j60 datasheet, page 11
	// LEDB=yellow LEDA=green
	// 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
	// enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
	enc28j60PhyWrite(PHLCON, 0x476);
	_delay_ms(100);

	init_mac(mymac);
	client_ifconfig(myip, netmask);

	// Resolve MAC address from server IP.
	if (route_via_gw(serverip)) // Must be routed via gateway.
		get_mac_with_arp(gwip, TransNumGwmac, &arpresolverResultCallback);
	else                        // Server is on local network.
		get_mac_with_arp(serverip, TransNumGwmac, &arpresolverResultCallback);

	while (get_mac_with_arp_wait()) {
		received = enc28j60PacketReceive(BufferSize, buffer);
		// Call packetloop to process ARP reply.
		packetloop_arp_icmp_tcp(buffer, received);
	}


	while (1) { 		// Main loop start.
	PORTC=0xff;


		received = enc28j60PacketReceive(BufferSize, buffer);

		// Do something while no packet in queue.
		if (received == 0) {
			if (tx == 1){								// check if a package is ready to be transmitted
				send_udp(buffer, message, 1, 37351, serverip, 33750, gwmac);	// transfer UDP package to the save
				tx = 0;								// package has been transferred. ready for new data
			}
			continue;
		}

		// Answer to ARP requests.
		if (eth_type_is_arp_and_my_ip(buffer, received)) {
			make_arp_answer_from_request(buffer, received);
			continue;
		}

		// Check if IP packets (ICMP or UDP) are for us.
		if (eth_type_is_ip_and_my_ip(buffer, received) == 0)
			continue;

		// Answer ping with pong.
		if (
			buffer[IP_PROTO_P]  == IP_PROTO_ICMP_V &&
			buffer[ICMP_TYPE_P] == ICMP_TYPE_ECHOREQUEST_V) {

			make_echo_reply_from_request(buffer, received);
			continue;
		}



		// Listen for UDP packets on port 57351 (0xe007) and process received data.
		if (
			buffer[IP_PROTO_P] == IP_PROTO_UDP_V &&
			buffer[UDP_DST_PORT_H_P] == 0xe0 &&
			buffer[UDP_DST_PORT_L_P] == 0x07) {
	
			//if (rx == 1){
				ser_out_buffer = buffer[UDP_DATA_P];
			//	rx = 0;
			//}

			}		





// experimental data manipulation in main loop cause ISR is pissing me off



if(PIND& _BV(PD2)){			
clk_high = 1;
done = 0;
} else {
clk_high = 0;
}


if (clk_high == 0 && done == 0){

if(bit_counter == 0){
	ser_out = ser_out_buffer;
	ser_in = 0;
}

if(ser_out& _BV(7)){			// write byte to local gameboy
	PORTD|=(1<<PORTD4);		// 1
} else {
}	PORTD&=~(1<<PORTD4);		// 0

if(PIND& _BV(PD3)){				// read byte from local gameboy
	ser_in |= _BV(0);
}

if (bit_counter <= 6){
	ser_out = ser_out<<1;
	ser_in = ser_in<<1;
}	

bit_counter++;

if (bit_counter >= 8){
	bit_counter = 0;
	ser_in_buffer = ser_in;
	rx = 1;
	tx = 1;
}

done = 1;
}








	}				 // Main loop end.


	return (0);
}

void arpresolverResultCallback(uint8_t *ip, uint8_t refnum, uint8_t *mac) {
	if (refnum == TransNumGwmac)
		memcpy(gwmac, mac, 6);
}

//external interrupt INT0 (falling edge clk signal detection)

/*
ISR(INT0_vect)
{								//ISR INT0 start
y++;
y++;
y++;
y++;
PORTC=y;


if(y == 0){				// check if this is the first bit of a byte
	//ser_out = ser_out_buffer;		 
	//ser_in = 0;
}
	if(ser_out& _BV(7)){			// write byte to local gameboy
		PORTD|=(1<<PORTD4);		// 1
	} else {
		PORTD&=~(1<<PORTD4);		// 0
	}

	if(PIND& _BV(PD3)){				// read byte from local gameboy
		ser_in |= _BV(0);
	}

	if (y <= 7){
		ser_out = ser_out<<1;
		ser_in = ser_in<<1;
	}	
y++;
	
if (y >= 8){
     	PORTC = 0xff; 						// set status LED's to the value received from the local gameboy                       
	y = 0;
	//ser_in_buffer = ser_in;
	//tx = 1;					// ready to transmit byte to slave
	//rx = 1;					// byte transferred to local gameboy, ready to receive new byte.

}

}							//ISR INT0 ende
*/
