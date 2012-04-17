/* 
 * File:   IOUtil.h
 * Author: mone
 *
 * Created on 2 marzo 2012, 15.04
 */

#ifndef IOUTIL_H
#define	IOUTIL_H

/*
 * Setta il timeout del socket, e continua a ricevere dati finchè la read non ritorna 0. Esce 
 * soltanto in caso di EAGAIN (Resource temporarily unavailable)
 */
int readn(int fd, void *buf);

/*
 * Setta il timeout del socket, e invia dati al destinatario. Riprova finchè la 
 * write non ritorna un valore diverso da -1.
 */
int writen(int fd, void *buf, int len);

/*
 * Setta l'opzione reuse address del fd del socket passato come argomento, che 
 * permette il riuso immediato dell'indirizzo locale
 */
int setSockReuseAddr(int fd);

#endif	/* IOUTIL_H */

