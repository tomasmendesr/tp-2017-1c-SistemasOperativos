#include "operaciones.h"

void trabajarMensajeConsola(int socketConsola){

	int tipo_mensaje; //Para que la funcion recibir_string lo reciba
	void* paquete;
	int check = recibir_paquete(socketConsola, &paquete, &tipo_mensaje);

	FD_SET(socketConsola, &setConsolas);

	if (check <= 0) {
		log_warning(logger, "Se cerro el socket %d\n", socketConsola);
		close(socketConsola);
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
		log_info(logger,"Proceso agregado a la cola New");
		planificarLargoPlazo();
	break;
	case FINALIZAR_PROGRAMA:
		finalizarPrograma(consola_fd,atoi(package));
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
	if (check <= 0) {
		log_info(logger,"Se cerro el socket %d", socketCPU);
		close(socketCPU);
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
	case LIBERAR_MEMORIA: // TODO
		break;
	case IMPRIMIR_POR_PANTALLA:
		imprimirPorPantalla(package, socketCPU);
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

	default:
		log_warning(logger,"Se recibio un codigo de operacion invalido.");
	}
}

void leerVarCompartida(int socketCPU, char* variable){
	int valor = leerVariableGlobal(config->variablesGlobales, variable);

	header_t* header = malloc(sizeof(header_t));
	header->type = VALOR_VAR_COMPARTIDA;
	header->length = sizeof(valor);

	sendSocket(socketCPU, header, &valor);
	free(header);
}

void asignarVarCompartida(int socketCPU, void* buffer){
	char* variable;
	int valor, sizeVariable;

	memcpy(&sizeVariable, buffer, 4);
	variable = malloc(sizeof(sizeVariable));
	memcpy(variable, buffer+4, sizeVariable);
	memcpy(&valor, buffer+4+sizeVariable, 4);

	escribirVariableGlobal(config->variablesGlobales, variable, &valor);
	free(variable);

	aumentarEstadisticaPorSocketAsociado(socketCPU, estadisticaAumentarOpPriviligiada);
	enviar_paquete_vacio(ASIG_VAR_COMPARTIDA_OK, socketCPU);
}

void realizarSignal(int socketCPU, char* key){
	int valor;
	valor = semaforoSignal(config->semaforos, key);

	aumentarEstadisticaPorSocketAsociado(socketCPU, estadisticaAumentarOpPriviligiada);
	enviar_paquete_vacio(SIGNAL_OK, socketCPU);

	if(valor <= 0){
		desbloquearProceso(key);
		log_info(logger, "Desbloqueo un proceso");
	}
}

void realizarWait(int socketCPU, char* key){
	int valorSemaforo = semaforoWait(config->semaforos, key);
	int resultado;

	if(valorSemaforo < 0){
		resultado = WAIT_DETENER_EJECUCION;
	}else{
		resultado = WAIT_SEGUIR_EJECUCION;
	}

	aumentarEstadisticaPorSocketAsociado(socketCPU, estadisticaAumentarOpPriviligiada);
	enviar_paquete_vacio(resultado, socketCPU);

	if(resultado == WAIT_DETENER_EJECUCION){ //recibo el pcb

		int tipo_mensaje;
		void* paquete;
		recibir_paquete(socketCPU, &paquete, &tipo_mensaje);

		t_pcb* pcbRecibido = deserializar_pcb(paquete);
		bloquearProceso(key, pcbRecibido);
		desocupar_cpu(socketCPU);

		log_info(logger, "Bloqueo proceso %d", pcbRecibido->pid);
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
	header->type=FINALIZAR_EJECUCION;
	header->length=sizeof(pcbRecibido->exitCode);
	sendSocket(info->socketConsola,header,&(pcbRecibido->exitCode));

	header->type = FINALIZAR_PROGRAMA;
	header->length = sizeof(pcbRecibido->pid);
	sendSocket(socketConexionMemoria,header,&pcbRecibido->pid);

	cantProcesosSistema--;
	free(header);
}

void finalizacion_stackoverflow(void* paquete_from_cpu, int socket_cpu){
	t_pcb* pcbRecibido =  deserializar_pcb(paquete_from_cpu);
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
		queue_push(colaReady, pcb_recibido); 		// La planificación del PCP es Round Robin, por lo tanto lo inserto por orden de llegada.
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
cpu_t *obtener_cpu_por_socket_asociado(int soc_asociado) {

	cpu_t *cpu_asociado = NULL;

	t_link_element *nodo = listaCPUs->head;

	while (nodo != NULL && cpu_asociado == NULL) {
		cpu_t *cpu_aux = (cpu_t*) nodo->data;
		if(cpu_aux->socket == soc_asociado) {
			cpu_asociado = cpu_aux;
		} else {
			nodo = nodo->next;
		}
	}
	return cpu_asociado;
}

int buscarEnTabla(pedido_mem pedido, int32_t ind){
	uint16_t i=0;
	reserva_memoria* reserva;
	while(ind>i && (reserva=list_get(mem_dinamica, i++))){
		if(reserva->pid == pedido.pid && reserva->size - sizeof(metadata_bloque) >= pedido.cant){
			return --i;
		}
	}
	return -1;
}

t_puntero verificarEspacio(uint32_t cant, uint32_t pid, uint32_t pag){
	void* paquete;
	int tipo;
	int offset;
	metadata_bloque bloque;
	header_t header;
	offset = 0;
	header.type = SOLICITUD_BYTES;
	header.length = sizeof(metadata_bloque);
	t_pedido_bytes pedido;
	while(offset < pagina_size){
		do{
			pedido.pid = pid;
			pedido.pag = pag;
			pedido.offset = offset;
			pedido.size = header.length;
			sendSocket(socketConexionMemoria, &header, &pedido);
			recibir_paquete(socketConexionMemoria, &paquete, &tipo);
			memcpy(&bloque, paquete, header.length);
			offset += pedido.size+bloque.size;
		}
		while(bloque.used);
		if(bloque.size >= cant)
			return pagina_size * pag + offset - bloque.size;
	}
	return 0;
}

void reservarMemoria(int socket, void* paquete){

	pedido_mem pedido_memoria;
	t_puntero puntero;
	header_t* header;
	int resultado;
	size_t tamano = sizeof(uint32_t)*2;
	metadata_bloque bloque;
	reserva_memoria* reserva;
	t_pedido_iniciar* pedido;

	memcpy(&pedido_memoria, paquete, tamano);

	uint pos = buscarEnTabla(pedido_memoria,-1);
	if(pos != -1){
		reserva = list_get(mem_dinamica, pos);
		puntero = verificarEspacio(pedido_memoria.cant,reserva->pid,reserva->pag);
		if(puntero != 0){
			header=malloc(sizeof(header_t));
			header->type = RESERVAR_MEMORIA_OK;
			header->length = sizeof(t_puntero);
			sendSocket(socket, header, &puntero);

			reserva->size -= pedido_memoria.cant+sizeof(metadata_bloque);

			header->type = GRABAR_BYTES;
			header->length = sizeof(metadata_bloque);
			bloque.used = true;
			bloque.size = pedido_memoria.cant;
			sendSocket(socketConexionMemoria, header, &bloque);
			free(header);
		}
	}
	else{
		pedido = malloc(sizeof(t_pedido_iniciar));
		pedido->pid = pedido_memoria.pid;
		pedido->cant_pag = 1;
		header=malloc(sizeof(header_t));
		header->type = ASIGNAR_PAGINAS;
		header->length = sizeof(t_pedido_iniciar);
		sendSocket(socketConexionMemoria, header, pedido);
		free(header);

		recibir_paquete(socketConexionMemoria, &paquete, &resultado);
		if(resultado == OP_OK){
			reserva = malloc(sizeof(t_pedido_iniciar));
			reserva->pag = atoi(paquete);
		}

	}
	aumentarEstadisticaPorSocketAsociado(socket, estadisticaAumentarOpPriviligiada);
}

	void imprimirPorPantalla(void* paquete, int socketCpu){
/*	char* informacion = paquete;
	int* pid = paquete + strlen(informacion) + 1;

	info_estadistica_t * info = buscarInformacion(pid);
	printf("pid a escribir %d\n", pid);
	header_t header;
	header.type=IMPRIMIR_POR_PANTALLA;
	header.length= strlen(informacion) + 1;

	printf("imprimo %s\n", informacion);
*/
	//sendSocket(info->socketConsola, &header, (void*) imprimir->info);
}
