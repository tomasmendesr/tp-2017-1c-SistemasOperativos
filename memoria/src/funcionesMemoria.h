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

typedef struct {

        char* puerto;
        int marcos;
        int marcos_Size;
        int entradas_Cache;
        int cache_x_Proceso;
        char* reemplazo_cache;
        int retardo_Memoria;

}t_config_memoria;

t_config_memoria* levantarConfiguracionMemoria(char* archivo);

//Funciones de interfaz
void levantarInterfaz();
void retardo(char* comando, char* param);
void dump(char* comando, char* param);
void flush(char* comando, char* param);
void size(char* comando, char* param);


//Variables Globales
t_config_memoria* config;

#endif /* FUNCIONESMEMORIA_H_ */
