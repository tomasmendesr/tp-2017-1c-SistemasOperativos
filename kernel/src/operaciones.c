#include "operaciones.h"

void trabajarMensajeConsola(int socketConsola){

	int tipo_mensaje; //Para que la funcion recibir_string lo reciba
	void* paquete;
	int check = recibir_paquete(socketConsola, &paquete, &tipo_mensaje);

	FD_SET(socketConsola, &setConsolas);

	if(check <= 0){
		log_warning(logger, "Se cerro el socket %d\n", socketConsola);
		verificarProcesosConsolaCaida(socketConsola);
		close(socketConsola);
		if(paquete)free(paquete);
		FD_CLR(socketConsola, &master);
		FD_CLR(socketConsola, &setConsolas);
	}else{
		procesarMensajeConsola(socketConsola, tipo_mensaje, paquete);
	}

}

void procesarMensajeConsola(int consola_fd, int mensaje, char* package){
	proceso_en_espera_t* nuevoProceso;

	switch(mensaje){
	case HANDSHAKE_PROGRAMA:
		enviar_paquete_vacio(HANDSHAKE_KERNEL,consola_fd);
		log_info(logger,"handshake con consola");
		printf("\n");
		break;
	case ENVIO_CODIGO:
		log_info(logger, "Recibo codigo");
		nuevoProceso = crearProcesoEnEspera(consola_fd, package);
		//sem_wait(&sem_multi);
		sem_wait(&mutex_cola_new);
		queue_push(colaNew, nuevoProceso);
		sem_post(&mutex_cola_new);
		//sem_post(&sem_cola_new);
		crearInfoEstadistica(nuevoProceso->pid, consola_fd);
		crearEntradaArchivoProceso(nuevoProceso->pid);
		log_info(logger,"Proceso agregado a la cola New");
		planificarLargoPlazo();
	break;
	case FINALIZAR_PROGRAMA:
		finalizarPrograma(consola_fd,*(int*)package);
		break;
	default: log_warning(logger,"Se recibio un codigo de operacion invalido.");
	break;
	}
}

void trabajarMensajeCPU(int socketCPU){

	int tipo_mensaje; //Para que la funcion recibir_string lo reciba
	void* paquete;
	int check = recibir_paquete(socketCPU, &paquete, &tipo_mensaje);

	//Chequeo de errores
	if(check <= 0){
		log_info(logger,"Se cerro el socket %d", socketCPU);
		close(socketCPU);
		if(paquete)free(paquete);
		FD_CLR(socketCPU, &master);
		FD_CLR(socketCPU, &setCPUs);
	}else{
		procesarMensajeCPU(socketCPU, tipo_mensaje, paquete);
	}

	FD_SET(socketCPU, &setCPUs);
}

