#include "funcionesConsola.h"

void crearConfig(int argc, char* argv[]) {
	char* pathConfig = string_new();

	if (argc>1)string_append(&pathConfig, argv[1]);
		else string_append(&pathConfig, configuracionConsola);
	if (verificarExistenciaDeArchivo(pathConfig)) {
		config = levantarConfiguracionConsola(pathConfig);
	} else {
		log_info(logger, "No Pudo levantarse el archivo de configuracion");
		exit(EXIT_FAILURE);
	}
	printf("Configuracion levantada correctamente\n");
	return;
}

t_config_consola* levantarConfiguracionConsola(char * archivo) {

	t_config_consola* config = malloc(sizeof(t_config_consola));
	t_config* configConsola;

	configConsola = config_create(archivo);

	config->ip_Kernel = malloc(
			strlen(config_get_string_value(configConsola, "IP_KERNEL")) + 1);
	strcpy(config->ip_Kernel,
			config_get_string_value(configConsola, "IP_KERNEL"));

	config->puerto_Kernel = malloc(
			strlen(config_get_string_value(configConsola, "PUERTO_KERNEL")) + 1);
	strcpy(config->puerto_Kernel,
			config_get_string_value(configConsola, "PUERTO_KERNEL"));

	config_destroy(configConsola);

	return config;
}


int enviarArchivo(int kernel_fd, char* path){

	//Verifico existencia archivo (Aguante esta funcion loco!)
 	if( !verificarExistenciaDeArchivo(path) ){
 		log_error(logger, "no existe el archivo");
 		return -1;
 	}

 	FILE* file;
 	int file_fd, file_size;
 	struct stat stats;

 	//Abro el archivo y le saco los stats
 	file = fopen(path, "r");
 	//esto nunca deberia fallar porque ya esta verificado, pero por las dudas
 	if(file == NULL){
 		log_error(logger, "no pudo abrir archivo");
 		return -1;
 	}
 	file_fd = fileno(file);

 	fstat(file_fd, &stats);
 	file_size = stats.st_size;
 	header_t header;
 	char* buffer = malloc(file_size + sizeof(header_t));
 	int offset = 0;

 	if(buffer == NULL){
 		log_error(logger, "no pude reservar memoria para enviar archivo");
 		fclose(file);
 		return -1;
 	}

 	header.type = ENVIO_CODIGO;
 	header.length = file_size;
 	memcpy(buffer, &(header.type),sizeof(header.type)); offset+=sizeof(header.type);
 	memcpy(buffer + offset, &(header.length),sizeof(header.length)); offset+=sizeof(header.length);

 	if( fread(buffer + offset,file_size,1,file) < 1){
 		log_error(logger, "No pude leer el archivo");
 		free(buffer);
 		fclose(file);
 		return -1;
 	}

 	/*Esto lo hago asi porque de la otra forma habría que reservar MAS espacio para
 	 * enviar el paquete */
 	if ( sendAll(kernel_fd, buffer, file_size + sizeof(header_t), 0) <=0 ){
 		log_error(logger, "Error al enviar archivo");
 		free(buffer);
 		fclose(file);
 		return -1;
 	}

 	free(buffer);
 	fclose(file);
 	return 0;
}

//funciones interfaz
void levantarInterfaz() {
	//creo los comandos y el parametro
	comando* comandos = malloc(sizeof(comando) * 4);

	strcpy(comandos[0].comando, "start");
	comandos[0].funcion = iniciarPrograma;
	strcpy(comandos[1].comando, "stop");
	comandos[1].funcion = finalizarPrograma;
	strcpy(comandos[2].comando, "disconnect");
	comandos[2].funcion = desconectarConsola;
	strcpy(comandos[3].comando, "clean");
	comandos[3].funcion = limpiarMensajes;

	interface_thread_param* params = malloc(sizeof(interface_thread_param));
	params->comandos = comandos;
	params->cantComandos = 4;

	//Lanzo el thread
	pthread_attr_t atributos;
	pthread_attr_init(&atributos);
	pthread_create(&threadInterfaz, &atributos, (void*)interface, params);

	return;
}

void iniciarPrograma(char* comando, char* param) {

	int socket_cliente;

	if(!verificarExistenciaDeArchivo(param)){
		log_warning(logger, "no existe el archivo");
		printf("El archivo no se encuentra\n");
		return;
	}

	socket_cliente = createClient(config->ip_Kernel, config->puerto_Kernel);
	if (socket_cliente != -1) {
		printf("Cliente creado satisfactoriamente.\n");
	}else{
		perror("No se pudo crear el cliente");
	}
	enviar_paquete_vacio(HANDSHAKE_PROGRAMA, socket_cliente);
	int operacion = 0;
	void* paquete_vacio;

	if(recibir_paquete(socket_cliente, &paquete_vacio, &operacion) <= 0){
		log_error(logger, "El kernel se desconecto");
		close(socket_cliente);
		exit(EXIT_FAILURE);
	}

	if (operacion == HANDSHAKE_KERNEL) {
		printf("Conexion con Kernel establecida! :D \n");
		printf("Se procede a mandar el archivo: %s\n", param);

	} else {
		printf("El Kernel no devolvio handshake :( \n");
	}

	dataHilo* data = malloc(sizeof(dataHilo));
	data->pathAnsisop = malloc(strlen(param)+1);
	memcpy(data->pathAnsisop, param, strlen(param)+1);
	data->socket = socket_cliente;

	pthread_t thread;
	pthread_create(&thread, NULL, (void*)threadPrograma, data);
	pthread_detach(thread);
}

