/*
 * sockets.c
 *
 *  Created on: 4/4/2017
 *      Author: utnso
 */

#include "sockets.h"

/**
 * @NAME: getSocket
 * @DESC: Crea un socket y retorna su descriptor. Retorna -1 en caso de error.
 */
int getSocket(void) {
	int yes = 1;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		close(sockfd);
		return -1;
	}

	return sockfd;
}

/**
 * @NAME: bindSocket
 * @DESC: Bindea el socket a la dirección ip y puerto pasados por parámetro.
 * Retorna -1 en caso de error.
 */
int bindSocket(int sockfd, char *addr, char *port) {
	struct sockaddr_in my_addr;

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(atoi(port));
	my_addr.sin_addr.s_addr = inet_addr(addr);
	memset(&(my_addr.sin_zero), '\0', 8);

	return bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));
}

/**
 * @NAME: listenSocket
 * @DESC: Pone a la escucha el socket pasado por parámetro con el backlog pasado por parámetro.
 * Retorna -1 en caso de error.
 */
int listenSocket(int sockfd, int backlog) {
	return listen(sockfd, backlog);
}

/**
 * @NAME: acceptSocket
 * @DESC: Acepta una conexion al socket enviado por parámetro y retorna el nuevo descriptor de socket.
 * Retorna -1 en caso de error.
 */
int acceptSocket(int sockfd) {
	struct sockaddr_in their_addr;
	int sin_size = sizeof(struct sockaddr_in);

	return accept(sockfd, (struct sockaddr *) &their_addr,
			(socklen_t *) &sin_size);
}

/**
 * @NAME: connectSocket
 * @DESC: Conecta el socket a la dirección y puerto pasados por parámetro.
 * Retorna -1 en caso de error.
 */
int connectSocket(int sockfd, char *addr, char *port) {
	struct sockaddr_in their_addr;

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(atoi(port));
	their_addr.sin_addr.s_addr = inet_addr(addr);
	memset(&(their_addr.sin_zero), '\0', 8);

	return connect(sockfd, (struct sockaddr*) &their_addr,
			sizeof(struct sockaddr));
}

/**
 * @NAME: sendSocket
 * @DESC: Serializa el header y el string de datos y lo envía al scoket pasado por parámetro.
 * Retorna la cantidad de bytes enviados o -1 en caso de error.
 */
int sendSocket(int sockfd, header_t *header, void *data) {
	int bytesEnviados, offset = 0, tmp_len = 0;
	void *packet = malloc(sizeof(header_t) + header->length);

	memcpy(packet, &header->type, tmp_len = sizeof(int8_t));
	offset = tmp_len;

	memcpy(packet + offset, &header->length, tmp_len = sizeof(header->length));
	offset += tmp_len;

	memcpy(packet + offset, data, tmp_len = header->length);
	offset += tmp_len;

	bytesEnviados = sendallSocket(sockfd, packet, offset);//El offset representa la cantidad total de bytes del paquete a enviar
	free(packet);

	return bytesEnviados;
}

/**
 * @NAME: createServer
 * @DESC: Crea y devuelve un socket para ser utilizado como servidor.
 * En caso de error retorna -1.
 */
int createServer(char *addr, char *port, int backlog) {
	int sockfd = getSocket();

	if (bindSocket(sockfd, addr, port) == -1) {
		perror("bind");
		close(sockfd);
		return -1;
	}

	if (listenSocket(sockfd, backlog) == -1) {
		perror("listen");
		close(sockfd);
		return -1;
	}

	return sockfd;
}

/**
 * @NAME: finalizarConexion
 * @DESC: Finaliza la conexion de un socket.
 */
int finalizarConexion(int socket) {
	close(socket);
	return 0;
}


/**
 * @NAME: createClient
 * @DESC: Crea y devuelve un socket para ser utilizado como cliente.
 * En caso de error retorna -1.
 */
int createClient(char *addr, char *port) {
	int sockfd = getSocket();

	if (connectSocket(sockfd, addr, port) == -1) {
		perror("connect");
		close(sockfd);
		return -1;
	}

	return sockfd;
}

/**
 * @NAME: sendallSocket
 * @DESC: procura enviar to-do el paquete entero. Devuelve la cantidad de bytes enviados o -1 en caso de error.
 *
 */