void procesarMensajeCPU(int socketCPU, int mensaje, char* package){
	switch(mensaje){
	case HANDSHAKE_CPU:
		log_info(logger,"Conexion con nueva CPU establecida");
		enviar_paquete_vacio(HANDSHAKE_KERNEL,socketCPU);
		enviarTamanioStack(socketCPU);
		enviarQuantumSleep(socketCPU);
		agregarNuevaCPU(listaCPUs, socketCPU);
		break;
	case SEM_SIGNAL:
		realizarSignal(socketCPU, package);
		break;
	case SEM_WAIT:
		realizarWait(socketCPU, package);
		break;
	case LEER_VAR_COMPARTIDA:
		leerVarCompartida(socketCPU, package);
		break;
	case ASIG_VAR_COMPARTIDA:
		asignarVarCompartida(socketCPU, package);
		break;
	case RESERVAR_MEMORIA:
		reservarMemoria(socketCPU, package);
		break;
	case LIBERAR_MEMORIA:
		liberarMemoria(socketCPU, package);
		break;
	case ABRIR_ARCHIVO:
		abrirArchivo(socketCPU, package);
		break;
	case CERRAR_ARCHIVO:
		cerrarArchivo(socketCPU, package);
		break;
	case BORRAR_ARCHIVO:
		borrarArchivo(socketCPU, package);
		break;
	case ESCRIBIR:
		escribir(package, socketCPU);
		break;
	case LEER_ARCHIVO:
		leerArchivo(socketCPU, package);
		break;
	case MOVER_CURSOR:
		moverCursor(socketCPU, (t_cursor*) package);
		break;
	/* CPU DEVUELVE EL PCB */
	case FIN_PROCESO: //HACE ESTO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		finalizacion_proceso(package, socketCPU);
		break;
	case FIN_EJECUCION:
		finalizacion_quantum(package,socketCPU);
		break;
	/* ERRORES */
	case SEGMENTATION_FAULT:
		finalizacion_segment_fault(package, socketCPU);
		break;
	case STACKOVERFLOW:
		finalizacion_stackoverflow(package, socketCPU);
		break;
	case ERROR_MEMORIA:
		finalizacion_error_memoria(package, socketCPU);
		break;
	case SEMAFORO_NO_EXISTE:
		finalizacion_semaforo_no_existe(package, socketCPU);
		break;
	case GLOBAL_NO_DEFINIDA:
		finalizacion_global_no_definida(package, socketCPU);
		break;
	case NULL_POINTER:
		finalizacion_null_pointer(package, socketCPU);
		break;
	default:
		log_warning(logger,"Se recibio el codigo de operacion invalido.");
	}
}

void leerVarCompartida(int socketCPU, char* variable){
	if(dictionary_has_key(config->variablesGlobales, variable)){
		if(dictionary_get(config->variablesGlobales, variable) == NULL){
			log_error(logger, "El valor de %s es NULL", variable);
			 enviar_paquete_vacio(NULL_POINTER, socketCPU);
		}
		else{
			int valor = leerVariableGlobal(config->variablesGlobales, variable);
			header_t* header = malloc(sizeof(header_t));
			header->type = VALOR_VAR_COMPARTIDA;
			header->length = sizeof(valor);
			sendSocket(socketCPU, header, &valor);
			free(header);
		}
	}else{
		log_error(logger, "Error al leer var compartida %s. No se encontro", variable);
		enviar_paquete_vacio(GLOBAL_NO_DEFINIDA, socketCPU);
	}
}

void asignarVarCompartida(int socketCPU, void* buffer){
	char* variable;
	int valor, sizeVariable;
	memcpy(&sizeVariable, buffer, 4);
	variable = malloc(sizeof(sizeVariable));
	memcpy(variable, buffer+4, sizeVariable);
	if(dictionary_has_key(config->variablesGlobales, variable)){
		memcpy(&valor, buffer+4+sizeVariable, 4);
		escribirVariableGlobal(config->variablesGlobales, variable, valor);
		free(variable);
		aumentarEstadisticaPorSocketAsociado(socketCPU, estadisticaAumentarOpPriviligiada);
		enviar_paquete_vacio(ASIG_VAR_COMPARTIDA_OK, socketCPU);
	}
	else{
		log_error(logger, "No se encontro la variable compartida %s. Se finaliza ejecucion", variable);
		enviar_paquete_vacio(GLOBAL_NO_DEFINIDA, socketCPU);
	}
}

void realizarSignal(int socketCPU, char* key){
	aumentarEstadisticaPorSocketAsociado(socketCPU, estadisticaAumentarOpPriviligiada);
	if(dictionary_has_key(config->semaforos, key)){
		if(dictionary_get(config->semaforos, key) == NULL){
			log_error(logger, "El valor de %s es NULL", key);
			 enviar_paquete_vacio(NULL_POINTER, socketCPU);
			 return;
		}
		else{
			int valor = semaforoSignal(config->semaforos, key);
			if(valor <= 0){
				desbloquearProceso(key);
				log_info(logger, "Desbloqueo un proceso");
			}
			enviar_paquete_vacio(SIGNAL_OK, socketCPU);
		}
	}
	else{
		log_error(logger ,"No se encontro el semaforo %s. Se finaliza la ejecucion", key);
		enviar_paquete_vacio(SEMAFORO_NO_EXISTE, socketCPU);
	}
}

