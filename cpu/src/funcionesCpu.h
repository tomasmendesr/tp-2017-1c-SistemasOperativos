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
#define TAM_PAG 256 //se recibe desde memoria

typedef struct{
	char* puerto_Memoria;
	char* ip_Memoria;
	char* puerto_Kernel;
	char* ip_Kernel;
}t_config_cpu;

t_config_cpu* config;
int socketConexionKernel;
int socketConexionMemoria;
t_log* logger;

uint32_t tamanioStack;
/*lo manda memoria*/
uint32_t tamanioPagina;

t_pcb_* crearPCB(char* buf);
t_list* llenarLista(t_intructions * indiceCodigo, t_size cantInstruc);
void levantarArchivo(char*path);
t_config_cpu* levantarConfiguracionCPU(char* archivo);
int conexionConKernel(void);
int conexionConMemoria(void);
void freeConf(t_config_cpu* config);
t_puntero definirVariable(t_nombre_variable nombre);
void inicializarFunciones(void);
void procesarProgramas(void);
int crearLog(void);
void crearConfig(int argc, char* argv[]);
void atenderKernel();
void recibirTamanioStack(void* paquete);
void recibirPCB(void* paquete);
void recibirValorVariableCompartida(void* paquete);
void recibirAsignacionVariableCompartida(void* paquete);
void recibirSignalSemaforo(void* paquete);
void recibirTamanioPagina(void* paquete);

#endif /* FUNCIONESCPU_H_ */
