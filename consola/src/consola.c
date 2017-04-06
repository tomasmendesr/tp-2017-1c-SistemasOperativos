/*
 ============================================================================
 Name        : consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include "funcionesConsola.h"

int main(int argc, char** argv){

	crearLog();

	char* pathConfig=string_new();
	if(argv[1]!=NULL)string_append(&pathConfig,argv[1]);
		else string_append(&pathConfig,configuracionConsola);

	if(verificarExistenciaDeArchivo(pathConfig)){
		config = levantarConfiguracionConsola(pathConfig);
	}else{
		log_info(logger,"No Pudo levantarse el archivo de configuracion");
		return EXIT_FAILURE;
	}

   char *comando = malloc(MAX_COMMAND_SIZE);
   int socket_cliente;
   fgets(comando,MAX_COMMAND_SIZE,stdin);

   char **comandos = string_split(comando, ".");

   if (comandos)
   {
	   //Iniciar Proceso
	   if(!strcmp(comandos[0],IniciarProceso)){
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







