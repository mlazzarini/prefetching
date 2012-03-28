/* 
 * File:   Parser.h
 * Author: mone
 *
 * Created on 22 febbraio 2012, 15.24
 */

#ifndef PARSER_H
#define	PARSER_H

#include "Consts.h"
#include "RequestList.h"
#include "Cache.h"

/* in queste definizione è compreso lo 0 terminatore della stringa*/
#define MAX_IP_LENGHT 16
#define MAX_PORT_LENGHT 6


char *getIp(char* req);
int getPort(char* req);


BOOL parseRequest (char *req, request *r);
char *stringRequest(request *r);

void estraiRefString(char *req, char *refs[]);
char *matchSubstr(char *str, char *sub);
int matchSubstrBool(char *str, char *sub);
response *parseResponse(char *res_buf);
void parseRef(char *res, char refs[MAXNUMREF+1][MAXLENPATH], char idxRefs[MAXNUMREF+1][MAXLENPATH]);

#endif	/* PARSER_H */

