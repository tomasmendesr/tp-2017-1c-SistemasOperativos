/*
 * memoria.h
 *
 *  Created on: 5/4/2017
 *      Author: utnso
 */

#ifndef FUNCIONESMEMORIA_H_
#define FUNCIONESMEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/sockets.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/interface.h>
#include <pthread.h>

#define configuracionMemoria "confMemoria.init"
#define MAX_LEN_PUERTO 6

typedef struct{
        char* puerto;
        int marcos;
        int marcos_Size;
        int entradas_Cache;
        int cache_x_Proceso;
        char* reemplazo_cache;
        int retardo_Memoria;
}t_config_memoria;

typedef struct{
	int pid;
	int nroPag;
}t_entrada_tabla;

typedef struct{
	int pid;
	int nroPag;
	char* content;
}t_entrada_cache;

void crearConfig(int argc, char* argv[]);
t_config_memoria* levantarConfiguracionMemoria(char* archivo);
void destruirConfiguracionMemoria(t_config_memoria* config);

/* Esta funcion hace la creacion de la memoria y todas las estructuras
 * administrativas necesarias para que el sistema arranque
 */
void inicializarMemoria();

//Funciones de interfaz
void levantarInterfaz();
void retardo(char* comando, char* param);
void dump(char* comando, char* param);
void flush(char* comando, char* param);
void size(char* comando, char* param);

//Variables Globales
t_log* log;
t_config_memoria* config;
char* memoria; /*Este va a ser el bloque que simula la memoria principal.
				Uso char* porque sizeof(char) = 1 y facilita la aritmetica,
				pero no tiene nada que ver con caracteres*/
t_entrada_cache* cache;


#endif /* FUNCIONESMEMORIA_H_ */
