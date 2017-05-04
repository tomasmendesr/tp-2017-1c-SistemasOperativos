#include "operaciones.h"

void trabajarMensajeConsola(int socketConsola){

	int tipo_mensaje; //Para que la funcion recibir_string lo reciba
	void* paquete;
	int check = recibir_paquete(socketConsola, &paquete, &tipo_mensaje);

	FD_SET(socketConsola, &setConsolas);

	if (check <= 0) {
		printf("Se cerro el socket %d\n", socketConsola);
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
		log_info(logger, "recibo codigo");
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
		//finalizarPrograma(consola_fd,package);
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
  t_pcb* pcbRecibido;
	switch(mensaje){
	case HANDSHAKE_CPU:
		log_info(logger,"Conexion con nueva CPU establecida");
		enviar_paquete_vacio(HANDSHAKE_KERNEL,socketCPU);
		enviarTamanioStack(socketCPU);
		agregarNuevaCPU(listaCPUs, socketCPU);
//		enviarQuantum(socketCPU);
		break;
	/*case ENVIO_PCB:
		break;*/
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

	/* CPU DEVUELVE EL PCB */
	case FIN_PROCESO:
		//finalizacion_proceso(void* paquete_from_cpu, int socket_cpu_asociado);
		break;
	case FIN_EJECUCION:
		finalizacion_quantum(package,socketCPU);
		break;
	/* ERRORES */
	case SEGMENTATION_FAULT:
		break;
	case STACKOVERFLOW:
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
	enviar_paquete_vacio(RESPUESTA_ASIG_VAR_COMPARTIDA_OK, socketCPU);
}

void realizarSignal(int socketCPU, char* key){
	semaforoSignal(config->semaforos, key);

	aumentarEstadisticaPorSocketAsociado(socketCPU, estadisticaAumentarOpPriviligiada);
	enviar_paquete_vacio(RESPUESTA_SIGNAL_OK, socketCPU);
}

void realizarWait(int socketCPU, char* key){
	int valorSemaforo = semaforoWait(config->semaforos, key);
	int resultado;

	if(valorSemaforo <= 0){
		resultado = RESPUESTA_WAIT_DETENER_EJECUCION;
	}else{
		resultado = RESPUESTA_WAIT_SEGUIR_EJECUCION;
	}

	aumentarEstadisticaPorSocketAsociado(socketCPU, estadisticaAumentarOpPriviligiada);
	enviar_paquete_vacio(resultado, socketCPU);
}

void finalizacion_quantum(void* paquete_from_cpu, int socket_cpu) {

	t_pcb* pcb_recibido =  deserializar_pcb(paquete_from_cpu);

	estadisticaAumentarRafaga(pcb_recibido->pid);

	info_estadistica_t* info = buscarInformacion(pcb_recibido->pid);

	if(info->matarSiguienteRafaga){
		estadisticaCambiarEstado(pcb_recibido->pid, FINISH);
		queue_push(colaFinished, pcb_recibido);
	}else{

		// Se debe actualizar el PCB. Para ello, directamente se lo elimina de EXEC y se ingresa en READY el pcb recibido (que resulta ser el pcb actualizado del proceso).
		sem_wait(&mutex_cola_exec);
		//TODO: Remover PCB()
		sem_wait(&mutex_cola_exec);

		// Se encola el pcb del proceso en READY.
		sem_wait(&mutex_cola_ready);
		queue_push(colaReady, pcb_recibido); 		// La planificación del PCP es Round Robin, por lo tanto lo inserto por orden de llegada.
		sem_post(&mutex_cola_ready);

		//Aumento el semanforo de procesos en ready
		sem_post(&sem_cola_ready);
	}

	// Se desocupa la CPU
	desocupar_cpu(socket_cpu);

}
void finalizacion_proceso(void* paquete_from_cpu, int socket_cpu_asociado) {
	t_pcb* pcbRecibido = deserializar_pcb(paquete_from_cpu);
}
void desocupar_cpu(int socket_asociado) {

	sem_wait(&mutex_lista_CPUs);
	cpu_t *cpu = obtener_cpu_por_socket_asociado(socket_asociado);
	if(cpu != NULL){
		if(cpu->pcb != NULL) { //TODO: nuevo fijarse si solo con el pcb en null alcanza
			cpu->pcb = NULL;
			sem_post(&semCPUs_disponibles); // Aumento el semaforo contador de cpus disponibles.
		}
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

