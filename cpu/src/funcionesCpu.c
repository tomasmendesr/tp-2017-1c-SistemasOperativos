#include "funcionesCpu.h"

bool cerrarCPU = false;
bool huboStackOver = false;

AnSISOP_funciones functions = { .AnSISOP_asignar = asignar,
		.AnSISOP_asignarValorCompartida = asignarValorCompartida,
		.AnSISOP_definirVariable = definirVariable, .AnSISOP_dereferenciar =
				dereferenciar, .AnSISOP_finalizar = finalizar,
		.AnSISOP_irAlLabel = irAlLabel, .AnSISOP_llamarConRetorno = llamarConRetorno,
		.AnSISOP_llamarSinRetorno=llamarSinRetorno,.AnSISOP_obtenerPosicionVariable =
				obtenerPosicionVariable, .AnSISOP_obtenerValorCompartida =
				obtenerValorCompartida, .AnSISOP_retornar = retornar,
		};

AnSISOP_kernel kernel_functions = { .AnSISOP_abrir =abrir, .AnSISOP_borrar = borrar,
		.AnSISOP_cerrar = cerrar, .AnSISOP_escribir = escribir, .AnSISOP_leer = leer,
		.AnSISOP_liberar=liberar, .AnSISOP_moverCursor = moverCursor,
		.AnSISOP_reservar=reservar, .AnSISOP_signal = signalAnsisop,
		.AnSISOP_wait = wait
};

int crearLog(void){
	logger = log_create("logCpu","cpu", 1, LOG_LEVEL_TRACE);
	if(logger)
		return 1;
	else
		return 0;
}

void crearConfig(int argc, char* argv[]){
	if(argc>1){
		if(verificarExistenciaDeArchivo(argv[1]))
			config=levantarConfiguracionCPU(argv[1]);
		else{
			log_error(logger,"La ruta especificada es incorrecta");
			exit(EXIT_FAILURE);
		}
	}
	else if(verificarExistenciaDeArchivo(configuracionCPU)){
		config=levantarConfiguracionCPU(configuracionCPU);
		log_info(logger,"Configuracion levantada correctamente");
	}
	else if(verificarExistenciaDeArchivo(string_substring_from(configuracionCPU,3))){
		config=levantarConfiguracionCPU(string_substring_from(configuracionCPU,3));
		log_info(logger,"Configuracion levantada correctamente");
	}
	else{
		log_error(logger,"No pudo levantarse el archivo de configuracion");
		exit(EXIT_FAILURE);
	}
}

t_config_cpu* levantarConfiguracionCPU(char* archivo) {

        t_config_cpu* conf = malloc(sizeof(t_config_cpu));
        t_config* configCPU;

        configCPU = config_create(archivo);
        conf->puerto_Kernel = malloc(strlen(config_get_string_value(configCPU, "PUERTO_KERNEL"))+1);
        strcpy(conf->puerto_Kernel, config_get_string_value(configCPU, "PUERTO_KERNEL"));
        conf->puerto_Memoria = malloc(strlen(config_get_string_value(configCPU, "PUERTO_MEMORIA"))+1);
        strcpy(conf->puerto_Memoria, config_get_string_value(configCPU, "PUERTO_MEMORIA"));
        conf->ip_Memoria = malloc(strlen(config_get_string_value(configCPU, "IP_MEMORIA"))+1);
        strcpy(conf->ip_Memoria, config_get_string_value(configCPU, "IP_MEMORIA"));
        conf->ip_Kernel = malloc(strlen(config_get_string_value(configCPU, "IP_KERNEL"))+1);
        strcpy(conf->ip_Kernel, config_get_string_value(configCPU, "IP_KERNEL"));
        config_destroy(configCPU);

        return conf;
}

void freeConf(t_config_cpu* config){
	free(config->ip_Kernel);
	free(config->ip_Memoria);
	free(config->puerto_Kernel);
	free(config->puerto_Memoria);
	free(config);
}

