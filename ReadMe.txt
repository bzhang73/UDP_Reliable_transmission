1. Instruction about how to run my client and server
in each folder have a Makefile, just type make, then get the target execute file. Then ./client filename hostname port or ./server port

2.Completion status of the project
a. I using UDP socket to transmit the file.
b. Design my own protocal, using structure design head and packet, head conatin the kind of the packet, the data length, the transmit times, the checksum and ack_received status.
c. My packets are 1500B elngth, header is 24B, and data length 1476B.
d. I implement the stop and wait function. the client must wait the ACK from the sever, if received is that the data is accepted by the server and can continue send another frame. If the ACK is not received from the server, it means the data is not received by the frams, so that client will resend the previous data to the sever.
f. I show my IP address and my Host name in my server 
g. I can send my data from one computer to another
h. I test my project, correct transmit txt file, PDF file and video file, all can open correctly

3. I do finish this project ahead of time, using clear idea and code finish the project. Solve many problems that so weird for me. I test my code several times it can run correctly. Thanks for help me solve the binary copy using memcpy function.
