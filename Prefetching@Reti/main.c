#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAXLENPATH 2048

/*prototipi funzioni*/
int request_parse (char *request);
char* my_strstr (char* str, char* sub);
char* my_strcpy (char* dest, char* src);

/*funzione prende intero e puntatore a stringa (**argv)*/ 
int main (int argc, char *argv[]) { 
    char request1[]="GET mhttp://130.136.2.23:55554/dir/a.txt\n\n";
    if (request_parse (request1)== 1){
        printf ("1");
    }
    else printf ("0");
    
    
    
    return 0;
}

/*funzione request_parse, prende in input una stringa (la req)
 dichiaro come stringa "GET mhttp://" e come stringa res, il risultato
 della my_strstr, dichiaro come int, il primo intero, ovvero il primo numero
 dell'indirizzo IP con l'atoi che parte dall'indirizzo di res+la lunghezza di
 httpreq0. Dichiaro anche gli altri 3 interi dell'indirizzo ip (ip2,ip3,ip4) e int port (per la porta).
 Temp indica il punto in cui sono nella stringa, che va modificato di volta in volta.*/

int request_parse (char *request) 
{
    char *httpreq0="GET mhttp://";
    char *res = NULL;
    char *temp = NULL;
    int ip1;    
    int ip2;
    int ip3;
    int ip4;
    int port;
    char dir[MAXLENPATH];

/* se ip1 è maggiore di 99 allora è a 3 cifre, quindi ip2 sarà all'indirizzo
di res + lunghezza di httpreq0 + 4 (il punto compreso), se ip1 è minore di 100 e maggiore di 9,
ovviamente aggiungiamo 3 al posto di 4, e se ip1 < 10 aggiungiamo solo 2.*/

    res = my_strstr(request, httpreq0);

    ip1 = atoi(res + strlen(httpreq0));

    temp = res + strlen (httpreq0);
/*in posizione temp-1 deve esserci un punto.*/
    if(temp-1 != ".")
        return 0;

    if (ip1 > 99) {
        ip2 = atoi (res + strlen (httpreq0) + 4);
        temp += 4;
    }
    else if (ip1 < 100 && ip1 > 9) {
        ip2 = atoi (res + strlen (httpreq0) + 3);
        temp += 3;
    }
    else if (ip1 < 10) {
        ip2 = atoi ( res + strlen (httpreq0) + 2);
        temp += 2;
    }

    if(temp-1 != ".")
        return 0;

/* la stessa cosa la facciamo per ip3, sempre aggiornando il valore di temp*/

    if (ip2 > 99) {
        ip3 = atoi (temp + 4);
        temp += 4;
    }
    else if (ip2 < 100 && ip2 > 9) {
        ip3 = atoi (temp + 3);
        temp += 3;
    }
    else if (ip2 < 10) {
        ip3 = atoi (temp + 2);
        temp += 2;
    }
   
    if(temp-1 != ".")
        return 0;
/* la stessa cosa la facciamo per ip4, sempre aggiornando il valore di temp*/
    if (ip3 > 99) {
        ip4 = atoi (temp + 4);
        temp += 4;
    }
    else if (ip3 < 100 && ip2 > 9) {
        ip4 = atoi (temp + 3);
        temp += 3;
    }
    else if (ip3 < 10) {
        ip4 = atoi (temp + 2);
        temp += 2;
    }

    if(temp-1 != ".")
        return 0;
/* la stessa cosa la facciamo per la porta,dato che è sempre un intero, sempre aggiornando il valore di temp*/
    if (ip4 > 99) {
        port = atoi (temp + 4);
        temp += 4;
    }

    else if (ip4 < 100 && ip4 > 9) {
        port = atoi (temp + 3);
        temp += 3;
    }
    else if (ip4 < 10) {
        port = atoi (temp + 2);
        temp += 2;
    }

    if(temp-1 != ":")
        return 0;

/* aggiorniamo temp in base al numero della porta */

    if (port < 9) {
        temp += 2;
    }
    else if (port > 9 && port < 100) {
        temp += 3;
    }
    else if (port > 99 && port < 1000) {
        temp += 4;
    }
    else if (port > 999 && port < 10000) {
        temp += 5;
    }
    else if (port > 9999 && port < 100000) {
        temp += 6;
    }
    
    my_strcpy(dir, temp);

    dir[strlen(dir)-2] = 0;

/* gli ultimi due caratteri DEVONO essere "\n" e "\n"*/

    if (temp [strlen (temp)-1] == '\n' && temp [strlen (temp)] == '\n') {
        return 1;
    } else {
        return 0;
    }
}
/* funzione my_strstr che prende 2 stringhe e controlla se una è contenuta 
 nell'altra usando un ciclo for che incrementa ogni volta finche str[i] sia
 uguale al primo carattere di sub, cosi da fare il confronto ed eventalmente 
 restituire &(str[i]), altrimenti restituisce NULL.*/
char* my_strstr (char* str, char* sub) {
    int i;
    for (i=0; i<strlen(str); i++) {
        if (strcmp (&(str [i]), sub)== 0) {
            return &(str[i]);
        }
    }
    return NULL;
}
/* funzione my_strcpy che prende due stringhe e finchè non incontra lo 0 terminatore, copia src in dest,
   uscito dal ciclo pone dest[i]=0; per dargli lo 0 terminatore. */
char* my_strcpy (char* dest, char* src) {
    int i = 0;
    while (src [i] != 0) {
        dest [i] = src [i]; 
        i++;
    }
    dest[i] = 0;
    return dest;
}
