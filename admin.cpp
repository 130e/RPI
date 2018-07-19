#include "uartools.h"

#include <string.h>
#include <time.h>

int logwrite(int logfd, char* rx_buffer)
{
	char wr_buffer[255];
	time_t timep = time(NULL);
	strcpy(wr_buffer, ctime(&timep));
	strcat(wr_buffer, " ");
	strcat(wr_buffer, rx_buffer);//trans to str
	strcat(wr_buffer, "\n");
	
	char* ptr = wr_buffer;
	int len = 0;
	while(*ptr != 0xff)
		len++;
	
	write(logfd, wr_buffer, len);
}


int main()
{
	int uartfd = uartsetup();
	
	//to do
	unsigned char rx_buffer[255];
	
	pid_t pid = fork();
	if (pid == 0)
	{
		//child
		adminloop(uartfd);
	}
	else
	{
		//parent
		// txt log
		int rx_len = 0;
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
				/*switch (rx_buffer[3])
				{
					case 0x90:
						//sensor triggered
						logwrite(logfd, (char*)rx_buffer);
						break;
					default:
						break;
				}*/
				{
					// db log
					sqlite3 *db;
					int rc;
					char sql[255] = "INSERT INTO SENSOR (SID, TIME) VALUES ";
					// open db
					rc = sqlite3_open("sensor_log.db", &db);
					if (rc)
						printf("Can't open database: %s\n", sqlite3_errmsg(db));
					// (1, date)
					char value[48];
					int id = 0;
					time_t timep = time(NULL);
					sprintf(value, "(%d, %s)", id, ctime(&timep));
					strcat(sql, value);
					printf("%s\n", sql);
					sqlite3_close(db);
				}
			
		}
	}
	close(uartfd);
	return 0;
}
