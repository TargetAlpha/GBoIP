// server.c -*-c-*-


#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void  error(char *msg);
char* uint8_t2bin(uint8_t num);


int main() {
	struct sockaddr_in clientAddr, serverAddr;
	int      len, received, sockfd;
	char     recvBuffer[1];


	puts("                  ___________________");
	puts("                 |  ,-------------.  |");
	puts(" _______ ______  | |  .---------.  | |  _______ ______");
	puts("|     __|   __ \\ | |  | .-----. |  | | |_     _|   __ \\");
	puts("|    |  |   __ < | |  | |  _  | |  | |  _|   |_|    __/");
	puts("|_______|______/ | |  | |_____| |  | | |_______|___|");
	puts("                 | |  |         |  | |");
	puts("                 | |  `---------'  | |");
	puts("                 | `---------------' |");
	puts("                 |   _  SERVER       |");
	puts("                 | _| |_         ,-. |");
	puts("                 ||_ O _|   ,-. \"._,\"|");
	puts("                 |  |_|    \"._,\"   A |");
	puts("                 |    _  _    B      |");
	puts("                 |   // //           |");
	puts("                 |  // //    \\\\\\\\\\\\  |");
	puts("                 |  `  `      \\\\\\\\\\\\ ,");
	puts("                 |________...______,\"\n");


	sockfd                     = socket(AF_INET, SOCK_DGRAM, 0);
	serverAddr.sin_family      = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port        = htons(33750);


	if ((bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr))) == -1) {
		error("Couldn't bind name to socket: ");
		return 1;

	} else
		printf("Listening on port 33750\n\n");


	while (1) {
		len = sizeof(clientAddr);
		received = recvfrom(sockfd, recvBuffer, 1, 0, (struct sockaddr *)&clientAddr, &len);
		if (received == -1) {
			error("Couldn't receive message: ");
			continue;
		}

		printf("%s\n", uint8_t2bin(recvBuffer[0]));
	}


	close(sockfd);
	return 0;
}


void error(char *msg) {
	fprintf(stderr, "%s%s\n", msg, strerror(errno));
}


char* uint8_t2bin(uint8_t num) {
	int   bitStrLen = sizeof(uint8_t) * 8 * sizeof(char);
	char* bin       = (char*)malloc(bitStrLen);

	for (int i = (bitStrLen - 1); i >= 0; i--) {
		int k = 1 & num;
		*(bin + i) = ((k == 1) ? '1' : '0');
		num >>= 1;
	}

	bin[8] = '\0';

	return bin;
}
