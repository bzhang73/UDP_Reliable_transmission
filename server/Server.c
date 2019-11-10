#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>


typedef struct packet{
  char data[1476];
}Packet;

typedef struct header{
  int frame_kind;
  int sq_no;  //sequence number
  int ack;  //ack accept
  
  int datalength; //length of the data
  int checksum; //check this is not the end of data
  int total_size; //the total size of the data
}Header;

typedef struct frame{
  Header head; //header of the packet
  Packet packet;  //the data transfer
}Frame;

//returns hostname for the local computer
void checkHostName(int hostname){
  if(hostname == -1){
    perror("gethostname error");
    exit(1);
  }
}

//return host information corresponding to host name
void checkHostEntry(struct hostent *hostentry){
  if (hostentry == NULL){
    perror("get host by name error");
    exit(1);
  }
}

//covert space-delimited IPv4 address to dotted-decimal format
void checkIPbuffer(char *IPbuffer){
  if(NULL == IPbuffer){
    perror("inet_ntoa");
    exit(1);
  }
}

void main(int argc, char **argv){
  //determin whether there will be enough input of the port number
  if(argc !=2)
  {	printf("Usage: %s <port>\n", argv[0]);
  	exit(0);
  }

  //host name and IP
  char hostbuffer[256];
  char *IPbuffer;
  struct hostent *host_entry;
  int hostname;
  
  //set the port number and socket
  int port = atoi(argv[1]);
  int sockfd;
  //the sturcture of the servetr address and the buffer size
  struct sockaddr_in serverAddr, newAddr;
  char buffer[1476];
  socklen_t addr_size;

  //Frame define, id, send, and receive
  int frame_id = 0;
  Frame frame_recv;
  Frame frame_send;

  char filename[1024];
  FILE *fp;

  //To retrieve hostname
  hostname = gethostname(hostbuffer, sizeof(hostbuffer));
  checkHostName(hostname);

  //toretrieve host information
  host_entry = gethostbyname(hostbuffer);
  checkHostEntry(host_entry);

  //To cover an internet address into ASCII string
  IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));

  printf("Hostname: %s\n",hostbuffer);
  printf("Host IP: %s\n",IPbuffer);


  //using UDP socket 
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  //set our address
  //serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  //htons converts a short integer into the network representation
  //htonl converts a long integer into the network representation
  //INADDR_ANY is the special IP address 0.0.0.0 which binds the 
  //transport endpoint to all IP addresses on the machine.
  memset(&serverAddr, '\0', sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  //serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serverAddr.sin_addr.s_addr = inet_addr(IPbuffer);  

  //bind to the address to which the service will be offered 
  bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
  addr_size = sizeof(newAddr);
  //receive message form UDP
  recvfrom(sockfd, filename, 1024, 0, (struct sockaddr*)&newAddr, &addr_size);

  //test whether the client can connect to server and receive data
  printf("filename is: %s\n",filename);

  if((fp = fopen(filename,"wb"))== NULL){
      perror("Create file failed\n");
      exit(0);
  }

  //0-ack
  //1-seq
  while(1){
      printf("receive successful!\n");
    int f_recv_size =  recvfrom(sockfd, &frame_recv, sizeof(Frame), 0, (struct sockaddr*)&newAddr, &addr_size);
    printf("ACK Value: %d and sequence number: %d\n", frame_recv.head.ack, frame_recv.head.sq_no);
    
  //check whether this frame is we want
    if(f_recv_size >0 && frame_recv.head.frame_kind == 1 && frame_recv.head.sq_no == frame_id){
      printf("[+] Frame Received: \n" );
      int tem=0;
      strncpy(buffer,frame_recv.packet.data,frame_recv.head.datalength);
      for(tem=0;tem<1476;tem++)
	printf("%c",buffer[tem]);

      printf("\n");
      //printf("[+] Frame Received: \n");
      
      //the last transfer data
      if(frame_recv.head.sq_no == frame_recv.head.checksum){
	printf("the length of the last data: %d \n", frame_recv.head.datalength);
	int write_length = fwrite(frame_recv.packet.data, sizeof(char), frame_recv.head.datalength, fp);
	if(write_length <= 0 ){
	  printf("Fail write the file \n");
	  exit(0);
	}
	printf("Server finish receive the file\n");
	break;
      }

      //write data 
      // int data_length = f_recv_size -24;
      //printf("the length of the data: %d\n", frame_recv.datalength);
      int write_length = fwrite(frame_recv.packet.data, sizeof(char), frame_recv.head.datalength, fp);
      //printf("%dthe nubmer write: \n", write_length);
      if(write_length <= 0){
        printf("Fail write file \n");
	exit(0);
      }
      
      //ack frame
      frame_send.head.sq_no=0;
      frame_send.head.frame_kind =0;
      frame_send.head.ack = frame_recv.head.sq_no + 1;

      //send data
      sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr*)&newAddr, addr_size);
      printf("[+]ACK Sent\n");
      frame_id++;
    }else{
      printf("[+]ACK Not Received\n");
    }

    //break;
    //frame_id++;
  }

  fclose(fp);
  close(sockfd);
}
