#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "RequestList.h"

struct list_head request_list = LIST_HEAD_INIT(request_list);
pthread_mutex_t req_mutex, full_mutex, empty_mutex;
int n_req;

void initReq() {
    pthread_mutex_init(&req_mutex, NULL);
    pthread_mutex_init(&full_mutex, NULL);
    pthread_mutex_init(&empty_mutex, NULL);
    pthread_mutex_lock(&empty_mutex);
    n_req = 0;
}

int insertReq(request *req) {
    int ret;
    if(n_req == MAXREQ)
        pthread_mutex_lock(&full_mutex);
    pthread_mutex_lock(&req_mutex);
    if (n_req < MAXREQ) {
        request_elem *element;
        if ((element = (request_elem*) malloc(sizeof (request_elem))) != NULL) {
            element->req = req;
            list_add_tail(&element->next, &request_list);
            n_req++;
            ret = 1;
            pthread_mutex_unlock(&empty_mutex);
        } else {
            ret = 0;
        }
    }
    pthread_mutex_unlock(&req_mutex);
    return ret;
}

request *popReq() {
    request *ret;
    if(n_req == 0)
        pthread_mutex_lock(&empty_mutex);
    pthread_mutex_lock(&req_mutex);
    if (n_req > 0) {
        request_elem *element = container_of(list_next(&request_list), request_elem, next);
        list_del(list_next(&request_list));
        ret = element->req;
        free(element);
        n_req--;
        pthread_mutex_unlock(&full_mutex);
    }
    pthread_mutex_unlock(&req_mutex);
    return ret;
}

char *getServer(request *req) {
    char *s = malloc(sizeof(char)*22);
    sprintf(s,"%s:%d",req->ip,req->port);
    return s;
}