void realizarWait(int socketCPU, char* key){
	int resultado;
	aumentarEstadisticaPorSocketAsociado(socketCPU, estadisticaAumentarOpPriviligiada);

	if(dictionary_has_key(config->semaforos,key)){
		if(dictionary_get(config->semaforos, key) == NULL){
			log_error(logger, "El valor de %s es NULL", key);
			 enviar_paquete_vacio(NULL_POINTER, socketCPU);
			 return;
		}
		else{
			int valor = semaforoWait(config->semaforos, key);
			if(valor < 0) resultado = WAIT_DETENER_EJECUCION;
			else resultado = WAIT_SEGUIR_EJECUCION;

			enviar_paquete_vacio(resultado, socketCPU);

			if(resultado == WAIT_DETENER_EJECUCION){ //recibo el pcb

				int tipo_mensaje;
				void* paquete;
				recibir_paquete(socketCPU, &paquete, &tipo_mensaje);
				if(tipo_mensaje == PROC_BLOCKED){
					t_pcb* pcbRecibido = deserializar_pcb(paquete);
					bloquearProceso(key, pcbRecibido);
					desocupar_cpu(socketCPU);
					log_info(logger, "Bloqueo proceso %d", pcbRecibido->pid);
					free(paquete);
				}
				else{
					log_error(logger, "Se esperaba el mensaje PROC_BLOCKED y se recibio otro");
				}
			}
		}
	}else{
		log_error(logger ,"No se encontro el semaforo %s. Se finaliza la ejecucion", key);
		enviar_paquete_vacio(SEMAFORO_NO_EXISTE, socketCPU);
	}
}

void finalizarPrograma(int consola_fd, int pid){
	info_estadistica_t* info = buscarInformacion(pid);
	info->matarSiguienteRafaga = true;
	info->exitCode = FINALIZAR_DESDE_CONSOLA;
	log_info(logger, "Se termina la ejecucion del proceso %d por comando STOP", pid);
}

void finalizacion_segment_fault(void* paquete_from_cpu, int socket_cpu){
	t_pcb* pcbRecibido =  deserializar_pcb(paquete_from_cpu);
	log_error(logger, "Finaliza el proceso %d por segment fautt", pcbRecibido->pid);
	pcbRecibido->exitCode = SUPERA_LIMITE_ASIGNACION_PAGINAS;
	terminarProceso(pcbRecibido, socket_cpu);
}

void finalizacion_error_memoria(void* paquete_from_cpu, int socket_cpu){
	t_pcb* pcbRecibido =  deserializar_pcb(paquete_from_cpu);
	log_error(logger, "Finaliza el proceso %d por error en memoria", pcbRecibido->pid);
	pcbRecibido->exitCode = ERROR_MEMORIA;
	terminarProceso(pcbRecibido, socket_cpu);
}

void finalizacion_semaforo_no_existe(void* paquete_from_cpu, int socket_cpu){
	t_pcb* pcbRecibido =  deserializar_pcb(paquete_from_cpu);
	log_error(logger, "Finaliza el proceso #%d por querer acceder a un semaforo no inicializado", pcbRecibido->pid);
	pcbRecibido->exitCode = SEMAFORO_NO_EXISTE;
	terminarProceso(pcbRecibido, socket_cpu);
}

void finalizacion_global_no_definida(void* paquete_from_cpu, int socket_cpu){
	t_pcb* pcbRecibido =  deserializar_pcb(paquete_from_cpu);
	log_error(logger, "Finaliza el proceso #%d por intentar acceder a un una variable global no definida", pcbRecibido->pid);
	pcbRecibido->exitCode = GLOBAL_NO_DEFINIDA;
	terminarProceso(pcbRecibido, socket_cpu);
}

