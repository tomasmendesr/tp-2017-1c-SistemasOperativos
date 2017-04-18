#include "pcb.h"


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

t_pcb* crearPCB(char* codigo, int id){
	t_pcb* pcb = malloc(sizeof(pcb));

	t_metadata_program* metadata = metadata_desde_literal(codigo);

	pcb->pid = id;
	pcb->cantPaginasCodigo = 0;
	pcb->exitCode = 0;
	pcb->programCounter = 0;

	pcb->indiceCodigo = malloc(metadata->instrucciones_size * 8);
	memcpy(pcb->indiceCodigo, metadata->instrucciones_serializado, metadata->instrucciones_size * 8);

	pcb->etiquetas = malloc(metadata->etiquetas_size);
	memcpy(pcb->etiquetas, metadata->etiquetas, metadata->etiquetas_size);

	pcb->indiceStack = list_create();
	insertarNuevoStack(pcb);

	metadata_destruir(metadata);
	return pcb;
}

void insertarNuevoStack(t_pcb* pcb){
	t_entrada_stack* nuevo = crearPosicionStack();
	list_add(pcb->indiceStack, nuevo);
}

void eliminarUltimaPosicionStack(t_pcb* pcb){
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

t_var_local* crearVariableStack(char* id, uint32_t pagina, uint32_t offset, uint32_t size){
	t_var_local* var = malloc(sizeof(t_var_local));
	var->idVariable = malloc(strlen(id));

	var->idVariable = id;
	var->pagina = pagina;
	var->offset = offset;
	var->size = size;

	return var;
}

void destruirVariableStack(t_var_local* var){
	free(var->idVariable);
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

