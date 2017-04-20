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
#include <commons/collections/dictionary.h>

typedef struct{
	uint32_t pagina;
	uint32_t offset;
	uint32_t size;
}t_argumento;

typedef struct{
	uint32_t pagina;
	uint32_t offset;
	uint32_t size;
}t_posicion;

typedef struct{
	uint32_t pid;
	uint32_t pag;
	uint32_t offset;
	uint32_t size;
}pedido_bytes_t;

typedef struct{
	char idVariable;
	uint32_t pagina;
	uint32_t offset;
	uint32_t size;
}t_var_local;

typedef struct stack{
	t_list* argumentos;
	t_list* variables;
	uint32_t direcretorno;
	t_posicion * retVar;
}t_entrada_stack;

//t_dictionary* etiquetas;

typedef struct indiceCodigo{
	uint32_t offset;
	uint32_t size;
}t_indice_codigo;

typedef struct{
	uint32_t pid;  //Identificador único del Programa en el sistema
	uint32_t programCounter; //Número de la próxima instrucción a ejecutar
	uint32_t cantPaginasCodigo;
	t_list* indiceCodigo;
	t_list* indiceStack;
	int16_t exitCode;
	uint32_t consolaFd;
	char* etiquetas;
	uint32_t stackPointer; // el ultimo offset
	uint32_t tamanioEtiquetas;
	uint32_t codigo; // cant de instrucciones
}t_pcb;

typedef enum{
	ERROR, NOTHING, SUCCESS
} opciones_generales_ops;

#endif /* STRUCTSUTILES_H_ */