void finalizacion_null_pointer(void* paquete_from_cpu, int socket_cpu){
	t_pcb* pcbRecibido =  deserializar_pcb(paquete_from_cpu);
	log_error(logger, "Finaliza el proceso #%d por null pointer", pcbRecibido->pid);
	pcbRecibido->exitCode = NULL_POINTER;
	terminarProceso(pcbRecibido, socket_cpu);
}

void terminarProceso(t_pcb* pcbRecibido, int socket_cpu){
	//modifico informacion estadistica
	estadisticaAumentarRafaga(pcbRecibido->pid);
	log_info(logger, "Proceso %d agregado a la cola de FINISHED", pcbRecibido->pid);
	estadisticaCambiarEstado(pcbRecibido->pid, FINISH);

	//pongo pcb en cola finish
	queue_push(colaFinished, pcbRecibido);

	//libero la cpu
	desocupar_cpu(socket_cpu);

	//aviso a consola que termino el proceso
	info_estadistica_t * info = buscarInformacion(pcbRecibido->pid);

	header_t* header=malloc(sizeof(header_t));
	if(!(pcbRecibido->exitCode == DESCONEXION_CONSOLA)){
		header->type=FINALIZAR_EJECUCION;
		header->length=sizeof(pcbRecibido->exitCode);
		sendSocket(info->socketConsola,header,&(pcbRecibido->exitCode));
	}

	header->type = FINALIZAR_PROGRAMA;
	header->length = sizeof(pcbRecibido->pid);
	sendSocket(socketConexionMemoria,header,&pcbRecibido->pid);

	cantProcesosSistema--;
	freePCB(pcbRecibido);
	free(header);
	planificarLargoPlazo();
}

void finalizacion_stackoverflow(void* paquete_from_cpu, int socket_cpu){
	t_pcb* pcbRecibido = deserializar_pcb(paquete_from_cpu);
	log_error(logger, "Finaliza el proceso %d por stack overflow", pcbRecibido->pid);
	pcbRecibido->exitCode = SUPERO_TAMANIO_PAGINA;
	terminarProceso(pcbRecibido, socket_cpu);
}

void finalizacion_quantum(void* paquete_from_cpu, int socket_cpu) {
	t_pcb* pcb_recibido =  deserializar_pcb(paquete_from_cpu);
	log_debug(logger, "Fin de quantum proceso %d", pcb_recibido->pid);

	estadisticaAumentarRafaga(pcb_recibido->pid);

	info_estadistica_t* info = buscarInformacion(pcb_recibido->pid);

	if(info->matarSiguienteRafaga){
		pcb_recibido->exitCode = info->exitCode;
		terminarProceso(pcb_recibido, socket_cpu);
		return;
	}else{
		// Se debe actualizar el PCB. Para ello, directamente se lo elimina de EXEC y se ingresa en READY el pcb recibido (que resulta ser el pcb actualizado del proceso).
		sem_wait(&mutex_cola_exec);
		//TODO: Remover PCB()
		sem_post(&mutex_cola_exec);

		// Se encola el pcb del proceso en READY.
		sem_wait(&mutex_cola_ready);
		queue_push(colaReady, pcb_recibido); // La planificación del PCP es Round Robin, por lo tanto lo inserto por orden de llegada.
		sem_post(&mutex_cola_ready);

		estadisticaCambiarEstado(pcb_recibido->pid, READY);

		//Aumento el semanforo de procesos en ready
		sem_post(&sem_cola_ready);
	}
	// Se desocupa la CPU
	desocupar_cpu(socket_cpu);
}

void finalizacion_proceso(void* paquete_from_cpu, int socket_cpu_asociado) {
	t_pcb* pcbRecibido = deserializar_pcb(paquete_from_cpu);
	log_debug(logger, "Finalizar proceso %d por fin de ejecucion", pcbRecibido->pid);

	pcbRecibido->exitCode = FINALIZO_BIEN;
	terminarProceso(pcbRecibido, socket_cpu_asociado);
}

