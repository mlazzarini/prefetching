#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Consts.h"
#include "Parser.h"

char *getIp(char* req) {
    char* res = strstr(req, "//");
    char* ip = malloc(MAX_IP_LENGHT);
    int i = 2;

    while (res[i] != ':') {
        ip[i - 2] = res[i];
        i++;
    }
    ip[i - 2] = '\0';
    return ip;
}

int getPort(char* req) {
    int i = 1;
    char* port;
    char* res = strstr(req, "//");
    res = strstr(res, ":");
    port = malloc(MAX_PORT_LENGHT);

    while (res[i] != '/') {
        port[i - 1] = res[i];
        i++;
    }
    port[i - 1] = '\0';
    return atoi(port);
}

/* prende in input una stringa (la req) e la parsa riempiendo i campi della struttura request r
 * passata per riferimento come parametro della funzione*/
BOOL parseRequest(char *req, request *r) {
    int lenreq = strlen(req);
    int res = 0;
    char format[100] = {0};
    int ip[4];
    /* richiesta completa con GET e \n\n */
    char reqComp[MAXLENREQ];

    if (!matchSubstrBool(req, "GET")) {
        sprintf(reqComp, "GET %s\n\n", req);
    } else {
        strcpy(reqComp, req);
    }

    sprintf(format, "%%9s %%19[^0-9]%%d.%%d.%%d.%%d:%%d%%%d[^\n]", MAXLENPATH);

    if ((res = sscanf(reqComp, format,
            r->type,
            r->protocol,
            &(ip[0]),
            &(ip[1]),
            &(ip[2]),
            &(ip[3]),
            &(r->port),
            &r->dir)) != 8) { /* se ha trovato piu o meno di 5 elementi ritorna FALSE in quanto la richiesta non è ben formata */
        return FALSE;
    }

    sprintf(r->ip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    if (reqComp[lenreq - 1] != '\n' || reqComp[lenreq - 2] != '\n')
        return FALSE;

    return TRUE;
}

char *stringRequest(request *r) {
    char* str_req = malloc(sizeof (char) *MAXLENREQ);
    sprintf(str_req, "%s %s%s:%d%s\n\n", r->type, r->protocol, r->ip, r->port, r->dir);
    return str_req;
}

/*Date due stringhe, ritorna 1 se la sottostringa compare nella stringa, 0 altrimenti */
int matchSubstrBool(char *str, char *sub) {
    int i, k;
    i = 0;
    k = 0;
    while (i < strlen(str)) {
        if (str[i] == sub[0]) {
            while (str[i] == sub[k]) {
                if (k == strlen(sub) - 1) {
                    return 1;
                }
                i++;
                k++;
            }
            k = 0;
        } else {
            i++;
        }
    }
    return 0;
}

/* Date due stringhe, ritorna un puntatore alla fine della sottostringa nella 
 * stringa (la prima occorrenza di sub che trova) */
char *matchSubstr(char *str, char *sub) {
    int i, k;
    if (matchSubstrBool(str, sub) == 1) {
        i = 0;
        k = 0;
        while (i < strlen(str)) {
            if (str[i] == sub[0]) {
                while (str[i] == sub[k]) {
                    if (k == strlen(sub) - 1) {
                        return &str[i + 1];
                    }
                    i++;
                    k++;
                }
                k = 0;
            } else {
                i++;
            }
        }
    } else {
        printf("Error:Non matcha, richiesta/risposta non ben formata\n");
    }
    return NULL;
}

/* Parsing del blocco ricevuto, alla ricerca di eventuali REF o IDX+REF:
 * vengono salvati nei vettori passati in input
 * Nel caso il parametro idxRef passato sia NULL, vengono cercati solo i REF 
 * contenuti nel blocco
 */
void parseRef(char *res, char refs[MAXNUMREF + 1][MAXLENPATH], char idxRefs[MAXNUMREF + 1][MAXLENPATH]) {
    int i = 0;
    int k = 0;
    char *s = malloc(MAXLENRESP * sizeof (char));
    strcpy(s, res);

    while (matchSubstrBool(s, "<REF=")) {
        s = matchSubstr(s, "<REF=");
        while (s[i] != '>') {
            refs[k][i] = s[i];
            i++;
        }
        k++;
        s = &s[i];
        i = 0;

        /* Esco se il numero massimo di ref è stato raggiunto */
        if (k == MAXNUMREF)
            break;
    }

    refs[k][0] = (int) NULL;


    if (idxRefs != NULL) {
        strcpy(s, res);
        i = 0;
        k = 0;
        /* Come si evince dalle specifiche un REF preceduto da un ; è sicuramente un 
           IDX+REF quindi prendo le risorse in esso contenute e le metto in un'altro
           array*/
        while (matchSubstrBool(s, ";REF=")) {
            s = matchSubstr(s, ";REF=");
            while (s[i] != '>') {
                idxRefs[k][i] = s[i];
                i++;
            }
            k++;
            s = &s[i];
            i = 0;

            /* Esco se il numero massimo di ref è stato raggiunto */
            if (k == MAXNUMREF)
                break;
        }
        idxRefs[k][0] = (int) NULL;
    }
}

/* Parsing della risposta: estrae da una risposta l'expire e il blocco, dopo aver
 * verificato che il blocco sia lungo LEN bytes. In caso di errore (risposta non 
 * ben formata oppure block incomplete) la funzione ritorna una response con retcode = -1,*/
response *parseResponse(char *resp_buf) {

    response *ret = malloc(sizeof (response));
    char num[4];
    char len[4];
    char exp[5];
    char *s, *data;
    int i = 0;

    for (i = 0; i < 3; i++) {
        num[i] = resp_buf[i];
    }
    num[3] = '\0';

    data = malloc(MAXLENDATA * sizeof (char));

    ret->retcode = atoi(num);
    switch (ret->retcode) {
        case 200:
            i = 0;
            s = matchSubstr(resp_buf, "Len ");
            if (s == NULL) { /* la funzione matchSubstr non ha trovato "Len" nella risposta */
                ret->retcode = -1;
                break;
            }
            while (s[i] != '\n') {
                if (s[i] == '\0') {
                    ret->retcode = -1;
                    break;
                }
                len[i] = s[i];
                i++;

            }

            i = 0;
            s = matchSubstr(resp_buf, "\n\n");
            if (s == NULL) { /* la funzione matchSubstr non ha trovato "\n\n" nella risposta */
                ret->retcode = -1;
                break;
            }
            while (i != atoi(len)) {
                if (s[i] == '\0') {
                    ret->retcode = -1;
                    break;
                }
                data[i] = s[i];
                i++;
            }

            if (strlen(data) != atoi(len)) {
                fprintf(stderr, "Block incomplete\n");
            }

            strcpy(ret->block, resp_buf);

            i = 0;
            s = matchSubstr(resp_buf, "Expire ");
            if (s == NULL) { /* la funzione matchSubstr non ha trovato "Expire" nella risposta */
                ret->retcode = -1;
                break;
            }
            while (s[i] != '\n') {
                if (s[i] == '\0') {
                    ret->retcode = -1;
                    break;
                }
                exp[i] = s[i];
                i++;
            }

            ret->expire = atoi(exp);

            break;

        case 402:
            fprintf(stderr, "Error 402: unknown error\n");
            break;
        case 403:
            fprintf(stderr, "Error 403: wrong request\n");
            break;
        case 404:
            fprintf(stderr, "Error 404: file not found\n");
            break;
        case 405:
            fprintf(stderr, "Error 405: interval not found\n");
            break;
        default:
            fprintf(stderr, "Bad response\n");
            ret->retcode = -1;
            break;
    }
    return ret;
}




