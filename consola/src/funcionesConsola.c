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
}

t_config_consola* levantarConfiguracionConsola(char * archivo) {

	t_config_consola* config = malloc(sizeof(t_config_consola));
	t_config* configConsola;

	verificarExistenciaDeArchivo(archivo);
	configConsola = config_create(archivo);

	config->ip_Kernel = malloc(
			strlen(config_get_string_value(configConsola, "IP_KERNEL")) + 1);
	strcpy(config->ip_Kernel,
			config_get_string_value(configConsola, "IP_KERNEL"));

	config->puerto_Kernel = malloc(
			strlen(config_get_string_value(configConsola, "PUERTO_KERNEL"))
					+ 1);
	strcpy(config->puerto_Kernel,
			config_get_string_value(configConsola, "PUERTO_KERNEL"));

	config_destroy(configConsola);

	return config;
}

void enviarArchivo(int kernel_fd, char* path){

	//Verifico existencia archivo (Aguante esta funcion loco!)
	if( !verificarExistenciaDeArchivo(path) ){
		log_error(logger, "no existe el archivo");
		return;
	}

	FILE* file;
	int file_fd, file_size;
	struct stat stats;

	//Abro el archivo y le saco los stats
	file = fopen(path, "r");
	if(file == NULL){//esto nunca deberia fallar porque ya esta verificado, pero por las dudas
		log_error(logger, "no pudo abrir archivo");
		return;
	}
	file_fd = fileno(file);

	fstat(file_fd, &stats);
	file_size = stats.st_size;

	uint32_t header[2];
	char* buffer = malloc(file_size + 2 * sizeof(uint32_t));

	if(buffer == NULL){
		log_error(logger, "no pude reservar memoria para enviar archivo");
		return;
	}

	header[0] = ENVIO_CODIGO;
	header[1] = file_size;

	if( fread(buffer + 2*sizeof(uint32_t),file_size,1,file) < file_size ){
		log_error(logger, "no leyo bien el archivo");
		return;
	}

	memcpy(buffer,header,2*sizeof(uint32_t));

	if( sendAll(kernel_fd,buffer,file_size + 2*sizeof(uint32_t),0) == -1 ){
		log_error(logger, "no se pudo enviar el archivo");
		return;
	}

	return;
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

	pthread_create(&threadInterfaz, &atributos, (void*) interface, params);

	return;
}
void iniciarPrograma(char* comando, char* param) {

	int socket_cliente;

	verificarExistenciaDeArchivo(param);
	printf("Su proceso se inicializo");
	socket_cliente = createClient(config->ip_Kernel, config->puerto_Kernel);
	if (socket_cliente) {
		printf("Cliente creado satisfactoriamente.\n");
	}
	enviar_paquete_vacio(HANDSHAKE_PROGRAMA, socket_cliente);
	int operacion = 0;
	void* paquete_vacio;

	recibir_paquete(socket_cliente, &paquete_vacio, &operacion);

	if (operacion == HANDSHAKE_KERNEL) {
		printf("Conexion con Kernel establecida! :D \n");
		printf("Se procede a mandar el archivo: ", param);
	} else {
		printf("El Kernel no devolvio handshake :( \n");
	}
	printf("iniciarPrograma\n");
}
void finalizarPrograma(char* comando, char* param) {
	printf("finalizarPrograma\n");
}
void desconectarConsola(char* comando, char* param) {
	//Aca va a tener que ir toda la logica de limpiar variables finalizar proceso o algo
	//AL menos que se la prueba del cierre Total de los programas.
	exit(1);
}
void limpiarMensajes(char* comando, char* param) {
	printf("limpiarMensajes");
}

int crearLog() {
	logger = log_create(getenv("../logConsola"),"consola", 1, 0);
	if (logger) {
		return 1;
	} else {
		return 0;
	}
}