int conexionConKernel(void){

	int operacion;
	void* paquete;
	void* paquete_vacio;

	socketConexionKernel=createClient(config->ip_Kernel,config->puerto_Kernel);

	if(socketConexionKernel == -1){
		log_error(logger,"No pudo conectarse a Kernel");
		return -1;
	}else{
		log_info(logger,"Cliente a Kernel creado");
	}

	enviar_paquete_vacio(HANDSHAKE_CPU, socketConexionKernel);
	recibir_paquete(socketConexionKernel, &paquete_vacio, &operacion);

	if(operacion==HANDSHAKE_KERNEL){
		log_info(logger,"Conexion establecida con Kernel! :D");
	}else{
		log_info(logger,"El Kernel no devolvio handshake :(");
		return -1;
	}
	log_debug(logger, "Esperando tamaño del stack...");
	paquete=NULL;
	int rta = requestHandlerKernel(paquete);
	if(rta == -1){
		log_error(logger, "Error al recibir tamanio de stack");
		return -1;
	}

	return EXIT_SUCCESS;
}

int conexionConMemoria(void){
	int operacion;
	void* paquete;
	void* paquete_vacio;

	socketConexionMemoria = createClient(config->ip_Memoria, config->puerto_Memoria);
	if(socketConexionMemoria == -1){
		log_error(logger,"No pudo conectarse a Memoria");
		return -1;
	}else{
		log_info(logger,"Cliente a Memoria creado");
	}
	enviar_paquete_vacio(HANDSHAKE_CPU, socketConexionMemoria);
	recibir_paquete(socketConexionMemoria, &paquete_vacio, &operacion);

	if(operacion == HANDSHAKE_MEMORIA){
		log_info(logger,"Conexion establecida con Memoria! :D");
	}else{
		log_info(logger,"La Memoria no devolvio handshake :(");
		return -1;
	}
	log_debug(logger, "Esperando tamaño de pagina...");
	paquete=NULL;
	int rta = requestHandlerMemoria(paquete);
	if(rta != 0){
		log_error(logger, "Error al recibir tamanio de pagina");
		return -1;
	}

	return EXIT_SUCCESS;
}

int32_t requestHandlerKernel(void* paquete){

	int tipo_mensaje;
	conecFailKernel(recibir_info(socketConexionKernel,paquete,&tipo_mensaje));

	switch(tipo_mensaje){
		case EXEC_PCB:
			recibirPCB(paquete);
			break;
		case EXEC_QUANTUM:
			quantum=*(uint32_t*)paquete;
			free(paquete);
			break;
		case TAMANIO_STACK_PARA_CPU:
			tamanioStack=*(uint32_t*)paquete;
			log_info(logger, "Tamanio stack: %d", tamanioStack);
			free(paquete);
			break;
		case RESPUESTA_SIGNAL_OK:
		case RESPUESTA_WAIT_SEGUIR_EJECUCION:
			break;
		case RESPUESTA_WAIT_DETENER_EJECUCION:
			endBlockedProc();
			break;
		case VALOR_VAR_COMPARTIDA:
			break;
		default:
			log_warning(logger, "Mensaje Recibido Incorrecto");
			return -1;
		}
	return EXIT_SUCCESS;
}

void conecFailKernel(int cant){
	if(cant <= 0){
		log_error(logger, "Kernel caido... Terminando...");
		close(socketConexionKernel);
		finalizarCPU();
		exit(1);
	}
}

void conecFailMemoria(int cant){
	if(cant <= 0){
		log_error(logger, "Memoria... Terminando...");
		close(socketConexionKernel);
		finalizarCPU();
		exit(1);
	}
}

