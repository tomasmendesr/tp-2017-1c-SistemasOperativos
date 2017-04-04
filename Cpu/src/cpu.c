#include <stdio.h>
#include "funcionesCpu.h"
#include <Libs-Commons/sockets.h>

#define MAX_COMMAND_SIZE 256
#define IniciarProceso "iniciarProceso"

int main(){

	t_config_cpu* config = levantarConfiguracionCPU("confCpu.init");

   char *comando = malloc(MAX_COMMAND_SIZE);
   int socket_cliente;
   fgets(comando,MAX_COMMAND_SIZE,stdin);

   char **comandos = string_split(comando, ".");

   if (comandos)
   {
	   //Iniciar Proceso
	   if(strcmp(comandos[0],IniciarProceso)){
		   printf("Su proceso se inicializo");
		   socket_cliente = createClient(config->ip_Kernel, config->puerto_Kernel);
		   if(socket_cliente){
			   printf("Cliente creado satisfactoriamente.\n");
		   }
		   enviarHandshake(socket_cliente,HANDSHAKE_CPU);
		   int operacion = 0;
		   void* paquete_vacio;

		   recibir_paquete(socket_cliente, &paquete_vacio, &operacion);

			if(operacion == HANDSHAKE_KERNEL){
				printf("Conexion con Kernel establecida! :D \n");
			} else {
				printf("El Kernel no devolvio handshake :( \n");
			}
	   }
   }

	free(config);
   return 0;

}


