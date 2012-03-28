#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* boolean */
#define TRUE 1
#define FALSE 0

/* miniHttp define */
#define MAXLENPATH 2048
#define MAXLENREQ ((MAXLENPATH)+1024)
#define MAXLENDATA 5000
#define MAXLENRESP ((MAXLENREQ)+MAXLENDATA)


/* DA INVENTARSI UN NOME */
#define MAXNUMSOCKET 5
#define SOCKET_ERROR   ((int)-1)

typedef int BOOL;

typedef struct req {
	char type[10];
	char protocoll[20];
	int  ip[4];
	int  port;
	char dir[MAXLENPATH];
} request;


/*prototipi funzioni*/
BOOL request_parse (char *req, request *r);

/*funzione prende intero e puntatore a stringa (**argv)*/ 
int main1 (int argc, char *argv[]) 
{ 
	request r = {0};
    char request1[]="GET mhttp://130.136.2.23:55554/dir/a.txt\n\n";
	
    if (request_parse (request1, &r) == 1){
        printf ("1\n\n");
    } else 
	printf ("0\n\n");
    
    
    
    return 0;
}
/*funzione request_parse, prende in input una stringa (la req)
 dichiaro come stringa "GET mhttp://" e come stringa res, il risultato
 della my_strstr, dichiaro come int, il primo intero, ovvero il primo numero
 dell'indirizzo IP con l'atoi che parte dall'indirizzo di res+la lunghezza di
 httpreq0.*/
BOOL request_parse (char *req, request *r) 
{
    char tmp_char;
    int lenreq = strlen(req);
    int res = 0;
    char format[100]={0};
    sprintf(format, "%%9s %%19[^0-9]%%d.%%d.%%d.%%d:%%d%%%d[^\n]", MAXLENPATH);

    if((res = sscanf(req, format,
            r->type, 
            r->protocoll, 
            &(r->ip[0]), 
            &(r->ip[1]), 
            &(r->ip[2]), 
            &(r->ip[3]),
            &(r->port), 
            &r->dir)) != 8){
        return FALSE;
	}
    
    if(req[lenreq-1] != '\n' || req[lenreq-2] != '\n')        
	return FALSE;
    
    return TRUE;
}