int sendallSocket(int socket, void* paquete_a_enviar, int tamanio_paquete) {

	int total = 0;
	int bytesleft = tamanio_paquete;
	int n;

	while(total < tamanio_paquete) {
		n = send(socket, paquete_a_enviar+total, bytesleft, 0);
		if(n == -1) {break; }
		total += n;
		bytesleft -= n;
	}

	return n==-1?-1:total;

}

char *program_serializer(char *codigo_programa) {

	uint16_t longitud;
	longitud = strlen(codigo_programa);
	char *programa_ya_serializado = malloc(longitud + sizeof(longitud));

	memcpy(programa_ya_serializado, &longitud, sizeof(longitud));
	memcpy(programa_ya_serializado + sizeof(longitud), codigo_programa, longitud);

	return programa_ya_serializado;

}

/*
 * @NAME: recibir_paquete
 *
 * @DESC: Recibe un paquete por el socket que le llega por parametro, devuelve la cantidad de bytes recibidos, 0 si se cerro el socket o -1 si hubo algun error.
 *
 */
int recibir_paquete(int socket, void** paquete, int* tipo){

	int bytes_recibidos;
	void* buffer;
	header_t cabecera;

	bytes_recibidos = recv(socket, &cabecera, sizeof(header_t), MSG_WAITALL);

	if(bytes_recibidos == 0) return 0;
	if(bytes_recibidos == -1) return -1;

	*tipo = cabecera.type;

	if(cabecera.length!=0){
	buffer = malloc(cabecera.length);

	bytes_recibidos = recv(socket, buffer, cabecera.length, MSG_WAITALL);

	if(bytes_recibidos == 0) return 0;
	if(bytes_recibidos == -1) return -1;

	*paquete = buffer;
	}

	return bytes_recibidos;
}

/*
 * @NAME: recibir_string
 *
 * @DESC: Recibe un paquete que contiene un string, utiliza la funcion recibir_paquete y le agrega el caracter nulo (\0) al final.
 */
int recibir_string(int socket,void** puntero_buffer,int* tipo){
	int tamanio;

	if((tamanio=recibir_paquete(socket,puntero_buffer,tipo))>0){
		agregar_caracter_nulo(*puntero_buffer, tamanio);
		return tamanio+1;
	}
	return tamanio;
}

/*
 * @NAME: agregar_caracter_nulo
 *
 * @DESC: Agrega el caracter nulo (\0) al final de un stream de bytes recibido.
 */
int agregar_caracter_nulo(void* stream, int tamanio){

	void* otro_puntero;

	otro_puntero=stream;
	otro_puntero=realloc(otro_puntero,tamanio+1);
	char aux='\0';
	memcpy(otro_puntero+tamanio,&aux,1);
	stream=otro_puntero;

	return tamanio+1;

}

/*
 * @NAME: deserializar_string
 *
 * @DESC: Recibe un puntero a un stream de bytes, deserializa un string y devuelve la cantidad total de bytes leidos del stream.
 */
int deserializar_string(void* paquete, char** string){

	int bytes_totales_leidos = 0;
	int bytes_string;
	void* puntero_buffer = paquete;

	memcpy(&bytes_string, puntero_buffer, sizeof(int));

	puntero_buffer += sizeof(int);

	string = malloc(bytes_string);

	memcpy(*string, puntero_buffer, bytes_string);

	agregar_caracter_nulo(*string, bytes_string);

	bytes_totales_leidos = sizeof(int) + bytes_string;

	return bytes_totales_leidos;

}
/*
void* pcb_serializer(pcb_t* self, int16_t *length){

	void *serialized = malloc(sizeof(pcb_t));
	int offset = 0, tmp_size = 0;

	memcpy(serialized, &self->pid, tmp_size = sizeof(self->pid));
	offset += tmp_size;

	memcpy(serialized + offset, &self->programCounter, tmp_size = sizeof(self->programCounter));
	offset += tmp_size;

	memcpy(serialized+ offset, &self->cantPaginasCodigo, tmp_size = sizeof(self->cantPaginasCodigo));
	offset += tmp_size;

	memcpy(serialized+ offset, &self->exitCode, tmp_size = sizeof(self->exitCode));
	offset += tmp_size;

	memcpy(serialized+ offset, &self->consolaFd, tmp_size = sizeof(self->consolaFd));
	offset += tmp_size;

	*length = offset;

	return serialized;
}

pcb_t* pcb_deserializer(int socketfd) {
	pcb_t* self = malloc(sizeof(pcb_t));
	recv(socketfd, &self->pid, sizeof(self->pid), MSG_WAITALL);
	recv(socketfd, &self->programCounter, sizeof(self->programCounter), MSG_WAITALL);
	recv(socketfd, &self->cantPaginasCodigo, sizeof(self->cantPaginasCodigo), MSG_WAITALL);
	recv(socketfd, &self->exitCode, sizeof(self->exitCode), MSG_WAITALL);
	recv(socketfd, &self->consolaFd, sizeof(self->consolaFd), MSG_WAITALL);
	return self;
} */

