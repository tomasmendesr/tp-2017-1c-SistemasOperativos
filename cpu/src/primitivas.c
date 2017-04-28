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
	if(pcb->stackPointer + TAMANIO_VARIABLE > tamanioStack*tamanioPagina){
		/*esta verificacion me hace ruido*/
		if(!huboStackOver){
			log_error(logger, "StackOverflow. Se finaliza el proceso");
			huboStackOver = true;
		}
		return -1;
	}
	if(!esArgumento(identificador_variable)){
		log_debug(logger, "definir variable %c", identificador_variable);
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

		log_debug(logger, "memoria -> %c %i %i %i", nuevaVar->idVariable, nuevaVar->pagina,
				nuevaVar->offset, nuevaVar->size);
		log_debug(logger, "Posicion absoluta de %c: %i", identificador_variable, pcb->stackPointer-TAMANIO_VARIABLE );
		return pcb->stackPointer-TAMANIO_VARIABLE;

	}else{
		log_debug(logger, "definir variable argumento %c", identificador_variable);
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

		log_debug(logger, "memoria -> %c %i %i %i", identificador_variable, nuevoArg->pagina, nuevoArg->offset, nuevoArg->size);
		log_debug(logger, "Posicion absoluta de %c: %i", identificador_variable, pcb->stackPointer-TAMANIO_VARIABLE );
		return pcb->stackPointer-TAMANIO_VARIABLE;
	}
}

void asignar(t_puntero direccion_variable, t_valor_variable valor){
	log_debug(logger, "asignar posicion %d a valor %d", direccion_variable, valor);
	//calculo la posicion de la variable en el stack mediante el desplazamiento
	pedido_bytes_t* enviar = malloc(sizeof(uint32_t)*TAMANIO_VARIABLE);
	enviar->pag = direccion_variable / tamanioPagina + pcb->cantPaginasCodigo;
	enviar->offset = direccion_variable % tamanioPagina;
	enviar->size = TAMANIO_VARIABLE;
	enviar->pid = pcb->pid;
	if(almacenarBytes(enviar, &valor)!=0){
		log_error(logger, "la variable no pudo asignarse");
		free(enviar);
		return;
	}
	log_info(logger, "Variable asignada");
	free(enviar);
	return;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	uint32_t offset;
	void* buffer;
	offset=0;
	uint32_t sizeVariable=strlen(variable)+1;
	uint32_t sizeTotal = sizeof(sizeVariable) + sizeVariable + sizeof(valor);
	header_t* header = malloc(sizeof(header_t));
	header->type = ASIG_VAR_COMPARTIDA;
	header->length = sizeTotal;
	buffer = malloc(sizeTotal);
	memcpy(buffer, &sizeVariable, sizeof(sizeVariable));
	offset+=sizeof(sizeVariable);
	memcpy(buffer+offset, variable, strlen(variable)+1);
	offset+=strlen(variable)+1;
	memcpy(buffer+offset, &valor, sizeof(valor));
	if(sendSocket(socketConexionKernel,header,buffer) <= 0){
		log_error(logger,"Error al enviar");
		free(header);
		free(buffer);
		return -1;
	}
	requestHandlerKernel();
	log_info(logger, "valor asignado");
	free(header);
	return valor; //muy trucho
}

t_valor_variable dereferenciar(t_puntero direccion_variable){
	int32_t rta;
	log_debug(logger, "dereferenciar %d", direccion_variable);
	//calculo la posicion de la variable en el stack mediante el desplazamiento
	t_posicion posicionRet;
	pedido_bytes_t* solicitar;
	posicionRet.pagina = direccion_variable / tamanioPagina + pcb->cantPaginasCodigo;
	posicionRet.offset = direccion_variable % tamanioPagina;
	posicionRet.size = TAMANIO_VARIABLE;
	solicitar = malloc(sizeof(pedido_bytes_t));
	solicitar->pag = posicionRet.pagina;
	solicitar->size = posicionRet.size;
	solicitar->offset = posicionRet.offset;
	solicitar->pid = pcb->pid;
	if((rta=solicitarBytes(solicitar))!=0){
		free(solicitar);
		log_error(logger,"La variable no pudo dereferenciarse");
		return rta;
	}
	free(solicitar);
	log_info(logger, "Variable dereferenciada!");
	return *(t_valor_variable*)bytes;
}

void finalizar(void){
	log_debug(logger,"finalizar");
	t_entrada_stack* contexto;
	uint32_t i=0;
	t_posicion* retVar;
	contexto=list_remove(pcb->indiceStack,pcb->indiceStack->elements_count-1);
	i+=contexto->argumentos->elements_count+contexto->variables->elements_count;
	pcb->stackPointer-=TAMANIO_VARIABLE*i;
	for(i=0;i<contexto->argumentos->elements_count;i++){
		free(list_remove(contexto->argumentos,i));
	}
	for(i=0;i<contexto->variables->elements_count;i++){
		free(list_remove(contexto->variables,i));
	}
	list_destroy(contexto->argumentos);
	list_destroy(contexto->variables);
	retVar=contexto->retVar;
	free(contexto);
	free(retVar);
	if(!list_size(pcb->indiceStack))
		finalizarEjecucionPorFinPrograma();
	else{
		pcb->programCounter=contexto->direcretorno;
	}
	return;
}

