#include "funcionesCpu.h"

bool cerrarCPU = false;
bool huboStackOver = false;

int crearLog(void){
	logger = log_create(getenv("/home/utnso/tp-2017-1c-Dirty-Cow/cpu/logCpu"),"cpu", 1, 0);
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

	if(recibirTamanioStack() != 0) return -1;

	return EXIT_SUCCESS;
}

int conexionConMemoria(void){

	int operacion;
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
	if(recibirTamanioPagina() != 0) return -1;

	return EXIT_SUCCESS;
}

void ejecutarPrograma(void){
	char*content;
	inicializarFunciones();
	levantarArchivo(ansisop,&content);
	pcb = crearPCB(content, 1); //en realidad se recibe desde el kernel
	analizadorLinea("variables a, b, c, d", funciones, funcionesKernel);
	analizadorLinea("c = 1", funciones, funcionesKernel);
}

//por ahora no la estamos usando
int16_t recibirPCB(void){

	int tipo_mensaje;
	void* paquete;

	if(recibir_paquete(socketConexionKernel, &paquete, &tipo_mensaje) <= 0){
		log_error(logger, "Desconexion del kernel. Terminando...");
		close(socketConexionKernel);
		return EXIT_FAILURE;
	}
	if(tipo_mensaje==EXEC_PCB){
//		deserializarPCB(paquete);
		/*el kernel no esta esperando el OK de confirmacion*/
//		enviar_paquete_vacio(OK,socketConexionKernel);
	}else{
//		enviar_paquete_vacio(ERROR,socketConexionKernel);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


int16_t recibirTamanioPagina(void){

	void* paquete;
	int tipo_mensaje;

	log_debug(logger, "Esperando tamaño de pagina...");

	if(recibir_paquete(socketConexionMemoria, &paquete, &tipo_mensaje) <= 0){
		log_error(logger, "Desconexion de la memoria. Terminando...");
		close(socketConexionMemoria);
		exit(EXIT_FAILURE);
	}
	if(tipo_mensaje==ENVIAR_TAMANIO_PAGINA){
		tamanioPagina=*(uint32_t*)paquete;
		log_info(logger, "Tamaño de pagina: %d", tamanioPagina);
		/*la memoria no esta esperando el OK de confirmacion*/
//		enviar_paquete_vacio(OK,socketConexionMemoria);
	}
	else{
		log_error(logger, "Error al recibir tamanio de pagina");
//		enviar_paquete_vacio(ERROR,socketConexionMemoria);
		return -1;
	}
	return EXIT_SUCCESS;
}

int16_t recibirTamanioStack(void){

	int tipo_mensaje;
	void* paquete;

	log_debug(logger, "Esperando tamaño del stack...");

	if(recibir_paquete(socketConexionKernel, &paquete, &tipo_mensaje) <= 0){
		log_error(logger, "Desconexion del kernel. Terminando...");
		close(socketConexionKernel);
		return EXIT_FAILURE;
	}
	if(tipo_mensaje==TAMANIO_STACK_PARA_CPU){
		tamanioStack=*(uint32_t*)paquete;
		log_info(logger, "Tamanio stack: %d", tamanioStack);
//		enviar_paquete_vacio(OK,socketConexionKernel);
	}else{
		log_error(logger, "Error al recibir tamanio de stack");
//		enviar_paquete_vacio(ERROR,socketConexionKernel);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int32_t leerCompartida(void* paquete, char* variable){

	int tipo;
	int32_t var;

	header_t header;
	header.type = LEER_VAR_COMPARTIDA;
	header.length = sizeof(variable);

	//verificar envio
	sendSocket(socketConexionKernel, &header, variable);

	//verificar recepcion
	recibir_paquete(socketConexionKernel,&paquete,&tipo);
	if(tipo==VALOR_VAR_COMPARTIDA){
		var=*(int32_t*)paquete;
		return var;
	}
	else{
		return EXIT_FAILURE;
	}
}

void requestHandlerKernel(){

	void* paquete = NULL;
	int bytes;
	int tipo_mensaje;

	for(;;){
		bytes = recibir_info(socketConexionKernel, &paquete, &tipo_mensaje);
		if(bytes <= 0){
			log_error(logger, "Desconexion del kernel. Terminando...");
			close(socketConexionKernel);
			exit(1);
		}

		switch(tipo_mensaje){
		case EXEC_PCB:
			break;
		default:
			log_warning(logger, "Mensaje Recibido Incorrecto");
		}

		free(paquete);
		paquete = NULL;
	}
}

int16_t asignarCompartida(void* paquete, int valor, char* variable){

	int tipo,offset = 0;
	int sizeVariable = strlen(variable);
	int sizeTotal = sizeof(sizeVariable) + sizeVariable + sizeof(valor) + 1;
	header_t* header = malloc(sizeof(header_t));
	header->type = ASIG_VAR_COMPARTIDA;
	header->length = sizeTotal;

	void* buffer = malloc(sizeTotal); // TODO - revisar el strlen
	memcpy(buffer, &sizeVariable, sizeof(sizeVariable));
	offset += sizeof(sizeVariable);
	memcpy(buffer+offset, variable, strlen(variable)+1);
	offset += strlen(variable)+1;
	memcpy(buffer+offset, &valor, sizeof(valor));

	//verificar envio
	if( sendSocket(socketConexionKernel,header, buffer) <= 0 ){
		log_error(logger,"error al asignar valor a var compartida");
		free(header);
		return -1;
	}
	//verificar recepcion
	recibir_paquete(socketConexionKernel,&paquete,&tipo);
	free(header);
	free(buffer);

	if(tipo==OK){
		return EXIT_SUCCESS;
	}
	else{
		return EXIT_FAILURE;
	}
}

int16_t waitSemaforo(void* paquete, char* sem){

	int tipo;
	header_t* header = malloc(sizeof(header_t));
	header->type=SEM_WAIT;
	header->length=strlen(sem);
	sendSocket(socketConexionKernel,header,&sem);
	recibir_paquete(socketConexionKernel,&paquete,&tipo);
	switch(tipo){
		case RESPUESTA_WAIT_DETENER_EJECUCION:
			/*expulsarPCB()*/
			break;
		case RESPUESTA_WAIT_SEGUIR_EJECUCION:
			/*seguir ejecutando*/
			break;
		default:
			/*manejar errores*/
			free(header);
			return EXIT_FAILURE;
	}
	free(header);
	return EXIT_SUCCESS;
}

int16_t signalSemaforo(void* paquete, char* sem){
	int tipo;
	header_t* header = malloc(sizeof(header_t));
	header->type=SEM_SIGNAL;
	header->length=strlen(sem);
	sendSocket(socketConexionKernel,header,&sem);
	recibir_paquete(socketConexionKernel,&paquete,&tipo);

	if(tipo==RESPUESTA_SIGNAL_OK){
		free(header);
		return EXIT_SUCCESS;
	}
	else{
		free(header);
		return EXIT_FAILURE;
	}
}

int16_t solicitarBytes(pedido_bytes_t* pedido, void** paquete){

	int tipo;
	header_t header;
	header.type=SOLICITUD_BYTES;
	header.length=sizeof(pedido_bytes_t);

	//verificar envio
	sendSocket(socketConexionMemoria,&header,(void*)pedido);
	//verificar recepcion
	recibir_paquete(socketConexionMemoria,paquete,&tipo);
	switch(tipo){
		case OP_OK:
			return EXIT_SUCCESS;
		case SEGMENTATION_FAULT: /*se podria hacer la logica de terminacion aca*/
			enviar_paquete_vacio(FIN_ERROR_MEMORIA,socketConexionKernel);
			return EXIT_FAILURE;
		default:
			/*otros errores*/
			enviar_paquete_vacio(ERROR,socketConexionKernel);
			return EXIT_FAILURE;
	}
}

int16_t almacenarBytes(pedido_bytes_t* pedido, void* paquete){

	int tipo;
	char*buffer;
	int size;
	header_t header;
	size = sizeof(pedido_bytes_t);
	header.type=GRABAR_BYTES;
	header.length=size+pedido->size;
	buffer=malloc(header.length);
	memcpy(buffer,pedido,size);
	memcpy(buffer+size,paquete,pedido->size);

	/*
	 * Esta la version si mandamos solo un mensaje con
	 * mensaje = header + pedido + valor de la variable a guardar
	 * todo junto en un mismo mensaje
	 *
	 */
	//if(sendSocket(socketConexionMemoria,&header,(void*)buffer) <= 0 ){
	//		log_error(logger,"Error al enviar pedido para almacenar bytes en memoria");
	//		free(buffer);
	//		return EXIT_FAILURE;
	//	}


	/*
	 * Esta la version respetando la interfaz de comunicacion en memoria
	 * Memoria esta recibiendo primero el pedido y despues el valor de la variable
	 * que queremos guardar, en dos mensajes diferentes
	 *
	 */
	if(enviar_info(socketConexionMemoria,GRABAR_BYTES,size,(void*)pedido) <= 0 ){
		log_error(logger,"Error al enviar pedido para almacenar bytes en memoria");
		free(buffer);
		return EXIT_FAILURE;
	}
	if(enviar_info(socketConexionMemoria,GRABAR_BYTES,pedido->size,paquete) <= 0 ){
		log_error(logger,"Error al enviar pedido para almacenar bytes en memoria");
		free(buffer);
		return EXIT_FAILURE;
	}

	free(buffer);
	//}

	//verificar recepcion
	recibir_paquete(socketConexionMemoria,(void*)&buffer,&tipo);
	switch(tipo){
		case OP_OK:
			free(buffer);
			log_debug(logger, "Valor guardado correctamente");
			return EXIT_SUCCESS;
		case SEGMENTATION_FAULT: /*se podria hacer la logica de terminacion aca*/
			enviar_paquete_vacio(FIN_SEGMENTATION_FAULT,socketConexionKernel);
			log_debug(logger, "Segmentation Fault");
			free(buffer);
			return EXIT_FAILURE;
		case ERROR:
			enviar_paquete_vacio(FIN_ERROR_MEMORIA,socketConexionKernel);
			log_debug(logger, "Error de memoria");
			free(buffer);
			return EXIT_FAILURE;
		default:
			/*otros errores*/
//			enviar_paquete_vacio(ERROR,socketConexionKernel);
//			free(buffer);
			return EXIT_FAILURE;
	}
}


void levantarArchivo(char*path, char** buffer){

		FILE* file;
	 	int file_fd, file_size;
	 	struct stat stats;

	 	file = fopen(path, "r");
	 	file_fd = fileno(file);

	 	fstat(file_fd, &stats);
	 	file_size = stats.st_size;

	 	*buffer = malloc(file_size+1);
	 	if(*buffer == NULL) {
	 		log_error(logger, "archivo no levantado");
	 		exit(1);
	 	}
	 	memset(*buffer, '\0',file_size+1);
	 	fread(*buffer,file_size,1,file);
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
	if (signo == SIGUSR1) {
		log_info(logger, "Se recibe SIGUSR1");
		cerrarCPU = true;
		enviar_paquete_vacio(SIGURSR, socketConexionKernel);
		log_debug(logger, "Termina la rafaga actual y luego se cierra esta CPU.");
	}
}

void revisarFinalizarCPU() {
	if (cerrarCPU) {
		log_debug(logger, "Cerrando CPU");
		finalizarConexion(socketConexionKernel);
		finalizarConexion(socketConexionMemoria);
		log_info(logger, "CPU cerrada");
		log_destroy(logger);
		freeConf(config);
		return;
	}
}

//ejemplo de comenzarPRograma
//void comenzarEjecucionDePrograma() {
//	log_info(ptrLog, "Recibo PCB id: %i", pcb->pcb_id);
//	int contador = 1;
//
//	while (contador <= pcb->quantum) {
//		char* proximaInstruccion = solicitarProximaInstruccionAUMC();
//		limpiarInstruccion(proximaInstruccion);
//		if (pcb->PC >= (pcb->codigo - 1) && (strcmp(proximaInstruccion, "end") == 0)) {
//			finalizarEjecucionPorExit();
//			revisarFinalizarCPU();
//			return;
//		} else {
//
//			if (proximaInstruccion != NULL) {
//				if (strcmp(proximaInstruccion, "FINALIZAR") == 0) {
//					log_error(ptrLog, "Instruccion no pudo leerse. Hay que finalizar el Proceso.");
//					finalizarProcesoPorErrorEnUMC();
//					return;
//				} else {
//					log_debug(ptrLog, "Instruccion recibida: %s", proximaInstruccion);
//					if (strcmp(proximaInstruccion, "end") == 0) {
//						log_debug(ptrLog, "Finalizo la ejecucion del programa");
//						finalizarEjecucionPorExit();
//						revisarFinalizarCPU();
//						return;
//					}
//					analizadorLinea(proximaInstruccion, &functions,
//							&kernel_functions);
//					if (huboStackOver) {
//						finalizarProcesoPorStackOverflow();
//						revisarFinalizarCPU();
//						return;
//					}
//					contador++;
//					pcb->PC = (pcb->PC) + 1;
//					switch (operacion) {
//					case IO:
//						log_debug(ptrLog, "Finalizo ejecucion por operacion I/O");
//						finalizarEjecucionPorIO();
//						revisarFinalizarCPU();
//						return;
//					case WAIT:
//						log_debug(ptrLog, "Finalizo ejecucion por un Wait.");
//						finalizarEjecucionPorWait();
//						revisarFinalizarCPU();
//						return;
//					default:
//						break;
//					}
//					usleep(pcb->quantumSleep * 1000);
//				}
//			} else {
//				log_info(ptrLog, "No se pudo recibir la instruccion de UMC. Cierro la conexion");
//				finalizarConexion(socketUMC);
//				return;
//			}
//		}
//
//	}
//	log_debug(ptrLog, "Finalizo ejecucion por fin de Quantum");
//	finalizarEjecucionPorQuantum();
//
//	free(pcb);
//	revisarFinalizarCPU();
//}