t_buffer_tamanio* serializarIndiceStack(t_list* indiceStack) {
	int a;
	//Lo inicializo con este valor porque en la primer posicion del buffer
	//va a estar el tamanio total del stack, y en la segunda la cantidad de items del stack
	uint32_t tamanioTotalBuffer = sizeof(uint32_t);

	t_list * tamanioStackStack = list_create();
	uint32_t tamanioStackParticular;
	for (a = 0; a < list_size(indiceStack); a++) {
		t_entrada_stack* linea = list_get(indiceStack, a);
		tamanioStackParticular = 0;
		int cantidadArgumentos = list_size(linea->argumentos);
		tamanioTotalBuffer += cantidadArgumentos * sizeof(t_argumento); //tamanio de lista de argumentos
		tamanioStackParticular += cantidadArgumentos * sizeof(t_argumento);

		int cantidadVariables = list_size(linea->variables);
		tamanioTotalBuffer += cantidadVariables * sizeof(t_variable); //tamanio lista de variables
		tamanioStackParticular += cantidadVariables * sizeof(t_variable);

		tamanioTotalBuffer += sizeof(uint32_t); //tamanio de variable direcretorno
		tamanioStackParticular += sizeof(uint32_t);

		tamanioTotalBuffer += sizeof(t_posicion); //tamanio de retVar
		tamanioStackParticular += sizeof(t_posicion);

		tamanioTotalBuffer += sizeof(uint32_t) * 3; //agrego 3 ints para indicar la cantidad de elemento de las 2 listas y los bytes de t_stack

		t_tamanio_stack_stack * auxiliar = malloc(sizeof(uint32_t) + tamanioStackParticular);
		auxiliar->stack = linea;
		auxiliar->tamanioStack = tamanioStackParticular;
		list_add(tamanioStackStack, auxiliar);
	}

	char * buffer = malloc(tamanioTotalBuffer + sizeof(uint32_t));
	int tamanioUint32 = sizeof(uint32_t), offset = 0;

	memcpy(buffer + offset, &tamanioTotalBuffer, tamanioUint32);
	offset += tamanioUint32;

	uint32_t cantidadItemsEnStack = list_size(indiceStack);

	memcpy(buffer + offset, &cantidadItemsEnStack, tamanioUint32);
	offset += tamanioUint32;

	for(a = 0; a < list_size(tamanioStackStack); a++) {
		t_tamanio_stack_stack * linea = list_get(tamanioStackStack, a);

		memcpy(buffer + offset, &(linea->tamanioStack), tamanioUint32);
		offset += tamanioUint32;

		t_entrada_stack * stack = linea->stack;

		uint32_t cantidadArgumentos = list_size(stack->argumentos);
		memcpy(buffer + offset, &cantidadArgumentos, tamanioUint32);
		offset += tamanioUint32;

		int p;
		for(p = 0; p < cantidadArgumentos; p++) {
			t_argumento *argumento = list_get(stack->argumentos, p);
			memcpy(buffer + offset, &(argumento->pagina), tamanioUint32);
			offset += tamanioUint32;
			memcpy(buffer + offset, &(argumento->offset), tamanioUint32);
			offset += tamanioUint32;
			memcpy(buffer + offset, &(argumento->size), tamanioUint32);
			offset += tamanioUint32;
			free(argumento);
		}

		uint32_t cantidadVariables = list_size(stack->variables);
		memcpy(buffer + offset, &cantidadVariables, tamanioUint32);
		offset += tamanioUint32;

		for(p = 0; p < cantidadVariables; p++) {
			t_var_local *variable = list_get(stack->variables, p);
			memcpy(buffer + offset, &(variable->idVariable), sizeof(char));
			offset += sizeof(char);
			memcpy(buffer + offset, &(variable->pagina), tamanioUint32);
			offset += tamanioUint32;
			memcpy(buffer + offset, &(variable->offset), tamanioUint32);
			offset += tamanioUint32;
			memcpy(buffer + offset, &(variable->size), tamanioUint32);
			offset += tamanioUint32;
			free(variable);
		}

		memcpy(buffer + offset, &(stack->direcretorno), tamanioUint32);
		offset += tamanioUint32;

		t_posicion * retVarStack = stack->retVar;
		if(retVarStack == NULL) {
			retVarStack = malloc(sizeof(t_argumento));
			retVarStack->offset = 0;
			retVarStack->pagina = 0;
			retVarStack->size = 0;
			memcpy(buffer + offset, &(retVarStack->pagina), tamanioUint32);
			offset += tamanioUint32;
			memcpy(buffer + offset, &(retVarStack->offset), tamanioUint32);
			offset += tamanioUint32;
			memcpy(buffer + offset, &(retVarStack->size), tamanioUint32);
			offset += tamanioUint32;
			free(retVarStack);
		}else{
			memcpy(buffer + offset, &(retVarStack->pagina), tamanioUint32);
			offset += tamanioUint32;
			memcpy(buffer + offset, &(retVarStack->offset), tamanioUint32);
			offset += tamanioUint32;
			memcpy(buffer + offset, &(retVarStack->size), tamanioUint32);
			offset += tamanioUint32;
			free(retVarStack);
		}
	}

	for(a = 0; a < list_size(tamanioStackStack); a++) {
		t_tamanio_stack_stack * tamanioStack = list_get(tamanioStackStack, a);
		t_entrada_stack * stack = tamanioStack->stack;
		list_remove(tamanioStackStack, a);
		a--;
		free(stack);
		free(tamanioStack);
	}
	free(tamanioStackStack);

	t_buffer_tamanio *bufferTamanio = malloc((sizeof(uint32_t)*2) + tamanioTotalBuffer);
	bufferTamanio->tamanioBuffer = tamanioTotalBuffer + sizeof(uint32_t);
	bufferTamanio->buffer = buffer;

	return bufferTamanio;
}

