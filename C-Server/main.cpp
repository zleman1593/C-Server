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
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <cerrno>
//using namespace std;
#define NUM_THREADS     5
#define MAX_BACKLOG     10
int openConnections = 0;


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void *handelRequest(void *sock_fd)
{
    long sock = (long)sock_fd;

    char buffer[256];
    bool is11 = true;
    while (is11) {
        
    
    bzero(buffer,256);
    
    int n = read(sock,buffer,255);
    
    std::cout << buffer << std::endl;
    //scan the request for a GET
    char *requestType = (char*) malloc(n);
    int i = 0;
    while (isalpha(buffer[i])) {
        requestType[i] = buffer[i];
        i++;
    }
    std::cout << "request type: " << requestType << std::endl;
    if (strcmp(requestType, "GET") == 0) {
        //GET request present
        //skip space
        while (isspace(buffer[i])) {
            i++;
        }
        //get the url path
        int start = i;
        char* urlstr = (char*) malloc(n);
        char* filetype = (char*) malloc(n);
        int ftype = 0;
        int ftypeit = 0;
        int it = 0;
        while (isspace(buffer[start]) == 0) {
            urlstr[it] = buffer[start];

            it++;
            start++;
        }
        std::cout << "file type: " << filetype << std::endl;
        i = start;
        std::cout << "path: " << urlstr << std::endl;
        //MAKE SURE WE CHECK URL PATH FITS PATH PATTERN
        while (isspace(buffer[i])) {
            i++;
        }
        //get HTTP type
        char* httpstr = (char*) malloc(n);
        it = 0;
        while (isspace(buffer[i]) == 0) {
            httpstr[it] = buffer[i];
            it++;
            i++;
        }
    
        std::cout << "HTTP Version: " << httpstr << std::endl;
        //determine HTTP version
        if (strcmp(httpstr, "HTTP/1.0") == 0 || strcmp(httpstr, "HTTP/1.1") == 0) {
            if(strcmp(httpstr, "HTTP/1.0") == 0){
                is11 = false;
                openConnections--;
            }
            //HTTP request ok
            //try to get file path
            /*FILE *fs = fopen("/Users/thegreenfrog/Desktop/Systems/C-Server/C-Server/hello.html", "r");*/
            
            FILE *fs = fopen("/Users/zackleman/Desktop/hello.html", "r");
            if (fs == NULL) {
                //could be 404, 403, 401
                
                if (errno == EACCES){
                     std::cout << "Permission denied" << std::endl;
                     write(sock, "403: Permission denied", 16);
                    //cerr << "Permission denied" << endl;
                
                } else{
                    //cerr << "Something went wrong: " << strerror(errno) << endl;
                std::cout << "404: Not Found" << std::endl;
               write(sock, "404: Not Found", 16);
            }
        }
            
            
          
            std::string contents;
            fseek(fs, 0, SEEK_END);
            contents.resize(ftell(fs));
            rewind(fs);
            fread(&contents[0], 1, contents.size(), fs);
            //std::cout << contents << std::endl;
            
         
            fseek (fs , 0 , SEEK_END);
            long lSize = ftell (fs);
            rewind (fs);
                      std::cout << lSize << std::endl;
            
            char *temp;
            temp = (char *)alloca(contents.size() + 1);
            memcpy(temp, contents.c_str(), contents.size() + 1);
            write(sock, temp, lSize);
          
        }
        else
        {
            //incorrect HTTP version call
            std::cout << "400: Bad Request" << std::endl;
            write(sock, "400: Bad Request", 16);
        }
    }
    else
    {
        //no valid request (400)
        std::cout << "400: Bad Request" << std::endl;
        write(sock, "400: Bad Request", 16);
        
     // 404 (not found),
        
        
        //403 (forbidden, but request correct),
        //401 (invalid credentials)
        //400 (bad request) status codes
    }
    
    if (n < 0) error("ERROR reading from socket");
    //printf("Here is the message: %s\n",buffer);
    

    std::cout << "Handeling Request! Socket Descriptor: "  << sock <<    std::endl;
    
    
        
    }
     openConnections--;
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
    
    if(listen(sock_fd, MAX_BACKLOG) < 0)
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
            }else{
                openConnections++;
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







