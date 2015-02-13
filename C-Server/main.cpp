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
#include <sstream>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <cerrno>
#include <vector>
#include <time.h>
#define NUM_THREADS     5
#define MAX_BACKLOG     10
#define THREAD_TIMEOUT  60
int openConnections = 0;
timeval threadTimeout;

struct threadStruct{
    pthread_t pid;
    time_t startTime;
};

std::vector<threadStruct*> vectorThread;


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void *handelRequest(void *sock_fd)
{
    long sock = (long)sock_fd;
    
    char buffer[2000];
    bool is11 = true;
    while (is11) {
        int n;
        threadTimeout.tv_sec = 60;
        threadTimeout.tv_usec = 0;
        
        bzero(buffer,2000);
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(sock, &readset);
        int num = select(sock+1, &readset, NULL, NULL, &threadTimeout);
        if(num >0)
        {//data present to read
            n = read(sock,buffer,2000);
            std::cout << buffer<<std::endl;
        }
        else
        {//thread has reached timeout. Kill
            pthread_exit(NULL);
        }
        
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
            int it = 0;
            while (isspace(buffer[start]) == 0) {
                urlstr[it] = buffer[start];
                it++;
                start++;
            }
            //variable holding file type
            char* filetype = urlstr;
            if ((filetype = std::strchr(filetype, '.')) != NULL) {
            }
            else
            {
                //interpret / as index.html
                filetype = ".html";
                urlstr = "/index.html";
            }
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
                //FILE *fs = fopen("/Users/thegreenfrog/Desktop/Systems/C-Server/C-Server/3C.pdf", "r");
                
                char root[]  = "/Users/zackleman/Desktop";
                FILE *fs = fopen(strcat(root,urlstr), "r");
                if (fs == NULL) {
                    //could be 404, 403, 401
                    
                    if (errno == EACCES){
                        std::cout << "Permission denied" << std::endl;
                        write(sock, "403: Permission denied", 16);
                        
                    } else{
                        std::cout << "404: Not Found" << std::endl;
                        write(sock, "404: Not Found", 16);
                    }
                }
                //clear buffer
                memset(buffer, 0, 2000);
                
                std::string contents;
                fseek(fs, 0, SEEK_END);
                contents.resize(ftell(fs));
                rewind(fs);
                fread(&contents[0], 1, contents.size(), fs);
                
                
                fseek (fs , 0 , SEEK_END);
                long lSize = ftell (fs);
                rewind (fs);
                std::cout << lSize << std::endl;
                
                char *temp;
                temp = (char *)alloca(contents.size() + 1);
                memcpy(temp, contents.c_str(), contents.size() + 1);
                
                //Send HTTP status
                write(sock, "HTTP/1.1 200 Ok\r\n", 18);
                if (!strcmp(filetype, ".html")) {
                    write(sock, "content-type: text/html\r\n", 30);
                }
                else if (!strcmp(filetype, ".pdf"))
                {
                    write(sock, "content-type: application/pdf\r\n", 32);
                }
                else if (!strcmp(filetype, ".png")) {
                    write(sock, "content-type: image/png\r\n", 26);
                }  else if (!strcmp(filetype, ".jpg") || !strcmp(filetype, ".jpeg")) {
                    write(sock, "content-type: image/jpeg\r\n", 27);
                }   else if (!strcmp(filetype, ".gif")) {
                    write(sock, "content-type: image/gif\r\n", 26);
                }
                
                
                //Send Date
                char buffer [80];
                time_t currentTime;
                time(&currentTime);
                struct tm * timeinfo = localtime(&currentTime);
                strftime (buffer,80,"Date: %c \r\n",timeinfo);
                puts(buffer);
                write(sock, buffer, 35);
                
                //Send Content Length
                std::ostringstream oss;
                oss << "content-length: " << lSize << "\r\n\r\n";
                std::string var = oss.str();
                char *temp2;
                temp2 = (char *)alloca(var.size() + 1);
                memcpy(temp2, var.c_str(), var.size() + 1);
                
                
                int length = 1;
                int x = (int) lSize;
                while ( x /= 10 )
                    length++;
                
                std::cout <<(length)<<std::endl;
                write(sock, temp2,(20+length)); 
                
                //Send Body
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
        
        for (int j; j < vectorThread.size(); ++j) {
            if (vectorThread.at(j)->pid == pthread_self()) {
                vectorThread.at(j)->startTime = time(NULL);
            }
        }
        
        
    }
    openConnections--;
    pthread_exit(NULL);
}

//returns true if a thread was killed. false otherwise
bool manageThreads()
{
    return false;
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
    std::vector<threadStruct*> vectorThread;
    time_t timer;
    
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
            time(&timer);
            int rc = pthread_create(&newThread, NULL, handelRequest, (void *)newSocketfd);
            if (rc){
                std::cout << "Error:unable to create thread," << rc <<  std::endl;
                exit(-1);
            }else{
                
                
                openConnections++;
            }
        }
        
    }
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







