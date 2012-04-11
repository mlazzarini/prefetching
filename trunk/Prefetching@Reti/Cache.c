#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "Request.h"
#include "Parser.h"
#include "Cache.h"

struct list_head server_list = LIST_HEAD_INIT(server_list);
pthread_mutex_t insert_mutex, get_mutex, server_mutex;
pthread_cond_t readCond, writeCond;

void initCache() {
    pthread_mutex_init(&insert_mutex, NULL);
    pthread_mutex_init(&server_mutex, NULL);
    pthread_mutex_init(&get_mutex, NULL);
}

server_elem *insertServer(char *name) {
    server_elem *s;
    pthread_mutex_lock(&server_mutex);

    /*controlla che il server non sia gia presente nella lista dei server*/
    list_for_each_entry(s, &server_list, next, server_elem) {
        /*Se il server è gia presente all'interno della lista dei server viene
          spostato in testa alla lista siccome è probabile che riceverà altre
          richieste e quindi quando verrà fatta la ricerca successiva verrà 
          trovato prima*/
        if (!strcmp(s->name, name)) {
            list_del(&s->next);
            list_add(&s->next, &server_list);
            pthread_mutex_unlock(&server_mutex);
            return s;
        }
    }
    /*Lo aggiungiamo in testa siccome è piu probabile che venga richiesta a breve una 
      risorsa sullo stesso server*/
    if ((s = malloc(sizeof (server_elem))) != (server_elem *) - 1) {
        strcpy(s->name, name);
        INIT_LIST_HEAD(&s->resources);
        list_add(&s->next, &server_list);
        pthread_mutex_unlock(&server_mutex);
        return s;
    }

    pthread_mutex_unlock(&server_mutex);

    return NULL;
}

/*
 * Cerca una risorsa nella lista delle risorse del server specificato, se la 
 * trova la restituisce altrimenti restituisce NULL. NULL può anche esssere
 * restituito nel caso in cui il tempo della risorsa sia scaduto
 */
response *getResource(request *r) {
    char *s = getServer(r);
    server_elem *se;
    /*se il server non era gia nella lista viene inserito altrimenti viene spostato
      in testa*/
    insertServer(s);

    pthread_mutex_lock(&insert_mutex);

    /* Scorro la lista dei server in cerca di quello verso cui è indirizzata la
       richiesta*/
    list_for_each_entry(se, &server_list, next, server_elem) {
        if (!strcmp(se->name, s)) {
            resource_elem *re;

            /* Trovato il server cerco se la risorsa richiesta è gia stata cachata
               (ch = sc) in tal caso la restituisco*/
            list_for_each_entry(re, &se->resources, next, resource_elem) {
                if (!strcmp(r->dir, re->response->dir)) {
                    time_t current_time;
                    (void) time(&current_time);

                    /* controllo se la risorsa è scaduta: nel caso, la elimino dalla cache e ritorno NULL */
                    if (current_time - re->startTime >= re->response->expire) {
                        printf("La risorsa %s è stata cancellata dalla cache siccome sono passati %d secondi.\n", r->dir, re->response->expire);
                        list_del(&re->next);
                        pthread_mutex_unlock(&insert_mutex);
                        return NULL;
                    }


                    pthread_mutex_unlock(&insert_mutex);
                    return re->response;
                }
            }
        }
    }

    pthread_mutex_unlock(&insert_mutex);

    return NULL;
}

int insertResource(server_elem *server, response* r, int prefetch_flag) {
    int i = 0;
    resource_elem *e;
    /* L'elemnto a caso in piu serve per mettere NULL*/
    char ref[MAXNUMREF + 1][MAXLENPATH];
    char idxRef[MAXNUMREF + 1][MAXLENPATH];
    char buf[MAXLENPATH];

    /* il blocco va parsato solo se la richiesta arriva dal client OPPURE se è di tipo 2 */
    if (prefetch_flag == 0 || prefetch_flag == 2) {
        parseRef(r->block, ref, (prefetch_flag == 0) ? idxRef : NULL);

        /* Scorro l'array delle REF e se non sono gia in cache le metto nella lista
           delle richieste*/
        while (ref[i][0] != (int) NULL) {
            request *v = malloc(sizeof (request));
            strcpy(buf, ref[i]);
            parseRequest(buf, v);
            /* Rihieste di tipo REF*/
            v->prefetch = 1;
            /* metto la richiesta nella lista delle richieste se non è gia presente 
               in cache*/
            if (getResource(v) == NULL) {
                insertReq(v);
                printf("Cache: Ho inserito la richiesta REF: %s\n", stringRequest(v));
            }
            i++;
        }

        /* il blocco va parsato alla ricerca di IDX+REF solo se la richiesta e di tipo 0 */
        if (prefetch_flag != 2) {
            i = 0;
            memset(buf, '\0', MAXLENPATH);
            /* Scorro l'array delle IDX+REF e se non sono gia in cache le metto nella lista
               delle richieste*/
            while (idxRef[i][0] != (int) NULL) {
                request *v = malloc(sizeof (request));
                strcpy(buf, idxRef[i]);
                parseRequest(buf, v);
                /* Rihieste di tipo IDX+REF*/
                v->prefetch = 2;
                /* metto la richiesta nella lista delle richieste se non è gia presente 
                   in cache*/
                if (getResource(v) == NULL) {
                    insertReq(v);
                    printf("Cache: Ho inserito la richiesta IDX: %s\n", stringRequest(v));
                }
                i++;
            }
        }
    }

    pthread_mutex_lock(&insert_mutex);

    if ((e = malloc(sizeof (resource_elem))) != (resource_elem *) - 1) {
        e->response = r;
        list_add_tail(&e->next, &server->resources);
        (void) time(&e->startTime);
        pthread_mutex_unlock(&insert_mutex);
        return 0;
    }
    pthread_mutex_unlock(&insert_mutex);
    return -1;
}

/*gestione figa*/