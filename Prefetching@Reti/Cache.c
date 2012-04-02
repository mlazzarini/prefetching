#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "RequestList.h"
#include "Parser.h"
#include "Cache.h"

//http://lia.deis.unibo.it/Courses/sola0708-auto/materiale/10.thead%20linux%20(parte%202).pdf

struct list_head server_list = LIST_HEAD_INIT(server_list);
pthread_mutex_t insert_mutex, server_mutex;
pthread_cond_t cond;
BOOL modifyList = 0;
 
void initCache() {
    pthread_mutex_init(&insert_mutex, NULL);
    pthread_mutex_init(&server_mutex, NULL);
    pthread_cond_init(cond,NULL);
}

server_elem *insertServer(char *name) { 
    server_elem *s;
    
/*
    pthread_mutex_lock(&server_mutex);
*/
    
    /*controlla che il server non sia gia presente nella lista dei server*/
    list_for_each_entry(s, &server_list, next, server_elem) {
        /*Se il server è gia presente all'interno della lista dei server viene
          spostato in testa alla lista siccome è probabile che riceverà altre
          richieste e quindi quando verrà fatta la ricerca successiva verrà 
          trovato prima*/
        if(!strcmp(s->name, name)) {
            list_del(&s->next);
            list_add(&s->next, &server_list);
            return s;
        }
    }
    /*Lo aggiungiamo in testa siccome è piu probabile che venga richiesta a breve una 
      risorsa sullo stesso server*/
    if((s = malloc(sizeof(server_elem))) != (server_elem *)-1) {
        strcpy(s->name, name);
        INIT_LIST_HEAD(&s->resources);
        list_add(&s->next, &server_list);
        return s;
    }
    
/*
    pthread_mutex_unlock(&server_mutex);
*/
    
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
    if(modifyList) {
        pthread_cond_wait(cond,&insert_mutex);
    }
    pthread_mutex_unlock(&insert_mutex);
    
    /* Scorro la lista dei server in cerca di quello verso cui è indirizzata la
       richiesta*/
    list_for_each_entry(se,&server_list,next,server_elem) {
        if(!strcmp(se->name,s)) {
            resource_elem *re;
            /* Trovato il server cerco se la risorsa richiesta è gia stata cachata
               (ch = sc) in tal caso la restituisco*/
            list_for_each_entry(re,&se->resources,next,resource_elem) {
                if(!strcmp(r->dir,re->response->dir)) {
                    return re->response;
                }
            }
        }
    }
    return NULL;
}

int insertResource(server_elem *server, response* r) {
    int i=0;
    resource_elem *e; 
    /* L'elemnto a caso in piu serve per mettere NULL*/
    char ref[MAXNUMREF+1][MAXLENPATH] ;
    char idxRef[MAXNUMREF+1][MAXLENPATH];
    char buf[MAXLENPATH];
    parseRef(r->block, ref, idxRef);
    
    /* Scorro l'array delle REF e se non sono gia in cache le metto nella lista
       delle richieste*/
    while(ref[i][0] != (int)NULL) {
        request *v = malloc(sizeof(request));
        strcpy(buf,ref[i]);
        parseRequest(buf,v);
        /* Rihieste di tipo REF*/
        v->prefetch = 1;
        /* metto la richiesta nella lista delle richieste se non è gia presente 
           in chache*/
        if(getResource(v) == NULL)
            insertReq(v);
        i++;
    }
    
/*
    pthread_mutex_lock(&insert_mutex);
*/
    
    if((e = malloc(sizeof(resource_elem))) != (resource_elem *)-1) {
        e->response = r;
        list_add_tail(&e->next, &server->resources);
        (void)time(&e->startTime); 
        return 0;
    }
     
/*
    pthread_mutex_unlock(&insert_mutex); 
*/
    
    return -1;
}

/*gestione figa*/