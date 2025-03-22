#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MULTICAST_GROUP "239.0.0.1"
#define PORT 12345

int main() {
    int client_fd;
    struct sockaddr_in multicast_addr;
    struct ip_mreq multicast;
    char buffer[1024];
    int reuse = 1;

    // creating clientudp socket
    if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // enalbe SO_REUSEADDR to allow multiple clients
    if (setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) 
    {
        perror("setsockopt SO_REUSEADDR");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // bind on INADDR_ANY
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_port = htons(PORT);
    multicast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(client_fd, (struct sockaddr *)&multicast_addr, sizeof(multicast_addr)) < 0) 
    {
        perror("bind");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // add socket to multicast group using multicast group address
    multicast.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    multicast.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(client_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast, sizeof(multicast)) < 0) 
    {
        perror("setsockopt IP_ADD_MEMBERSHIP failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // receiving datas
    while (1) 
    {
        int len = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (len < 0) 
        {
            perror("Receive");
            break;
        }
        buffer[len] = '\0';  // null terminate string
        printf("Received: %s\n", buffer);
    }

    // removing the client to multicast group
    if (setsockopt(client_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &multicast, sizeof(multicast)) < 0) 
    {
        perror("setsockopt IP_DROP_MEMBERSHIP");
    }

    close(client_fd);
    return 0;
}
