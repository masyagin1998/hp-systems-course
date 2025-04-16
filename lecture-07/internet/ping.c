// gcc -std=c11 -O2 -Wall -Wextra ping.c -o ping
// ping 8.8.8.8
// sudo ./ping 8.8.8.8

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <errno.h>


#ifndef timersub
#define timersub(a, b, result)                 \
    do {                                       \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
        (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((result)->tv_usec < 0) {                     \
            --(result)->tv_sec;                          \
            (result)->tv_usec += 1000000;                \
        }                                              \
    } while (0)
#endif


unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    while(len > 1) {
        sum += *buf++;
        len -= 2;
    }
    if(len == 1) {
        sum += *(unsigned char*)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <IP_ADDRESS>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *target_ip = argv[1];
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (inet_aton(target_ip, &addr.sin_addr) == 0) {
        fprintf(stderr, "Invalid IP address: %s\n", target_ip);
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    pid_t pid = getpid();
    int seq = 0;
    printf("PING %s:\n", target_ip);

    while (1) {
        seq++;

        char packet[64];
        memset(packet, 0, sizeof(packet));
        struct icmphdr *icmp_hdr = (struct icmphdr *)packet;
        icmp_hdr->type = ICMP_ECHO;
        icmp_hdr->code = 0;
        icmp_hdr->un.echo.id = pid;
        icmp_hdr->un.echo.sequence = seq;

        int data_len = 32;
        memset(packet + sizeof(struct icmphdr), 0xA5, data_len);

        int packet_size = sizeof(struct icmphdr) + data_len;
        icmp_hdr->checksum = 0;
        icmp_hdr->checksum = checksum(packet, packet_size);

        struct timeval start, now, elapsed, timeout, remaining;
        gettimeofday(&start, NULL);

        if (sendto(sockfd, packet, packet_size, 0, (struct sockaddr *)&addr, sizeof(addr)) <= 0) {
            perror("sendto");
        }

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        remaining = timeout;
        int valid_reply = 0;
        int bytes_received = 0;
        char recv_buf[1024];
        struct sockaddr_in recv_addr;
        socklen_t addr_len = sizeof(recv_addr);

        while (1) {
            fd_set readset;
            FD_ZERO(&readset);
            FD_SET(sockfd, &readset);

            int rv = select(sockfd + 1, &readset, NULL, NULL, &remaining);
            if (rv < 0) {
                perror("select");
                break;
            } else if (rv == 0) {
                break;
            }

            bytes_received = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0,
                                      (struct sockaddr *)&recv_addr, &addr_len);
            if (bytes_received < 0) {
                perror("recvfrom");
                continue;
            }

            gettimeofday(&now, NULL);

            struct iphdr *ip_hdr = (struct iphdr *)recv_buf;
            int ip_hdr_len = ip_hdr->ihl * 4;
            if (bytes_received < ip_hdr_len + (int)sizeof(struct icmphdr)) {
                continue;
            }
            struct icmphdr *icmp_reply = (struct icmphdr *)(recv_buf + ip_hdr_len);

            if (icmp_reply->type == ICMP_ECHOREPLY && icmp_reply->un.echo.id == pid) {
                long rtt_ms = (now.tv_sec - start.tv_sec) * 1000 +
                              (now.tv_usec - start.tv_usec) / 1000;
                printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%ld ms\n",
                       bytes_received - ip_hdr_len, target_ip,
                       icmp_reply->un.echo.sequence, ip_hdr->ttl, rtt_ms);
                valid_reply = 1;
                break;
            }

            gettimeofday(&now, NULL);
            timersub(&now, &start, &elapsed);
            remaining.tv_sec = timeout.tv_sec - elapsed.tv_sec;
            remaining.tv_usec = timeout.tv_usec - elapsed.tv_usec;
            if (remaining.tv_usec < 0) {
                remaining.tv_sec--;
                remaining.tv_usec += 1000000;
            }
            if (remaining.tv_sec < 0)
                break;
        }

        if (!valid_reply) {
            printf("Request timed out for icmp_seq %d\n", seq);
        }

        sleep(1);
    }

    close(sockfd);
    return 0;
}