int buscarEnTabla(pedido_mem pedido, int32_t ind){
	uint16_t i=0;
	reserva_memoria* reserva;
	while((reserva=list_get(mem_dinamica, i++))){
		if(reserva->pid == pedido.pid && reserva->size - sizeof(metadata_bloque) >= pedido.cant){
			if(i-1>ind) return --i;
		}
	}
	return -1;
}

int buscarPos(uint32_t pid, uint32_t pag){
	uint16_t i=0;
	reserva_memoria* reserva;
	while((reserva=list_get(mem_dinamica, i++))){
		if(reserva->pid == pid && reserva->pag == pag){
			return --i;
		}
	}
	return -1;
}

t_puntero verificarEspacio(uint32_t cant, uint32_t pid, uint32_t pag){
	uint ind=0;
	uint offset;
	t_list* datos;
	metadata_bloque* bloque;
	header_t header;
	offset = 0;
	header.type = SOLICITUD_BYTES;
	header.length = sizeof(metadata_bloque);
	t_pedido_bytes pedido;
	datos=dictionary_get(bloques,string_itoa(pag));
	while(ind<list_size(datos) && offset<pagina_size){
		do{
			bloque=list_get(datos,ind);
			pedido.size = header.length;
			offset += pedido.size+bloque->size;
		}
		while(bloque->used);
		if(bloque->size >= cant)
			return pagina_size * pag + offset - bloque->size;
	}
	return 0;
}

void reservarMemoria(int socket, char* paquete){
	pedido_mem pedido_memoria;
	t_puntero puntero;
	header_t* header;
	int resultado;
	size_t tamano = sizeof(uint32_t)*2;
	metadata_bloque bloque;
	reserva_memoria* reserva;
	t_pedido_iniciar* pedido;
	t_pedido_bytes bytes;

	memcpy(&pedido_memoria, paquete, tamano);

	uint pos = buscarEnTabla(pedido_memoria,-1);
	void* package;
	while(pos != -1){
		reserva = list_get(mem_dinamica, pos);
		puntero = verificarEspacio(pedido_memoria.cant,reserva->pid,reserva->pag);
		if(puntero != 0){
			header=malloc(sizeof(header_t));
			header->type = RESERVAR_MEMORIA_OK;
			header->length = sizeof(t_puntero);
			sendSocket(socket, header, &puntero);

			reserva->size -= pedido_memoria.cant+sizeof(metadata_bloque);
			header->type = GRABAR_BYTES;
			header->length = sizeof(metadata_bloque)+sizeof(t_pedido_bytes);
			package = malloc(header->length);

			bytes.pid = reserva->pid;
			bytes.pag = reserva->pag;
			bytes.size = sizeof(metadata_bloque);
			bytes.offset = puntero;
			bloque.used = true;
			bloque.size = pedido_memoria.cant;

			memcpy(package, &bloque, sizeof(metadata_bloque));
			memcpy(package+sizeof(metadata_bloque), &bytes, sizeof(t_pedido_bytes));
			sendSocket(socketConexionMemoria, header, package);
			free(header);
			return;
		}
		pos = buscarEnTabla(pedido_memoria,pos);
	}
	if(paquete)free(paquete);
	datos = list_create();
	pedido = malloc(sizeof(t_pedido_iniciar));
	pedido->pid = pedido_memoria.pid;
	pedido->cant_pag = 1;
	header=malloc(sizeof(header_t));
	header->type = ASIGNAR_PAGINAS;
	header->length = sizeof(t_pedido_iniciar);
	sendSocket(socketConexionMemoria, header, pedido);
	free(header);
	free(pedido);
	info_estadistica_t* info = buscarInformacion(pedido_memoria.pid);
	recibir_paquete(socketConexionMemoria, (void*)&paquete, &resultado);
	if(resultado == OP_OK){
		header_t header;
		header.type = RESERVAR_MEMORIA_OK;
		header.length = sizeof(uint32_t);
		metadata_bloque* metadata;
		metadata = malloc(sizeof(metadata_bloque));
		metadata->used = true;
		metadata->size = pedido_memoria.cant;
		reserva = malloc(sizeof(t_pedido_iniciar));
		reserva->pag = info->cantPaginasHeap;
		reserva->size = pagina_size - sizeof(metadata_bloque);
		reserva->pid = pedido_memoria.pid;
		list_add(mem_dinamica, reserva);
		bytes.pid = pedido_memoria.pid;
		bytes.size = sizeof(metadata);
		bytes.offset = 0;
		bytes.pag = reserva->pag;
		puntero = bytes.pag * pagina_size + sizeof(metadata_bloque);
		estadisticaAlocarBytes(pedido_memoria.pid, pedido_memoria.cant);
		aumentarEstadisticaPorSocketAsociado(socket, estadisticaAumentarAlocar);
		aumentarEstadisticaPorSocketAsociado(socket, estadisticaAumentarOpPriviligiada);
		sendSocket(socket, &header, &puntero);
		header.type = GRABAR_BYTES;
		header.length = sizeof(metadata_bloque);
		sendSocket(socketConexionMemoria, &header, metadata);
		list_add(datos, metadata);
		dictionary_put(bloques, string_itoa(bytes.pag), datos);
	}
}