void irAlLabel(t_nombre_etiqueta etiqueta){
		log_debug(logger,"ir a: %s", etiqueta);
		t_puntero_instruccion numeroInstr = metadata_buscar_etiqueta(etiqueta, pcb->etiquetas, pcb->tamanioEtiquetas);
		log_debug(logger, "Instruccion del irALAbel: d", numeroInstr);
		if(numeroInstr == -1){
			log_error(logger,"El indice de etiquetas devolvio -1");
		}
		pcb->programCounter = numeroInstr - 1;
		return;
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	log_debug(logger, "llamar con retorno");
	uint32_t tamLineaStack;
	t_posicion*var;
	t_entrada_stack*nuevaLineaStack;
	tamLineaStack=TAMANIO_VARIABLE*sizeof(uint32_t);
	var=malloc(sizeof(t_posicion));
	var->pagina=donde_retornar/tamanioPagina;
	var->offset=donde_retornar%tamanioPagina;
	var->size=TAMANIO_VARIABLE;
	nuevaLineaStack=malloc(tamLineaStack);
	nuevaLineaStack->argumentos=list_create();
	nuevaLineaStack->variables=list_create();
	nuevaLineaStack->retVar=var;
	nuevaLineaStack->direcretorno=pcb->programCounter;
	list_add(pcb->indiceStack,nuevaLineaStack);
	irAlLabel(etiqueta);
	return;
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta){
	log_debug(logger, "llamar sin retorno");
	uint32_t tamLineaStack;
	t_entrada_stack*nuevaLineaStack;
	tamLineaStack=TAMANIO_VARIABLE*sizeof(uint32_t);
	nuevaLineaStack=malloc(tamLineaStack);
	nuevaLineaStack->argumentos=list_create();
	nuevaLineaStack->variables=list_create();
	nuevaLineaStack->retVar=NULL;
	nuevaLineaStack->direcretorno=pcb->programCounter;
	list_add(pcb->indiceStack,nuevaLineaStack);
	irAlLabel(etiqueta);
	return;
}

t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable){
	log_debug(logger, "obtener posicion de: %c", identificador_variable);
	uint32_t i;
	t_entrada_stack* entrada;
	t_var_local* var_local;
	t_argumento* argumento;
	t_puntero puntero;
	if(pcb->indiceStack->elements_count==0){
		return EXIT_FAILURE;
	}else{
		entrada=list_get(pcb->indiceStack, pcb->indiceStack->elements_count-1);
		if(!esArgumento(identificador_variable)){
			for(i=0; i<entrada->variables->elements_count; i++){
				var_local=list_get(entrada->variables,i);
				if(var_local->idVariable==identificador_variable)
					break;
			}
			if(entrada->variables->elements_count==i)
				return EXIT_FAILURE;
			else{
				puntero=(var_local->pagina-pcb->cantPaginasCodigo)*tamanioPagina+var_local->offset;
			}
		}
		else{
			if(identificador_variable > entrada->argumentos->elements_count){
				return EXIT_FAILURE;
			}else{
				argumento=list_get(entrada->argumentos,identificador_variable);
				puntero=(argumento->pagina-pcb->cantPaginasCodigo)*tamanioPagina+argumento->offset;
			}
		}
	}
	log_debug(logger, "posicion absoluta: %d", puntero);
	return puntero;
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){
	log_debug(logger, "obtener valor compartida %s", variable);
	int32_t valor;
	header_t header;
	header.type = LEER_VAR_COMPARTIDA;
	header.length = strlen(variable)+1;
	sendSocket(socketConexionKernel, &header, variable);
	requestHandlerKernel();
	log_info(logger,"asignado valor a compartida");
	valor=*(int32_t*)bytes;
	free(bytes);
	log_debug(logger, "Valor de %s: %d", variable, valor);
	return valor;
}

void retornar(t_valor_variable retorno){
	log_debug(logger, "retornar");
	//agarro contexto actual
	t_entrada_stack* contextoEjecucionActual = list_get(pcb->indiceStack, pcb->indiceStack->elements_count-1);
	//Limpio el contexto actual
	int i;
	for(i=0; i<list_size(contextoEjecucionActual->argumentos); i++){
		t_argumento* arg = list_get(contextoEjecucionActual->argumentos, i);
		pcb->stackPointer=pcb->stackPointer-TAMANIO_VARIABLE;
		free(arg);
	}
	for(i=0; i<list_size(contextoEjecucionActual->variables); i++){
		t_variable* var = list_get(contextoEjecucionActual->variables, i);
		pcb->stackPointer=pcb->stackPointer-TAMANIO_VARIABLE;
		free(var);
	}
	//calculo la direccion a la que tengo que retornar mediante la direccion de pagina start y offset que esta en el campo retvar
	t_posicion* retVar = contextoEjecucionActual->retVar;
	t_puntero direcVariable = retVar->pagina * tamanioPagina + retVar->offset + pcb->cantPaginasCodigo ;
	asignar(direcVariable, retorno);
	//elimino el contexto actual del indice del stack
	//Seteo el contexto de ejecucion actual en el anterior
	pcb->programCounter = contextoEjecucionActual->direcretorno;
	free(contextoEjecucionActual);
	list_remove(pcb->indiceStack, pcb->indiceStack->elements_count-1);
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

void signalAnsisop(t_nombre_semaforo identificador_semaforo){
	log_debug(logger, "signal a semaforo '%s'", identificador_semaforo);
	header_t* header=malloc(sizeof(header_t));
	header->type=SEM_SIGNAL;
	header->length=strlen(identificador_semaforo)+1;
	sendSocket(socketConexionKernel,header,identificador_semaforo);
	free(header);
}

void wait(t_nombre_semaforo identificador_semaforo){
	log_debug(logger, "wait a semaforo '%s'", identificador_semaforo);
	header_t* header=malloc(sizeof(header_t));
	header->type=SEM_WAIT;
	header->length=strlen(identificador_semaforo)+1;
	sendSocket(socketConexionKernel,header,identificador_semaforo);
	free(header);
	requestHandlerKernel();
}

bool esArgumento(t_nombre_variable identificador_variable){
	if(isdigit(identificador_variable)){
		return true;
	}else{
		return false;
	}
}
