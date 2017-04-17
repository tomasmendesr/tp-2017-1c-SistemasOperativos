/*
 * kernel.h
 *
 *  Created on: 4/4/2017
 *      Author: utnso
 */

#ifndef FUNCIONESKERNEL_H_
#define FUNCIONESKERNEL_H_

#define IP "127.0.0.1"
#define BACKLOG 10
#define configuracionKernel "confKernel.init"
#define MAX_LEN_PUERTO 6
#define MAX_LEN_IP 20

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
#include "plp.h"
#include "pcp.h"
#include "pcb.h"

typedef struct {
	char* puerto_PROG;
	char* puerto_CPU;
        char* ip_Memoria;
        char* puerto_Memoria;
        char* ip_FS;
        char* puerto_FS;
        int quantum;
        int quantum_Sleep;
        char* algoritmo;
        int grado_MultiProg;
        t_dictionary* semaforos;
        t_dictionary* variablesGlobales;
        int stack_Size;

} t_config_kernel;

void inicializarColas();
void inicializaciones();
void crearConfig(int argc, char* argv[]);
t_config_kernel* levantarConfiguracionKernel(char* archivo_conf);
void destruirConfiguracionKernel(t_config_kernel* config);
t_dictionary* crearDiccionarioConValue(char** array, char** valores);
t_dictionary* crearDiccionario(char** array);
void modificarValorDiccionario(t_dictionary* dic, char* key, void* data);
int semaforoSignal(t_dictionary* dic, char* key);
int semaforoWait(t_dictionary* dic, char* key);
int leerVariableGlobanl(t_dictionary* dic, char* key);
void escribirVariableGlobal(t_dictionary* dic, char* key, void* nuevoValor);

void establecerConexiones();
int conexionConFileSystem();
int conexionConMemoria();
void trabajarConexionCPU();

//Mensajes con consola
void trabajarConexionConsola();
void procesarMensajeConsola(int consola_fd, int mensaje, char* package);
pcb_t* crearProceso(int consola_fd, char* package);
int asignarPid();

//Funciones de interfaz
void levantarInterfaz();
void listProcesses(char* comando, char* param);
void processInfo(char* comando, char* param);
void getTablaArchivos(char* comando, char* param);
void gradoMultiprogramacion(char* comando, char* param);
void killProcess(char*,char*);
void stopPlanification(char*,char*);

//Variables Globales
t_config_kernel* config;
int socketConexionFS;
int socketConexionMemoria;
int max_pid;
t_log logger_kernel;
sem_t mutex_cola_ready;
sem_t mutex_cola_new;

//Colas procesos
t_queue *new, *ready, *finished;

#endif /* FUNCIONESKERNEL_H_ */
