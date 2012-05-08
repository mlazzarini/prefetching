/* 
 * File:   RequestList.h
 * Author: mone
 *
 * Created on 24 febbraico 2012, 15.01
 */

#ifndef REQUESTLIST_H
#define	REQUESTLIST_H

#include <pthread.h>
#include "Consts.h"
#include "list.h"


/**********/
/*pthread_mutex_t empty_mutex;*/
pthread_cond_t empty_cond;

/*pthread_mutex_t full_mutex;*/
pthread_cond_t full_cond;

int n_req;
/***********/

pthread_mutex_t full_mutex, empty_mutex;

typedef struct request_s {
    int client_fd;
    char type[10];
    char protocol[20];
    char ip[16];
    int port;
    /* 0: È una richiesta che arriva da un client e quindi quando arriva la risposta 
          va spedita al client e messa in cache 
       1: È una richiesta REF che arriva da una dei dispatcher (prefetching) e 
          quindi va semplicemente messa in cache senza essere rispedita al client
       2: È una richiesta IDX+REF che arriva da uno dei dispatcher (prefetching) 
          e quindi va messa in cache e poi parsata per trovare eventuali altre 
          richieste REF, senza essere rispedita al client*/
    int prefetch;
    char dir[MAXLENPATH];
} request;

typedef struct request_elem_s {
    struct list_head next;
    request *req;
} request_elem;

/*
 * Da chiamare per inizializzare le trutture utili alla gestione della coda delle richieste
 */
void initReq();


/*
 * Inserisce una richiesta in coda alla lista delle richieste
 * Se la lista è piena (n_req=MAXREQ), la insert è bloccante
 * return: 1 se l'operazione va a buon fine, 0 altriemnti
 */
int insertReq(request *req);

/*
 * Estrae una richiesta dalla testa della lista delle richieste.
 * Se la lista è vuota, la pop è bloccante
 * return: la richiesta oppure NULL se l'operazione fallisce
 */
request *popReq();

/*
 * Data una richiesta restituisce una stringa che contiene server:porta che serve
 * ad indetificare il server dentro alla lista dei server
 */
char *getServer(request *req);

#endif	/* REQUESTLIST_H */