int32_t requestHandlerMemoria(void* paquete){

	int tipo_mensaje;
	conecFailMemoria(recibir_info(socketConexionMemoria,paquete,&tipo_mensaje));

	switch(tipo_mensaje){
	// respuesta grabarBytes - asignarCompartida
	case OP_OK:
	// aca no quiero hacer un free del paquete porque lo voy a usar. hacer free en la funcion que pide el paquete
	//respuesta de solicitarBytes
	case RESPUESTA_BYTES:
		return EXIT_SUCCESS;
	case ENVIAR_TAMANIO_PAGINA:
		tamanioPagina=*(uint32_t*)paquete;
		log_info(logger, "Tamaño de pagina: %d", tamanioPagina);
		free(paquete);
		return EXIT_SUCCESS;
	//respuestas de operaciones fallidas
	case SEGMENTATION_FAULT:
		enviar_paquete_vacio(FIN_SEGMENTATION_FAULT,socketConexionKernel);
		log_error(logger, "Segmentation Fault");
		finalizarProcesoPorSegmentationFault();
		return -1;
	case STACKOVERFLOW:
		enviar_paquete_vacio(STACKOVERFLOW,socketConexionKernel);
		log_error(logger, "Stack Overflow");
		finalizarProcesoPorStackOverflow();
		return -1;
	default:
		log_warning(logger, "Mensaje Recibido Incorrecto");
		return -1;
	}
}

void recibirPCB(void* paquete){
	pcb = deserializar_pcb(paquete);
	free(paquete);
	setPCB(pcb);
}

int16_t solicitarBytes(pedido_bytes_t* pedido, void* paquete){
	header_t header;
	header.type=SOLICITUD_BYTES;
	header.length=sizeof(pedido_bytes_t);

	sendSocket(socketConexionMemoria,&header,(void*)pedido);
	return requestHandlerMemoria(paquete);
}

int16_t almacenarBytes(pedido_bytes_t* pedido, void* paquete){
	char*buffer;
	int size;
	header_t header;
	size = sizeof(pedido_bytes_t);
	header.type=GRABAR_BYTES;
	header.length=size+pedido->size;
	buffer=malloc(header.length);
	memcpy(buffer,pedido,size);
	memcpy(buffer+size,paquete,pedido->size);

	if(sendSocket(socketConexionMemoria,&header,(void*)buffer) <= 0 ){
		log_error(logger,"Error al enviar pedido para almacenar bytes en memoria");
		free(buffer);
		return EXIT_FAILURE;
	}

	free(buffer);

	int rta = requestHandlerMemoria((void*)&buffer);
	if(rta != 0){
		log_error(logger, "La variable no pudo asignarse. Se finaliza el Proceso.");
		return -1;
	}
	return rta;
}

