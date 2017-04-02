/*
 * interface.h
 *
 *  Created on: 1/4/2017
 *      Author: utnso
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <stdio.h>

#define command_size 10
#define buffer_size command_size * 2

typedef struct{
	char comando[command_size];
	void (*funcion)(char* , char* );
}comando;

typedef struct{
	comando* comandos;
	int cantComandos;
}interface_thread_param;

void interface(interface_thread_param* param);

#endif /* INTERFACE_H_ */
