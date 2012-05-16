#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <bits/sigthread.h>
#include "IOUtil.h"
#include "Parser.h"
#include "Request.h"
#include "Consts.h"
#include "Util.h"
#include "Cache.h"

#define IP_ADDR "127.0.0.1"
#define PORT 3307

void *proxy(void *param);
void *requestDispatcher(void* param);
void handleSigTerm(int n);
void exitProxy();

pthread_t server_t;
pthread_t dispatchers_t[MAXNUMTHREADWORKING];

/***************** Variabili del PROXY ********************/

/* Se non specificate, vengono usate quelle di default (127.0.0.1 e 3307) */
char *ip_addr;
int port;

/*indirizzo e porta sul quale il proxy sta in ascolto*/
struct sockaddr_in proxy_sk;
/*indirizzo che viene riempito quando si riceve una connessione dal client */
struct sockaddr_in client_sk;

/* Socket del proxy*/
int proxy_fd;
/* Socket del client*/
int client_fd;

/***************** Variabili del DISPATCHER *********/

/*Array di socket che connettono il dispatcher al server*/
int dispatcher2server_fd[MAXNUMTHREADWORKING];
/*Array di sockaddr_in del server sui quali vengono inoltrate le richieste dai dispatcher*/
struct sockaddr_in server_sk[MAXNUMTHREADWORKING];

pthread_t thread_name[MAXNUMTHREADWORKING];

int main(int argc, char* argv[]) {
    int i;

    if (argc - 1 == 2) {
        ip_addr = argv[1];
        port = atoi(argv[2]);
    } else if (argc - 1 > 2) {
        printf("Usage: ./Proxy.exe IP port OR just ./Proxy.exe to use default params");
    } else {
        ip_addr = IP_ADDR;
        port = PORT;
    }

    /* Registro il segnale che mi uccide tutti i thread quando viene premuto ctrl+c 
       per evitare di lasciare connessioni aperte*/
    if (signal(SIGINT, handleSigTerm) == SIG_ERR) {
        perror("Errore gestore SIGUSR1\n");
        exit(1);
    }

    initReq();
    initCache();

    /* Creo il thread proxy*/
    pthread_create(&server_t, NULL, proxy, NULL);

    /* Creo i thread dispatcher che si occupano di gestire le richieste*/
    for (i = 0; i < MAXNUMTHREADWORKING; i++) {
        int *param = malloc(sizeof (int));
        (*param) = i;
        pthread_create(&dispatchers_t[i], NULL, requestDispatcher, (void *) param);
    }

    /* Attendo la fine del proxy*/
    pthread_join(server_t, NULL);
    printf("pthread_join proxy\n");

    exit(0);
}

void *proxy(void *param) {
    socklen_t len;

    proxy_fd = socket(AF_INET, SOCK_STREAM, 0);
    setSockReuseAddr(proxy_fd);

    proxy_sk.sin_port = htons(port);
    proxy_sk.sin_addr.s_addr = inet_addr(ip_addr);
    proxy_sk.sin_family = AF_INET;

    /*collega il socket a un indirizzo locale*/
    bind(proxy_fd, (struct sockaddr*) &proxy_sk, sizeof (proxy_sk));

    /*Fa passare il socket dallo stato CLOSED allo stato LISTEN*/
    listen(proxy_fd, 1000);

    while (1) {
        /*azzera la struttura*/
        memset(&client_sk, 0, sizeof (struct sockaddr_in));
        len = sizeof (client_sk);

        printf("Proxy: Attendo connessioni....\n");
        client_fd = accept(proxy_fd, (struct sockaddr *) &client_sk, &len);
        setSockReuseAddr(client_fd);
        if (client_fd < 0) {
            fprintf(stderr, "accept() error: %d\n", errno);
        } else { /* se la connect con il client è andata a buon fine */
            request *req = malloc(sizeof (request));
            char req_buf[MAXLENREQ];
            memset(req_buf, '\0', MAXLENREQ);

            if (recv(client_fd, req_buf, MAXLENREQ, MSG_NOSIGNAL) == -1) {
                fprintf(stderr, "recv() error: \n");
            } else {
                printf("Proxy: Ricevo dal client...\n");

                /* parso la richiesta arrivata dal client*/
                parseRequest(req_buf, req);
                req->client_fd = client_fd;
                /* La richiesta arriva dal client il quale esige una risposta */
                req->prefetch = 0;
                /* Inserisce la richiesta nella lista delle richieste*/
                insertReq(req);
                printf("Proxy: Ho inserito la richiesta: %s\n", stringRequest(req));
            }
        }
    }
    return NULL;
}

