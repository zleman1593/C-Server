//
//  main.cpp
//  C-Server
//
//  Created by Zackery leman & Chris Lu on 1/28/15.
//  Copyright (c) 2015 Zleman-Clu. All rights reserved.
//
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
//using namespace std;
#define NUM_THREADS     5
void *PrintHello(void *threadid)
{
    long tid;
    tid = (long)threadid;
    std::cout << "Hello World! Thread ID, " << tid <<    std::endl;
    pthread_exit(NULL);
}

int main(int argc, const char * argv[]) {
    //Prints the comamnd line arguments
   std::cout << "argc = " << argc << std::endl;
    for(int i = 0; i < argc; i++)
        std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
    
    
    // multithreading example
    pthread_t threads[NUM_THREADS];
    int rc;
    int i;
    for( i=0; i < NUM_THREADS; i++ ){
           std::cout << "main() : creating thread, " << i <<    std::endl;
        rc = pthread_create(&threads[i], NULL, PrintHello, (void *)i);
        if (rc){
               std::cout << "Error:unable to create thread," << rc <<    std::endl;
            exit(-1);
        }
    }
    pthread_exit(NULL);
    
    
    
    // socket example
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
    
    if(bind(sock_fd, (struct sockaddr*) &myaddr, sizeof(myaddr)) < 0) {
        // error opening socket
        return -1;
    }
    
    std::cout <<"opened and bound socket!\n";
    
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