t_list* deserializarIndiceStack(char* buffer) {
	t_list * stack = list_create();
	int offset = 0, tamanioUint32 = sizeof(uint32_t);

	uint32_t cantidadElementosEnStack;
	memcpy(&cantidadElementosEnStack, buffer + offset, tamanioUint32);
	offset += tamanioUint32;

	int i;
	for(i = 0; i < cantidadElementosEnStack; i++) {
		uint32_t tamanioItemStack;
		memcpy(&tamanioItemStack, buffer + offset, tamanioUint32);
		offset += tamanioUint32;

		t_entrada_stack * stack_item = malloc(tamanioItemStack);

		t_list * argumentosStack = list_create();
		uint32_t cantidadArgumentosStack;
		memcpy(&cantidadArgumentosStack, buffer + offset, tamanioUint32);
		offset += tamanioUint32;
		int p;
		for(p = 0; p < cantidadArgumentosStack; p++) {
			t_argumento * argStack = malloc(sizeof(t_argumento));
			memcpy(&(argStack->pagina), buffer + offset, tamanioUint32);
			offset += tamanioUint32;
			memcpy(&(argStack->offset), buffer + offset, tamanioUint32);
			offset += tamanioUint32;
			memcpy(&(argStack->size), buffer + offset, tamanioUint32);
			offset += tamanioUint32;
			list_add(argumentosStack, argStack);
		}
		stack_item->argumentos = argumentosStack;

		t_list * variablesStack = list_create();
		uint32_t cantidadVariablesStack;
		memcpy(&cantidadVariablesStack, buffer + offset, tamanioUint32);
		offset += tamanioUint32;
		for(p = 0; p < cantidadVariablesStack; p++) {
			t_var_local * varStack = malloc(sizeof(t_variable));
			memcpy(&(varStack->idVariable), buffer + offset, sizeof(char));
			offset += sizeof(char);
			memcpy(&(varStack->pagina), buffer + offset, tamanioUint32);
			offset += tamanioUint32;
			memcpy(&(varStack->offset), buffer + offset, tamanioUint32);
			offset += tamanioUint32;
			memcpy(&(varStack->size), buffer + offset, tamanioUint32);
			offset += tamanioUint32;
			list_add(variablesStack, varStack);
		}
		stack_item->variables = variablesStack;

		uint32_t direcRetorno;
		memcpy(&direcRetorno, buffer + offset, tamanioUint32);
		stack_item->direcretorno = direcRetorno;
		offset += tamanioUint32;

		t_posicion * retVarStack = malloc(sizeof(t_posicion));
		memcpy(&(retVarStack->pagina), buffer + offset, tamanioUint32);
		offset += tamanioUint32;
		memcpy(&(retVarStack->offset), buffer + offset, tamanioUint32);
		offset += tamanioUint32;
		memcpy(&(retVarStack->size), buffer + offset, tamanioUint32);
		offset += tamanioUint32;
		if(retVarStack->pagina == 0 && retVarStack->offset == 0 && retVarStack->size == 0){
			retVarStack = NULL;
		}
		stack_item->retVar = retVarStack;

		list_add(stack, stack_item);
	}

	return stack;
}

