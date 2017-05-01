#include "pcb.h"

/*
 * Crea la estructura PCB y lo introduce en la cola de New
 * Envia a la memoria para que le reserve espacio para el codigo
 * Graba el codigo en Memoria y devuelve el PCB con todas las referencias
 */
pcb_t* crearPCB(char* codigo, int id, int fd){

	pcb_t* pcb = malloc(sizeof(pcb));

	t_metadata_program* metadata = metadata_desde_literal(codigo);

	pcb->pid = id;
	pcb->stackPointer = 0;
	pcb->cantPaginasCodigo = strlen(codigo) / PAG_SIZE;
	if(strlen(codigo)%PAG_SIZE != 0) pcb->cantPaginasCodigo++;
	pcb->exitCode = 0;
	pcb->programCounter = metadata->instruccion_inicio;
	pcb->codigo = metadata->instrucciones_size;
	pcb->tamanioEtiquetas = metadata->etiquetas_size;
	pcb->consolaFd=fd;

	//Indice de Codigo ----> tenemos que decidir si va como lista o no
//	pcb->indiceCodigo = malloc(metadata->instrucciones_size * sizeof(t_intructions));
//	memcpy(pcb->indiceCodigo, metadata->instrucciones_serializado, metadata->instrucciones_size * sizeof(t_intructions));

	pcb->indiceCodigo = llenarLista(metadata->instrucciones_serializado,metadata->instrucciones_size);

	if (metadata->etiquetas_size) {
		pcb->etiquetas = malloc(metadata->etiquetas_size);
		memcpy(pcb->etiquetas, metadata->etiquetas, metadata->etiquetas_size);
	}else{
		pcb->etiquetas = NULL;
	}

	pcb->indiceStack = list_create();
	insertarNuevoStack(pcb);
/*
	//Reservo el espacio para el codigo y almaceno el codigo
	u_int32_t direccion_segmento = guardarEnMemoria(socketConexionMemoria,strlen(codigo),pcb->pid);
	pcb->indice_codigo = direccion_segmento;

	if(direccion_segmento == 0){
		pcb->pid = 0; // Hubo un error al querer reservar espacio de memoria
		return pcb;
	}
	//Reservo el espacio para el Stack del programa.
	u_int32_t direccion_stack = reservarMemoria(socketConexionMemoria,strlen(config->stack_Size),pcb->pid);
	pcb->stackPointer = direccion_stack;

	if(direccion_stack == 0){
		pcb->pid = 0;
		return pcb;
	}
*/
	metadata_destruir(metadata);
	return pcb;
}


t_list* llenarLista(t_intructions * indiceCodigo, t_size cantInstruc) {
	t_list * lista = list_create();
	int b = 0;
	for (b = 0; b < cantInstruc; b++) {
		t_indice_codigo* linea = malloc(sizeof(t_indice_codigo));
		linea->offset = indiceCodigo[b].start;
		linea->size = indiceCodigo[b].offset;
		list_add(lista, linea);
	}
	return lista;
}