void *requestDispatcher(void *param) {
    /* Indice del thread*/
    int i = *((int*) param);
    thread_name[i] = pthread_self();
    while (1) {
        int s;
        char *req_buf;
        char resp_buf[MAXLENRESP];
        response *resp;
        /* Estraggo una richiesta dalla testa della lista delle richieste */
        request *req = popReq();

        /*Cerca la risorsa in cache*/
        resp = getResource(req);

        /* la getResource non ha trovato la risorsa nella cache
           quindi mi devo connettere al server */
        if (resp == NULL) {
            memset(&server_sk[i], 0, sizeof (struct sockaddr_in));
            memset(resp_buf, '\0', MAXLENRESP);
            server_sk[i].sin_addr.s_addr = inet_addr(req->ip);
            server_sk[i].sin_port = htons(req->port);
            server_sk[i].sin_family = AF_INET;
            req_buf = stringRequest(req);
            printf("RequestDispatcher[%d]: Tento di soddisfare la richiesta:%s\n", i, req_buf);

            while (TRUE) {
                dispatcher2server_fd[i] = socket(AF_INET, SOCK_STREAM, 0);
                setSockReuseAddr(dispatcher2server_fd[i]);

                if (connect(dispatcher2server_fd[i], (struct sockaddr *) &server_sk[i], sizeof (struct sockaddr_in)) == -1) {
                    fprintf(stderr, "RequestDispatcher[%d]: connect() error: %s\n", i, strerror(errno));
                } else {
                    writen(dispatcher2server_fd[i], req_buf, strlen(req_buf));
                    printf("RequestDispatcher[%d]: Ho inviato al server la richiesta di tipo:%d --> %s \n", i, req->prefetch, req_buf);
                    s = readn(dispatcher2server_fd[i], resp_buf);
                    printf("RequestDispatcher[%d]: Ho ricevuto  risposta:%d \n _______________________ %s \n________________________\n", i, s,resp_buf);

                    if (s != -1) {
                        /* se la receive è andata a buon fine, inserisco in cache quello che ho ricevuto */
                        server_elem *serv = insertServer(getServer(req)); /*Inserisce un server nella lista dei server cachati*/
                        close(dispatcher2server_fd[i]);
                        printf("RequestDispatcher[%d]: Chiudo la connessione con il server\n", i);
                        resp = parseResponse(resp_buf);
                        /* Nel caso in cui la risposta sia NULL (il parser si è accorto di un errore), me la faccio reinviare dal server */
                        if (resp->retcode == -1) {
                            printf("RequestDispatcher[%d]: E R R O R E \n", i);
                            continue;
                        }
                        strcpy(resp->dir, req->dir);
                        /* Se i dati contenuti nel blocco hanno la stessa dimensione di len allora parso la risposta
                            e la inserisco in cache.. altrimenti rischio segmentation FAULT*/
                        if(resp->complete == TRUE)
                            insertResource(serv, resp, req->prefetch);
                        break;
                    } else { /* la receive NON è andata a buon fine */
                        fprintf(stderr, "RequestDispatcher[%d]: Errore: %s\n", i, strerror(errno));
                    }
                }
            } /* fine del while(TRUE) */

        } else { /* Ho trovato la risorsa nella cache*/
            printf("RequestDispatcher[%d]: Ho trovato la risorsa %s nella cache\n", i, req->dir);
        }

        /* Rispondo solo se la request è di tipo 0 */
        if (req->prefetch == 0) {
            printf("RequestDispatcher[%d]: la request è di tipo 0 quindi rimando al client:\n", i);
            send(req->client_fd, resp->block, strlen(resp->block), MSG_NOSIGNAL);
            printf("RequestDispatcher[%d]: Ho inoltrato la risposta al client\n", i);
            close(req->client_fd);
            printf("RequestDispatcher[%d]: Chiudo la connessione con il client\n", i);
        }
    }
    printf("RequestDispatcher[%d]: Termino la mia esecuzione...\n", i);
    return NULL;
}

/*
 * Termina tutti i thread in esecuzione e chiude tutte le connessioni attive
 */
void handleSigTerm(int n) {
    int i;
    printf("\nElimino tutti i thread e chiudo il Proxy...\n");
    fflush(stdout);
    for (i = 0; i < MAXNUMTHREADWORKING; i++) {
        close(dispatcher2server_fd[i]);
        pthread_kill(dispatchers_t[i], SIGABRT);
    }
    pthread_mutex_destroy(&full_mutex);
    pthread_cond_destroy(&empty_cond);
    pthread_cond_destroy(&full_cond);
    pthread_exit(NULL);
    close(proxy_fd);
    close(client_fd);
    pthread_kill(server_t, SIGABRT);
    fflush(stdout);
    exit(0);
}