void liberarMemoria(int socket, char* paquete){
	t_pedido_bytes pedido;
	header_t header;
	void* package;
	int tipo;
	size_t size;
	metadata_bloque metadata;
	size = sizeof(t_pedido_bytes);
	header.type = SOLICITUD_BYTES;
	header.length = sizeof(t_pedido_bytes);
	memcpy(&pedido, paquete, sizeof(t_pedido_bytes));
	int32_t valor = buscarPos(pedido.pid, pedido.pag);
	pedido.size = sizeof(metadata_bloque);
	pedido.offset -= sizeof(metadata_bloque);
	if(valor != -1 && pedido.offset > 0){
		sendSocket(socketConexionMemoria, &header, &pedido);
		recibir_paquete(socketConexionMemoria, &package, &tipo);
		memcpy(&metadata, package, pedido.size);
		free(package);
		info_estadistica_t* info = buscarInformacion(pedido.pid);
		if(tipo == OP_OK){
			if(metadata.used){
				pedido.size = sizeof(metadata.used);
				pedido.offset -= sizeof(metadata.size);
				header.type = GRABAR_BYTES;
				header.length = size+pedido.size;
				metadata.used = false;
				package = malloc(header.length);
				memcpy(package, &pedido, size);
				memcpy(package+sizeof(t_pedido_bytes), &metadata.used, pedido.size);
				if(sendSocket(socketConexionMemoria, &header, package)<=0){
					log_debug(logger, "problemas de conexion");
					free(package);
				}
				free(package);
				recibir_paquete(socketConexionMemoria, &package, &tipo);
				if(tipo == OP_OK){
					enviar_paquete_vacio(LIBERAR_MEMORIA_OK, socket);
					aumentarEstadisticaPorSocketAsociado(socket, estadisticaAumentarOpPriviligiada);
					aumentarEstadisticaPorSocketAsociado(socket, estadisticaAumentarLiberar);
				}
			}else{
				info->matarSiguienteRafaga = true;
			}
		}

	}
}

void desocupar_cpu(int socket_asociado) {

	sem_wait(&mutex_lista_CPUs);
	cpu_t *cpu = obtener_cpu_por_socket_asociado(socket_asociado);
	if(cpu != NULL){
			log_info(logger, "Se desocupa cpu %d\n", socket_asociado);
			cpu->disponible = true;
			sem_post(&semCPUs_disponibles); // Aumento el semaforo contador de cpus disponibles.
	}
	sem_post(&mutex_lista_CPUs);
}

