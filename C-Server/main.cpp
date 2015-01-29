//
//  main.cpp
//  C-Server
//
//  Created by Zackery leman & Chris Lu on 1/28/15.
//  Copyright (c) 2015 Zleman-Clu. All rights reserved.
//
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
//using namespace std;
#define NUM_THREADS     5
#define MAX_BACKLOG     10


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void *handelRequest(void *sock_fd)
{
    long sock;
    sock = (long)sock_fd;
    
    int n;
    char buffer[256];
    
    bzero(buffer,256);
    
    n = read(sock,buffer,255);
    
    if (n < 0) error("ERROR reading from socket");
    //printf("Here is the message: %s\n",buffer);
    

    std::cout << "Handeling Request! Socket Descriptor: "  << sock <<    std::endl;
    
     n = write(sock,"Successful message: ",18);
        n = n + write(sock,buffer,18);
    if (n < 0) {error("ERROR writing to socket");}
    
    char* databuf[1024];
    /* int getMsg = recv(sock_fd, databuf, sizeof(databuf), 0);
     if (getMsg < 0)
     {
     std::cout << "Error while recieving HTTP request" << std::endl;
     exit(-1); //or break?
     }  */
    pthread_exit(NULL);
}



int main(int argc, const char * argv[]) {
    //Prints the comamnd line arguments
    std::cout << "argc = " << argc << std::endl;
    for(int i = 0; i < argc; i++)
        std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
    
    
    // Create socket to listen for connections
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("error opening socket\n");
        return -1;
    }
    
    struct sockaddr_in myaddr;
    /*if (argv[i]){
     myaddr.sin_port = htons(argv[i]);
     }else{*/
    myaddr.sin_port = htons(8888); // use port default of 8888
    // }
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //Bind Socket
    if(bind(sock_fd, (struct sockaddr*) &myaddr, sizeof(myaddr)) < 0) {
        // error opening socket
        return -1;
    }
    
    if(int listen_begin = listen(sock_fd, MAX_BACKLOG) < 0)
    {
        std::cout << "Error while establishing listening socket" << std::endl;
        return -1;
    }
    
    int adressSize = sizeof(myaddr);
    int *size = &adressSize;
    
    while(true)
    {
        int newSocketfd = accept(sock_fd, (struct sockaddr*) &myaddr, (socklen_t *) &size );
        
        if( newSocketfd < 0)
        {
            std::cout << "Error while accepting" << std::endl;
            return -1;
        } else{
            
            pthread_t newThread;
            std::cout << "Creating  new thread " <<    std::endl;
            int rc = pthread_create(&newThread, NULL, handelRequest, (void *)newSocketfd);
            if (rc){
                std::cout << "Error:unable to create thread," << rc <<  std::endl;
                exit(-1);
            }
        }
        
        
        
        
    }
    
    //close();
    
    std::cout <<"End!\n";
    
    return 0;
    
}






/*
 
 Forever loop:
 Listen for connections
 Accept new connection from incoming client
 Parse HTTP request
 Ensure well-formed request (return error otherwise)
 Determine if target file exists and if permissions are set properly (return error otherwise)
 Transmit contents of file to connect (by performing reads on the file and writes on the socket)
 Close the connection (if HTTP/1.0)
 
 */







