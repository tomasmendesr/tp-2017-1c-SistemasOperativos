/*
 * pcb.h
 *
 *  Created on: 12/4/2017
 *      Author: utnso
 */

#ifndef PCB_H_
#define PCB_H_

#include <parser/metadata_program.h>
#include <commons/structsUtiles.h>
#include <sys/stat.h>
#include <parser/parser.h>

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



char* ansisop_a_string(char* path);

//Prototipos de creacion y manejo de pcb
pcb_t* crearPCB(char* codigo, int id);
t_stack* crearPosicionStack();
void insertarNuevoStack(pcb_t* pcb);
void eliminarUltimaPosicionStack(pcb_t* pcb);
void destruirPosicionStack(t_stack* stack);
t_var_local* crearVariableStack(char* id, uint32_t pagina, uint32_t offset, uint32_t size);
void destruirVariableStack(t_var_local* var);
t_argumento* crearArgumentoStack(uint32_t pagina, uint32_t offset, uint32_t size);
void destruirArgumentoStack(t_argumento* arg);
void agregarVariable(t_stack* stack, t_var_local* variable);
void agregarArgumento(t_stack* stack, t_argumento* argumento);

#endif /* PCB_H_ */
