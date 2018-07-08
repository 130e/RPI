#include "uartools.h"

#define ON 0x01
#define OFF 0x02
#define BRIGHTER 0x03
#define DARKER 0x04
#define AUTO 0x05
//#define BLINK 0x06

int uartsetup()
{
	//-------------------------
	//----- SETUP USART 0 -----
	//-------------------------
	//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively
	int uart0_filestream = -1;
	
	//OPEN THE UART
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//											immediately with a failure status if the output can't be written immediately.
	//
	//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
	
	uart0_filestream = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
	if (uart0_filestream == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}
	
	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);
	
	return uart0_filestream;
}

int transbytes(int uart0_filestream, unsigned char* tx_pbuffer, int len)//warning is off
{
	if (uart0_filestream != -1)
	{
		int count = write(uart0_filestream, tx_pbuffer, len);		//Filestream, bytes to write, number of bytes to write
		//if (count < 0)
			//printf("UART TX error\n");
		return count;
	}
	printf("uart0_filestream error\n");
	return -1;
}

int recebytes(int uart0_filestream, unsigned char* rx_pbuffer)//while query
{
	int rx_length = 0;
	unsigned char* cp = rx_pbuffer;
	printf("RX recive bytes: ");
	while(1)
	{
		int rc_len = 0;
		rc_len = read(uart0_filestream, (void*)cp, 255);
		if (rc_len ==-1)
		{
			if ((errno == EAGAIN) || (errno == EINTR))
				continue;
			//true error
			rx_length = -1;
			perror("Read error");
			break;
		}
		else if (rc_len == 0);
		else
		{
			//weird shit i can only get 8 bytes per read
			for(int i = 0; i < rc_len ; i++)//read fixed length
			{
				printf("%02X", *cp);
				cp++;
			}
			rx_length += rc_len;
			if (*(cp-1) != 0xff)
				continue;
			printf(" %d \n", rx_length);
			break;
		}
	}
	return rx_length;
}

int lighton(int uart0_filestream, int interval)
{
	unsigned char tx_buffer[10];
	
	if(interval > 255)
	{
		printf("interval.range from 0~255\n");
		return -1;
	}
	tx_buffer[0] = 0xFE;
	tx_buffer[1] = 0x05;
	tx_buffer[2] = 0x91;
	tx_buffer[3] = 0x20;
	tx_buffer[4] = 0x00;
	tx_buffer[5] = 0x00;
	tx_buffer[6] = (unsigned char)interval;
	tx_buffer[7] = 0xFF;
	
	return transbytes(uart0_filestream, &tx_buffer[0], 8);
}

int getaddr(int uart0_filestream)
{
	unsigned char tx_buffer[10];
	
	tx_buffer[0] = 0xFE;
	tx_buffer[1] = 0x05;
	tx_buffer[2] = 0x91;
	tx_buffer[3] = 0x21;
	tx_buffer[4] = 0x00;
	tx_buffer[5] = 0x00;
	tx_buffer[6] = 0x01;
	tx_buffer[7] = 0xFF;
	
	return transbytes(uart0_filestream, &tx_buffer[0], 8);
}

int pksend(int uartfd, char cmd, char chn)//only 1 byte of node
{
	unsigned char tx_buffer[8];
	
	tx_buffer[0] = 0xFE;
	tx_buffer[1] = 0x05;
	tx_buffer[2] = 0x90;
	tx_buffer[3] = 0x90;
	tx_buffer[4] = chn;
	tx_buffer[5] = 0x00;
	tx_buffer[6] = cmd;
	tx_buffer[7] = 0xFF;
	
	return transbytes(uartfd, &tx_buffer[0], 8);
}


void adminloop(int uartfd)
{
	printf("<LIGHT SYSTEM ADMIN CONSOLE>--------\n");
	char cmd[60];
	char chn;
	//input: on 1
	while(1)
	{
		printf(">>");
		if (fgets(cmd, 60, stdin)!=NULL)
		{
			char* cp = cmd;
			while(*cp==0x20)
				cp++;
			switch (cmd[1])
			{
				case 'N':
				case 'n':
					chn = cmd[3] - 0x30;
					pksend(uartfd, ON, chn);
					break;
				case 'F':
				case 'f':
					chn = cmd[4] - 0x30;
					pksend(uartfd, OFF, chn);
					break;
				case 'R':
				case 'r':
					chn = cmd[9] - 0x30;
					pksend(uartfd, BRIGHTER, chn);
					break;
				case 'A':
				case 'a':
					chn = cmd[7] - 0x30;
					pksend(uartfd, DARKER, chn);
					break;
				case 'U':
				case 'u':
					chn = cmd[3] - 0x30;
					pksend(uartfd, AUTO, chn);
					break;
				case 'E':
				case 'e':
					// get local address
					getaddr(uartfd);
					break;
				case 'I':
				case 'i':
					// light local 
					lighton(uartfd, 50);
					break;
				default:
					break;
			}		
		}
	}
		
}
