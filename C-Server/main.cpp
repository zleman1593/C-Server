//
//  main.cpp
//  C-Server
//
//  Created by Zackery leman on 1/28/15.
//  Copyright (c) 2015 Zleman-Clu. All rights reserved.
//
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>

int main(int argc, const char * argv[]) {
  
    // socket example
    
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("error opening socket\n");
        return -1;
    }
    
    struct sockaddr_in myaddr;
    myaddr.sin_port = htons(8888); // use port 8888
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(sock_fd, (struct sockaddr*) &myaddr, sizeof(myaddr)) < 0) {
        // error opening socket
        return -1;
    }
    
    std::cout <<"opened and bound socket!\n";
    
    return 0;
}
