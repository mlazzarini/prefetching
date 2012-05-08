#include <stdlib.h>
#include <stdio.h>
#include "Request.h"
#include "Parser.h"

struct list_head request_list = LIST_HEAD_INIT(request_list);

void initReq() {
    pthread_cond_init(&empty_cond, NULL);
    pthread_cond_init(&full_cond, NULL);
    pthread_mutex_init(&empty_mutex, NULL);
    pthread_mutex_init(&full_mutex, NULL);
    n_req = 0;
}

int insertReq(request *req) {
    int ret;

    pthread_mutex_lock(&full_mutex);
    if (n_req == MAXREQ) {
        printf("insertReq: la lista delle richieste è piena.\n");
        pthread_cond_wait(&full_cond, &full_mutex);
    }

    if (n_req < MAXREQ) {
        request_elem *element;
        if ((element = (request_elem*) malloc(sizeof (request_elem))) != NULL) {
            element->req = req;
            list_add_tail(&element->next, &request_list);
            n_req++;
            ret = 1;
            pthread_cond_signal(&empty_cond);
        } else {
            ret = 0;
        }
    }
    pthread_mutex_unlock(&empty_mutex);
    return ret;
}

request *popReq() {
    request *ret;

    request_elem *elem;

    printf("popReq: stampo la lista delle richieste\n");

    list_for_each_entry(elem, &request_list, next, request_elem) {
        printf(" - %s di tipo %d", stringRequest(elem->req), elem->req->prefetch);
    }
    printf("Totale n. richieste: %d\n", n_req);


    pthread_mutex_lock(&empty_mutex);
    if (n_req == 0) {
        pthread_cond_wait(&empty_cond, &empty_mutex);
    }

    if (n_req > 0) {
        request_elem *element = container_of(list_next(&request_list), request_elem, next);
        list_del(list_next(&request_list));
        ret = element->req;
        free(element);
        n_req--;
        pthread_cond_signal(&full_cond);
    }

    pthread_mutex_unlock(&full_mutex);
    return ret;
}

char *getServer(request *req) {
    char *s = malloc(sizeof (char) *22);
    sprintf(s, "%s:%d", req->ip, req->port);
    return s;
}