void crearProceso(int socketProceso, pthread_t threadPrograma, int pid){
	t_proceso* proc = malloc(sizeof(t_proceso));
	proc->socket = socketProceso;
	proc->thread = threadPrograma;
	proc->pid = pid;
	time_t tiempo = time(0);
    struct tm * inicio = localtime(&tiempo);
    proc->fechaInicio = inicio;

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    proc->start = start;

	list_add(procesos, proc);
}

void threadPrograma(dataHilo* data){

	int operacion;
	void* paquete;
	bool procesoActivo = true;
	int* pidAsignado;
	int socketProceso = data->socket;
	pthread_t thread = pthread_self();

	if((enviarArchivo(socketProceso, data->pathAnsisop))==-1){
		log_error(logger,"No se pudo mandar el archivo");
		printf("No pudo enviarse el archivo\n");
		return;
	}
	log_info(logger,"Archivo enviado correctamente");

	if(recibir_paquete(socketProceso, &paquete, &operacion) <= 0){
		log_error(logger, "El kernel se desconecto");
		close(socketProceso);
		exit(EXIT_FAILURE);
	}

	switch(operacion){
	case PROCESO_RECHAZADO:
		log_error(logger, "El kernel rechazo el proceso");
		return;
		break;
	case PID_PROGRAMA: // TODO
		pidAsignado = (int*)paquete;
		log_info(logger, "Programa %d aceptado por el kernel", *pidAsignado);
		break;
	default:
		log_warning(logger, "Se recibio una operacion invalida\n");
		break;
	}

	crearProceso(socketProceso,thread,*pidAsignado);

	while(procesoActivo){

		/*ambos se quedan esperando una respuesta del otro*/
		if(recibir_paquete(socketProceso, (void*)&paquete, &operacion) <= 0){
			log_error(logger, "El kernel se desconecto");
			if(paquete)free(paquete);
			exit(EXIT_FAILURE);
		}else{

			switch (operacion) {
			case FINALIZAR_EJECUCION:
				finalizarEjecucionProceso(&procesoActivo, data);
				break;
			case IMPRIMIR_TEXTO_PROGRAMA:
				printf("%s\n", (char*)paquete);
				break;
			case IMPRIMIR_VARIABLE_PROGRAMA:
				printf("%d\n", (int)paquete);
				break;
			default:
				break;
			}

		}
		if(paquete)free(paquete);

	}
}


void finalizarEjecucionProceso(bool* procesoActivo, dataHilo* data){
	bool buscarPorSocket(t_proceso* proc){
		return proc->socket == data->socket ? true : false;
	}

	t_proceso* proc = list_find(procesos, buscarPorSocket);
	log_info(logger, "Termino la ejecucion del programa %d", proc->pid);

	cargarFechaFin(proc);
	imprimirInformacion(proc); // TODO

	procesoActivo = false;
	list_remove_and_destroy_by_condition(procesos, buscarPorSocket, free);
	free(data);
}

void cargarFechaFin(t_proceso* proc){
	time_t tiempo = time(0);
	struct tm * fin = localtime(&tiempo);
	proc->fechaFin = fin;
	struct timespec end;
	clock_gettime(CLOCK_MONOTONIC_RAW, &end);
	proc->end = end;
}

void imprimirInformacion(t_proceso* proceso){
	struct timespec;
	printf("----------------------\n");
	printf("Proceso %d\n", proceso->pid);
	printf("Inicio: %d-%d-%d %d%d:%d\n", proceso->fechaInicio->tm_year + 1900, proceso->fechaInicio->tm_mon + 1, proceso->fechaInicio->tm_mday, proceso->fechaInicio->tm_hour, proceso->fechaInicio->tm_min, proceso->fechaInicio->tm_sec);
	printf("Fin:  %d-%d-%d %d:%d:%d\n", proceso->fechaFin->tm_year + 1900, proceso->fechaFin->tm_mon + 1, proceso->fechaFin->tm_mday, proceso->fechaFin->tm_hour, proceso->fechaFin->tm_min, proceso->fechaFin->tm_sec);
	uint64_t msInicio = proceso->start.tv_nsec / 1000000 + proceso->start.tv_sec * 1000;
	uint64_t msFin = proceso->end.tv_nsec / 1000000 + proceso->end.tv_sec * 1000;
	printf("Duracion: %d seg - %d ms\n", proceso->end.tv_sec - proceso->start.tv_sec, msFin - msInicio);
	printf("----------------------\n");
}

void finalizarPrograma(char* comando, char* param){

	if(!esNumero(param)){
		printf("Valor de pid invalido\n");
		return;
	}

	int pid = strtol(param, NULL, 10);

	bool buscarProceso(t_proceso* p){
		return p->pid == pid? true : false;
	}

	t_proceso* proceso = list_find(procesos, buscarProceso);
	if(proceso == NULL){
		printf("Ese proceso no se encuentra en el sistema\n");
		return;
	}
	cargarFechaFin(proceso);
	imprimirInformacion(proceso);

	//evaluar si debo avisar al kernel o si al desconectarse el socket el kernel lo maneje solo
	terminarProceso(proceso);

	printf("Proceso finalizado\n");
}

void desconectarConsola(char* comando, char* param) {
	list_destroy_and_destroy_elements(procesos,terminarProceso);

	exit(0);
}

void terminarProceso(t_proceso* proc){
	pthread_cancel(proc->thread);
	free(proc);
}

void limpiarMensajes(char* comando, char* param) {
	//me doy asco por usar system
	system("clear");
}

int crearLog() {
	logger = log_create("logConsola","consola", 1, LOG_LEVEL_TRACE);
	if (logger) {
		return 1;
	} else {
		return 0;
	}
}
