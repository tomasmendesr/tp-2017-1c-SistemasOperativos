/*
 * primitivas.c
 *
 *  Created on: 17/4/2017
 *      Author: utnso
 */
#include "primitivas.h"

void setPCB(t_pcb_ * pcbDeCPU) {
	pcb = pcbDeCPU;
}

t_puntero definirVariable(t_nombre_variable identificador_variable){

	tamanioPagina = TAM_PAG; // HARDODEO TODO
	if(!esArgumento(identificador_variable)){//si entra a este if es porque es una variable, si no entra es porque es un argumento, me tengo que fijar si es del 0 al 9, no solo del 0
		log_debug(logger, "Definir variable %c", identificador_variable);
		t_var_local* nuevaVar = malloc(sizeof(t_var_local));
		t_entrada_stack* lineaStack = list_get(pcb->indiceStack,  pcb->indiceStack->elements_count-1);

		if(pcb->stackPointer + 4 > tamanioPagina){
			if(!huboStackOver){
				log_error(logger, "StackOverflow. Se finaliza el proceso");
				huboStackOver = true;
			}
			return -1;
		}else{
			if(lineaStack == NULL){
				//el tamaño de la linea del stack seria de los 4 ints mas
//				uint32_t tamLineaStack = 7*sizeof(uint32_t)+1;
//				lineaStack = malloc(tamLineaStack);
				lineaStack = malloc(sizeof(t_entrada_stack));
				lineaStack->retVar = NULL;
				lineaStack->direcretorno = 0;
				lineaStack->argumentos = list_create();
				lineaStack->variables = list_create();
				list_add(pcb->indiceStack, lineaStack);
		}
		//me fijo si el offset de la ultima + el tamaño superan o son iguales el tamaño de la pagina, si esto sucede, tengo que pasar a una pagina nueva
		if(pcb->stackPointer + TAMANIO_VARIABLE > tamanioPagina){
			nuevaVar->idVariable = identificador_variable;
			t_indice_codigo* indCodigo = list_get(pcb->indiceCodigo,  pcb->indiceCodigo->elements_count-1);
			uint32_t paginaActual = indCodigo->offset / tamanioPagina;
			nuevaVar->posicion.pagina = paginaActual+1;
			nuevaVar->posicion.size = TAMANIO_VARIABLE;
			nuevaVar->posicion.offset = 0;
			pcb->stackPointer = TAMANIO_VARIABLE;
			list_add(lineaStack->variables, nuevaVar);
		}else{
			nuevaVar->idVariable = identificador_variable;
			t_indice_codigo* indCodigo = list_get(pcb->indiceCodigo,  pcb->indiceCodigo->elements_count-1);
			uint32_t paginaActual = indCodigo->offset / tamanioPagina;
			nuevaVar->posicion.pagina = paginaActual;
			nuevaVar->posicion.size = TAMANIO_VARIABLE;
			nuevaVar->posicion.offset = pcb->stackPointer;
			pcb->stackPointer+=TAMANIO_VARIABLE;
			list_add(lineaStack->variables, nuevaVar);
		}
		//calculo el desplazamiento desde la primer pagina del stack hasta donde arranca mi nueva variable
		uint32_t posicionRet = (nuevaVar->posicion.pagina * tamanioPagina) + nuevaVar->posicion.offset;
		log_debug(logger, "%c %i %i %i", nuevaVar->idVariable, nuevaVar->posicion.pagina,
				nuevaVar->posicion.offset, nuevaVar->posicion.size);
		return posicionRet;
		}
	}else{
		//en este caso es un argumento, realizar toda la logica aca y tambien en obtener posicion variable, asignar imprimir y retornar
		log_debug(logger, "Definir variable - argumento %c", identificador_variable);
		t_var_local* nuevoArg = malloc(sizeof(t_var_local));
		t_entrada_stack* lineaStack = list_get(pcb->indiceStack, pcb->indiceStack->elements_count -1);
		if(pcb->stackPointer + 4 > tamanioPagina){
			if(!huboStackOver){
				log_error(logger, "StackOverflow. Se finaliza el proceso");
				huboStackOver = true;
			}
			return -1;
		}else{
			//me fijo si el offset de la ultima + el tamaño superan o son iguales el tamaño de la pagina, si esto sucede, tengo que pasar a una pagina nueva
			if(pcb->stackPointer + TAMANIO_VARIABLE > tamanioPagina){
				t_indice_codigo* indCodigo = list_get(pcb->indiceCodigo,  pcb->indiceCodigo->elements_count -1);
				uint32_t paginaActual = indCodigo->offset / tamanioPagina;
				nuevoArg->posicion.pagina = paginaActual+1;
				nuevoArg->posicion.size = TAMANIO_VARIABLE;
				nuevoArg->posicion.offset = 0;
				pcb->stackPointer = TAMANIO_VARIABLE;
				list_add(lineaStack->argumentos, nuevoArg);
			}else{
				t_indice_codigo* indCodigo = list_get(pcb->indiceCodigo,  pcb->indiceCodigo->elements_count -1);
				uint32_t paginaActual = indCodigo->offset / tamanioPagina;
				nuevoArg->posicion.pagina = paginaActual;
				nuevoArg->posicion.size = TAMANIO_VARIABLE;
				nuevoArg->posicion.offset = pcb->stackPointer;
				pcb->stackPointer += TAMANIO_VARIABLE;
				list_add(lineaStack->argumentos, nuevoArg);
			}
			//calculo el desplazamiento desde la primer pagina del stack hasta donde arranca mi nueva variable
			uint32_t posicionRet = (nuevoArg->posicion.pagina * tamanioPagina) + nuevoArg->posicion.offset;
			log_debug(logger, "%c %i %i %i", identificador_variable, nuevoArg->posicion.pagina,
					nuevoArg->posicion.offset, nuevoArg->posicion.size);
			return posicionRet;
		}
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