t_buffer_tamanio* serializarIndiceDeCodigo(t_list* indiceCodigo) {
	uint32_t offset = 0, m = 0;
	int tmp_size = sizeof(uint32_t);
	char* bufferIndCod = malloc((list_size(indiceCodigo) * (2 * sizeof(uint32_t))) + sizeof(uint32_t));
	uint32_t itemsEnLista = list_size(indiceCodigo);

	memcpy(bufferIndCod + offset, &itemsEnLista, tmp_size);
	offset += tmp_size;

	t_indice_codigo* linea;
	for (m = 0; m < itemsEnLista; m++) {
		linea = list_get(indiceCodigo, m);
		memcpy(bufferIndCod + offset, &(linea->offset), tmp_size);
		offset += tmp_size;
		memcpy(bufferIndCod + offset, &(linea->size), tmp_size);
		offset += tmp_size;
	}

	t_buffer_tamanio * tamanioBuffer = malloc(sizeof(uint32_t) + offset);
	tamanioBuffer->tamanioBuffer = offset;
	tamanioBuffer->buffer = bufferIndCod;

	return tamanioBuffer;
}

t_list* deserializarIndiceCodigo(char* indice, uint32_t tam) {
	t_list* ret = list_create();
	uint32_t offset = 0;
	uint32_t tmp_size = sizeof(uint32_t);
	int i = 0;
	for (i = 0; i < tam; i++) {
		t_indice_codigo* linea = malloc(sizeof(t_indice_codigo));
		memcpy(&(linea->offset), indice + offset, tmp_size);
		offset += tmp_size;
		memcpy(&(linea->size), indice + offset, tmp_size);
		offset += tmp_size;
		list_add(ret, linea);
	}
	return ret;
}

