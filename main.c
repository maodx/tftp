#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PKT_LEN 516

void error_handler(char* msg){
    perror(msg);
    exit(2);
}

//return opcode from buffer
uint16_t parse_opcode(char* buf){
    return (uint16_t)((buf[0]<<8) + buf[1]);
}

char* map_opcode(uint16_t code){
    switch (code){
        case 1:
            return "RRQ";
        case 2:
            return "WRQ";
        case 3:
            return "DATA";
        case 4:
            return "ACK";
        case 5:
            return "ERROR";
        default:
            error_handler("wrong opcode:");
    }
}

struct err_packet{
    uint16_t opcode;
    uint16_t error_code;
    char err_msg[MAX_PKT_LEN];

};

void pack_err(struct err_packet* buf, u_int16_t err_code, char* err_msg){
    buf->opcode = htons(5);
    buf->error_code = htons(err_code);
    strcpy(buf->err_msg,err_msg);
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr,cliaddr;
    int sockaddr_in_len = sizeof(cliaddr);
    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(15000);

    bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

    while(1){
        //char* buffer = (char*)calloc(sizeof(char),MAX_PKT_LEN);
        char buffer[MAX_PKT_LEN] = {};
        if(recvfrom(sockfd,buffer,MAX_PKT_LEN,0,&cliaddr,&sockaddr_in_len)<0){
            error_handler("recefrom ERROR:");
        }

        u_int16_t opcode = parse_opcode(buffer);
        char* opcode_str = map_opcode(opcode);

        int filename_length = strlen(&buffer[2]);
        char* filename = (char*)calloc(sizeof(char),filename_length);
        if(filename==NULL){
            error_handler("calloc error:");
        }
        strcpy(filename, &buffer[2]);

        int mode_length = strlen(&buffer[2+filename_length]);
        char* mode = (char*)calloc(sizeof(char),mode_length);
        if(mode==NULL){
            error_handler("calloc error:");
        }
        strcpy(mode, &buffer[2+filename_length+1]);

        char* ip_address = (char*)calloc(sizeof(char),40);
        if(ip_address==NULL){
            error_handler("calloc error:");
        }
        ip_address = inet_ntoa(cliaddr.sin_addr);
        int port_num = cliaddr.sin_port;
        printf("%s %s %s from %s:%d\n",opcode_str,filename,mode,ip_address,port_num);


        size_t err_size = sizeof(struct err_packet);
        struct err_packet *err_pk=(struct err_packet*)calloc(err_size,1);
        if(err_pk==NULL){
            error_handler("calloc error:");
        }
        pack_err(err_pk, 5,"file not found");

        if(sendto(sockfd,err_pk,err_size,0,&cliaddr,sockaddr_in_len)<0){
            error_handler("sendto ERROR:");
        }

    }

    return 0;
}