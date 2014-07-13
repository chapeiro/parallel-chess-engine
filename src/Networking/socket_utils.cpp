#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include "socket_utils.hpp"


int outsck = -1;
int insck  = -1;

void dup2sock(int argc, char* argv[]){
    int m       = -1;
    int inport  = 54870;
    int outport = 54871;
    for (int i = 0 ; i < argc ; ++i){
        if (strncmp(argv[i], "-m=", 3) == 0){
            m = i;
        }
        if (strncmp(argv[i], "-inp=", 5) == 0){
            int t = atoi(argv[i]+5);
            if (t) inport = t;
        }
        if (strncmp(argv[i], "-outp=", 6) == 0){
            int t = atoi(argv[i]+6);
            if (t) outport = t;
        }
    }
    if (m == -1) return;
    duplicate2socket(argv[m]+3, outport, inport);
}

void duplicate2socket(char * extrn_addr, int outport, int inport){
    int loutsck = connect(extrn_addr, outport);
    if (loutsck < 0) return;
    int linsck  = connect(extrn_addr,  inport);
    if (linsck  < 0){
        close(loutsck);
        return;
    }
    std::cout << "Duping fds..." << std::endl;
    if (dup2(loutsck , STDOUT_FILENO) == -1 || dup2(loutsck , STDERR_FILENO) == -1 || dup2(linsck , STDIN_FILENO) == -1){
        close(loutsck);
        close(linsck );
        std::cerr << "Failed to redirect to sockets" << std::endl;
    }
    outsck = loutsck;
    insck  = linsck;
}

int connect(char * extrn_addr, int port){
    int sck = socket(AF_INET, SOCK_STREAM, 0);              //create socket
    if (sck == -1){
        std::cerr << "socket_mngmt:failed to create socket";
        std::cerr << strerror(errno) << std::endl;
        return -1;
    }

    int sopt = 1;                       //set option to reuse address and port
    if (setsockopt(sck, SOL_SOCKET, SO_REUSEADDR, &sopt, sizeof(sopt))){
        std::clog << "socket_mngmt:option: " << strerror(errno) << std::endl;
    }
    if (setsockopt(sck, SOL_SOCKET, SO_REUSEPORT, &sopt, sizeof(sopt))){
        std::clog << "socket_mngmt:option: " << strerror(errno) << std::endl;
    }
    
    //create address data for incoming connection
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr)); //initialize to zeros
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = inet_addr(extrn_addr);

    std::cout << std::string(extrn_addr) << std::endl;
    std::cout << inet_addr(extrn_addr) << std::endl;
    if (connect(sck, (struct sockaddr *) &addr, sizeof(sockaddr_in)) < 0){
        std::cerr << "socket_mngmt:failed to connect (";
        std::cerr << strerror(errno) << ")" << std::endl;
        return -2;
    }

    return sck;
}

void closeSockets(){
    if (outsck >= 0) close(outsck);
    if (insck  >= 0) close(insck);
}