/*
 * @NAME: obtener_cpu_por_socket_asociado
 * @DESC: Devuelve un puntero al cpu asociado a soc_asociado, si no lo encuentra devuelve NULL.
 */
cpu_t *obtener_cpu_por_socket_asociado(int soc_asociado){

	cpu_t *cpu_asociado = NULL;

	t_link_element *nodo = listaCPUs->head;

	while(nodo != NULL && cpu_asociado == NULL){
		cpu_t *cpu_aux = (cpu_t*) nodo->data;
		if(cpu_aux->socket == soc_asociado) {
			cpu_asociado = cpu_aux;
		} else {
			nodo = nodo->next;
		}
	}
	return cpu_asociado;
}

void abrirArchivo(int socketCpu, void* package){
	int sizePath, sizePermisos, pid, offset = 0;
	char* path;
	char* permisos;

	memcpy(package, &pid, sizeof(int)); offset += sizeof(int);
	memcpy(package, &sizePath, sizeof(int)); offset += sizeof(int);
	memcpy(package+offset, path, sizePath); offset += sizePath;
	memcpy(package+offset, &sizePermisos, sizeof(int)); offset += sizeof(int);
	memcpy(package+offset, permisos, sizePermisos);

	agregarArchivo_aProceso(pid, path, permisos);

	enviar_paquete_vacio(ABRIR_ARCHIVO_OK, socketCpu);
}

void borrarArchivo(int socketCpu, void* package){
	uint32_t fd = *(uint32_t*)package;
	int pid = *(int*) (package + sizeof(uint32_t));

	char* path = buscarPathDeArchivo(fd);

	header_t header;
	header.type = BORRAR_ARCHIVO;
	header.type = strlen(path);

	sendSocket(socketConexionFS, &header, path);

	int tipo;
	void* paquete;
	recibir_paquete(socketConexionFS, &paquete, &tipo);

	int respuesta;

	if(tipo == BORRAR_ARCHIVO_OK)
		respuesta = BORRAR_ARCHIVO_OK;
	else
		respuesta = ARCHIVO_INEXISTENTE;

	enviar_paquete_vacio(respuesta, socketCpu);

}

void cerrarArchivo(int socketCpu, void* package){
	uint32_t fd = *(uint32_t*)package;
	int pid = *(int*) (package + sizeof(uint32_t));

	eliminarFd(fd, pid);

	enviar_paquete_vacio(CERRAR_ARCHIVO_OK, socketCpu);
}

void escribir(void* paquete, int socketCpu){ // TODO
	uint32_t fd = *(uint32_t*)paquete;
	int pid = *(int*) (paquete + sizeof(uint32_t));
	int sizeEscritura = *(int*) (paquete + sizeof(uint32_t) + sizeof(int));
	char* escritura = paquete + sizeof(int) * 2 + sizeof(uint32_t);

	archivo* archivo = buscarArchivo(pid, fd);
	int offsetEscritura = archivo->cursor;

	info_estadistica_t * info = buscarInformacion(pid);

	header_t header;

	if(fd == 1){ // TODO - FALTA MANDARLE EL PAQUETE A FS - tiene que ser 1?
		int sizePedido = sizeof(int) + strlen(escritura) + 1;

		header.type=IMPRIMIR_POR_PANTALLA;
		header.length= sizePedido;

		char* buffer = malloc(sizePedido);
		memcpy(buffer, &pid, sizeof(int));
		memcpy(buffer+sizeof(int), escritura, strlen(escritura) + 1);

		sendSocket(info->socketConsola, &header, buffer);
	}else{
		char* path = buscarPathDeArchivo(fd);
		void * buffer = malloc(2*sizeof(int)+strlen(path)+strlen(escritura));
		int offset = 0, sizePath, sizeEscritura;


		memcpy(&offsetEscritura, buffer, sizeof(int)); offset += sizeof(int);
		memcpy(&sizeEscritura, buffer+offset, sizeof(int)); offset += sizeof(int);
		sizePath = strlen(path);
		memcpy(&sizePath, buffer+offset, sizeof(int)); offset += sizeof(int);
		memcpy(path, buffer+offset, strlen(path)); offset += strlen(path);
		sizeEscritura = strlen(sizeEscritura);
		memcpy(&sizeEscritura, buffer+offset, sizeof(int)); offset += sizeof(int);
		memcpy(escritura, buffer+offset, strlen(path));

		header.length = 2*sizeof(int)+strlen(path)+strlen(escritura);
		header.type = OBTENER_DATOS;

		sendSocket(socketConexionFS, &header, buffer);

		free(buffer);

		void* paquete;
		int tipo;

		recibir_paquete(socketConexionFS, &paquete, &tipo);
		if(tipo == ESCRITURA_OK){
			enviar_paquete_vacio(ESCRITURA_OK, socketCpu);
		}else{
			enviar_paquete_vacio(ARCHIVO_INEXISTENTE, socketCpu);
		}

	}
}

