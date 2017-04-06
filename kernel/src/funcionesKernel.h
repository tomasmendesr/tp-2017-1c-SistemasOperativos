/*
 * kernel.h
 *
 *  Created on: 4/4/2017
 *      Author: utnso
 */

#ifndef FUNCIONESKERNEL_H_
#define FUNCIONESKERNEL_H_

#define IP "127.0.0.1"
#define BACKLOG "10"

#include <stdio.h>
#include <stdlib.h>
#include <commons/sockets.h>
#include <string.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <commons/interface.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct {
        char* puerto_PROG;
        char* puerto_CPU;
        char* ip_Memoria;
        char* puerto_Memoria;
        char* ip_FS;
        char puerto_FS;
        int quantum;
        int quantum_Sleep;
        char* algoritmo;
        int grado_MultiProg;
        t_dictionary* semaforos;
        t_dictionary* variablesGlobales;
        int stack_Size;

} t_config_kernel;

t_config_kernel* levantarConfiguracionKernel(char* archivo_conf);
t_dictionary* crearDiccionarioConValue(char** array, char** valores);
t_dictionary* crearDiccionario(char** array);

//Funciones de interfaz
void levantarInterfaz();
void listProcesses(char* comando, char* param);
void processInfo(char* comando, char* param);
void getTablaArchivos(char* comando, char* param);
void gradoMultiprogramacion(char* comando, char* param);
void killProcess(char*,char*);
void stopPlanification(char*,char*);



#endif /* FUNCIONESKERNEL_H_ */
