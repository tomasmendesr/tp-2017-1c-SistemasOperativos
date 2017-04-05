/*
 ============================================================================
 Name        : consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#define MAX_COMMAND_SIZE 256
#define IniciarProceso "iniciarProceso"

#include "consola.h"

int main(){
	if(crearLog()){
		config = levantarConfiguracionConsola("/home/utnso/TPOperativos/tp-2017-1c-Dirty-Cow/consola/consolaConfiguracion");
	}else{
		log_info(logger,"La consola no pudo iniciarse");
		return EXIT_FAILURE;
	}

   char *comando = malloc(MAX_COMMAND_SIZE);
   int socket_cliente;
   fgets(comando,MAX_COMMAND_SIZE,stdin);

   char **comandos = string_split(comando, ".");

   if (comandos)
   {
	   //Iniciar Proceso
	   if(strcmp(comandos[0],IniciarProceso)){
		   verificarExistenciaDeArchivo(comandos[1]);
    	   printf("Su proceso se inicializo");
    	   socket_cliente = createClient(config->ip_Kernel, config->puerto_Kernel);
    	   if(socket_cliente){
    		   printf("Cliente creado satisfactoriamente.\n");
    	   }
    	   enviar_paquete_vacio(HANDSHAKE_PROGRAMA,socket_cliente);
    	   int operacion = 0;
    	   void* paquete_vacio;

    	   recibir_paquete(socket_cliente, &paquete_vacio, &operacion);

    	   	if(operacion == HANDSHAKE_KERNEL){
    	   		printf("Conexion con Kernel establecida! :D \n");
    	   		printf("Se procede a mandar el archivo: ",comando[1]);
    	   	} else {
    	   		printf("El Kernel no devolvio handshake :( \n");
    	   	}
       }
   }

   return EXIT_SUCCESS;
}


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

int verificarExistenciaDeArchivo(char* path) {
        FILE * archivoConfig = fopen(path, "r");
        if (archivoConfig!=NULL){
			 fclose(archivoConfig);
			 return 1;
        }else{
        	log_info(logger,"El archivo no existe");
        }
        return -1;
}


