#include "operaciones.h"

void trabajarMensajeConsola(int32_t socketConsola){

	int32_t tipo_mensaje; //Para que la funcion recibir_string lo reciba
	void* paquete;
	int32_t check = recibir_paquete(socketConsola, &paquete, &tipo_mensaje);

	FD_SET(socketConsola, &setConsolas);

	if(check <= 0){
		log_warning(logger, "Se cerro el socket %d (Consola)", socketConsola);
		verificarProcesosConsolaCaida(socketConsola);
		close(socketConsola);
		if(paquete)free(paquete);
		FD_CLR(socketConsola, &master);
		FD_CLR(socketConsola, &setConsolas);
	}else{
		procesarMensajeConsola(socketConsola, tipo_mensaje, paquete);
	}

}

void procesarMensajeConsola(int32_t consola_fd, int32_t mensaje, char* package){
	proceso_en_espera_t* nuevoProceso;

	switch(mensaje){
	case HANDSHAKE_PROGRAMA:
		enviar_paquete_vacio(HANDSHAKE_KERNEL,consola_fd);
		log_info(logger,"Handshake con consola");
		printf("\n");
		break;
	case ENVIO_CODIGO:
		log_info(logger, "Recibo codigo");
		nuevoProceso = crearProcesoEnEspera(consola_fd, package);
		sem_wait(&mutex_cola_new);
		queue_push(colaNew, nuevoProceso);
		sem_post(&mutex_cola_new);
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

void trabajarMensajeCPU(int32_t socketCPU){

	int32_t tipo_mensaje; //Para que la funcion recibir_string lo reciba
	void* paquete;
	int32_t check = recibir_paquete(socketCPU, &paquete, &tipo_mensaje);

	//Chequeo de errores
	if(check <= 0){
		log_warning(logger,"Se cerro el socket %d (cpu)", socketCPU);
		verificarProcesosEnCpuCaida(socketCPU);
		close(socketCPU);
		if(paquete)free(paquete);
		FD_CLR(socketCPU, &master);
		FD_CLR(socketCPU, &setCPUs);
	}else{
		procesarMensajeCPU(socketCPU, tipo_mensaje, paquete);
	}

	FD_SET(socketCPU, &setCPUs);
}

void procesarMensajeCPU(int32_t socketCPU, int32_t mensaje, char* package){
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
		leerArchivo(socketCPU, (t_lectura*) package);
		break;
	case MOVER_CURSOR:
		moverCursor(socketCPU, (t_cursor*) package);
		break;
	/* CPU DEVUELVE EL PCB */
	case FIN_PROCESO:
		finalizacion_proceso(package, socketCPU);
		break;
	case FIN_EJECUCION:
		finalizacion_quantum(package,socketCPU);
		break;
	/* ERRORES */
	case STACKOVERFLOW:
		finalizacion_stackoverflow(package, socketCPU);
		break;
	case ERROR_MEMORIA:
	case SEMAFORO_NO_EXISTE:
	case GLOBAL_NO_DEFINIDA:
	case NULL_POINTER:
	case ARCHIVO_INEXISTENTE:
	case FALLA_RESERVAR_RECURSOS:
	case SEGMENTATION_FAULT:
	case SIN_ESPACIO_FS:
		finalizacion_error(package, socketCPU, mensaje);
		break;
	default:
		log_warning(logger,"Se recibio el codigo de operacion invalido.");
	}
}

void leerVarCompartida(int32_t socketCPU, char* variable){
	if(dictionary_has_key(config->variablesGlobales, variable)){
		if(dictionary_get(config->variablesGlobales, variable) == NULL){
			log_error(logger, "El valor de %s es NULL", variable);
			 enviar_paquete_vacio(NULL_POINTER, socketCPU);
		}
		else{
			int32_t valor = leerVariableGlobal(config->variablesGlobales, variable);
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

void asignarVarCompartida(int32_t socketCPU, void* buffer){
	char* variable;
	int32_t valor, sizeVariable;
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

void realizarSignal(int32_t socketCPU, char* key){
	aumentarEstadisticaPorSocketAsociado(socketCPU, estadisticaAumentarOpPriviligiada);
	if(dictionary_has_key(config->semaforos, key)){
		if(dictionary_get(config->semaforos, key) == NULL){
			log_error(logger, "El valor de %s es NULL", key);
			 enviar_paquete_vacio(NULL_POINTER, socketCPU);
			 return;
		}
		else{
			int32_t valor = semaforoSignal(config->semaforos, key);
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

void realizarWait(int32_t socketCPU, char* key){
	int32_t resultado;
	aumentarEstadisticaPorSocketAsociado(socketCPU, estadisticaAumentarOpPriviligiada);

	if(dictionary_has_key(config->semaforos,key)){
		if(dictionary_get(config->semaforos, key) == NULL){
			log_error(logger, "El valor de %s es NULL", key);
			 enviar_paquete_vacio(NULL_POINTER, socketCPU);
			 return;
		}
		else{
			int32_t valor = semaforoWait(config->semaforos, key);
			if(valor < 0) resultado = WAIT_DETENER_EJECUCION;
			else resultado = WAIT_SEGUIR_EJECUCION;

			enviar_paquete_vacio(resultado, socketCPU);

			if(resultado == WAIT_DETENER_EJECUCION){ //recibo el pcb

				int32_t tipo_mensaje;
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

void finalizarPrograma(int32_t consola_fd, int32_t pid){
	info_estadistica_t* info = buscarInformacion(pid);
	info->matarSiguienteRafaga = true;
	info->exitCode = FINALIZAR_DESDE_CONSOLA;
	log_info(logger, "Se termina la ejecucion del proceso #%d por comando STOP (Consola)", pid);
}

void finalizacion_segment_fault(void* paquete_from_cpu, int32_t socket_cpu){
	t_pcb* pcbRecibido =  deserializar_pcb(paquete_from_cpu);
	log_error(logger, "Finaliza el proceso #%d por segmentation fault", pcbRecibido->pid);
	pcbRecibido->exitCode = ERROR_MEMORIA;
	terminarProceso(pcbRecibido, socket_cpu);
}

void finalizacion_error(void* paquete_from_cpu, int32_t socket_cpu, int32_t exitCode){
	t_pcb* pcbRecibido = deserializar_pcb(paquete_from_cpu);
	log_error(logger, "Finaliza el proceso #%d por error", pcbRecibido->pid);
	pcbRecibido->exitCode = exitCode;
	terminarProceso(pcbRecibido, socket_cpu);
}

void terminarProceso(t_pcb* pcbRecibido, int32_t socket_cpu){
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

	quitarDeMemoriaDinamica(pcbRecibido->pid);

	header->type = FINALIZAR_PROGRAMA;
	header->length = sizeof(pcbRecibido->pid);
	sendSocket(socketConexionMemoria,header,&pcbRecibido->pid);

	cantProcesosSistema--;
	freePCB(pcbRecibido);
	free(header);
	planificarLargoPlazo();
}

void finalizacion_stackoverflow(void* paquete_from_cpu, int32_t socket_cpu){
	t_pcb* pcbRecibido = deserializar_pcb(paquete_from_cpu);
	log_error(logger, "Finaliza el proceso %d por stack overflow", pcbRecibido->pid);
	pcbRecibido->exitCode = SUPERO_TAMANIO_PAGINA;
	terminarProceso(pcbRecibido, socket_cpu);
}

void finalizacion_quantum(void* paquete_from_cpu, int32_t socket_cpu) {
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

void finalizacion_proceso(void* paquete_from_cpu, int32_t socket_cpu_asociado) {
	t_pcb* pcbRecibido = deserializar_pcb(paquete_from_cpu);
	log_debug(logger, "Finalizar proceso %d por fin de ejecucion", pcbRecibido->pid);

	pcbRecibido->exitCode = FINALIZO_BIEN;
	terminarProceso(pcbRecibido, socket_cpu_asociado);
}

t_puntero verificarEspacio(uint32_t cant, uint32_t pid, uint32_t pag){
	uint32_t ind=0;
	uint32_t offset=0;
	t_list* datos;
	t_entrada_datos* entrada;
	t_bloque* bloque;

	bool buscar(t_entrada_datos* entrada){
		return entrada->pid == pid;
	}
	sem_wait(&mutex_datos);
	entrada = list_find(bloques,buscar);
	sem_post(&mutex_datos);

	datos = entrada->list;
	while(offset<pagina_size){
		do{
			bloque=list_get(datos, ind++);
			offset+=sizeof(meta_bloque)+bloque->size;
		}
		while(bloque->used);
		if(bloque->size-sizeof(meta_bloque) >= cant){
			return pagina_size*pag + offset-bloque->size;
		}
	}
	return 0;
}

void reservarMemoria(int32_t socket, char* paquete){
	pedido_mem pedido_memoria;
	t_puntero posicion;
	header_t* header;
	int32_t resultado;
	int32_t pid, pag, cant;
	size_t tamano = sizeof(uint32_t)*3;
	meta_bloque metadata;
	t_bloque* bloque;
	reserva_memoria* reserva;
	t_pedido_iniciar* pedido;
	t_pedido_bytes bytes;
	int32_t ind = 0;
	int32_t sizeBloque;
	t_entrada_datos* entrada;
	void* package;
	memcpy(&pedido_memoria, paquete, tamano);

	if(pedido_memoria.cant > pagina_size - sizeof(meta_bloque)*2){
		enviar_paquete_vacio(FALLA_RESERVAR_RECURSOS,socket);
		return;
	}
	bool buscarEntrada(t_entrada_datos* entrada){
		return entrada->pid == pedido_memoria.pid;
	}
	sem_wait(&mutex_datos);
	entrada = list_find(bloques,buscarEntrada);
	sem_post(&mutex_datos);
	sem_wait(&mutex_dinamico);
	reserva = list_get(mem_dinamica, ind++);
	sem_post(&mutex_dinamico);

	if(entrada){
		while(reserva){
		if(reserva->pid == pedido_memoria.pid){
			posicion = verificarEspacio(cant=pedido_memoria.cant,pid=reserva->pid,pag=reserva->pag);

			if(posicion != 0){
				bool buscar(t_bloque* bloque){
					return bloque->pos == posicion;
				}
				bloque = list_find(entrada->list,buscar);
				header = malloc(sizeof(header_t));
				header->type = GRABAR_BYTES;
				header->length = sizeof(meta_bloque)+sizeof(t_pedido_bytes);
				reserva->size -= pedido_memoria.cant+sizeof(meta_bloque);

				package = malloc(header->length);
				bytes.pid = reserva->pid;
				bytes.pag = reserva->pag;
				bytes.size = sizeof(meta_bloque);
				bytes.offset = (posicion-sizeof(meta_bloque))%pagina_size;

				metadata.used = true;
				metadata.size = pedido_memoria.cant;
				memcpy(package, &bytes, sizeof(t_pedido_bytes));
				memcpy(package+sizeof(t_pedido_bytes), &metadata, sizeof(meta_bloque));
				sizeBloque = bloque->size;
				bloque->used = metadata.used;
				bloque->size = metadata.size;

				sendSocket(socketConexionMemoria, header, package);
				recibir_paquete(socketConexionMemoria, &paquete, &resultado);
				if(resultado == OP_OK){
					bloque = malloc(sizeof(t_bloque));
					metadata.used = false;
					metadata.size = sizeBloque - pedido_memoria.cant - sizeof(meta_bloque);
					bytes.pid = reserva->pid;
					bytes.pag = reserva->pag;
					bytes.size = sizeof(meta_bloque);
					bytes.offset = (posicion+pedido_memoria.cant)%pagina_size;
					bloque->used = metadata.used;
					bloque->size = metadata.size;
					bloque->pos = posicion + pedido_memoria.cant + sizeof(meta_bloque);

					list_add(entrada->list,bloque);
					memcpy(package, &bytes, sizeof(t_pedido_bytes));
					memcpy(package+sizeof(t_pedido_bytes), &metadata, sizeof(meta_bloque));
					sendSocket(socketConexionMemoria, header, package);
					recibir_paquete(socketConexionMemoria, &paquete, &resultado);
					header->type = RESERVAR_MEMORIA_OK;
					header->length = sizeof(t_puntero);

					if(resultado == OP_OK){
						log_debug(logger,"Reserva exitosa con posicion %d",posicion);
						sendSocket(socket, header, &posicion);
						free(header);
						return;
					}
					else{
						log_error(logger,"Segmentation fault");
						enviar_paquete_vacio(SEGMENTATION_FAULT,socket);
						return;
					}
				}
				else{
					log_error(logger,"Segmentation fault");
					enviar_paquete_vacio(SEGMENTATION_FAULT,socket);
					return;
				}
			}
		}
		sem_wait(&mutex_dinamico);
		reserva = list_get(mem_dinamica, ind++);
		sem_post(&mutex_dinamico);
		}
	}
	if(paquete)free(paquete);
	bool valor = false;
	if(entrada == NULL){
		entrada = malloc(sizeof(t_entrada_datos));
		entrada->pid = pedido_memoria.pid;
		entrada->list = list_create();
		valor = true;
	}

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
	recibir_paquete(socketConexionMemoria, &paquete, &resultado);

	if(resultado == OP_OK){
		header_t header;
		metadata.used = true;
		metadata.size = pedido_memoria.cant;
		reserva = malloc(sizeof(reserva_memoria));
		reserva->pag = pedido_memoria.pagBase+info->cantPaginasHeap+config->stack_Size;
		reserva->size = pagina_size - sizeof(meta_bloque);
		reserva->pid = pedido_memoria.pid;

		sem_wait(&mutex_dinamico);
		list_add(mem_dinamica, reserva);
		sem_post(&mutex_dinamico);

		bytes.pid = pedido_memoria.pid;
		bytes.size = sizeof(metadata);
		bytes.offset = 0;
		bytes.pag = reserva->pag;
		posicion = bytes.pag * pagina_size + sizeof(meta_bloque);
		bloque = malloc(sizeof(t_bloque));
		bloque->used = metadata.used;
		bloque->size = metadata.size;
		bloque->pos = posicion;

		estadisticaAlocarBytes(pedido_memoria.pid, pedido_memoria.cant);
		aumentarEstadisticaPorSocketAsociado(socket, estadisticaAumentarAlocar);
		aumentarEstadisticaPorSocketAsociado(socket, estadisticaAumentarOpPriviligiada);
		info->cantPaginasHeap++;
		reserva->size -= pedido_memoria.cant;
		header.type = GRABAR_BYTES;
		header.length = sizeof(meta_bloque)+sizeof(t_pedido_bytes);

		package = malloc(header.length);
		memcpy(package, &bytes, sizeof(t_pedido_bytes));
		memcpy(package+sizeof(t_pedido_bytes), &metadata, sizeof(meta_bloque));
		sendSocket(socketConexionMemoria, &header, package);
		list_add(entrada->list, bloque);
		recibir_paquete(socketConexionMemoria, &paquete, &resultado);

		if(resultado == OP_OK){
			reserva->size -= sizeof(meta_bloque);
			metadata.used = false;
			metadata.size = pagina_size - sizeof(meta_bloque)*2 - pedido_memoria.cant;
			bytes.offset = sizeof(meta_bloque)+pedido_memoria.cant;
			bloque = malloc(sizeof(t_bloque));
			bloque->used = metadata.used;
			bloque->size = metadata.size;
			bloque->pos = posicion + bytes.offset;

			list_add(entrada->list, bloque);
			memcpy(package, &bytes, sizeof(t_pedido_bytes));
			memcpy(package+sizeof(t_pedido_bytes), &metadata, sizeof(meta_bloque));
			sendSocket(socketConexionMemoria, &header, package);
			recibir_paquete(socketConexionMemoria, &paquete, &resultado);
			header.type = RESERVAR_MEMORIA_OK;
			header.length = sizeof(uint32_t);

			if(resultado == OP_OK){
				if(valor){
					sem_wait(&mutex_datos);
					list_add(bloques, entrada);
					sem_post(&mutex_datos);
				}
				log_debug(logger, "Reserva exitosa con posicion %d",posicion);
				sendSocket(socket, &header, &posicion);
			}
			else{
				log_error(logger,"Segmentation fault");
				enviar_paquete_vacio(SEGMENTATION_FAULT,socket);
			}
			return;
		}
		else{
			log_error(logger,"Segmentation fault");
			enviar_paquete_vacio(SEGMENTATION_FAULT,socket);
		}
	}
	else{
		log_error(logger,"Memoria se quedo sin espacio");
		enviar_paquete_vacio(FALLA_RESERVAR_RECURSOS,socket);
	}
}

int32_t buscarEntrada(uint32_t pid){
	t_entrada_datos* item;
	int32_t k;
	for(k=0; k<bloques->elements_count; k++){
		item = list_get(bloques,k);
		if(item->pid == pid) return k;
	}
	return -1;
}

void liberarMemoria(int32_t socket, char* paquete){
	t_pedido_bytes pedido;
	header_t header;
	void* package;
	int32_t tipo;
	int32_t pid, posicion;
	size_t size;
	meta_bloque metadata;
	size = sizeof(t_pedido_bytes);
	int32_t tamano = sizeof(uint32_t);
	reserva_memoria* reserva;
	t_list* list;
	t_bloque* bloque;
	t_entrada_datos* entrada;
	uint16_t pos=0;
	uint16_t resultado,ind=0;
	memcpy(&pid, paquete, tamano);
	memcpy(&posicion, paquete+tamano, tamano);

	pedido.pid = pid;
	pedido.pag = posicion/pagina_size;
	free(paquete);
	sem_wait(&mutex_dinamico);
	reserva = list_get(mem_dinamica, pos++);
	sem_post(&mutex_dinamico);

	while(reserva){
	if(reserva->pid == pid && reserva->pag == pedido.pag){
		if(reserva->size <= pagina_size - sizeof(meta_bloque) && reserva->size > 0){
			sem_wait(&mutex_datos);
			resultado=buscarEntrada(pid);
			entrada = list_get(bloques,resultado);
			sem_post(&mutex_datos);
			list = entrada->list;

			while((bloque = list_get(list, ind))){
				if(bloque->pos == posicion){
					if(bloque->used){
						log_info(logger,"Se libera la posicion: %d", bloque->pos);
						header.type = GRABAR_BYTES;
						header.length = size + sizeof(meta_bloque);

						metadata.used = bloque->used = false;
						metadata.size = bloque->size;
						reserva->size += metadata.size;
						package = malloc(header.length);

						pedido.size = metadata.size;
						pedido.offset = posicion % pagina_size - sizeof(meta_bloque);
						memcpy(package, &pedido, size);
						memcpy(package+sizeof(t_pedido_bytes), &metadata, sizeof(meta_bloque));

						if(sendSocket(socketConexionMemoria, &header, package) <= 0){
							log_debug(logger, "problemas de conexion");
							free(package);
						}
						free(package);

						enviar_paquete_vacio(LIBERAR_MEMORIA_OK, socket);
						aumentarEstadisticaPorSocketAsociado(socket, estadisticaAumentarOpPriviligiada);
						aumentarEstadisticaPorSocketAsociado(socket, estadisticaAumentarLiberar);

						bool notEmpty = false;
						uint16_t k = 0;
						while(k < list->elements_count && !notEmpty){
							bloque = list_get(list, k++);
							if(bloque->used && reserva->pag == bloque->pos/pagina_size){
								notEmpty = true;
							}
						}
						recibir_paquete(socketConexionMemoria, &paquete, &tipo);

						if(tipo == OP_OK){
							if(!notEmpty){
								pedido.offset = 0;
								pedido.size = sizeof(meta_bloque);

								package = malloc(header.length);
								metadata.size = pagina_size - sizeof(meta_bloque);
								memcpy(package, &pedido, size);
								memcpy(package+sizeof(t_pedido_bytes), &metadata, sizeof(meta_bloque));
								sendSocket(socketConexionMemoria, &header, package);
								recibir_paquete(socketConexionMemoria, &paquete, &tipo);

								k=0;
								while((bloque = list_get(list,k))){
									if(bloque->pos/pagina_size == reserva->pag){
										free(list_remove(list, k));
										reserva->size += sizeof(meta_bloque);
									}
									else k++;
								}

								if(tipo == OP_OK){
									header.type = LIBERAR_PAGINA;
									header.length = sizeof(uint32_t)*2;

									t_pedido_iniciar pedido;
									pedido.pid = pid;
									pedido.cant_pag = reserva->pag;
									reserva->size -= sizeof(meta_bloque);

									sem_wait(&mutex_dinamico);
									free(list_remove(mem_dinamica,pos-1));
									sem_post(&mutex_dinamico);

									printf("pag %d\n",reserva->pag);

									sendSocket(socketConexionMemoria, &header, &pedido);
									recibir_paquete(socketConexionMemoria, &paquete, &tipo);
									free(package);
								}
								else{
									log_error(logger,"Segmentation fault");
									enviar_paquete_vacio(SEGMENTATION_FAULT,socket);
									free(package);
								}
								return;
							}
						}
						else{
							log_error(logger,"Segmentation fault");
							enviar_paquete_vacio(SEGMENTATION_FAULT,socket);
						}
						return;
					}
					else{
						log_error(logger,"No se puede liberar la memoria");
						enviar_paquete_vacio(NULL_POINTER,socket);
					}
					return;
				}
				ind++;
			}
			log_error(logger,"No se puede liberar la memoria");
			enviar_paquete_vacio(NULL_POINTER,socket);
		}
	}
	sem_wait(&mutex_dinamico);
	reserva = list_get(mem_dinamica, pos++);
	sem_post(&mutex_dinamico);
	}
	log_error(logger,"No se puede liberar la memoria");
	enviar_paquete_vacio(NULL_POINTER,socket);
}

void desocupar_cpu(int32_t socket_asociado) {

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
cpu_t *obtener_cpu_por_socket_asociado(int32_t soc_asociado){

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

void abrirArchivo(int32_t socketCpu, void* package){
	uint32_t sizeDireccion,pid;
	memcpy(&pid,package,sizeof(uint32_t));
	memcpy(&sizeDireccion,package + sizeof(uint32_t),sizeof(uint32_t));
	char* direccion = package + (sizeof(uint32_t) * 2);
	t_banderas* banderas = package + sizeof(uint32_t) * 2  + sizeDireccion;

	char* permisos = string_new();
	if(banderas->creacion) string_append(&permisos, "C");
	if(banderas->escritura) string_append(&permisos, "E");
	if(banderas->lectura) string_append(&permisos, "L");

	int fd = agregarArchivo_aProceso(pid, direccion, permisos);

	//mando mensaje a fs
	header_t header;
	header.length = string_length(direccion)+1;
	header.type = CREAR_ARCHIVO;

	sem_wait(&mutex_fs);

	sendSocket(socketConexionFS, &header, direccion);

	int tipo;
	void* paquete;
	recibir_paquete(socketConexionFS, &paquete, &tipo);

	sem_post(&mutex_fs);

	if(tipo == ABRIR_ARCHIVO_OK){
		header_t header;
		header.type = ABRIR_ARCHIVO_OK;
		header.length = sizeof(int);
		sendSocket(socketCpu, &header, &fd);
	}
	else
		enviar_paquete_vacio(tipo, socketCpu);
}

void borrarArchivo(int32_t socketCpu, void* package){
	int fd = *(int*) package;
	char* path = buscarPathDeArchivo(fd);
	header_t header;
	header.type = BORRAR_ARCHIVO;
	uint32_t size = strlen(path) + 1;
	header.type = size;

	sem_wait(&mutex_fs);

	sendSocket(socketConexionFS, &header, path);

	int32_t tipo;
	void* paquete;
	recibir_paquete(socketConexionFS, &paquete, &tipo);

	sem_post(&mutex_fs);

	int32_t respuesta;
	if(tipo == BORRAR_ARCHIVO_OK) respuesta = BORRAR_ARCHIVO_OK;
	else respuesta = ARCHIVO_INEXISTENTE;

	enviar_paquete_vacio(respuesta, socketCpu);

}

void cerrarArchivo(int32_t socketCpu, void* package){
	int pid = *(int*) package;
	int fd = *(int*) (package + sizeof(int));
	eliminarFd(fd, pid);
	enviar_paquete_vacio(CERRAR_ARCHIVO_OK, socketCpu);
}

void escribir(void* paquete, int32_t socketCpu){
	uint32_t fd = *(uint32_t*) paquete;
	int pid = *(int*) (paquete + sizeof(uint32_t));
	uint32_t sizeEscritura = *(uint32_t*) (paquete + sizeof(uint32_t) + sizeof(int));
	void* escritura = paquete + sizeof(int)  + sizeof(uint32_t) * 2;
	info_estadistica_t * info = buscarInformacion(pid);
	header_t header;

	if(fd == 1){
		int32_t sizePedido = sizeof(int) + strlen(escritura) + 1;

		header.type=IMPRIMIR_POR_PANTALLA;
		header.length= sizePedido;

		char* buffer = malloc(sizePedido);
		memcpy(buffer, &pid, sizeof(int));
		memcpy(buffer+sizeof(int), escritura, strlen(escritura) + 1);

		sendSocket(info->socketConsola, &header, buffer);
		enviar_paquete_vacio(ESCRITURA_OK, socketCpu);
	}else{
		t_archivo* archivo = buscarArchivo(pid, fd);
		if(archivo == NULL){
			log_error(logger, "No se encontro el archivo para escribir");
			enviar_paquete_vacio(ARCHIVO_INEXISTENTE, socketCpu);
			return;
		}
		int32_t offsetEscritura = archivo->cursor;
		char* path = buscarPathDeArchivo(fd);
		uint32_t sizeTotal = 3 * sizeof(int) + strlen(path) + 1 + sizeEscritura;
		void * buffer = malloc(sizeTotal);
		int32_t offset = 0;
		int sizePath = strlen(path) + 1;


		memcpy(buffer, &offsetEscritura, sizeof(int));
		offset += sizeof(int);
		memcpy(buffer+offset, &sizeEscritura, sizeof(int));
		offset += sizeof(int);
		memcpy(buffer+offset, &sizePath, sizeof(int));
		offset += sizeof(int);
		memcpy(buffer+offset, path, sizePath);
		offset += sizePath;
		memcpy(buffer+offset, escritura, sizeEscritura);

		header.length = sizeTotal;
		header.type = GUARDAR_DATOS;

		sem_wait(&mutex_fs);

		sendSocket(socketConexionFS, &header, buffer);

		free(buffer);

		void* paquete;
		int32_t tipo;

		recibir_paquete(socketConexionFS, &paquete, &tipo);

		sem_post(&mutex_fs);

		if(tipo == ESCRITURA_OK){
			enviar_paquete_vacio(ESCRITURA_OK, socketCpu);
		}else{
			enviar_paquete_vacio(tipo, socketCpu);
		}

	}
}

void leerArchivo(int socketCpu, t_lectura* lectura){
	t_archivo* archivo = buscarArchivo(lectura->pid, lectura->descriptor);
	if(archivo == NULL){
		log_error(logger, "No se encontro el archivo");
		enviar_paquete_vacio(ARCHIVO_INEXISTENTE, socketCpu);
		return;
	}
	int offsetPedidoLectura = archivo->cursor;
	char* path = buscarPathDeArchivo(lectura->descriptor);
	uint32_t sizePath = strlen(path) + 1;
	uint32_t sizeTotal = sizeof(uint32_t) * 3 + sizePath;
	void* buffer = malloc(sizeTotal);

	uint32_t offset = 0;
	memcpy(buffer, &offsetPedidoLectura, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(buffer+offset, &(lectura->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(buffer+offset, &sizePath, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(buffer+offset, path, sizePath);

	header_t header;
	header.length = sizeTotal;
	header.type = OBTENER_DATOS;

	sem_wait(&mutex_fs);

	sendSocket(socketConexionFS, &header, buffer);

	void* paquete;
	int32_t tipo;

	recibir_paquete(socketConexionFS, &paquete, &tipo);

	sem_post(&mutex_fs);

	if(tipo == LECTURA_OK){
		header.length = lectura->size;
		header.type = LECTURA_OK;
		sendSocket(socketCpu, &header, paquete);
	}else{
		log_error(logger, "File system no pudo leer el archivo");
		enviar_paquete_vacio(ARCHIVO_INEXISTENTE, socketCpu);
	}

}

void moverCursor(int32_t socketCPU, t_cursor* cursor){ // TODO con esto alcanza?
	t_archivo* archivo = buscarArchivo(cursor->pid, cursor->descriptor);
	if(archivo == NULL){
		log_error(logger, "No se encontro el archivo para escribir");
		enviar_paquete_vacio(ARCHIVO_INEXISTENTE, socketCPU);
		return;
	}

	archivo->cursor = cursor->posicion;
	enviar_paquete_vacio(MOVER_CURSOR_OK, socketCPU);
}

void verificarProcesosConsolaCaida(int32_t socketConsola){
	info_estadistica_t* info = buscarInformacionPorSocketConsola(socketConsola);
	if(info->estado != FINISH){
		info->matarSiguienteRafaga = true;
		info->exitCode = DESCONEXION_CONSOLA;
		log_info(logger, "Se termina la ejecucion del proceso %d por desconexion de la consola", info->pid);
	}
}

info_estadistica_t* buscarInformacionPorSocketConsola(int32_t socketConsola){

	bool buscar(info_estadistica_t* info){
		return info->socketConsola == socketConsola ? true : false;//&& (info->estado != FINISH) ? true : false;
	}

	return list_find(listadoEstadistico, buscar);
}

void quitarDeMemoriaDinamica(int pid){
	int i;
	for(i=0; i<list_size(mem_dinamica); i++){
		sem_wait(&mutex_dinamico);
		reserva_memoria* reserva = list_get(mem_dinamica, i);
		if(reserva->pid == pid){
			list_remove(mem_dinamica, i);
			free(reserva);
		}
		sem_post(&mutex_dinamico);
	}
}