t_buffer_tamanio* serializar_pcb(pcb_t* pcb) {
	t_buffer_tamanio* indcod = serializarIndiceDeCodigo(pcb->indiceCodigo);
	t_buffer_tamanio* stackSerializado = serializarIndiceStack(pcb->indiceStack);

	uint32_t tamanioPCB = 0;
	tamanioPCB += stackSerializado->tamanioBuffer;
	tamanioPCB += indcod->tamanioBuffer;
	tamanioPCB += sizeof(uint32_t); //Tamanio Etiquetas
	if (pcb->etiquetas != NULL && pcb->tamanioEtiquetas > 0) {
		tamanioPCB += pcb->tamanioEtiquetas;
	}
	tamanioPCB += (sizeof(uint32_t) * 8); //Cantidad de uint32_t que tiene PCB
	tamanioPCB += sizeof(uint32_t); //Para indicar tamanio de PCBs

	//Comienzo serializacion
	char* paqueteSerializado = malloc(tamanioPCB);
	uint32_t offset = 0;
	uint32_t tmp_size = sizeof(uint32_t);

	//Serializo tamanio de PCB
	memcpy(paqueteSerializado + offset, &tamanioPCB, tmp_size);
	offset += tmp_size;

	//Serializo los 8 uint32_t del PCB
	memcpy(paqueteSerializado + offset, &(pcb->pid), tmp_size);
	offset += tmp_size;
	memcpy(paqueteSerializado + offset, &(pcb->codigo), tmp_size);
	offset += tmp_size;
	memcpy(paqueteSerializado + offset, &(pcb->programCounter), tmp_size);
	offset += tmp_size;
	memcpy(paqueteSerializado + offset, &(pcb->stackPointer), tmp_size);
	offset += tmp_size;
	memcpy(paqueteSerializado + offset, &(pcb->tamanioEtiquetas), tmp_size);
	offset += tmp_size;
	memcpy(paqueteSerializado, &(pcb->cantPaginasCodigo), tmp_size);
 	offset += tmp_size;
 	memcpy(paqueteSerializado, &(pcb->exitCode), tmp_size);
 	offset += tmp_size;
	memcpy(paqueteSerializado, &(pcb->consolaFd), tmp_size);
 	offset += tmp_size;
	memcpy(paqueteSerializado, &(pcb->exitCode), tmp_size);
 	offset += tmp_size;

	//Serializo Indice de Codigo
	memcpy(paqueteSerializado + offset, indcod->buffer, indcod->tamanioBuffer);
	offset += indcod->tamanioBuffer;

	//Serializo Indice de Stack
	memcpy(paqueteSerializado + offset, stackSerializado->buffer, stackSerializado->tamanioBuffer);
	offset += stackSerializado->tamanioBuffer;

	//Serializo Indice de Etiquetas
	if(pcb->etiquetas != NULL && pcb->tamanioEtiquetas > 0) {
		//uint32_t longitudIndiceEtiquetas = strlen(pcb->ind_etiq);
		memcpy(paqueteSerializado + offset, &pcb->tamanioEtiquetas, tmp_size);
		offset += tmp_size;
		memcpy(paqueteSerializado + offset, pcb->etiquetas, pcb->tamanioEtiquetas);
	}else{
		uint32_t longitudIndiceEtiquetas = 0;
		memcpy(paqueteSerializado + offset, &longitudIndiceEtiquetas, tmp_size);
	}


	free(indcod->buffer);
	free(indcod);
	free(stackSerializado->buffer);
	free(stackSerializado);

	t_buffer_tamanio * buffer_tamanio = malloc(sizeof(uint32_t) + tamanioPCB);
	buffer_tamanio->tamanioBuffer = tamanioPCB;
	buffer_tamanio->buffer = paqueteSerializado;

	return buffer_tamanio;
}

pcb_t* deserializar_pcb(char* package) {
 	uint32_t offset = 0, tmp_size = sizeof(uint32_t);
 	uint32_t tamanioPCB;

 	//Tomo el tamanio del PCB
  	memcpy(&tamanioPCB, package + offset, tmp_size);
 	pcb_t *pcb;

 	pcb = malloc(tamanioPCB);
  	offset += tmp_size;

  	//Tomo los 8 uint32_t del PCB
 	memcpy(&(pcb->pid), package + offset, tmp_size);
 	offset += tmp_size;
 	memcpy(&(pcb->codigo), package + offset, tmp_size);
 	offset += tmp_size;
 	memcpy(&(pcb->programCounter), package + offset, tmp_size);
 	offset += tmp_size;
 	memcpy(&(pcb->stackPointer), package + offset, tmp_size);
 	offset += tmp_size;
 	memcpy(&(pcb->tamanioEtiquetas), package + offset, tmp_size);
 	offset += tmp_size;
 	memcpy(&(pcb->cantPaginasCodigo), package + offset, tmp_size);
 	offset += tmp_size;
 	memcpy(&(pcb->exitCode), package + offset, tmp_size);
 	offset += tmp_size;
	memcpy(&(pcb->consolaFd), package + offset, tmp_size);
 	offset += tmp_size;

 	//Tomo Indice de Codigo
 	uint32_t itemsIndiceDeCodigo;
 	memcpy(&itemsIndiceDeCodigo, package + offset, tmp_size);
 	offset += tmp_size;
 	uint32_t tamanioIndiceDeCodigo = sizeof(t_indice_codigo) * itemsIndiceDeCodigo;
 	char * bufferIndiceDeCodigo = malloc(tamanioIndiceDeCodigo);
 	memcpy(bufferIndiceDeCodigo, package + offset, tamanioIndiceDeCodigo);
 	offset += tamanioIndiceDeCodigo;

 	t_list * indiceDeCodigo = deserializarIndiceCodigo(bufferIndiceDeCodigo, itemsIndiceDeCodigo);
 	pcb->indiceCodigo = indiceDeCodigo;

 	//Tomo Indice de Stack
 	uint32_t tamanioStack;
 	memcpy(&tamanioStack, package + offset, tmp_size);
 	offset += tmp_size;
 	char * bufferIndiceStack = malloc(tamanioStack);
 	memcpy(bufferIndiceStack, package + offset, tamanioStack);
 	offset += tamanioStack;

 	t_list * indiceStack = deserializarIndiceStack(bufferIndiceStack);
 	pcb->indiceStack = indiceStack;

 	//Tomo Indice de Etiquetas
 	uint32_t longitudIndiceEtiquetas;
 	memcpy(&longitudIndiceEtiquetas, package + offset, tmp_size);
 	offset+= tmp_size;
 	if(longitudIndiceEtiquetas>0) {
 		char * indiceEtiquetas = malloc(pcb->tamanioEtiquetas);
 		memcpy(indiceEtiquetas, package + offset, longitudIndiceEtiquetas);
 		pcb->etiquetas = indiceEtiquetas;
 		//memcpy(pcb->ind_etiq, package + offset, longitudIndiceEtiquetas);
 	}else{
 		pcb->etiquetas = NULL;
 	}

 	free(bufferIndiceDeCodigo);
 	free(bufferIndiceStack);

 	return pcb;
 }

