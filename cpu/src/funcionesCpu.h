/*
 * cpu.h
 *
 *  Created on: 5/4/2017
 *      Author: utnso
 */

#ifndef FUNCIONESCPU_H_
#define FUNCIONESCPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/sockets.h>
#include <commons/structUtiles.h>
#include <parser/metadata_program.h>
#include <parser/parser.h>
#include <sys/stat.h>
#include "primitivas.h"
#define configuracionCPU "../confCpu.init"
#define ansisop "facil.ansisop"
#define TAM_PAG 256 //se recibe desde memoria
#define TAM_STACK 2

typedef struct{
	char* puerto_Memoria;
	char* ip_Memoria;
	char* puerto_Kernel;
	char* ip_Kernel;
}t_config_cpu;

int socketConexionKernel;
int socketConexionMemoria;
t_config_cpu* config;
t_log* logger;
/*desde kernel*/
uint32_t tamanioStack;
int quantum;
/*desde memoria*/
uint32_t tamanioPagina;

t_pcb* crearPCB(char* buf, int pid);
t_list* llenarLista(t_intructions * indiceCodigo, t_size cantInstruc);
void levantarArchivo(char*path,char**content);
t_config_cpu* levantarConfiguracionCPU(char* archivo);
int conexionConKernel(void);
int conexionConMemoria(void);
void freeConf(t_config_cpu* config);
t_puntero definirVariable(t_nombre_variable nombre);
void inicializarFunciones(void);
void ejecutarPrograma(void);
int crearLog(void);
void crearConfig(int argc, char* argv[]);
int16_t atenderKernel(void* paquete);
int16_t recibirTamanioStack(void* paquete);
int16_t recibirPCB(void* paquete);
int16_t leerCompartida(void* paquete);
int16_t asignarCompartida(void* paquete, int valor);
int16_t waitSemaforo(void* paquete, char* sem);
int16_t signalSemaforo(void* paquete, char* sem);
int16_t recibirTamanioPagina(void* paquete);

#endif /* FUNCIONESCPU_H_ */
