#include <stdio.h>
#include <time.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h> 

int main(int argc, char *argv[])
{
	if (argc > 1)
    {
    	if (strcmp("-s", argv[1])== 0) //server 
    	{
    		if (argc!=4)
    		{
    			printf("Error: missing or additional arguments\n");
    			return 0;
    		}
    		else if ((atoi(argv[3])<1024)||(atoi(argv[3])>65535))
    		{
    			printf("Error: port number must be in the range 1024 to 65535\n");
    			return 0;
    		}
    		
    		// create a socket
    		int server_port=atoi(argv[3]);
    		int sd;
			struct sockaddr_in sever_addr, client_addr;
			if ((sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
			{
				perror("opening TCP socket");
				abort();
			}
			memset(&sever_addr, 0, sizeof (sever_addr));
			sever_addr.sin_family = AF_INET;
			sever_addr.sin_addr.s_addr = INADDR_ANY;
			sever_addr.sin_port = htons(server_port);

			//label the socket with a port 
			if (bind(sd, (struct sockaddr *) &sever_addr, sizeof (sever_addr)) < 0) 
			{
				perror("bind");
				printf("Cannot bind socket to address\n");
				abort();
			}

			//listen the port
			int qlen=5;
			if (listen(sd, qlen) < 0) {
				perror("error listening");
				abort();
			}

			//accept 
			int client_addr_len = sizeof(client_addr);
			int td;
			td = accept(sd, (struct sockaddr *) &client_addr, &client_addr_len);
			if (td < 0) {
				perror("error accepting connection");
				abort();
			}

			// read from buffer
			char buffer[1000];
			bzero(buffer,1000);
			int blen=1000;
			int bytes=0;
			int recv_number=0;
			struct timeval st1, st2;
    		double time;
		    gettimeofday(&st1, NULL);		    
			while (1) 
			{
				recv(td, buffer, 1, MSG_PEEK);
				if (strcmp("e", buffer)== 0)
				{
					if (send(td, "finAck", 6, 0)== -1)
					{
		                perror("send");
					}
					gettimeofday(&st2, NULL);
					break;
				}
				recv_number=recv(td, buffer, blen, MSG_WAITALL);
				if (recv_number> 0)
				{					
					bytes = bytes+recv_number;
				} 
				else 
				{
					perror("recv");
				}
			}
			close(sd);
			close(td);

			// output
			time = (st2.tv_sec - st1.tv_sec);
   	 		time += (st2.tv_usec - st1.tv_usec) / 1000000.0;
   	 		int sizeKB=bytes/1000;			
			double rate;			
			rate=sizeKB/(time*1000);
			printf("received=%d KB rate=%f Mbps\n",sizeKB,rate);

			return 0;


    	}
    	else if (strcmp("-c", argv[1])== 0) //client 
    	{
    		if (argc!=8)
    		{
    			printf("Error: missing or additional arguments\n");
    			return 0;
    		}
    		else if ((atoi(argv[5])<1024)||(atoi(argv[5])>65535))
    		{
    			printf("Error: port number must be in the range 1024 to 65535\n");
    			return 0;
    		}

    		// create socket
    		char cbuffer[256];
			bzero(cbuffer,256);
			int cblen=256;
    		int sd;
			if ((sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
			{
				perror("socket");
				printf("Failed to create socket\n");
				abort();
			}

			// connect
			int server_port=atoi(argv[5]);
			char *server_name=argv[3]; 
			int time=atoi(argv[7]);
			struct sockaddr_in sin;
			struct hostent *host = gethostbyname(server_name);
			memset(&sin, 0, sizeof(sin));
			sin.sin_family = AF_INET;
			sin.sin_addr.s_addr = *(unsigned long *) host->h_addr_list[0];
			sin.sin_port = htons(server_port);
			if (connect(sd, (struct sockaddr *) &sin, sizeof (sin)) < 0) 
			{
				perror("connect");
				printf("Cannot connect to server\n");
				abort();
			}

			//send
			char buffer[1000];
			bzero(buffer,1000);
			int blen=1000;
			struct timeval t1, t2, t3;
    		double elapsedTime;
    		double realTime;
		    gettimeofday(&t1, NULL);
		    int sizeKB=0;
			while (elapsedTime<=time) 
			{
				if (send(sd, buffer, blen, 0)== -1)
				{
	                perror("send");
				}
				gettimeofday(&t2, NULL);
			    elapsedTime = (t2.tv_sec - t1.tv_sec);
			    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000000.0;
			    sizeKB=sizeKB+1;
			}
			if (send(sd, "e", 1, 0)== -1)
			{
                perror("send");
			}
			if (recv(sd, cbuffer, cblen, 0) < 0)
			{
				perror("recv");
			}
			if (strcmp("finAck", cbuffer)== 0)
			{
				gettimeofday(&t3, NULL);
				realTime = (t3.tv_sec - t1.tv_sec);
   	 			realTime += (t3.tv_usec - t1.tv_usec) / 1000000.0;			
				double rate;				
				rate=sizeKB/(realTime*1000);
				printf("sent=%d KB rate=%f Mbps\n",sizeKB,rate);
				close(sd);
			}
			else
			{
				printf("Fin not correct \n");
			}
			return 0;
    	}
    }
  	else
    {
      	printf("Error: missing or additional arguments\n");
    }
    return 0;
}
