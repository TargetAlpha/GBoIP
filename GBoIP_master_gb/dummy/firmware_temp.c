// firmware.c -*-c-*-


#include "firmware.h"

// variablen initialisierung

volatile uint8_t send=0;
volatile uint8_t i=7;
volatile uint8_t rx = 0;
volatile uint8_t tx = 0;
volatile uint8_t ser_in = 0;
volatile uint8_t ser_out = 0;
volatile uint8_t ser_in_buffer = 0;
volatile uint8_t ser_out_buffer = 0;
volatile uint8_t sync = 0;
volatile uint8_t sync_timer = 0;
volatile uint8_t mode = 0;

int main(void) {

char *message;
message = &ser_in_p1_buffer;

// timer initialisierung

TCCR0=(1<<CS00);
TIMSK|=(1<<TOIE0);
TCNT0=0;

// external interrupt konfiguration

GICR = 1<<INT0;
MCUCR = 1<<ISC01;

//enable interrupts

sei();

// Eingaenge / Ausgaenge konfigurieren

DDRC = 0xff;				//PORTC - output / signal LED's

DDRD = 0x00;				//PORTD - input

//Netzwerk

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


	while (1) { // Main loop start.
		received = enc28j60PacketReceive(BufferSize, buffer);


		// Do something while no packet in queue.
		if (received == 0) {
			if (send == 1){
			     PORTC = ser_in_p1_buffer;                         
				send_udp(buffer, message, 1, 37351, serverip, 33750, gwmac);				
				send = 0;
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
	} // Main loop end.


	return (0);
}


void arpresolverResultCallback(uint8_t *ip, uint8_t refnum, uint8_t *mac) {
	if (refnum == TransNumGwmac)
		memcpy(gwmac, mac, 6);
}


//external interrupt INT0 (falling edge clk signal detection)

ISR(INT0_vect)
{								//ISR INT0 start

switch(rx) {

case 0:							//erstes bit - bei fallender flanke am externen interrupt 0 wird der  Pegel PD3 geprueft und der wert in "ser_in" geschrieben
   ser_in_p1=0;
   if(PIND& _BV(PD3)){
      ser_in_p1 = ser_in_p1 | (1<<0);
   }
   ser_in_p1 = ser_in_p1<<1;
   rx++;
   sync_timer=0;					//timer wird auf 0 gesetzt
break;

case 1 ... 6:						//gleicher Ablauf fuer die naechsten 6 bits. Bei fallender Flanke an INT0 wird PD3 gelesen, der wert nach "ser_in" geschrieben und alles nach links verschoben
   if (sync_timer < 12){				//wenn sync_timer zu hoch ist war die wartezeit zwischen 2 flanken zu hoch. Das Programm laeuft nicht synchron zum clock signal und bei der naechsten flanke wird wieder beim ersten bit gestartet
      if(PIND& _BV(PD3)){				//solange der microcontroller von anfang an mit den gameboys verbunden ist sollte es vermutlich keine probleme damit geben. Der Code kann also eventuell entfernt werden.
         ser_in_p1 = ser_in_p1 | (1<<0);  
      }
      ser_in_p1 = ser_in_p1<<1;
      rx++;
      sync_timer=0;
   } else {
      rx=0;							// rx und sync auf 0 setzen wenn die wartezeit zu hoch war
      sync=0;
   }
break;

case 7:							//letztes bit
   if (sync_timer < 12){				
      if(PIND& _BV(PD3)){
         ser_in_p1 = ser_in_p1 | (1<<0);
    }
      ser_in_p1_buffer = ser_in_p1;			//gespeichertes byte nach "ser_in_buffer" schreiben bevor "ser_in" beim naechsten clock signal ueberschrieben wird
      send=1;
      sync=1;						//Alle 8 bits erfolgreich gelesen. sync = 1
      rx=0;							//Wieder von vorne beginnen.
   } else {
      sync=0;
      rx=0;
   }
break;

}
}							//ISR INT0 ende


// timer interrupt (timer == 8 == 128us)

ISR(TIMER0_OVF_vect)
{
sync_timer++;
}