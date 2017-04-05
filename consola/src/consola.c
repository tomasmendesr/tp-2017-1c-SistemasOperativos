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
#define IP "127.0.0.1"
#define PUERTO "8080"

#include "consola.h"

int main(){
   char *comando = malloc(MAX_COMMAND_SIZE);
   int socket_cliente;
   fgets(comando,MAX_COMMAND_SIZE,stdin);

   char **comandos = string_split(comando, ".");

   if (comandos)
   {
	   //Iniciar Proceso
	   if(strcmp(comandos[0],IniciarProceso)){
    	   printf("Su proceso se inicializo");
    	   socket_cliente = createClient(IP, PUERTO);
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
