#include "funcionesConsola.h"

void crearConfig(int argc, char* argv[]) {
	char* pathConfig = string_new();

	if (argc>0)string_append(&pathConfig, argv[1]);
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
	pthread_t threadInterfaz;
	pthread_attr_t atributos;
	pthread_attr_init(&atributos);
	pthread_attr_setdetachstate(&atributos, PTHREAD_CREATE_DETACHED);

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
	printf("desconectarConsola\n");
}
void limpiarMensajes(char* comando, char* param) {
	printf("limpiarMensajes");
}

int crearLog() {
	logger =
			log_create(
					getenv(
							"/home/utnso/TPOperativos/tp-2017-1c-Dirty-Cow/consola/logConsola"),
					"consola", 1, 0);
	if (logger) {
		return 1;
	} else {
		return 0;
	}
}
