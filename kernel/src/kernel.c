/*
 ============================================================================
 Name        : kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "kernel.h"

#define IP "127.0.0.1"
#define PUERTO "8080"
#define BACKLOG "10"

int main(void) {

	int socket_servidor_kernel = createServer(IP,PUERTO,BACKLOG);

	int numero_maximo_socket;
	int newSocket;

	fd_set read_fds;
	fd_set socket_master;

	FD_ZERO(&read_fds);
	FD_ZERO(&socket_master);
	FD_SET(socket_servidor_kernel,&socket_master);
	numero_maximo_socket = socket_servidor_kernel;
	int iterador_sockets;
	void* paquete;

	while(1){
		read_fds = socket_master;

		if(select(numero_maximo_socket + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}
	for(iterador_sockets = 0; iterador_sockets <= numero_maximo_socket; iterador_sockets++) {
		if(FD_ISSET(iterador_sockets, &read_fds)) {
			if(iterador_sockets == socket_servidor_kernel) {

				newSocket = acceptSocket(socket_servidor_kernel);

				if(newSocket == -1) {
					perror("Error al aceptar");
				} else {
					FD_SET(newSocket, &socket_master);
					if(newSocket > numero_maximo_socket) numero_maximo_socket = newSocket;
				}
			} else {
				//Gestiono cada conexi�n -> Recibo los programas y creo sus PCB.
				int tipo_mensaje; //Para que la funcion recibir_string lo reciba
				int check = recibir_paquete(iterador_sockets, &paquete, &tipo_mensaje);

				//Chequeo de errores
				if (check == 0) {
					printf("Se cerro el socket %d\n", iterador_sockets);
					close(iterador_sockets);
					FD_CLR(iterador_sockets, &socket_master);
				}

				if(check == -1){
					perror("recv");
					close(iterador_sockets);
					FD_CLR(iterador_sockets, &socket_master);
				}
				// Fin chequeo de errores

				if(check > 0) {
					switch(tipo_mensaje){
						case HANDSHAKE_PROGRAMA:{
							enviar_paquete_vacio(HANDSHAKE_KERNEL,iterador_sockets);
							break;
						}
						default:{ printf("Se recibio un codigo no valido");
							break;
						}
					}
				}
			}
		}
	}
	}

	return EXIT_SUCCESS;
}


t_config_kernel* levantarConfiguracionKernel(char* archivo_conf) {

        t_config_kernel* conf = malloc(sizeof(t_config_kernel));
        t_config* configNucleo;
        char** varGlob, semID, semInit;

        configNucleo = config_create(archivo_conf);
        conf->puerto_CPU = config_get_int_value(configNucleo, "PUERTO_CPU");
        conf->puerto_PROG = config_get_int_value(configNucleo, "PUERTO_PROG");
        conf->ip_Memoria = config_get_string_value(configNucleo, "IP_MEMORIA");
        conf->puerto_Memoria = config_get_int_value(configNucleo, "PUERTO_MEMORIA");
        conf->ip_FS = config_get_string_value(configNucleo, "IP_FS");
        conf->puerto_FS = config_get_int_value(configNucleo, "PUERTO_FS");
        conf->quantum = config_get_int_value(configNucleo, "QUANTUM");
        conf->quantum_Sleep = config_get_int_value(configNucleo, "QUANTUM_SLEEP");
        conf->algoritmo = config_get_string_value(configNucleo, "ALGORITMO");
        conf->grado_MultiProg = config_get_int_value(configNucleo, "GRADO_MULTIPROG");

        semID = config_get_array_value(configNucleo, "SEM_ID");
        semInit = config_get_array_value(configNucleo, "SEM_INIT");
        conf->semaforos = crearDiccionarioConValue(semID,semInit);

        varGlob = config_get_array_value(configNucleo, "SHARED_VARS");
        conf->variablesGlobales = crearDiccionario(varGlob);

        conf->stack_Size = config_get_int_value(configNucleo, "STACK_SIZE");


        return conf;
}
t_dictionary* crearDiccionarioConValue(char** array, char** valores){

        t_dictionary* dic = malloc(sizeof(t_dictionary));
        char** auxArray = array;
        char** auxValores = valores;

        while(auxArray != NULL){
                dictionary_put(dic, auxArray, auxValores);
                auxArray++;
                auxValores++;
        }

        return dic;
}

t_dictionary* crearDiccionario(char** array){

        t_dictionary* dic = malloc(sizeof(t_dictionary));
        char** auxArray = array;

        while(auxArray != NULL){
                dictionary_put(dic, auxArray, 0);
                auxArray++;
        }

        return dic;
}

//funciones interfaz
void levantarInterfaz(){
	//creo los comandos y el parametro
	comando* comandos = malloc(sizeof(comando)*6);

	strcpy(comandos[0].comando, "list");
	comandos[0].funcion = listProcesses;
	strcpy(comandos[1].comando, "info");
	comandos[1].funcion = processInfo;
	strcpy(comandos[2].comando, "tablaArchivos");
	comandos[2].funcion = getTablaArchivos;
	strcpy(comandos[3].comando, "grMulti");
	comandos[3].funcion = gradoMultiprogramacion;
	strcpy(comandos[4].comando, "kill");
	comandos[4].funcion = killProcess;
	strcpy(comandos[5].comando, "stopPlan");
	comandos[5].funcion = stopPlanification;

	interface_thread_param* params = malloc(sizeof(interface_thread_param));
	params->comandos = comandos;
	params->cantComandos = 6;

	//Lanzo el thread
	pthread_t threadInterfaz;
	pthread_attr_t atributos;
	pthread_attr_init(&atributos);
	pthread_attr_setdetachstate(&atributos, PTHREAD_CREATE_DETACHED);

	pthread_create(&threadInterfaz, &atributos, interface, params);

	return;
}
void listProcesses(char* comando, char* param){
        printf("listProcesses\n");
}
void processInfo(char* comando, char* param){
        printf("process info\n");
}
void getTablaArchivos(char* comando, char* param){
        printf("get tabla archivos\n");
}
void gradoMultiprogramacion(char* comando, char* param){
        printf("gradoMultiprogramacion\n");
}
void killProcess(char* comando, char* param){
        printf("killProcess\n");
}
void stopPlanification(char* comando, char* param){
        printf("stopPlanification\n");
}
