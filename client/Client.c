#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct packet{
  char data[1476];
}Packet;

typedef struct header{
  int frame_kind;
  int sq_no; //sequence number
  int ack; //ack accept
  
  int datalength; //length of the data;
  int checksum; //check this is not the end of data
  int total_size; //the total sie of the data
}Header;

typedef struct frame{
  Header head; //header of packet
  Packet packet; //the data
}Frame;

void main(int argc, char **argv){
  //deteermin whether there will be enough input 
  if(argc!=4){
    printf("Usage %s <filename> <hostname> <port> \n",argv[0]);
    exit(0);
  }
  //set the port number and socket
  int port = atoi(argv[3]);
  int sockfd;
  //the structure of the server addtress and the buffer size
  struct sockaddr_in serverAddr, newAddr;
  char buffer[1476];
  socklen_t addr_size;

  // frame id  the number of the sequence send and received, the ack numver
  int frame_id =0;
  Frame frame_send;
  Frame frame_recv;
  int ack_recv = 1;

  char filename[1024];
  int read_len;
  int filesize;  //total file size
  int transmit_time;  // transmit times
  int remaining_data;

  FILE *fp;

  //calculate the data transfer times and remaining data
  if((fp = fopen(argv[1],"rb")) == NULL){
    perror("Openfile failed\n");
  }else{
    fseek(fp,0,SEEK_END); //point end of file
    filesize=ftell(fp); //calculate this pointer from the start of file

    printf("Size of file: %d bytes.\n",filesize);
  }

  fclose(fp);

  //calcualte transmit time and last frame data length
  transmit_time = filesize/1476;
  remaining_data = filesize%1476;

  printf("the transmit times: %d, the remaining dat size: %d\n", transmit_time, remaining_data);
  printf("---------------------\n");
  
  if( (fp = fopen(argv[1],"rb")) == NULL){
    perror("Open file failed\n");
    exit(0);
  }
  //using UDP establish the socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  //set our address
  memset(&serverAddr, '\0', sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
   serverAddr.sin_addr.s_addr = inet_addr("10.33.1.13");
  //printf("%s\n",argv[2]);
  serverAddr.sin_addr.s_addr = inet_addr(argv[2]);
  
  //test my connection and send function
  //strcpy(buffer, "Hello UDP server\n");
  //sendto(sockfd, buffer, 1024, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
  strcpy(filename, argv[1]);
  sendto(sockfd, filename, 1024, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
  
  
  
  

  // 0- ACK
  // 1 - SEQ
  // while(frame_id <= transmit_time){
  while(1){
    //if ack equal 1 then send data      
    if(ack_recv = 1){
      frame_send.head.sq_no = frame_id;
      frame_send.head.frame_kind = 1;
      frame_send.head.ack = 0;
      frame_send.head.datalength=1476;
      frame_send.head.checksum=transmit_time;
      frame_send.head.total_size=filesize;
      //printf("the length of the packet: %d\n", frame_send.datalength);
      //read the file
      read_len = fread(buffer,sizeof(char),1476,fp);
	if(read_len == 0){
	  printf("readlen == 0\n");
	break;
	//  fclose(fp);
	//	strcpy(frame_send.packet.data,"Finish send the end of string\n");
	//	printf("Finish send the end of string\n");
	//	sendto(sockfd, &frame_send, sizeof(Frame), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	//	break;
      }
	printf("The data is: \n");
	//test my data received
      printf("%s\n",buffer);
      //scanf("%s", buffer);
      //    printf("the length of the packet: %d\n", frame_send.datalength);
      //	strncpy(frame_send.packet.data, buffer,read_len);
      //int j;
      //for(j=0;j<read_len;j++){
      //frame_send.packet.data[j] = buffer[j];
      //}
      //copy the data from the read buffer
      memcpy(frame_send.packet.data, buffer, read_len);

      
      // printf("the length of the packet: %d\n", frame_send.datalength);
      // printf("size of the buffer sent: %d, size of packet: %d\n ", sizeof(buffer), sizeof(frame_send.packet.data));
      //printf("the length of the packet: %d\n", frame_send.datalength);
      
      //change the data length to the fread function read length data
      frame_send.head.datalength=read_len;
      printf("data length %d, %d\n",read_len,frame_send.head.datalength);
      //if this is the end of the file, break this
      if(frame_id == transmit_time){
        //frame_send.datalength= remaining_data;
	sendto(sockfd, &frame_send, sizeof(Frame),0,(struct sockaddr*)&serverAddr, sizeof(serverAddr));
	printf("[+]Frame Sent, Frame size: %d, Frame number %d\n",sizeof(Frame),frame_send.head.sq_no);
	break;
      }

      //printf("test data %s\n",frame_send.packet.data);
      sendto(sockfd, &frame_send, sizeof(Frame),0,(struct sockaddr*)&serverAddr, sizeof(serverAddr));
      printf("[+]Frame Sent, Frame size: %d, Frame number %d\n",sizeof(Frame),frame_send.head.sq_no);
      printf("the length of the data: %d\n",frame_send.head.datalength);
    }

    int addr_size = sizeof(serverAddr);

    //    printf("test addr size: %d\n", addr_size);

    //receive AFK from server, then print and transmit next frame
    //not receive AFK from server, then print and retransmit current frame
    int f_recv_size = recvfrom(sockfd,&frame_recv, sizeof(Frame), 0, (struct sockaddr*)&serverAddr, &addr_size);
    printf("Ack Value: %d\n", frame_recv.head.ack);

    //send whether the afk received, if not retransmit our data
    if(f_recv_size >0 && frame_recv.head.sq_no == 0 && frame_recv.head.ack == frame_id+1){
      printf("[+]ACK Received\n");
      ack_recv = 1;
    }else{
      printf("[-]ACK Not Received\n");
      ack_recv = 0;
      sendto(sockfd, &frame_send,sizeof(Frame), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
      printf("[+] Frame resent, Frame size: %d, Frame number: %d \n",sizeof(Frame),frame_send.head.sq_no);
    }
    frame_id++;
    //break;
  }

  fclose(fp);
  close(sockfd);
}

