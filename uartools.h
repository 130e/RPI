#include <stdio.h>
#include <errno.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART

int uartsetup();

int transbytes(int uart0_filestream, unsigned char* tx_pbuffer, int len);

int recebytes(int uart0_filestream, unsigned char* rx_pbuffer);

int lighton(int uart0_filestream, int interval);

int getaddr(int uart0_filestream);

int pksend(int uartfd, char cmd, char chn);

void adminloop(int uartfd);
