/* 
 * File:   Cache.h
 * Author: mone
 *
 * Created on 7 marzo 2012, 11.16
 */

#ifndef CACHE_H
#define	CACHE_H

#include <time.h>
#include "Consts.h"
#include "list.h"

/* Struttura per risposta, contiene le informazione necessarie a soddisfare una 
 * risposta di un client*/
typedef struct resp_s {
    int retcode;
    int expire;
    char dir[MAXLENPATH];
    /* contiene l'intera risposta,cioè tutto quello che arriva dal server (compreso Len, Expire ecc..) */
    char block[MAXLENRESP];
    /* True se la risposta è completa ovvero la lunghezza de blocco è uguale a quella dichiarata in len*/
    BOOL complete; 
} response;

/* Contiene una stringa che identifica un server e le risorse ad esso associate*/
typedef struct server_s {
    struct list_head next;
    struct list_head resources;
    char name[MAXLENPATH];
} server_elem;


/* Risorsa della cache*/
typedef struct resource_s {
    struct list_head next;
    time_t startTime;
    response *response;
} resource_elem;

/*
 * Inizializza i mutex che gestiscono la concorrenza nella Cache
 */
void initCache();

/* 
 * Inserisce un server nella lista dei server cachati (ch = sci), se il server è 
 * gia presente nella lista viene spostato in testa per motivi di efficenza.
 * return l'indirizzo del server oppure NULL se qualcosa va storto
 */
server_elem *insertServer(char *name);

/*
 * Cerca una risorsa nella lista delle risorse del server specificato, se la 
 * trova la restituisce altrimenti restituisce NULL. NULL può anche esssere
 * restituito nel caso in cui il tempo della risorsa sia scaduto
 */
response *getResource(request *r);

/* 
 * Inserisce una risorsa nella lista dei server cachati (ch = sci). Viene passato 
 * il flag che indica di che tipo era la richiesta (see also Request.h). Se è di 
 * tipo 0 oppure 2 viene parsata alla ricerca ripettivamente di REF e IDX+REF 
 * oppure solo di REF. Nel caso in cui vengano trovati vengono create le richieste
 * corrispondenti che verranno soddisftte dai dispatcher e messe in cache.
 * 
 * return 1 se l'inserimento va a buon fine
 *       -1 in caso di errore 
 */
int insertResource(server_elem *server, response* r, int prefetch_flag);



#endif	/* CACHE_H */

