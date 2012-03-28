/* 
 * File:   IOUtil.h
 * Author: mone
 *
 * Created on 2 marzo 2012, 15.04
 */

#ifndef IOUTIL_H
#define	IOUTIL_H

int recvn(int fd, void *buf);
int writen(int fd, void *buf, int len);
int setSockReuseAddr(int fd);

#endif	/* IOUTIL_H */

