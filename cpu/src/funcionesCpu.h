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
#define configuracionCPU "../confCpu.init"

typedef struct{
	char* puerto_Memoria;
	char* ip_Memoria;
	char* puerto_Kernel;
	char* ip_Kernel;
}t_config_cpu;

typedef struct{
	uint32_t pid;
	uint32_t programCounter;
	uint32_t cantPaginasCodigo;
	t_intructions* indiceCodigo;
	char* etiquetas;
	t_list* indiceStack;
	int16_t exitCode;
	uint32_t consolaFd;
}pcb_t;

t_config_cpu* levantarConfiguracionCPU(char* archivo);
void conexionConKernel();
void conexionConMemoria();
int crearLog();
void crearConfig(int argc, char* argv[]);
void freeConf(t_config_cpu* config);
t_puntero definirVariable(t_nombre_variable nombre);
void inicializarFunciones(void);
void procesarProgramas(void);

t_config_cpu* config;
int socketConexionKernel;
int socketConexionMemoria;
t_log* logger;
AnSISOP_funciones* funciones;
AnSISOP_kernel* funcionesKernel;

#endif /* FUNCIONESCPU_H_ */
