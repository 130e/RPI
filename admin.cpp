#include "uartools.h"

#include <string.h>
#include <time.h>


int logwrite(int logfd, char* buffer)
{
	char wr_buffer[255] = {0};
	time_t timep = time(NULL);
	strcpy(wr_buffer, ctime(&timep));
	strcat(wr_buffer, buffer);//trans to str
	strcat(wr_buffer, "\n");
	
	write(logfd, wr_buffer, strlen(wr_buffer));
}


int main()
{
	int uartfd = uartsetup();
	
	//to do
	
	pid_t pid = fork();
	if (pid == 0)
	{
		//child
		adminloop(uartfd);
	}
	else
	{
		//parent
		int rx_len = 0;
		unsigned char rx_buffer[64];
		char str_buffer[128];
		int logfd = open("sensor.log", O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if (logfd < 0 )
			printf("file error");
		
		while(1)
		{
			//0FE 1len 2src_p 3dst_p 4src_addr 6data *FF
			rx_len = recebytes(uartfd, rx_buffer);
			if (rx_len == -1)
				perror("receive error");
			else
				switch (rx_buffer[3])
				{
					case 0x91:
						//sensor triggered
						byte2str((char*)rx_buffer, str_buffer);
						logwrite(logfd, str_buffer);
						break;
					default:
						break;
				}
			
		}
	}
	close(uartfd);
	return 0;
}
