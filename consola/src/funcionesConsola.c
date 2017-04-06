

#include "funcionesConsola.h"

t_config_consola* levantarConfiguracionConsola(char * archivo) {

        t_config_consola* config = malloc(sizeof(t_config_consola));

        t_config* configConsola;
        verificarExistenciaDeArchivo(archivo);
        configConsola = config_create(archivo);
        config->ip_Kernel = config_get_string_value(configConsola, "IP_KERNEL");
        config->puerto_Kernel = config_get_string_value(configConsola, "PUERTO_KERNEL");

        return config;
}

//funciones interfaz
void levantarInterfaz(){
	//creo los comandos y el parametro
	comando* comandos = malloc(sizeof(comando)*4);

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

	pthread_create(&threadInterfaz, &atributos, (void*)interface, params);

	return;
}
void iniciarPrograma(char* comando, char* param){
        printf("iniciarPrograma\n");
}
void finalizarPrograma(char* comando, char* param){
        printf("finalizarPrograma\n");
}
void desconectarConsola(char* comando, char* param){
        printf("desconectarConsola\n");
}
void limpiarMensajes(char* comando, char* param){
        printf("limpiarMensajes");
}

int crearLog() {
        logger = log_create(getenv("/home/utnso/TPOperativos/tp-2017-1c-Dirty-Cow/consola/logConsola"), "consola", 1, 0);
        if (logger) {
                return 1;
        } else {
                return 0;
        }
}
