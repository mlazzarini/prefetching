#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include "Consts.h"

int setSockTimeout(int fd) {
    struct timeval timeout;
    int ret;
    timeout.tv_sec = INACTIVITY_TIMEOUT_SECONDS;
    timeout.tv_usec = 0;
    if ((ret = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const void *) &timeout, sizeof (timeout))) == -1) {
        fprintf(stderr, "setSocket error: %s\n", strerror(errno));
    }
    return ret;
}

int setSockReuseAddr(int fd) {
    /* avoid EADDRINUSE error on bind() */
    int r = 1;
    int ris = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (int *) &r, sizeof (r));
    if (ris == -1) {
        printf("setSockReuseAddr() failed, Err: %d \"%s\"\n", errno, strerror(errno));
        return (0);
    }
    return 1;
}

int recvn(int fd, char *buf) {
    int ret, nleft;
    setSockTimeout(fd);
    nleft = MAXLENRESP;

    while (nleft > 0) {
        do {
            if ((ret = read(fd, buf, MAXLENRESP)) == -1) {
                fprintf(stderr, "recvn errore %d: %s\n", errno, strerror(errno));
                if (errno == EAGAIN)
                    return -1;
            }
        } while (ret < 0);

        if (ret == 0) {
            return 0;
        }

        nleft -= ret;
        buf += ret;
    }
    return ret;
}

int writen(int fd, void *buf, int len) {
    int ret;
    setSockTimeout(fd);
    do {
        if ((ret = write(fd, buf, len)) == -1) {
            fprintf(stderr, "sendn errore: %s\n", strerror(errno));
        };
    } while (ret < 0);

    return ret;
}


