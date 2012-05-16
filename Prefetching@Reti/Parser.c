#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Consts.h"
#include "Parser.h"

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

    /* se ha trovato piu o meno di 8 elementi ritorna FALSE in quanto la richiesta
     * non è ben formata */
    if ((res = sscanf(reqComp, format,
            r->type,
            r->protocol,
            &(ip[0]),
            &(ip[1]),
            &(ip[2]),
            &(ip[3]),
            &(r->port),
            &r->dir)) != 8) {
        return FALSE;
    }

    sprintf(r->ip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    /* Se manca \n\n allora la richista non è ben formata*/
    if (reqComp[lenreq - 1] != '\n' || reqComp[lenreq - 2] != '\n')
        return FALSE;

    return TRUE;
}

char *stringRequest(request *r) {
    char* str_req = malloc(sizeof (char) *MAXLENREQ);
    sprintf(str_req, "%s %s%s:%d%s\n\n", r->type, r->protocol, r->ip, r->port, r->dir);
    return str_req;
}

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
            /* Parso la Len */
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
            len[i]='\0';

            /* Parso l'expire time */

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

            /* Parso il blocco dati... qualunque sia la sua dimensione lo invio cmq al client che 
               reagirà di conseguenza.*/

            i = 0;
            s = matchSubstr(resp_buf, "\n\n");
            if (s == NULL) { /* la funzione matchSubstr non ha trovato "\n\n" nella risposta */
                ret->retcode = -1;
                break;
            }
            while (i != atoi(len)) {
                if (s[i] == '\0') {
                    break;
                }
                data[i] = s[i];
                i++;
            }
            
            if (i != atoi(len))
                ret->complete = FALSE;
            else
                ret->complete = TRUE;
            printf("----------- ret->complete = %d\ti = %d\t len %d\n",ret->complete,i,atoi(len));
            strcpy(ret->block, resp_buf);

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




