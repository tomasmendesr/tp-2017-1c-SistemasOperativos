 /* structsUtiles.c
 *
 *  Created on: 11/4/2017
 *      Author: utnso
 */
#include "structUtiles.h"

void insertarNuevoStack(pcb_t* pcb){
	t_entrada_stack* nuevo = crearPosicionStack();
	list_add(pcb->indiceStack, nuevo);
}

void eliminarUltimaPosicionStack(pcb_t* pcb){
	int cantElementos = list_size(pcb->indiceStack);
	list_remove_and_destroy_element(pcb->indiceStack, cantElementos - 1, destruirPosicionStack);
}

t_entrada_stack* crearPosicionStack(){
	t_entrada_stack* stack = malloc(sizeof(t_entrada_stack));

	stack->argumentos = list_create();
	stack->variables = list_create();
	stack->retVar = malloc(sizeof(t_argumento));

	return stack;
}

void destruirPosicionStack(t_entrada_stack* stack){
	free(stack->retVar);
	list_destroy_and_destroy_elements(stack->argumentos, destruirArgumentoStack);
	list_destroy_and_destroy_elements(stack->variables, destruirVariableStack);
	free(stack);
}

t_var_local* crearVariableStack(char id, uint32_t pagina, uint32_t offset, uint32_t size){
	t_var_local* var = malloc(sizeof(t_var_local));

	var->idVariable = id;
	var->pagina = pagina;
	var->offset = offset;
	var->size = size;

	return var;
}

void destruirVariableStack(t_var_local* var){
	free(var);
}

t_argumento* crearArgumentoStack(uint32_t pagina, uint32_t offset, uint32_t size){
	t_argumento* arg = malloc(sizeof(t_argumento));
	arg->pagina = pagina;
	arg->offset = offset;
	arg->size = size;

	return arg;
}

void destruirArgumentoStack(t_argumento* arg){
	free(arg);
}

void agregarVariable(t_entrada_stack* stack, t_var_local* variable){
	list_add(stack->variables, variable);
}

void agregarArgumento(t_entrada_stack* stack, t_argumento* argumento){
	list_add(stack->variables, argumento);
}

char* ansisop_a_string(char* path){

	FILE* file;
	int file_fd, file_size;
	struct stat stats;
	file = fopen(path, "r");

	file_fd = fileno(file);
	fstat(file_fd, &stats);
	file_size = stats.st_size;

	char* codigo = malloc(file_size);

	fread(codigo,file_size,1,file);
	return codigo;
}

