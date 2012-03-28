#include <stdio.h>
#include <pthread.h>
#include <string.h>

void dbg_printf(char *str) {
    pthread_t t_id = pthread_self();
    fprintf(stderr,strcat(str," %d "),t_id);
    fflush(stderr);
}