void* variable_serializer(t_variable *self, int16_t *length){

	void *serialized = malloc(sizeof(char) + sizeof(u_int32_t));
	int offset = 0, tmp_size = 0;

	memcpy(serialized, &self->nombre, tmp_size = sizeof(self->nombre));
	offset = tmp_size;

	memcpy(serialized + offset, &self->valor, tmp_size = sizeof(self->valor));
	offset += tmp_size;

	*length = offset;

	return serialized;
}

t_variable* variable_deserializer(int socketfd){
	t_variable* self = malloc(sizeof(t_variable));
	recv(socketfd, &self->nombre, sizeof(self->nombre), MSG_WAITALL);
	recv(socketfd, &self->valor, sizeof(self->valor), MSG_WAITALL);
	return self;
}

void* codeIndex_serializer(codeIndex *self, int16_t *length){

	void *serialized = malloc(sizeof(codeIndex));
	int offset = 0, tmp_size = 0;

	memcpy(serialized, &self->tamanio, tmp_size = sizeof(self->tamanio));
	offset = tmp_size;

	memcpy(serialized + offset, &self->offset, tmp_size = sizeof(self->offset));
	offset += tmp_size;

	*length = offset;

	return serialized;
}

codeIndex* codeIndex_deserializer(int socketfd){
	codeIndex* self = malloc(sizeof(codeIndex));
	recv(socketfd, &self->offset, sizeof(self->offset), MSG_WAITALL);
	recv(socketfd, &self->tamanio, sizeof(self->tamanio), MSG_WAITALL);
	return self;
}


char *paqueteEnviarAEjecutar_serializer(u_int16_t quantum, uint32_t retardo_quantum,pcb_t *pcb_proceso) {

	char *serialized = malloc(sizeof(quantum) + sizeof(retardo_quantum) + sizeof(pcb_t));
	int offset = 0, tmp_size = 0;

	memcpy(serialized, &quantum, tmp_size = sizeof(quantum));
	offset = tmp_size;

	memcpy(serialized + offset, &retardo_quantum, tmp_size = sizeof(retardo_quantum));
	offset += tmp_size;

	memcpy(serialized + offset, pcb_proceso, tmp_size = sizeof(pcb_t));
	offset += tmp_size;

	return serialized;

}


header_t crear_cabecera(int codigo, int length){

	header_t cabecera;

	cabecera.type = codigo;
	cabecera.length = length;

	return cabecera;
}

int enviar_paquete_vacio(int codigo_operacion, int socket){

	int bytes_enviados;
	bytes_enviados=enviar_info(socket, codigo_operacion, 0, NULL);

	return bytes_enviados;
}

