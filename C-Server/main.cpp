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
#define DEFAULT_PORT 8887
#define NUM_THREADS     5
#define MAX_BACKLOG     10
#define THREAD_TIMEOUT  60
int openConnections = 0;
timeval threadTimeout;

struct argStruct{
    int newSocketfd;
    char root[];
};


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void *handelRequest(void *inputStruct)
{
    struct argStruct *inputArg = (struct argStruct *)inputStruct;
    long sock = (long)inputArg->newSocketfd;
    std::cout << "Handeling Request! Socket Descriptor: "  << sock <<    std::endl;
    int requestNum = 0;
    char buffer[2000];
    bzero(buffer,2000);
    bool is11 = true;
    int n;
    char* requestType = (char*) malloc(2000);
    char* urlstr = (char*) malloc(2000);
    char* httpstr = (char*) malloc(2000);
    char* filetype = (char*) malloc(2000);
    char* fullPath = (char*) malloc(2000);
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(sock, &readset);
    threadTimeout.tv_sec = 60/openConnections;
    threadTimeout.tv_usec = 0;
    while (is11) {
        
        strcpy(fullPath, inputArg->root);
        int num = select(sock+1, &readset, NULL, NULL, &threadTimeout);
        if(num >0)
        {//data present to read
            requestNum++;
            n = read(sock,buffer,2000);
            if (n > 0) {
                memset(requestType, 0, 2000);
                memset(urlstr, 0, 2000);
                memset(httpstr, 0, 2000);
                memset(filetype, 0, 2000);
            }
        }
        else
        {//thread has reached timeout. Kill
             std::cout << "Closing Connection due to Timeout" << std::endl;
            pthread_exit(NULL);
            
        }

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
            int it = 0;
            while (isspace(buffer[start]) == 0) {
                urlstr[it] = buffer[start];
                it++;
                start++;
            }
            //variable holding file type
            filetype = urlstr;
            if ((filetype = strchr(filetype, '.')) == NULL) {
                //interpret / as index.html
                strcpy(filetype, ".html");
                strcpy(urlstr, "/index.html");
            }
            i = start;
            //MAKE SURE WE CHECK URL PATH FITS PATH PATTERN
            while (isspace(buffer[i])) {
                i++;
            }
            //get HTTP type
            it = 0;
            while (isspace(buffer[i]) == 0) {
                httpstr[it] = buffer[i];
                it++;
                i++;
            }

            //determine HTTP version
            if (strcmp(httpstr, "HTTP/1.0") == 0 || strcmp(httpstr, "HTTP/1.1") == 0) {
                if(strcmp(httpstr, "HTTP/1.0") == 0){
                    is11 = false;
                    openConnections--;
                }
                //HTTP request ok
                
                strcat(fullPath,urlstr);
                puts(fullPath);
                
                FILE *fs = fopen(fullPath, "r");

                if (fs == NULL) {
                    //could be 404, 403, 401
                    if (errno == EACCES){
                        std::cout << "Permission denied" << std::endl;
                        send(sock, "403: Permission denied", 16, 0);
                        continue;
                        
                    } else{
                        std::cout << "404: Not Found" << std::endl;
                        send(sock, "404: Not Found", 16, 0);
                        continue;
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
                
                char *temp = (char *)malloc(contents.size() + 1);
                if (temp == NULL) {
                    std::cout << "variable not given memory" << std::endl;
                }
                memcpy(temp, contents.c_str(), contents.size() + 1);
                
                std::cout << "copies to temps" << std::endl;
                //if (!strcmp(newHTTP, "HTTP/1.1")) {
                    //Send HTTP status to 1.1 client
                    send(sock, "HTTP/1.1 200 Ok\r\n", 18, 0);
                //}

                //Send Date
                char timebuf [80];
                time_t currentTime;
                time(&currentTime);
                struct tm * timeinfo = localtime(&currentTime);
                strftime (timebuf,80,"Date: %c \r\n",timeinfo);
                send(sock, timebuf, 35, 0);

                if (!strcmp(filetype, ".html")) {
                    send(sock, "content-type: text/html\r\n", 26, 0);
                }
                else if (!strcmp(filetype, ".pdf"))
                {
                    send(sock, "content-type: application/pdf\r\n", 32, 0);
                }
                else if (!strcmp(filetype, ".png")) {
                    send(sock, "content-type: image/png\r\n", 26, 0);
                }  else if (!strcmp(filetype, ".jpg") || !strcmp(filetype, ".jpeg")) {
                    send(sock, "content-type: image/jpeg\r\n", 27, 0);
                }   else if (!strcmp(filetype, ".gif")) {
                    send(sock, "content-type: image/gif\r\n", 26, 0);
                } else if (!strcmp(filetype, ".txt")) {
                    send(sock, "content-type: text/plain\r\n", 27, 0);
                }

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
                
                send(sock, temp2,(20+length), 0);
                
                //Send Body
                send(sock, temp, lSize, 0);
                
            }
            else
            {
                //incorrect HTTP version call
                std::cout << "400: Bad Request" << std::endl;
                send(sock, "400: Bad Request", 16, 0);
                continue;
            }
        }
        else
        {
            //no valid request (400)
            std::cout << "400: Bad Request" << std::endl;
            send(sock, "400: Bad Request", 16, 0);
            if (requestNum) {
                //clear buffer
                memset(buffer, 0, 2000);
            }
            else
            {
                openConnections--;
                 std::cout << "Closing Connection" << std::endl;
                pthread_exit(NULL);
            }
        }
        
        if (n < 0) error("ERROR reading from socket");
        //printf("Here is the message: %s\n",buffer);
        
        
        
    }
    openConnections--;
     std::cout << "Closing Connection" << std::endl;
    pthread_exit(NULL);
}


int main(int argc, const char * argv[]) {
    
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
    char root[strlen(argv[1])];
    if (argc == 5) {
        strcpy(root, argv[2]);
        if (strcmp(argv[3], "-port") == 0) {
            myaddr.sin_port = htons(atoi(argv[4]));
            std::cout << myaddr.sin_port << std::endl;
        }
        else
        {
            std::cout << "port option not specificied" << std::endl;
            return 0;
        }
    }
    else{
        std::cout << "Not enough arguments. Pass home directory path and port number" << std::endl;
        return 0;
        //myaddr.sin_port = htons(DEFAULT_PORT); // use port default of 8888
    }
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
        int newSocketfd = accept(sock_fd, (struct sockaddr*) &myaddr, (socklen_t *) size );
        
        if( newSocketfd < 0)
        {
            std::cout << "Error while accepting" << std::endl;
            return -1;
        } else{
            pthread_t newThread;
            std::cout << "Creating  new thread " <<    std::endl;
            struct argStruct *inputStruct = new argStruct;
            inputStruct->newSocketfd = newSocketfd;
            strcpy(inputStruct->root, root);
            int rc = pthread_create(&newThread, NULL, handelRequest, (void *)inputStruct);
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







