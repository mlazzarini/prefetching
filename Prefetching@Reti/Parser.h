/* 
 * File:   Parser.h
 * Author: mone
 *
 * Created on 22 febbraio 2012, 15.24
 */

#ifndef PARSER_H
#define	PARSER_H

#include "Consts.h"
#include "Request.h"
#include "Cache.h"

/* in queste definizione è compreso lo 0 terminatore della stringa*/
#define MAX_IP_LENGHT 16
#define MAX_PORT_LENGHT 6


/* 
 * Prende in input una stringa (la req) e la parsa riempiendo i campi della struttura request r
 * passata per riferimento come parametro della funzione
 */
BOOL parseRequest (char *req, request *r);

/*
 * Trasforma una request in una stringa
 */
char *stringRequest(request *r);

/*
 * Date due stringhe, ritorna 1 se la sottostringa compare nella stringa, 0 altrimenti
 */
int matchSubstrBool(char *str, char *sub);

/* 
 * Date due stringhe, ritorna un puntatore alla fine della sottostringa nella 
 * stringa (la prima occorrenza di sub che trova) 
 */
char *matchSubstr(char *str, char *sub);

/* 
 * Parsing del blocco ricevuto, alla ricerca di eventuali REF o IDX+REF:
 * vengono salvati nei vettori passati in input
 * Nel caso il parametro idxRef passato sia NULL, vengono cercati solo i REF 
 * contenuti nel blocco
 */
void parseRef(char *res, char refs[MAXNUMREF+1][MAXLENPATH], char idxRefs[MAXNUMREF+1][MAXLENPATH]);

/* 
 * Parsing della risposta: estrae da una risposta l'expire e il blocco, dopo aver
 * verificato che il blocco sia lungo LEN bytes. In caso di errore (risposta non 
 * ben formata oppure block incomplete) la funzione ritorna una response con retcode = -1,
 */
response *parseResponse(char *res_buf);


#endif	/* PARSER_H */

