/*
 * StructsUtiles.h
 *
 *  Created on: 11/4/2017
 *      Author: utnso
 */

#ifndef STRUCTSUTILES_H_
#define STRUCTSUTILES_H_

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <commons/log.h>

typedef struct{
	uint32_t pagina;
	uint32_t offset;
	uint32_t size;
}t_posicion_stack;

typedef struct{
	uint32_t pagina;
	uint32_t offset;
	uint32_t size;
}t_argumento;

typedef struct stack{
	t_list* argumentos;
	t_list* variables;
	uint32_t direcretorno;
	t_argumento * retVar;
}t_stack;

typedef struct{
	char idVariable;
	uint32_t pagina;
	uint32_t offset;
	uint32_t size;
}t_var_local;

typedef struct indiceCodigo{
	uint32_t offset;
	uint32_t start;
}t_indice_codigo;

typedef enum {
	ERROR, NOTHING, SUCCESS
} opciones_generales_ops;

typedef struct{
	int pid;
	int pag;
	int offset;
	int size;
}t_operacion_bytes;

typedef struct{
	int pid;
	int cantPag;
}t_operacion_pag;

#endif /* STRUCTSUTILES_H_ */

