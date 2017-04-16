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
#include <commons/structsUtiles.h>

#define MAX_LEN_PUERTO 6
#define MAX_LEN_IP 20
#define configuracionCPU "confCpu.init"

typedef struct {
		char* puerto_Memoria;
        char* ip_Memoria;
        char* puerto_Kernel;
        char* ip_Kernel;

}t_config_cpu;

//Variables Globales
t_config_cpu* config;
int socketConexionKernel;
int socketConexionMemoria;
t_log* logger;

uint32_t tamanioStack;
uint32_t tamanioPagina;
uint32_t operacion;

t_config_cpu* levantarConfiguracionCPU(char* archivo);
int conexionConKernel();
int conexionConMemoria();
int crearLog();
void crearConfig(int argc, char* argv[]);
void atenderKernel();
void recibirTamanioStack(void* paquete);
void recibirPCB(void* paquete);
void recibirValorVariableCompartida(void* paquete);
void recibirAsignacionVariableCompartida(void* paquete);
void recibirSignalSemaforo(void* paquete);
void recibirTamanioPagina(void* paquete);
void recibirInstruccion(void* paquete);

#endif /* FUNCIONESCPU_H_ */
