

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>


#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>


int sockfd;

void fatal()
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	close(sockfd);
}
int SIZE = 200000;
int clients = 65000;


int main(int argc, char **argv)
{
	struct sockaddr_in servaddr;
	fd_set activeS, readyS, wr;
	int id = 0;
	int arr[clients];
	char buffer[SIZE];
	char messag[SIZE];

	if (argc != 2)
	{
		write(2,"Wrong number of arguments\n", strlen("Wrong number of arguments\n") );
		exit(1);
	}
	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		fatal();
		exit(1);
	}

	// assign IP, PORT 
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(argv[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		fatal();
		exit(1);
	}

	if (listen(sockfd, 100) != 0) {
		fatal();
		exit(1);
	}

	int maxfd = sockfd;
	FD_ZERO(&activeS);
	FD_SET(sockfd, &activeS);

	while (1)
	{
		readyS = wr = activeS;

		if (select(maxfd + 1 , &readyS, &wr, NULL, NULL) < 0)
			continue ;
		for(int ip = 0; ip <= maxfd; ++ip)
		{
			if (FD_ISSET(ip, &readyS))
			{
				bzero(&messag,strlen(messag));
				bzero(&buffer,strlen(buffer));

				if (ip == sockfd)
				{
					struct sockaddr_in cli;
					socklen_t len = sizeof(cli);
					int connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
					if (connfd < 0)
						continue;
					maxfd = (connfd > maxfd) ? connfd : maxfd;
					sprintf(buffer, "server: client %d just arrived\n", id);
					arr[connfd] = id++;
					for(int j = 2; j <= maxfd; j++)
					{
						if (FD_ISSET(j, &wr) && j != sockfd)
						{
							if (send(j, buffer, strlen(buffer), 0) < 0)
							{
								fatal();
								exit(1);
							}
						}
					}
					FD_SET(connfd, &activeS);
				}
				else if (ip != sockfd)
				{
					int count = 1;
					while (count == 1 && messag[strlen(messag) - 1] != '\n')
					{
						count = recv(ip, messag + strlen(messag), 1, 0);
					}
					if (count <= 0)
					{
						sprintf(buffer, "server: client %d just left\n", arr[ip]);
						FD_CLR(ip, &activeS);
						close(ip);
						for (int z = 2; z <= maxfd; z++)
						{
							if (FD_ISSET(z, &wr) && z != ip)
							{
								if (send(z , buffer, strlen(buffer), 0) < 0)
								{
									fatal();
									exit(1);
								}
							}
						}
					}
					else
					{
						sprintf(buffer, "client %d: %s", arr[ip], messag);
						for (int z = 2; z <= maxfd; z++)
						{
							if (FD_ISSET(z, &wr) && z != ip)
							{
								if (send(z , buffer, strlen(buffer), 0) < 0)
								{
									fatal();
									exit(1);
								}
							}
						}
					}
				}
				FD_CLR(ip, &readyS);
			}
			
		}
	}
	return(0);
}
