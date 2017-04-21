/*
 * primitivas.c
 *
 *  Created on: 17/4/2017
 *      Author: utnso
 */
#include "primitivas.h"

void setPCB(pcb_t * pcbDeCPU){
	pcb = pcbDeCPU;
}

t_puntero definirVariable(t_nombre_variable identificador_variable){

	if(pcb->stackPointer + TAMANIO_VARIABLE > TAM_STACK*TAM_PAG){
		/*esta verificacion me hace ruido*/
		if(!huboStackOver){
			log_error(logger, "StackOverflow. Se finaliza el proceso");
			huboStackOver = true;
		}
		return -1;
	}
	if(!esArgumento(identificador_variable)){
		log_debug(logger, "Definir variable %c", identificador_variable);
		t_var_local* nuevaVar = malloc(sizeof(t_var_local));
		t_entrada_stack* lineaStack = list_get(pcb->indiceStack, pcb->indiceStack->elements_count-1);

		if(lineaStack == NULL){
			lineaStack = malloc(sizeof(t_entrada_stack));
			lineaStack->retVar = NULL;
			lineaStack->direcretorno = -1;
			lineaStack->argumentos = NULL;
			lineaStack->variables = list_create();
			list_add(pcb->indiceStack, lineaStack);
		}
		nuevaVar->idVariable = identificador_variable;
		nuevaVar->pagina = pcb->stackPointer / tamanioPagina + pcb->cantPaginasCodigo; /*por la posicion en memoria*/
		nuevaVar->offset = pcb->stackPointer % tamanioPagina;
		nuevaVar->size = TAMANIO_VARIABLE;
		list_add(lineaStack->variables, nuevaVar);
		pcb->stackPointer+=TAMANIO_VARIABLE;

		log_debug(logger, "%c %i %i %i", nuevaVar->idVariable, nuevaVar->pagina,
				nuevaVar->offset, nuevaVar->size);
		return pcb->stackPointer-TAMANIO_VARIABLE;

	}else{
		log_debug(logger, "Definir variable - argumento %c", identificador_variable);
		t_argumento* nuevoArg = malloc(sizeof(t_argumento));
		t_entrada_stack* lineaStack = list_get(pcb->indiceStack, pcb->indiceStack->elements_count -1);

		if(lineaStack == NULL){
			lineaStack = malloc(sizeof(t_entrada_stack));
			lineaStack->retVar = NULL;
			lineaStack->direcretorno = pcb->programCounter;
			lineaStack->argumentos = list_create();
			lineaStack->variables = list_create();
			list_add(pcb->indiceStack, lineaStack);
		}
		nuevoArg->pagina = pcb->stackPointer / tamanioPagina + pcb->cantPaginasCodigo;
		nuevoArg->offset = pcb->stackPointer % tamanioPagina;
		nuevoArg->size = TAMANIO_VARIABLE;
		list_add(lineaStack->argumentos, nuevoArg);
		pcb->stackPointer += TAMANIO_VARIABLE;

		log_debug(logger, "%c %i %i %i", identificador_variable, nuevoArg->pagina,
				nuevoArg->offset, nuevoArg->size);
		return pcb->stackPointer-TAMANIO_VARIABLE;
	}
}

void asignar(t_puntero direccion_variable, t_valor_variable valor){
	printf("asignar!\n");
	return;
}
t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	printf("asignarVariableCompartida!\n");
	return 0;
}

t_valor_variable dereferenciar(t_puntero direccion_variable){
	printf("dereferenciar!\n");
	return 0;
}
void finalizar(void){
	printf("finalizar!\n");
	return;
}
void irAlLabel(t_nombre_etiqueta t_nombre_etiqueta){
	printf("irAlLabel!\n");
	return;
}
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	printf("llamarConRetorno!\n");
	return;
}
void llamarSinRetorno(t_nombre_etiqueta etiqueta){
	printf("llamarSinRetorno!\n");
	return;
}
t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable){
	printf("obtenerPosicionVariable!\n");
	return 0;
}
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){
	printf("obtenerValorCompartida!\n");
	return 0;
}
void retornar(t_valor_variable retorno){
	printf("retornar!\n");
	return;
}
t_descriptor_archivo abrir(t_direccion_archivo direccion, t_banderas flags){
	printf("abrir!\n");
	return 0;
}
void borrar(t_descriptor_archivo direccion){
	printf("borrar!\n");
}
void cerrar(t_descriptor_archivo descriptor_archivo){
	printf("cerrar!\n");
}
void escribir(t_descriptor_archivo descriptor_archivo, void* informacion, t_valor_variable tamanio){
	printf("escribir!\n");
}
void leer(t_descriptor_archivo descriptor_archivo, t_puntero informacion, t_valor_variable tamanio){
	printf("leer!\n");
}
void liberar(t_puntero puntero){
	printf("liberar!\n");
}
void moverCursor(t_descriptor_archivo descriptor_archivo, t_valor_variable posicion){
	printf("moverCursor!\n");
}
t_puntero reservar(t_valor_variable espacio){
	printf("reservar!\n");
	return 0;
}
void signal(t_nombre_semaforo identificador_semaforo){
	printf("signal!\n");
}
void wait(t_nombre_semaforo identificador_semaforo){
	printf("wait!\n");
}

void inicializarFunciones(void){
	funciones = malloc(sizeof(AnSISOP_funciones));
	funcionesKernel = malloc(sizeof(AnSISOP_funciones));

	funciones->AnSISOP_asignar = asignar;
	funciones->AnSISOP_asignarValorCompartida = asignarValorCompartida;
	funciones->AnSISOP_definirVariable = definirVariable;
	funciones->AnSISOP_dereferenciar = dereferenciar;
	funciones->AnSISOP_finalizar = finalizar;
	funciones->AnSISOP_irAlLabel = irAlLabel;
	funciones->AnSISOP_llamarConRetorno = llamarConRetorno;
	funciones->AnSISOP_llamarSinRetorno = llamarSinRetorno;
	funciones->AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable;
	funciones->AnSISOP_obtenerValorCompartida = obtenerValorCompartida;
	funciones->AnSISOP_retornar = retornar;
	funcionesKernel->AnSISOP_abrir = abrir;
	funcionesKernel->AnSISOP_borrar = borrar;
	funcionesKernel->AnSISOP_cerrar = cerrar;
	funcionesKernel->AnSISOP_escribir = escribir;
	funcionesKernel->AnSISOP_leer = leer;
	funcionesKernel->AnSISOP_liberar = liberar;
	funcionesKernel->AnSISOP_moverCursor = moverCursor;
	funcionesKernel->AnSISOP_reservar = reservar;
	funcionesKernel->AnSISOP_signal = signal;
	funcionesKernel->AnSISOP_wait = wait;
}

bool esArgumento(t_nombre_variable identificador_variable){
	if(isdigit(identificador_variable)){
		return true;
	}else{
		return false;
	}
}
