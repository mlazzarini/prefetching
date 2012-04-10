/* 
 * File:   Consts.h
 * Author: mone
 *
 * Created on 22 febbraio 2012, 16.55
 */

#ifndef CONSTS_H
#define	CONSTS_H

#define MAXLENPATH 2048
#define MAXLENREQ ((MAXLENPATH)+1024)
#define MAXLENDATA 5000
#define MAXLENRESP ((MAXLENREQ)+MAXLENDATA)
#define GET 1
#define INF 2
#define MAXNUMTHREADWORKING 1
#define MAXREQ 100
#define INACTIVITY_TIMEOUT_SECONDS 10
#define NMAXDOCS 10
#define MAXNUMREF 10

/* boolean */
#define TRUE 1
#define FALSE 0

typedef int BOOL;

#endif	/* CONSTS_H */