void leerArchivo(int socketCpu, void* package){
	int offset = 0;
	uint32_t fd = *(uint32_t*)package; offset += sizeof(uint32_t);
	int pid = *(int*) (package + offset); offset += sizeof(int);
	int size = *(int*) (package + offset);

	archivo* archivo = buscarArchivo(pid, fd);
	int offsetPedidoLectura = archivo->cursor;

	char* path = buscarPathDeArchivo(fd);
	void* buffer = malloc(2*sizeof(int)+strlen(path));

	offset = 0;
	memcpy(&offsetPedidoLectura, buffer, sizeof(int)); offset += sizeof(int);
	memcpy(&size, buffer+offset, sizeof(int)); offset += sizeof(int);
	int sizePath = strlen(path);
	memcpy(&sizePath, buffer+offset, sizeof(int)); offset += sizeof(int);
	memcpy(path, buffer+offset, strlen(path));

	header_t header;
	header.length = 2*sizeof(int)+strlen(path);
	header.type = OBTENER_DATOS;

	sendSocket(socketConexionFS, &header, buffer);

	void* paquete;
	int tipo;

	recibir_paquete(socketConexionFS, &paquete, &tipo);
	if(tipo == LEER_ARCHIVO_OK){
		header.length = size;
		header.type = LEER_ARCHIVO_OK;
		sendSocket(socketCpu, &header, paquete);
	}else{
		enviar_paquete_vacio(ARCHIVO_INEXISTENTE, socketCpu);
	}

}

void moverCursor(int socketCPU, t_cursor* cursor){ // TODO con esto alcanza?
	bool buscarPorProceso(entrada_tabla_archivo_proceso* entrada){
			return entrada->proceso == cursor->pid ? true : false;
		}

		bool buscarPorFd(archivo* archivo){
			return archivo->fd == cursor->descriptor ? true : false;
		}

		entrada_tabla_archivo_proceso* entrada = list_find(processFileTable, buscarPorProceso);
		archivo* archivo = 	list_find(entrada->archivos, eliminar);
		archivo->cursor = cursor->posicion;

}

void verificarProcesosConsolaCaida(int socketConsola){ // TODO pueden haber varios procesos
	info_estadistica_t* info = buscarInformacionPorSocketConsola(socketConsola);
	if(info->estado != FINISH){
		info->matarSiguienteRafaga = true;
		info->exitCode = DESCONEXION_CONSOLA;
		log_info(logger, "Se termina la ejecucion del proceso %d por desconexion de la consola", info->pid);
	}
}

info_estadistica_t* buscarInformacionPorSocketConsola(int socketConsola){

	bool buscar(info_estadistica_t* info){
		return info->socketConsola == socketConsola ? true : false;//&& (info->estado != FINISH) ? true : false;
	}

	return list_find(listadoEstadistico, buscar);
}
