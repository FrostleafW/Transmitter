#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 17010

void* thread_run(void* arg)
{
	printf("New connection created\n");
	int* fd = (int*)arg;
	char buf[1024];
	while(1)
	{
		memset(buf, '\0', sizeof(buf));
		int _s = recv(fd[0], buf, sizeof(buf), 0);
		if(_s > 0)
		{
			send(fd[1], buf, _s, 0);
		}
		else if(_s == 0)
		{
			printf("Client closed...\n");
			close(fd[0]);
			close(fd[1]);
			break;
		}
		else
		{
			close(fd[0]);
			close(fd[1]);
			printf("Read error...\n");
			break;
		}
	}
	fd[0] = -1;
}

int main(int argc, char* argv[])
{
	int port = PORT;
	if(argc == 2)
	{
		int in_port = atoi(argv[1]);
		if(port > 0 && port < 30000)
		{
			port = in_port;
		}
	}
	printf("Port being used: %d\n", port);
	
	//CreateSocket
	int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock < 0)
	{
		perror("socket");
		return 2;
	}

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	//Bind
	if(bind(listen_sock, (struct sockaddr*)&local, sizeof(local)) < 0)
	{
		perror("bind");
		return 3;
	}

	//Always Listen
	listen(listen_sock, 3);

	//Accept
	struct sockaddr_in peer;
	socklen_t len = sizeof(peer);
	
	int fd[2] = {-1};
	int fd_s[2];
	
	// First Client
	fd[0] = accept(listen_sock, (struct sockaddr*)&peer, &len);
	if(fd[0] < 0)
	{
		perror("accept");
		return 4;
	}

	printf("Got a new tcp link... socket -> %s : %d\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
	
	// Second Client
	fd[1] = accept(listen_sock, (struct sockaddr*)&peer, &len);
	if(fd[1] < 0)
	{
		perror("accept");
		return 4;
	}

	printf("Got a new tcp link... socket -> %s : %d\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
	
	// Notify clients and assign id
	char id1[2] = {0x01, 0x00};
	char id2[2] = {0x02, 0x00};
	send(fd[0], id1, 2, 0);
	send(fd[1], id2, 2, 0);

	fd_s[0] = fd[1];
	fd_s[1] = fd[0];

	pthread_t id;
	pthread_create(&id, NULL, thread_run, (void*)fd);
	pthread_detach(id);
	
	pthread_create(&id, NULL, thread_run, (void*)fd_s);
	pthread_detach(id);
	
	// Close Listen
	close(listen_sock);

	while(1)
	{
		if(fd[0] == -1 || fd_s[0] == -1)
		{
			break;
		}
	}
	
	close(fd[0]);
	close(fd[1]);
	return 0;
}