int enviar_paquete_vacio_a_cpu(int codigo_operacion, int socket){

	int bytes_enviados;
	header_t cabecera;
	cabecera.type = codigo_operacion;
	cabecera.length = 0;

	bytes_enviados = sendSocket(socket, &cabecera, '\0');

	return bytes_enviados;

}

/* sendAll y recvAll: Te aseguran que se envio o recibio tod-o.
 * Respetan la misma interfaz que send y recv originales*/
int sendAll(int fd, char *cosa, int size, int flags){

	int cant_enviada = 0;
	int aux;

	while(cant_enviada < size){

		aux = send(fd, cosa + cant_enviada, size - cant_enviada, flags);

		if(aux == -1){
			return -1;
		}else if(aux == 0){
			return -1;
		}

		cant_enviada += aux;
	}

	//Ya envie correctamente
	return cant_enviada;
}

int recvAll(int fd, char *buffer, int size, int flags){

	int cant_recibida = 0;
	int aux;

	while(cant_recibida < size){

		aux = recv(fd, buffer + cant_recibida, size - cant_recibida, flags);

		if(aux == -1){
			return -1;
		}else if(aux == 0)
			return 0;

		cant_recibida += aux;
	}

	return cant_recibida;
}


bool enviarHandshake(int socket, int handshakeEnviar, int handshakeRespuesta){

	int operacion = 0;
	void* paquete_vacio;

	enviar_paquete_vacio(handshakeEnviar,socket);

	if(recibir_paquete(socket, &paquete_vacio, &operacion) == 0){
		return false;
	}

	if(operacion == handshakeRespuesta ){
		enviar_paquete_vacio(handshakeEnviar,socket);
	}

	return true;
}

bool recibirHanshake(int socket, int handshakeRecibir, int handshakeRespuesta){
	int operacion = 0;
	void* paquete_vacio;

	if (recibir_paquete(socket, &paquete_vacio, &operacion)) return false;

	if (operacion == handshakeRecibir) {
		enviar_paquete_vacio(handshakeRespuesta,socket);

		recibir_paquete(socket, &paquete_vacio, &operacion);

		if(operacion == handshakeRecibir){
			return true;
		}
	}

	return false;
}

int enviar_info(int sockfd, int codigo_operacion, int length, void* buff){

	int bytes_enviados;
	header_t cabecera = crear_cabecera(codigo_operacion, length);

	bytes_enviados = sendAll(sockfd, (char*)&cabecera, sizeof(cabecera), 0);

	if(bytes_enviados == 0) return 0;
	if(bytes_enviados == -1) return -1;

	if(cabecera.length!=0){
		bytes_enviados += sendAll(sockfd, (char*)buff, cabecera.length, 0);
		if(bytes_enviados == 0) return 0;
		if(bytes_enviados == -1) return -1;
	}

	return bytes_enviados;
}

int recibir_info(int socket, void** paquete, int *tipo_mensaje){

	int bytes_recibidos;
	header_t cabecera;

	bytes_recibidos = recvAll(socket, (char*)&cabecera, sizeof(header_t), MSG_WAITALL);

	if(bytes_recibidos == 0) return 0;
	if(bytes_recibidos == -1) return -1;

	*tipo_mensaje=cabecera.type;

	if(cabecera.length){
		*paquete = malloc(cabecera.length);
		bytes_recibidos = recvAll(socket, (char*)*paquete, cabecera.length, MSG_WAITALL);
		if(bytes_recibidos == 0) return 0;
		if(bytes_recibidos == -1) return -1;
	}
	else
		*paquete=NULL;

	return bytes_recibidos;
}

t_buffer_tamanio * serializarInstruccion(char* instruccion, int tamanioInstruccion) {
	int offset = 0, tmp_size = sizeof(uint32_t);
	char * buffer = malloc(tamanioInstruccion + 4);

	memcpy(buffer, &tamanioInstruccion, tmp_size);
	offset += tmp_size;
	memcpy(buffer + offset, instruccion, tamanioInstruccion);
	t_buffer_tamanio * buffer_tamanio;
	buffer_tamanio = malloc(tamanioInstruccion + 8);
	buffer_tamanio->tamanioBuffer = tmp_size + tamanioInstruccion;
	buffer_tamanio->buffer = buffer;

	return buffer_tamanio;
}

