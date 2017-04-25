#include "operaciones.h"

void trabajarMensajeConsola(int socketConsola){

	int tipo_mensaje; //Para que la funcion recibir_string lo reciba
	void* paquete;
	int check = recibir_paquete(socketConsola, &paquete, &tipo_mensaje);

	//Chequeo de errores
	if (check <= 0) {
		printf("Se cerro el socket %d\n", socketConsola);
		close(socketConsola);
		FD_CLR(socketConsola, &master);
		FD_CLR(socketConsola, &setConsolas);
	}else{
		procesarMensajeConsola(socketConsola, tipo_mensaje, paquete);
	}

	FD_SET(socketConsola, &master);
}

void procesarMensajeConsola(int consola_fd, int mensaje, char* package){

	pcb_t* nuevoProceso;

	switch(mensaje){
	case HANDSHAKE_PROGRAMA:
		enviar_paquete_vacio(HANDSHAKE_KERNEL,consola_fd);
		log_info(&logger_kernel,"Conexion con la consola establecida");
		break;
	case ENVIO_CODIGO:
	nuevoProceso = crearProceso(consola_fd, package);
	if(nuevoProceso->pid == 0){
		log_error(&logger_kernel,"Se produjo un error en intentar guardar los datos del proceso.");
	}else{
		//Admitido en el sistema con todas sus estructuras guardadas en memoria.
		log_info(&logger_kernel,"Proceso creado correctamente %d",nuevoProceso->pid);
		sem_wait(&mutex_cola_ready);
		queue_push(colaReady,nuevoProceso);
		sem_post(&mutex_cola_ready);
	}
	break;
	case FINALIZAR_PROGRAMA:
		//finalizarPrograma(consola_fd,package);
		break;
	default: log_warning(&logger_kernel,"Se recibio un codigo de operacion invalido.");
	break;
	}

}

void trabajarMensajeCPU(int socketCPU){

	int tipo_mensaje; //Para que la funcion recibir_string lo reciba
	void* paquete;
	int check = recibir_paquete(socketCPU, &paquete, &tipo_mensaje);

	//Chequeo de errores
	if (check <= 0) {
		printf("Se cerro el socket %d\n", socketCPU);
		close(socketCPU);
		FD_CLR(socketCPU, &master);
		FD_CLR(socketCPU, &setCPUs);
	}else{
		procesarMensajeCPU(socketCPU, tipo_mensaje, paquete);
	}

	FD_SET(socketCPU, &master);
}

void procesarMensajeCPU(int socketCPU, int mensaje, char* package){

	switch(mensaje){
	case ENVIO_PCB:
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
	default:
		log_warning(&logger_kernel,"Se recibio un codigo de operacion invalido.");
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
	memcpy(variable, buffer+4, sizeVariable);
	memcpy(&valor, buffer+4+sizeVariable, 4);

	escribirVariableGlobal(config->variablesGlobales, variable, &valor);

	enviar_paquete_vacio(OK, socketCPU);
}

void realizarSignal(int socketCPU, char* key){
	semaforoSignal(config->semaforos, key);

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

	enviar_paquete_vacio(resultado, socketCPU);
}