pcb_t* crearPCB(char* programa, int pid) {

	log_debug(logger, "Se crea un PCB para el Programa Solicitado.");
	t_metadata_program* datos;
	char* indiceEtiquetas;

	//Obtengo la metadata utilizando el preprocesador del parser
	datos = metadata_desde_literal(programa);

	pcb_t* pcb = malloc(sizeof(pcb_t));

	pcb->pid = pid;
	pcb->stackPointer = 0;
	pcb->programCounter = datos->instruccion_inicio;
	pcb->codigo = datos->instrucciones_size;
	pcb->cantPaginasCodigo = strlen(programa) / tamanioPagina;
	if(strlen(programa)%tamanioPagina != 0) pcb->cantPaginasCodigo++;
	t_list *pcbStack = list_create();
	pcb->indiceStack = pcbStack;
	pcb->tamanioEtiquetas = datos->etiquetas_size;
	t_list *listaIndCodigo = llenarLista(datos->instrucciones_serializado,
			datos->instrucciones_size);
	pcb->indiceCodigo = listaIndCodigo;
	if (datos->cantidad_de_etiquetas > 0
			|| datos->cantidad_de_funciones > 0) {
		indiceEtiquetas = malloc(datos->etiquetas_size);
		memcpy(indiceEtiquetas,datos->etiquetas,datos->etiquetas_size);
		pcb->etiquetas = indiceEtiquetas;
	}else{
		pcb->etiquetas = NULL;
	}
	metadata_destruir(datos);
	free(programa);

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

void revisarSigusR1(int signo){
	if(signo == SIGUSR1){
		log_info(logger, "Se recibe SIGUSR1");
		cerrarCPU = true;
		enviar_paquete_vacio(DESCONEXION_CPU, socketConexionKernel);
		log_debug(logger, "Desconectando CPU...");
	}
}

void revisarFinalizarCPU(void){
	if(cerrarCPU){
	finalizarConexion(socketConexionKernel);
	finalizarConexion(socketConexionMemoria);
	log_info(logger, "CPU cerrada. Adios!");
	log_destroy(logger);
	freeConf(config);
	}

}

void comenzarEjecucionDePrograma(void){
	log_info(logger, "Recibo PCB id: %i", pcb->pid);
	void* paquete;
	paquete = NULL;
//	int res;

	for(;;){
	requestHandlerKernel(paquete);
	if(paquete) free(paquete);

	int i = 1;
	while (i <= quantum) {
		if(solicitarProximaInstruccion(paquete) != 0)
			return;
		//creo que no hace falta
		limpiarInstruccion(paquete);

		if (pcb->programCounter >= (pcb->codigo - 1) && (strcmp(paquete, "end") == 0)) {
			finalizarEjecucionPorFinPrograma();
			revisarFinalizarCPU();
			return;
		} else {
			if (paquete) {
				log_debug(logger, "Instruccion recibida: %s", paquete);
//				if (strcmp(paquete, "end") == 0) {
//					log_debug(logger, "Finalizo la ejecucion del programa");
//					finalizarEjecucionPorFinPrograma();
//					revisarFinalizarCPU();
//					return;
//				}
				analizadorLinea(paquete, &functions, &kernel_functions);
				if (huboStackOver)
				finalizarProcesoPorStackOverflow();
				revisarFinalizarCPU();
				i++;
				pcb->programCounter++;
				// usleep -----------------> no se que es esto
			}
			else {
				log_info(logger, "No se pudo recibir la instruccion de memoria. Cierro la conexion");
				finalizarConexion(socketConexionMemoria);
				return;
			}
		}
	}
	log_debug(logger, "Finalizo ejecucion por fin de Quantum");
	finalizarEjecucionPorFinQuantum();

	freePCB(pcb);
	revisarFinalizarCPU();
	}
}

int16_t solicitarProximaInstruccion(void* paquete) {
	t_indice_codigo *indice = list_get(pcb->indiceCodigo, pcb->programCounter);
	uint32_t requestStart = indice->offset;
	uint32_t requestSize = indice->size;

	uint32_t i = 0;
	while (requestStart >= (tamanioPagina + (tamanioPagina * i++)));
	uint32_t paginaAPedir = --i;
	pedido_bytes_t* solicitar = malloc(sizeof(pedido_bytes_t));
	solicitar->pag = paginaAPedir;
	solicitar->offset = requestStart - (tamanioPagina * paginaAPedir);
	solicitar->size = requestSize;
	solicitar->pid = pcb->pid;

	log_info(logger, "Pido a Memoria -> Pagina: %d - Start: %d - Offset: %d",
			paginaAPedir, requestStart - (tamanioPagina * paginaAPedir),
			solicitar->offset);

	if(solicitarBytes(solicitar, paquete) != 0 ){
			free(solicitar);
			log_error(logger, "Error al solicitar bytes a memoria.");
			return -1;
	}
	free(solicitar);
	return EXIT_SUCCESS;
}

void limpiarInstruccion(char * instruccion) {
	char* instr = instruccion;
	int a = 0;
	while (*instruccion != '\0') {
		if (*instruccion
				!= '\t'&& *instruccion != '\n' && !iscntrl(*instruccion)) {
			if (a == 0 && isdigit((int )*instruccion)) {
				++instruccion;
			} else {
				*instr++ = *instruccion++;
				a++;
			}
		} else {
			++instruccion;
		}
	}
	*instr = '\0';
}

void finalizarEjecucionPorFinQuantum(void) {
	t_buffer_tamanio* paquete = serializar_pcb(pcb);
	header_t header;
	header.type= FIN_EJECUCION;
	header.length=paquete->tamanioBuffer;
	if(sendSocket(socketConexionKernel, &header, (void*)paquete->buffer) <= 0){
		log_error(logger,"Error al notificar kernel el fin de ejecucion");
		free(paquete->buffer);
		free(paquete);
		return;
	}
	free(paquete->buffer);
	free(paquete);
}

void finalizarEjecucionPorFinPrograma(void) {
	t_buffer_tamanio* paquete = serializar_pcb(pcb);
	header_t header;
	header.type= FIN_PROCESO;
	header.length=paquete->tamanioBuffer;
	if(sendSocket(socketConexionKernel, &header, (void*)paquete->buffer) <= 0){
		log_error(logger,"Error al notificar kernel el fin de programa");
		free(paquete->buffer);
		free(paquete);
		return;
	}
	free(paquete->buffer);
	free(paquete);
}

void finalizarProcesoPorStackOverflow(void) {
	t_buffer_tamanio* paquete = serializar_pcb(pcb);
	header_t header;
	header.type=STACKOVERFLOW;
	header.length=paquete->tamanioBuffer;
	huboStackOver = false;
	if(sendSocket(socketConexionKernel, &header, (void*)paquete->buffer) <= 0){
		log_error(logger,"Error al devolver PCB por StackOverflow al kernel");
		free(paquete->buffer);
		free(paquete);
		return;
	}
	free(paquete->buffer);
	free(paquete);
}

void finalizarProcesoPorErrorEnMemoria(void) {
	t_buffer_tamanio* paquete = serializar_pcb(pcb);
	header_t header;
	header.type= FIN_ERROR_MEMORIA;
	header.length=paquete->tamanioBuffer;
	if(sendSocket(socketConexionKernel, &header, (void*)paquete->buffer) <= 0){
		log_error(logger,"Error al devolver PCB por StackOverflow al kernel");
		free(paquete->buffer);
		free(paquete);
		return;
	}
	free(paquete->buffer);
	free(paquete);
}

void finalizarProcesoPorSegmentationFault(void){
	t_buffer_tamanio* paquete = serializar_pcb(pcb);
	header_t header;
	header.type= FIN_SEGMENTATION_FAULT;
	header.length=paquete->tamanioBuffer;
	huboStackOver = false;
	if(sendSocket(socketConexionKernel, &header, (void*)paquete->buffer) <= 0){
		log_error(logger,"Error al devolver PCB por segmentation fault al kernel");
		free(paquete->buffer);
		free(paquete);
		return;
	}
	free(paquete->buffer);
	free(paquete);
}


int32_t endBlockedProc(void){
	t_buffer_tamanio* buffer;
	buffer = serializar_pcb(pcb);
	if(enviar_info(socketConexionKernel,PROC_BLOCKED,buffer->tamanioBuffer,(void*)buffer->buffer) <= 0){
		log_error(logger,"Error al notificar kernel el fin de ejecucion");
		free(buffer->buffer);
		free(buffer);
		return -1;
	}
	free(buffer->buffer);
	free(buffer);
	return EXIT_SUCCESS;
}

void freePCB(pcb_t* pcb){
	free(pcb->etiquetas);
	list_destroy(pcb->indiceCodigo);
	list_destroy(pcb->indiceStack);
	free(pcb);
}

void recibirRafaga(void){
	void*paquete;
	int buffer;
	int type_msj;
	buffer = sizeof(uint32_t);
	recvAll(socketConexionKernel, (char*)&type_msj, buffer, MSG_WAITALL);
	if(type_msj == EXEC_QUANTUM){
		recvAll(socketConexionKernel, (char*)&paquete, buffer, MSG_WAITALL);
		recvAll(socketConexionKernel, (char*)&quantum, buffer, MSG_WAITALL);
	}
	free(paquete);
}

void finalizarCPU(void){
	finalizarConexion(socketConexionKernel);
	finalizarConexion(socketConexionMemoria);
	log_info(logger, "CPU desconectada!");
	log_destroy(logger);
	freeConf(config);
}
