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
void handleSegFault(int n);
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

BOOL in_esecuzione = 1;

/*TODO da mettere ip come argv[1]*/
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

    if (signal(SIGSEGV, handleSegFault) == SIG_ERR) {
        perror("Errore gestore SIGSEGV\n");
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

    /* Attendo la fine dei dispatcher (che avviene solo dopo la fine del proxy)*/
    for (i = 0; i < MAXNUMTHREADWORKING; i++) {
        pthread_join(dispatchers_t[i], NULL);
        printf("pthread_join dispatcher[%d]\n", i);
    }

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
    
    int iasjfvaksucv = 0;

    while (in_esecuzione) {
        /*azzera la struttura client_sk*/
        memset(&client_sk, 0, sizeof (struct sockaddr_in));
        len = sizeof (client_sk);
        
        printf("Proxy: Attendo connessioni....\n");
        client_fd = accept(proxy_fd, (struct sockaddr *) &client_sk, &len);
        setSockReuseAddr(client_fd);
        if (client_fd < 0) {
            fprintf(stderr, "accept() error: %d\n", errno);
        } else { /* se la connect col client è andata a buon fine */
            request *req = malloc(sizeof(request));
            char req_buf[MAXLENREQ];
            int ricevuti;
            memset(req_buf,'\0',MAXLENREQ);

            if ((ricevuti = recv(client_fd, req_buf, MAXLENREQ, MSG_NOSIGNAL)) == -1) {
                fprintf(stderr, "recv() error: \n");
            } else {
                printf("Proxy: Ricevo %d byte dal client...\n", ricevuti);
                
                iasjfvaksucv ++;

                /* Controllo che non si tratti di una richiesta fittizia, in tal caso
                 esco direttamente*/
                if (!strcmp(req_buf, "close")) {
                    request *bad_req = malloc(sizeof (request));
                    memset(bad_req,'\0',sizeof(bad_req));
                    strcpy(bad_req->dir, "close");
                    insertReq(bad_req);
                    break;
                }
                
                if (iasjfvaksucv == 6) {
                    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++spacca\n");
                    handleSigTerm(0);
                }

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
    printf("Proxy: Termino la mia esecuzione...\n");
    return NULL;
}

void *requestDispatcher(void *param) {
    /* Indice del thread utile per accedere alle sue strutture dati*/
    int i = *((int*) param);
    while (in_esecuzione || n_req) {
        int s;
        char *req_buf;
        char resp_buf[MAXLENRESP];
        response *resp;
        /* Estraggo una richiesta dalla testa della lista delle richieste */
        request *req = popReq();
        
        printf("/////////////////////////////%s\n",req->dir);
        if (!strcmp(req->dir, "close")) {
            break;
        }

        /*Cerca una risorsa nella lista delle risorse del server specificato*/
        resp = getResource(req);

        if (resp == NULL) { /* la getResource non ha trovato la risorsa nella cache */
            /* quindi mi devo connettere al server */
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
                    printf("RequestDispatcher[%d]: Sono connesso al server\n", i);
                    writen(dispatcher2server_fd[i], req_buf, strlen(req_buf));
                    printf("RequestDispatcher[%d]: Ho inviato al server la richiesta di tipo:%d --> %s \n", i,req->prefetch, req_buf);
                    s = recvn(dispatcher2server_fd[i], resp_buf);
                    printf("RequestDispatcher[%d]: Ho ricevuto  risposta:%d\n", i, s);

                    if (s != -1) {
                        /* se la receive è andata a buon fine, inserisco in cache quello che ho ricevuto */
                        server_elem *serv = insertServer(getServer(req)); /*Inserisce un server nella lista dei server cachati*/
                        close(dispatcher2server_fd[i]);
                        printf("RequestDispatcher[%d]: Chiudo la connessione con il server\n", i);
                        resp = parseResponse(resp_buf);
                        /* Nel caso in cui la risposta sia NULL (il parser si è accorto di un errore), me la faccio reinviare dal server */
                        if (resp->retcode == -1) {
                            printf("\nRequestDispatcher[%d]: E R R O R E \n", i);
                            continue; /* in caso di risposta non  * ben formata oppure block incomplete, mi riconnetto al server e mi faccio 
                                       rimandare la risorsa */
                        }
                        strcpy(resp->dir, req->dir);
                        insertResource(serv, resp, req->prefetch); /* Inserisce una risorsa in cache */
                        break;
                    } else { /* la receive NON è andata a buon fine */
                        fprintf(stderr, "RequestDispatcher[%d]: Errore: %s\n", i, strerror(errno));
                    }
                }
            } /* fine del while(TRUE) */

        } else { /* Ho trovato la risorsa nella cache*/
            printf("RequestDispatcher[%d]: Ho trovato la risorsa %s nella cache:\n+++++++++++++++++++++++++++++++++++++++++\n%s\n+++++++++++++++++++++++++++++++++++++++++\n", i, req->dir, resp->block);
        }

        /* Rispondo solo se la request è di tipo 0 */
        if (req->prefetch == 0) {
            printf("RequestDispatcher[%d]: la request è di tipo 0 quindi rimando al client:\n", i);
            send(req->client_fd, resp->block, strlen(resp->block), MSG_NOSIGNAL);
            /*printf("RequestDispatcher[%d]: Ho inoltrato la risposta al client\n> > > > > > GLI MANDO STA ROBA QUA:< < < < \n%s\n", i, resp->block);*/
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
    int bad_proxy_fd;
    printf("\nChiudo il proxy:\n");
    printf(" -Smetto di ricevere connessioni...\n");
    printf(" -Aspetto di svuotare la lista delle richieste...:\n");

    /* Invio una connessione fittizia al server per chiudere far si che riesegua 
     * il while e si accorga che è terminato */
    bad_proxy_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(bad_proxy_fd, (struct sockaddr*) &proxy_sk, sizeof (proxy_sk)) == -1) {
        printf("Non riesco a inviare la richiesta fittizia al Proxy: %s\n", strerror(errno));
    } else {
        int i;
        in_esecuzione = 0;
        printf("Ho inviato la richiesta del cazzo al Proxy e quindi mi aspetto che esca per diana!\n");
        writen(bad_proxy_fd, "close", 5);
        for(i = 0; i < MAXNUMTHREADWORKING; i++) {
            request *bad_req = malloc(sizeof (request));
            memset(bad_req,'\0',sizeof(bad_req));
            strcpy(bad_req->dir, "close");
            insertReq(bad_req);
            break;
        }
    }
}

void handleSegFault(int n) {
    exitProxy();
    fflush(stdout);
}

void exitProxy() {
    int i;
    printf("\nElimino tutti i thread e chiudo il Proxy...\n");
    fflush(stdout);
    for (i = 0; i < MAXNUMTHREADWORKING; i++) {
        close(dispatcher2server_fd[i]);
        pthread_kill(dispatchers_t[i], SIGABRT);
    }
    pthread_mutex_destroy(&req_mutex);
    pthread_cond_destroy(&empty_cond);
    pthread_cond_destroy(&full_cond);
    pthread_exit(NULL);
    close(proxy_fd);
    close(client_fd);
    pthread_kill(server_t, SIGABRT);
    fflush(stdout);
}